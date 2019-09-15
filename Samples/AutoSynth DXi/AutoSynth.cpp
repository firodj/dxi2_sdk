// AutoSynth.cpp: implementation of the CAutoSynth class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AutoSynth.h"

#include <math.h>

double	CAutoSynth::m_adFreq[ 128 ];
float		CAutoSynth::m_afPanTable[ 65 ][ 2 ];

static const double dPI = 3.14159265358979323846264338327950288419716939937510;
static const double d2PI = dPI * 2.0;
static const double d1v2PI = 1.0 / d2PI;
static const long	  FADE_SAMPLES = 32;

static const short WHEEL_MAX			= 16383;	// (2 << 14) - 1
static const short WHEEL_CENTER		=  8192;	// 0x2000 or (2 <<14) / 2
static const short WHEEL_MIN			=     0;

//////////////////////////////////////////////////////////////////////
// Ctors

CAutoSynth::CAutoSynth( CDXiSynthContext* pCtx, HRESULT* phr ) : CDXi( pCtx )
{
	ASSERT( NULL != m_pCtx );

	// TODO: put all initialization code in Initialize(), below.
}

CAutoSynth::~CAutoSynth()
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAutoSynth::Initialize()
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

	// Create pins as needed
	hr = configurePins();
	if (FAILED( hr ))
		return hr;

	// Initialize frequency tables
	hr = initTables();
	if (FAILED( hr ))
		return hr;

	// Remember the ID of our mix output
	m_idMixOutPin = m_pCtx->GetOutputPinId( 0 );
	
	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAutoSynth::IsValidInputFormat( const WAVEFORMATEX* pwfx ) const
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

HRESULT CAutoSynth::IsValidOutputFormat( const WAVEFORMATEX* pwfx ) const
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

HRESULT CAutoSynth::IsValidTransform( const WAVEFORMATEX* pwfxIn, const WAVEFORMATEX* pwfxOut ) const
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

HRESULT CAutoSynth::SuggestOutputFormat( WAVEFORMATEX* pwfx ) const
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

static inline double doEq( double input, double fc, double res, double out[4], double in[4] )
{
	// "Moog VCF, variation 2", from www.musicdsp.org.
	// References: CSound source code, Stilson/Smith CCRMA paper, Timo Tossavainen (?) version

	double f = fc * 1.16; 
	double fb = res * (1.0 - 0.15 * f * f); 

	input -= out[3] * fb; 
	input *= 0.35013 * (f*f)*(f*f); 
	out[0] = input + 0.3 * in[0] + (1 - f) * out[0]; // Pole 1 
	in[0] = input; 
	out[1] = out[0] + 0.3 * in[1] + (1 - f) * out[1]; // Pole 2 
	in[1] = out[0]; 
	out[2] = out[1] + 0.3 * in[2] + (1 - f) * out[2]; // Pole 3 
	in[2] = out[1]; 
	out[3] = out[2] + 0.3 * in[3] + (1 - f) * out[3]; // Pole 4 
	in[3] = out[2]; 
	return out[3];
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAutoSynth::Process( LONGLONG llSampAudioTimestamp,
											AudioBuffer* pbufIn,
											AudioBuffer* abufOut, long cBufOut,
											LONGLONG llSampMidiClock, deque<DXiEvent>& qMidi )
{
	BOOL const bGenerateTail = (NULL == pbufIn);

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
		if (0 == pbufIn->lOffset)
		{
			// No input, first buffer, so zero-fill the output
			for (ixBuf = 0; ixBuf < cBufOut; ++ixBuf)
				abufOut[ixBuf].SetZerofill( TRUE );
		}
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
		// Mix the master output
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

		// Get current EQ parameters
		double fc = GetParamValue( PARAM_FILTER_CUTOFF );
		double fc1 = 0, fc2 = 0;
		GetParamDeltas( PARAM_FILTER_CUTOFF, &fc1, &fc2 );
		double res = GetParamValue( PARAM_FILTER_RESONANCE );
		double res1 = 0, res2 = 0;
		GetParamDeltas( PARAM_FILTER_CUTOFF, &res1, &res2 );

		if (1 == GetOutputFormat()->nChannels)
		{
			for (int ixSamp = 0; ixSamp < cSampMix; ixSamp++)
			{
				// Apply the EQ
				pfBufMaster[ixSamp] = doEq( pfBufMaster[ixSamp], fc, res, m_out[0], m_in[0] );

				// Update deltas for fc and res
				fc += fc1;
				fc1 += fc2;
				res += res1;
				res1 += res2;
			}
		}
		else
		{
			for (int ixSamp = 0; ixSamp < cSampMix; ixSamp += 2)
			{
				// Apply the EQ
				pfBufMaster[ixSamp+0] = doEq( pfBufMaster[ixSamp+0], fc, res, m_out[0], m_in[0] );
				pfBufMaster[ixSamp+1] = doEq( pfBufMaster[ixSamp+1], fc, res, m_out[1], m_in[1] );

				// Update deltas for fc and res
				fc += fc1;
				fc1 += fc2;
				res += res1;
				res1 += res2;
			}
		}
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAutoSynth::IsValidEvent( DXiEvent& de )
{
	if (
		MfxEvent::Note == de.me.m_eType ||
		MfxEvent::Control == de.me.m_eType ||
		MfxEvent::Patch == de.me.m_eType ||
		MfxEvent::Wheel == de.me.m_eType
		)
	{
		return S_OK;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAutoSynth::InitializeNoteEvent( DXiEvent& de )
{
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

HRESULT CAutoSynth::ExpireNoteEvent( DXiEvent& de, BOOL bForce )
{
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

HRESULT CAutoSynth::AllocateResources()
{
	// Reset filter state
	memset( m_in, 0, sizeof(m_in) );
	memset( m_out, 0, sizeof(m_out) );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAutoSynth::FreeResources()
{
	// Create/destroy pins as necessary
	configurePins();

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

int CAutoSynth::PersistGetSize() const
{
	int const cb
		= sizeof(DWORD)											// # of persisted parameters
		+ NUM_PARAMS * (sizeof(DWORD) + sizeof(float));	// (index,value), for each parameter

	return cb;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAutoSynth::PersistLoad( IStream* pStream )
{
	ULONG		cb = 0;
	HRESULT	hr = S_OK;
	
	// Get the number of persisted parameters 
	DWORD cParams = 0;
	hr = pStream->Read( &cParams, sizeof(cParams), &cb );
	if (FAILED( hr ) || cb != sizeof(cParams))
		return E_FAIL;

	// Restore each parameter
	for (DWORD ix = 0; ix < cParams; ix++)
	{
		// Get the parameter index
		DWORD dwParam = 0;
		hr = pStream->Read( &dwParam, sizeof(dwParam), &cb );
		if (FAILED( hr ) || cb != sizeof(dwParam))
			return E_FAIL;

		// Get the parameter value
		float fValue = 0;
		hr = pStream->Read( &fValue, sizeof(fValue), &cb );
		if (FAILED( hr ) || cb != sizeof(fValue))
			return E_FAIL;

		// Set the parameter value
		if (m_pMediaParams)
			m_pMediaParams->SetParam( dwParam, fValue );
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAutoSynth::PersistSave( IStream* pStream )
{
	ULONG		cb = 0;
	HRESULT	hr = S_OK;
	
	// Put the number of persisted parameters 
	DWORD cParams = NUM_PARAMS;
	hr = pStream->Write( &cParams, sizeof(cParams), &cb );
	if (FAILED( hr ) || cb != sizeof(cParams))
		return E_FAIL;

	// Save each parameter
	for (DWORD dwParam = 0; dwParam < cParams; dwParam++)
	{
		float fValue = 0;

		// Get the parameter value
		if (m_pMediaParams)
			m_pMediaParams->GetParam( dwParam, &fValue );

		// Write the parameter index
		hr = pStream->Write( &dwParam, sizeof(dwParam), &cb );
		if (FAILED( hr ) || cb != sizeof(dwParam))
			return E_FAIL;

		// Write the parameter value
		hr = pStream->Write( &fValue, sizeof(fValue), &cb );
		if (FAILED( hr ) || cb != sizeof(fValue))
			return E_FAIL;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAutoSynth::initTables()
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

void CAutoSynth::panToLRGain( int nPan, float* pfGainL, float* pfGainR )
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

HRESULT CAutoSynth::synthNote( float* pf, DXiEvent& de, long lSampToPlay, BOOL bFadeTail )
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
	float const	fMode = GetParamValue( _PARAM_MODE );

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

			if (0 == fMode)
				*(pf++) += fVolL * fSamp;
			else
				*(pf++) *= fVolL * fSamp;

			if (bStereo)
			{
				if (0 == fMode)
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

			if (0 == fMode)
				*(pf++) += fVolL * fSamp;
			else
				*(pf++) *= fVolL * fSamp;

			if (bStereo)
			{
				if (0 == fMode)
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

			if (0 == fMode)
				*(pf++) += fVolL * fSamp;
			else
				*(pf++) *= fVolL * fSamp;

			if (bStereo)
			{
				if (0 == fMode)
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

HRESULT CAutoSynth::configurePins()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CAutoLock lock( this );

	HRESULT hr = S_OK;

	BOOL const bWantsMultiOut = (0 != GetParamValue( _PARAM_MULTI_OUT ));
	BOOL const bIsMultiOut = (m_pCtx->GetOutputPinCount() > 1);

	if (bWantsMultiOut != bIsMultiOut)
	{
		if (bWantsMultiOut)
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
	}

	return hr;
}

