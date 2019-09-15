// Twonar.cpp: implementation of the CTwonar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Twonar.h"
#include <math.h>

double	CTwonar::m_adFreq[ 128 ];
float		CTwonar::m_afPanTable[ 65 ][ 2 ];

static const double dPI = 3.14159265358979323846264338327950288419716939937510;
static const double d2PI = dPI * 2.0;
static const double d1v2PI = 1.0 / d2PI;
static const long	  FADE_SAMPLES = 32;

static const short WHEEL_MAX			= 16383;	// (2 << 14) - 1
static const short WHEEL_CENTER		=  8192;	// 0x2000 or (2 <<14) / 2
static const short WHEEL_MIN			=     0;


//////////////////////////////////////////////////////////////////////
// Ctors

CTwonar::CTwonar( CDXiSynthContext* pCtx, HRESULT* phr ) : CDXi( pCtx )
{
	ASSERT( NULL != m_pCtx );

	// TODO: put all initialization code in Initialize(), below.
}

CTwonar::~CTwonar()
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::Initialize()
{
	HRESULT hr = S_OK;

	// Initialize per-channel info
	for (int nCh = 0; nCh < 16; ++nCh)
	{
		m_aMidiChan[ nCh ].byPatch = 0;
		m_aMidiChan[ nCh ].fWheelPitch = 1;
		for (int nCtl = 0; nCtl < 128; ++nCtl)
			m_aMidiChan[ nCh ].abyCC[ nCtl ] = 64;
	}

	// Initialize parameters
	hr = put_Mode( 0 );
	if (FAILED( hr ))
		return hr;
	m_p.bMultiOut = FALSE;
	hr = put_IsMultiOut( TRUE );
	if (FAILED( hr ))
		return hr;

	m_p.bEnableMetro = FALSE;
	m_p.nMetroNoteMeasTop = 92;
	m_p.nMetroVelMeasTop = 80;
	m_p.nMetroNoteBeat = 80;
	m_p.nMetroVelBeat = 64;

	// Initialize frequency tables
	hr = initTables();

	// Remember the ID of our mix output
	m_idMixOutPin = m_pCtx->GetOutputPinId( 0 );
	
	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::IsValidInputFormat( const WAVEFORMATEX* pwfx ) const
{
	// The plug-in base class will have already validated pwfx to ensure that
	// it is 16-bit PCM or 32-bit float, 1 or 2 channels.

	// TODO: Add any additional checks here, such as sample rate, etc.

	// By default, only 32-bit float buffers are supported.
	if (WAVE_FORMAT_IEEE_FLOAT != pwfx->wFormatTag)
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::IsValidOutputFormat( const WAVEFORMATEX* pwfx ) const
{
	// The plug-in base class will have already validated pwfx to ensure that
	// it is 16-bit PCM or 32-bit float, 1 or 2 channels.

	// TODO: Add any additional checks here, such as sample rate, etc.

	// By default, only 32-bit float buffers are supported.
	if (WAVE_FORMAT_IEEE_FLOAT != pwfx->wFormatTag)
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::IsValidTransform( const WAVEFORMATEX* pwfxIn, const WAVEFORMATEX* pwfxOut ) const
{
	// The plug-in base class will have already validated pwfxIn/pwfxOut to ensure that
	// it is 16-bit PCM or 32-bit float, 1 or 2 channels, and that both have the same
	// sample rate.

	// TODO: Add any additional checks here, such as sample rate, etc.

	// By default, only 32-bit float buffers are supported.
	if (WAVE_FORMAT_IEEE_FLOAT != pwfxIn->wFormatTag)
		return VFW_E_TYPE_NOT_ACCEPTED;
	if (WAVE_FORMAT_IEEE_FLOAT != pwfxOut->wFormatTag)
		return VFW_E_TYPE_NOT_ACCEPTED;

	// We don't allow any channel conversions
	if (pwfxIn->nChannels != pwfxOut->nChannels)
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::SuggestOutputFormat( WAVEFORMATEX* pwfx ) const
{
	// The plug-in base class will have already validated pwfx to ensure that
	// it is 16-bit PCM or 32-bit float, 1 or 2 channels, and that both have the same
	// sample rate.
	// TODO: Add any additional checks here, such as sample rate, etc.

	// pwfx is initially set to the input format.  If your plug-in doesn't need
	// to change the output format, simply return S_OK.
	// TODO: change pwfx if necessary.

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::Process( LONGLONG llSampAudioTimestamp,
											AudioBuffer* pbufIn,
											AudioBuffer* abufOut, long cBufOut,
											LONGLONG llSampMidiClock, deque<DXiEvent>& qMidi )
{
	BOOL const bGenerateTail = (NULL == pbufIn);

	// Note about deferred zero filling:
	//
	// AudioBuffer will automatically take advantage of IDeferZeroFill,
	// if the host app supports it.  To avoid unnecessary and wasteful buffer
	// fills, always check the 'bZero' flag in AudioBuffer before calling
	// the GetPointer() method.  This is because calling GetPointer() will
	// trigger a zero-fill if the underlying data buffer was marked as "defer
	// zero fill."
	//
	// Similarly, to allow downstream filters to benefit from deferred
	// zero filling, be sure to set the 'bZero' flag in an AudioBuffer, if
	// your DSP code is producing a completely silent buffer.

	float* pfBufI = (NULL != pbufIn && !pbufIn->GetZerofill()) ? pbufIn->GetPointer() : NULL;
	long	 cSampBuf = pbufIn->cSamp;

	// See we're producing output for the mix output pin, and make sure it's
	// zeroed out the first time we access it
	int ixBufMaster = -1;
	for (int ixBuf = 0; ixBuf < cBufOut; ++ixBuf)
	{
		if (abufOut[ixBuf].idPin == m_idMixOutPin)
		{
			ixBufMaster = ixBuf;
			memset( abufOut[ixBufMaster].GetPointer(), 0, cSampBuf * GetOutputFormat()->nBlockAlign );
			break;
		}
	}

	// Pass through inputs to outputs before processing -- except to the mix
	// output, since this always created by summing the inputs
	if (pfBufI)
	{
		for (ixBuf = 0; ixBuf < cBufOut; ++ixBuf)
		{
			if (ixBufMaster != ixBuf)
				memcpy( abufOut[ixBuf].GetPointer(), pfBufI, cSampBuf * GetOutputFormat()->nBlockAlign );
		}
	}
	else
	{
		// No input, so zero-fill the output
		for (ixBuf = 0; ixBuf < cBufOut; ++ixBuf)
			abufOut[ixBuf].SetZerofill( TRUE );
	}

	deque<DXiEvent>::iterator it;
	for (it = qMidi.begin(); it != qMidi.end(); it++)
	{
		DXiEvent& de = *it;
		if (MfxEvent::Note == de.me.m_eType)
		{
			// Get our state info about this note
			NoteState* pns = reinterpret_cast<NoteState*>( de.pvData );
			if (NULL == pns)
				return E_UNEXPECTED;
 
			// Initialize the shape (tone) if we haven't seen this note yet
			if (0 == pns->lSampCurrent)
				pns->eWaveShape = static_cast< EWaveShape >( m_aMidiChan[ de.me.m_byChan & 0xF ].byPatch );

			// Synthesize this note to the mix output pin, unless we find
			// the specific wave's output pin in our buffer list.
			int	nShape = pns->eWaveShape % MAX_WAVESHAPE;
			DWORD	idPin = m_aidOutPin[ nShape ];
			int	ixBufOut = ixBufMaster;
			for (ixBuf = 0; ixBuf < cBufOut; ++ixBuf)
			{
				if (abufOut[ixBuf].idPin == idPin)
				{
					ixBufOut = ixBuf;
					break;
				}
			}

			// Bail out (expire the note) if there's no output buffer to mix into
			if (-1 == ixBufOut)
			{
				pns->lSampCurrent = pns->lSampDuration;
				continue;
			}

			// Get the output buffer to fill
			float* pfBufO = abufOut[ ixBufOut ].GetPointer();

			// Determine the buffer offset for this note's synthesized data
			long lBufOfs = static_cast<long>( max( 0, de.llSampTimestamp - llSampMidiClock - pns->lSampCurrent ) );

			// Determine how many samples are yet to be synthesized for this note
			long cSampToGo = pns->lSampDuration - pns->lSampCurrent;

			// Determine how many samples we must synthesize this time
			long cSampSynth = min( cSampToGo, cSampBuf - lBufOfs );
			ASSERT( cSampSynth >= 0 );
			cSampSynth = max( cSampSynth, 0 );

			// Note-off events should be faded out immediately
			BOOL const bNoteOff = (0 == de.me.m_dwDuration);
			BOOL const bFadeTail = bNoteOff || (pns->lSampCurrent + cSampSynth >= pns->lSampDuration);

			// Synthesize the note
			HRESULT hr = synthNote( pfBufO + (lBufOfs * GetOutputFormat()->nChannels), de, cSampSynth, bFadeTail );
			if (FAILED( hr ))
				return hr;

			// Update the note position
			if (bNoteOff)
				pns->lSampCurrent = pns->lSampDuration = 0;
			else
				pns->lSampCurrent += cSampSynth;
		}
		else if (MfxEvent::Control == de.me.m_eType)
		{
			m_aMidiChan[ de.me.m_byChan & 0xF ].abyCC[ de.me.m_byNum & 0x7F ] = de.me.m_byVal;
		}
		else if (MfxEvent::Patch == de.me.m_eType)
		{
			m_aMidiChan[ de.me.m_byChan & 0xF ].byPatch = de.me.m_byPatch;
		}
		else if (MfxEvent::Wheel == de.me.m_eType)
		{
			// Convert pitch wheel to a frequency shift
			int	nWheel = de.me.m_nVal - WHEEL_CENTER;
			float	fWheel = static_cast<float>( nWheel ) / WHEEL_CENTER;
			float fPitch = static_cast<float>( pow( 2.0, fWheel ) );
			m_aMidiChan[ de.me.m_byChan & 0xF ].fWheelPitch = fPitch;
		}
	}

	// Mix all output pins into the master, if necessary
	if (-1 != ixBufMaster)
	{
		float* pfBufMaster = abufOut[ ixBufMaster ].GetPointer();
		int	 cSampMix = cSampBuf * GetOutputFormat()->nChannels; 
		for (ixBuf = 0; ixBuf < cBufOut; ++ixBuf)
		{
			if (ixBuf != ixBufMaster)
			{
				float* pfBuf = abufOut[ixBuf].GetPointer();
				for (int ixSamp = 0; ixSamp < cSampMix; ixSamp++)
					pfBufMaster[ixSamp] += pfBuf[ixSamp];
			}
		}
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::IsValidEvent( DXiEvent& de )
{
	// TODO: return S_FALSE if this event isn't recognized by your DXi, and
	// should not be passed along for further processing or rendering.

	if (
		MfxEvent::Note == de.me.m_eType ||
		MfxEvent::Control == de.me.m_eType ||
		MfxEvent::Patch == de.me.m_eType ||
		MfxEvent::Wheel == de.me.m_eType
		)
	{
		return S_OK;
	}

	return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::InitializeNoteEvent( DXiEvent& de )
{
	// TODO: set de.pvData to your own implementation-specific data here
	// for note-on events.

	NoteState* pns = new NoteState;
	if (NULL == pns)
		return E_OUTOFMEMORY;

	pns->lSampCurrent = 0;
	pns->dTheta = 0;
	pns->dStep = d2PI * m_adFreq[ de.me.m_byKey ] / GetOutputFormat()->nSamplesPerSec;

	// Note-on events that are played live have "infinite" duration.  Check for these
	// and handle them specially.
	if (ULONG_MAX == de.me.m_dwDuration)
		pns->lSampDuration = LONG_MAX;
	else
		pns->lSampDuration = m_pCtx->TicksToSamples( de.me.m_lTime + de.me.m_dwDuration ) - m_pCtx->TicksToSamples( de.me.m_lTime );

	de.pvData = pns;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::ExpireNoteEvent( DXiEvent& de, BOOL bForce )
{
	// TODO: add your own code here to test whether a note event
	// has been played to completion and/or to free any memory,
	// allocated for de.pvData and return S_OK.

	NoteState* pns = reinterpret_cast<NoteState*>( de.pvData );
	if (NULL == pns)
		return S_OK;

	if (bForce || pns->lSampCurrent >= pns->lSampDuration)
	{
		delete pns;
		de.pvData = NULL;
		return S_OK;
	}

	return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::AllocateResources()
{
	// TODO: add code to here to prepare the for the start of streaming
	
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::FreeResources()
{
	// TODO: add code to here to clean up after streaming

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

int CTwonar::PersistGetSize() const
{
	return sizeof(m_p);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::PersistLoad( IStream* pStream )
{
	ULONG		cb = 0;
	HRESULT	hr = S_OK;
	
	// Read Params1 chunk
	Params1 p1;
	hr = pStream->Read( &p1, sizeof(p1), &cb );
	if (FAILED( hr ))
		return hr;
	if (cb != sizeof(p1))
		return E_FAIL;

	// Stop now if this is v1 persistence 
	if (0 == (p1.bMultiOut & 0x80000000))
	{
		m_p.nMode = p1.nMode;
		m_p.bMultiOut = p1.bMultiOut;
		return S_OK;
		/////
	}

	// Read v2 stuff
	hr = pStream->Read( &m_p, sizeof(m_p), &cb );
	if (FAILED( hr ))
		return hr;
	if (cb != sizeof(m_p))
		return E_FAIL;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::PersistSave( IStream* pStream )
{
	ULONG		cb = 0;
	HRESULT	hr = S_OK;
	
	// Write Params1 chunk (for backwards compatibility)
	Params1 p1;
	p1.nMode = m_p.nMode;
	p1.bMultiOut = m_p.bMultiOut | 0x80000000;
	hr = pStream->Write( &p1, sizeof(p1), &cb );
	if (FAILED( hr ))
		return hr;
	if (cb != sizeof(p1))
		return E_FAIL;

	hr = pStream->Write( &m_p, sizeof(m_p), &cb );
	if (FAILED( hr ))
		return hr;
	if (cb != sizeof(m_p))
		return E_FAIL;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::initTables()
{
	double const dStep = pow( 2.0, 1.0 / 12.0 );

	// A5 = note number 69 = 440Hz
	m_adFreq[ 69 ] = 440;
	for (int ix = 70; ix < 127; ++ix)
		m_adFreq[ ix ] = m_adFreq[ ix - 1 ] * dStep;
	for (ix = 68; ix >= 0; --ix)
		m_adFreq[ ix ] = m_adFreq[ ix + 1 ] / dStep;

	// Fill pan table for equal power using SQRT relationship
	for (int nPan = 0; nPan < 65; ++nPan)
	{
		double dx = (double)nPan / 128;
		m_afPanTable[ nPan ][ 0 ] = (float)sqrt( 2 * (1 - dx) );
		m_afPanTable[ nPan ][ 1 ] = (float)sqrt( 2 * dx );
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

void CTwonar::panToLRGain( int nPan, float* pfGainL, float* pfGainR )
{
	nPan = min( max( nPan, 0 ), 127 );
	nPan = (nPan * 128) / 127;

	if (nPan <= 64)
	{
		*pfGainL = m_afPanTable[ nPan ][ 0 ];
		*pfGainR = m_afPanTable[ nPan ][ 1 ];
	}
	else
	{
		*pfGainL = m_afPanTable[ 128 - nPan ][ 1 ];
		*pfGainR = m_afPanTable[ 128 - nPan ][ 0 ];
	}
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTwonar::synthNote( float* pf, DXiEvent& de, long lSampToPlay, BOOL bFadeTail )
{
	NoteState*	pns = reinterpret_cast<NoteState*>( de.pvData );
	const long	lFadeDur = FADE_SAMPLES;
	float const	fFadeScale = 1.0 / lFadeDur;
	long			lCurrSamp = pns->lSampCurrent;
	long			lFinalSamp = pns->lSampCurrent + lSampToPlay;
	long			lFadeSamp = (bFadeTail) ? lFinalSamp - lFadeDur : lFinalSamp;
	float			fVolume = 1;
	float			fVolL = 1;
	float			fVolR = 1;
	int			nShape = pns->eWaveShape;
	BOOL const	bStereo = (2 == GetOutputFormat()->nChannels);

	// Convert MIDI volume/pan to gain factors
	fVolume = (float)sqrt( (double)(m_aMidiChan[ de.me.m_byChan ].abyCC[ 7 ] * de.me.m_byVel) / (127 * 127) );
	fVolume *= 0.25; // allow for 12dB of headroom
	if (bStereo)
		panToLRGain( m_aMidiChan[ de.me.m_byChan ].abyCC[ 10 ], &fVolL, &fVolR );

	// Convert pitch wheel to a frequency shift
	double const dStep = pns->dStep * m_aMidiChan[ de.me.m_byChan ].fWheelPitch;

	if (SquareWave == nShape)
	{
		for (long ix = lCurrSamp; ix < lFinalSamp; ++ix)
		{
			float fSamp = (pns->dTheta < dPI) ? -fVolume : fVolume;
			if (ix > lFadeSamp)
				fSamp *= 1.0 - fFadeScale * (ix - lFadeSamp);

			if (0 == m_p.nMode)
				*(pf++) += fVolL * fSamp;
			else
				*(pf++) *= fVolL * fSamp;

			if (bStereo)
			{
				if (0 == m_p.nMode)
					*(pf++) += fVolR * fSamp;
				else
					*(pf++) *= fVolR * fSamp;
			}

			pns->dTheta += dStep;
			if (pns->dTheta > d2PI)
				pns->dTheta -= d2PI;
		}
	}
	else if (SineWave == nShape)
	{
		for (long ix = lCurrSamp; ix < lFinalSamp; ++ix)
		{
			float fSamp = static_cast<float>( fVolume * sin( pns->dTheta ) );
			if (ix > lFadeSamp)
				fSamp *= 1.0 - fFadeScale * (ix - lFadeSamp);

			if (0 == m_p.nMode)
				*(pf++) += fVolL * fSamp;
			else
				*(pf++) *= fVolL * fSamp;

			if (bStereo)
			{
				if (0 == m_p.nMode)
					*(pf++) += fVolR * fSamp;
				else
					*(pf++) *= fVolR * fSamp;
			}

			pns->dTheta += dStep;
		}
	}
	else if (SawWave == nShape)
	{
		for (long ix = lCurrSamp; ix < lFinalSamp; ++ix)
		{
			float fSamp = static_cast<float>( fVolume * ( 1.0 - 2.0 * (pns->dTheta * d1v2PI) ) );
			if (ix > lFadeSamp)
				fSamp *= 1.0 - fFadeScale * (ix - lFadeSamp);

			if (0 == m_p.nMode)
				*(pf++) += fVolL * fSamp;
			else
				*(pf++) *= fVolL * fSamp;

			if (bStereo)
			{
				if (0 == m_p.nMode)
					*(pf++) += fVolR * fSamp;
				else
					*(pf++) *= fVolR * fSamp;
			}

			pns->dTheta += dStep;
			if (pns->dTheta > d2PI)
				pns->dTheta -= d2PI;
		}
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// ITwonar

STDMETHODIMP CTwonar::get_Mode(int *pVal)
{
	if (NULL == pVal)
		return E_POINTER;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CAutoLock lock( this );

	*pVal = m_p.nMode;
	return S_OK;
}

STDMETHODIMP CTwonar::put_Mode(int newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CAutoLock lock( this );

	m_p.nMode = newVal;
	return S_OK;
}

STDMETHODIMP CTwonar::get_IsMultiOut(BOOL *pVal)
{
	if (NULL == pVal)
		return E_POINTER;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CAutoLock lock( this );

	*pVal = m_p.bMultiOut;
	return S_OK;
}

STDMETHODIMP CTwonar::put_IsMultiOut(BOOL newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CAutoLock lock( this );

	HRESULT hr = S_OK;

	if (m_p.bMultiOut != newVal)
	{
		if (newVal)
		{
			// Create additional output pins
			if (SUCCEEDED( hr ))
				hr = m_pCtx->CreateOutputPin( "Sine", &m_aidOutPin[ SineWave ] );
			if (SUCCEEDED( hr ))
				hr = m_pCtx->CreateOutputPin( "Square", &m_aidOutPin[ SquareWave ] );
			if (SUCCEEDED( hr ))
				hr = m_pCtx->CreateOutputPin( "Saw", &m_aidOutPin[ SawWave ] );
		}
		else
		{
			hr = m_pCtx->DestroyOutputPin( m_aidOutPin[ SineWave ] );
			if (FAILED( hr ))
				return hr;
			hr = m_pCtx->DestroyOutputPin( m_aidOutPin[ SquareWave ] );
			if (FAILED( hr ))
				return hr;
			hr = m_pCtx->DestroyOutputPin( m_aidOutPin[ SawWave ] );
			if (FAILED( hr ))
				return hr;
		}
		m_p.bMultiOut = newVal;
	}

	return hr;
}

STDMETHODIMP CTwonar::get_EnableMetro(BOOL *pVal)
{
	if (NULL == pVal)
		return E_POINTER;

	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	CAutoLock lock( this );

	*pVal = m_p.bEnableMetro;

	return S_OK;
}

STDMETHODIMP CTwonar::put_EnableMetro(BOOL newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	CAutoLock lock( this );

	m_p.bEnableMetro = newVal;

	return S_OK;
}

STDMETHODIMP CTwonar::get_MetroInfo(int *pnNoteMeasTop, int *pnVelMeasTop, int *pnNoteBeat, int *pnVelBeat)
{
	if (!pnNoteMeasTop || !pnVelMeasTop || !pnNoteBeat || !pnVelBeat)
		return E_POINTER;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CAutoLock lock(this);

	*pnNoteMeasTop = m_p.nMetroNoteMeasTop & 0x7F;
	*pnVelMeasTop = m_p.nMetroVelMeasTop & 0x7F;
	*pnNoteBeat = m_p.nMetroNoteBeat & 0x7F;
	*pnVelBeat = m_p.nMetroVelBeat & 0x7F;

	return S_OK;
}

STDMETHODIMP CTwonar::put_MetroInfo(int nNoteMeasTop, int nVelMeasTop, int nNoteBeat, int nVelBeat)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CAutoLock lock(this);

	m_p.nMetroNoteMeasTop = nNoteMeasTop & 0x7F;
	m_p.nMetroVelMeasTop = nVelMeasTop & 0x7F;
	m_p.nMetroNoteBeat = nNoteBeat & 0x7F;
	m_p.nMetroVelBeat = nVelBeat & 0x7F;

	return S_OK;
}
