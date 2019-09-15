// SoftSynth.cpp: implementation of the CSoftSynth class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "SoftSynth.h"
#include "Instrument.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const BYTE	NOTE_OFF	= 0x80;
const BYTE	NOTE_ON	= 0x90;
const BYTE	KEYAFT	= 0xA0;
const BYTE	CONTROL	= 0xB0;
const BYTE	PATCH		= 0xC0;
const BYTE	CHANAFT	= 0xD0;
const BYTE	WHEEL		= 0xE0;
const BYTE	SYSX		= 0xF0;

//////////////////////////////////////////////////////////////////////
// Ctors

CSoftSynth::CSoftSynth( CDXi* pDXi, HRESULT* phr ) :
	m_pDXi( pDXi ),
	m_pTempoMap(NULL),
	m_pTimeConverter(NULL),
	m_pInputCallback(NULL),
	m_pDefaultInstrument(NULL),
	m_pNotifyHost(NULL),
	m_llSampClock(0),
	m_llSampStart(0),
	m_llSampLoopUnroll(0),
	m_bMfxConnected(FALSE)
{
	ASSERT( NULL != m_pDXi );
	m_pDefaultInstrument = new CInstrument("AutoSynth");
	ASSERT( m_pDefaultInstrument );
	if (NULL == m_pDefaultInstrument)
		*phr = E_OUTOFMEMORY;
	else
		m_pDefaultInstrument->AddRef();
}

CSoftSynth::~CSoftSynth()
{
	releaseInterfaces();
	if (m_pDefaultInstrument)
		m_pDefaultInstrument->Release(), m_pDefaultInstrument = NULL;
}

//////////////////////////////////////////////////////////////////////

void CSoftSynth::releaseInterfaces()
{
	if (m_pTempoMap)
		m_pTempoMap->Release(), m_pTempoMap = NULL;
	if (m_pTimeConverter)
		m_pTimeConverter->Release(), m_pTimeConverter = NULL;
	if (m_pInputCallback)
		m_pInputCallback->Release(), m_pInputCallback = NULL;
	if (m_pNotifyHost)
		m_pNotifyHost->Release(), m_pNotifyHost = NULL;
}

//////////////////////////////////////////////////////////////////////
// IMfxSoftSynth

STDMETHODIMP CSoftSynth::Connect( IUnknown* pContext )
{
	CAutoLock lock( m_pDXi );

	// Here is our chance to acquire any IMFXEventFilter interfaces we need.
	// This should never be called multiple times without an intervening
	// call to Disconnect()

	HRESULT hr = S_OK;

	if (m_bMfxConnected)
	{
		hr = E_UNEXPECTED; // illegal second call
		goto DONE;
	}

	// A soft synth needs some kind of tempo map to convert between musical time
	// and samples, so this Q.I. must succeed.
	hr = pContext->QueryInterface( IID_IMfxTempoMap, (void**)&m_pTempoMap );
	if (SUCCEEDED( hr ))
	{
		// Get an IMfxTimeConverter if possible, since that is actually preferable
		pContext->QueryInterface( IID_IMfxTimeConverter, (void**)&m_pTimeConverter );
	}
	if (FAILED( hr ))
		goto DONE;

	// At this point we are sufficiently connected to be used as an MFX
	m_bMfxConnected = TRUE;

	// IMfxInputCallback is necessary if the synth wants to push live MIDI
	// back to the host, but otherwise not mandated.
	pContext->QueryInterface( IID_IMfxInputCallback, (void**)&m_pInputCallback );

	// IMfxNotifyHost is necessary if the synth wants to dynamically add
	// or remove pins.
	pContext->QueryInterface( IID_IMfxNotifyHost, (void**)&m_pNotifyHost );

DONE:

	// Since we're not retaining a copy of the original IUnknown*, be sure to
	// call Release() on it.
	pContext->Release();
	
	return hr;
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::Disconnect()
{
	CAutoLock lock( m_pDXi );

	releaseInterfaces();
	m_bMfxConnected = FALSE;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::OnStart( LONG lTime )
{
	CAutoLock lock( m_pDXi );

	ExpireEvents( TRUE );
	m_qPending.clear();
	m_qActive.clear();

	m_llSampStart = TicksToSamples( lTime );
	m_llSampClock = 0;
	m_llSampLoopUnroll = 0;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::OnLoop( LONG lTimeRestart, LONG lTimeStop )
{
	CAutoLock lock( m_pDXi );

	// Add the loop time to our unrolled loop counter
	m_llSampLoopUnroll += ( TicksToSamples( lTimeStop ) - TicksToSamples( lTimeRestart ) );
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::OnStop( LONG lTime )
{
	CAutoLock lock( m_pDXi );

	ExpireEvents( TRUE );
	m_qPending.clear();
	m_qActive.clear();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::OnEvents( LONG lTimeFrom, LONG lTimeThru, MFX_CHANNEL mfxChannel, IMfxEventQueue* pqIn )
{
	CAutoLock lock( m_pDXi );

	HRESULT hr = S_OK;

	if (NULL == m_pTempoMap)
		return E_UNEXPECTED;

	// NOTE: We are not allowed to retain the IMfxEventQueue pointer pqIn, nor
	// should we call AddRef() or Release() on it.

	int nCount;
	hr = pqIn->GetCount( &nCount );
	if (SUCCEEDED( hr ))
	{
		// Loop through events
		for (int ix = 0; ix < nCount; ++ix)
		{
			// Get each event
			DXiEvent de;
			hr = pqIn->GetAt( ix, &de.me );
			if (FAILED( hr ))
				break;

			// Convert the event start time from ticks to samples, relative to
			// the start of the project.
			LONGLONG llSampTime = TicksToSamples( de.me.m_lTime );

			// Set queued event properties
			de.llSampTimestamp = (llSampTime - m_llSampStart) + m_llSampLoopUnroll;
			de.mfxChannel = mfxChannel;
			de.pvData = NULL;
			de.dwFlags = DXiEvent::F_STREAMED;

			// See if the DXi wants to use this event, and queue up if so
			hr = m_pDXi->IsValidEvent( de );
			if (FAILED( hr ))
				break;
			if (S_OK == hr)
				m_qPending.push_back( de );
		}
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::OnInput( MFX_CHANNEL mfxChannel, IMfxDataQueue* pqIn )
{
	CAutoLock lock( m_pDXi );

	if (NULL == pqIn)
		return E_POINTER;
	if (NULL == m_pTempoMap)
		return E_UNEXPECTED;

	int	nCount = 0;

	HRESULT hr = pqIn->GetCount( &nCount );
	if (FAILED( hr ))
		return hr;

	for (int ix = 0; ix < nCount; ++ix)
	{
		// Get the next datum
		MfxData md;
		hr = pqIn->GetAt( ix, &md );
		if (FAILED( hr ))
			break;

		// Determine offset in samples of this event
		LONGLONG llSampOffset = TicksToSamples( md.m_lTime );

		// Populate an entry for our pending queue
		DXiEvent de;
		de.llSampTimestamp = m_llSampClock + llSampOffset;
		de.mfxChannel = mfxChannel;
		de.pvData = NULL;
		de.dwFlags = DXiEvent::F_LIVE;
		de.me.m_byChan = md.m_byStatus & 0x0F;
		de.me.m_lTime = md.m_lTime;
		de.me.m_byPort = 0;
		de.me.m_byVelOff = 64;
		BOOL bRecognizedEvent = TRUE;

		// Interpret the status byte to determine other fields for the queued event
		int const nKind = md.m_byStatus & 0xF0;
		if (NOTE_OFF == nKind || NOTE_ON == nKind)
		{
			BOOL const bNoteOff = (NOTE_OFF == nKind) || (0 == md.m_byData2);
			de.me.m_eType = MfxEvent::Note;
			de.me.m_byKey = md.m_byData1 & 0x7F;
			de.me.m_byVel = md.m_byData2 & 0x7F;
			de.me.m_dwDuration = bNoteOff ? 0 : ULONG_MAX;
		}
		else if (KEYAFT == nKind)
		{
			de.me.m_eType = MfxEvent::KeyAft;
			de.me.m_byKey = md.m_byData1 & 0x7F;
			de.me.m_byAmt = md.m_byData2 & 0x7F;
		}
		else if (CONTROL == nKind)
		{
			de.me.m_eType = MfxEvent::Control;
			de.me.m_byNum = md.m_byData1 & 0x7F;
			de.me.m_byVal = md.m_byData2 & 0x7F;
		}
		else if (PATCH == nKind)
		{
			de.me.m_eType = MfxEvent::Patch;
			de.me.m_byPatch = md.m_byData1 & 0x7F;
			de.me.m_byBankSelMethod = md.m_byData2 & 0x7F;
			de.me.m_nBank = -1;
		}
		else if (CHANAFT == nKind)
		{
			de.me.m_eType = MfxEvent::ChanAft;
			de.me.m_byAmt = md.m_byData1 & 0x7F;
		}
		else if (WHEEL == nKind)
		{
			de.me.m_eType = MfxEvent::Wheel;
			de.me.m_nVal = static_cast<short>( (static_cast<WORD>(md.m_byData2) << 7) | md.m_byData1 );
			de.me.m_nVal -= 8192; // convert to signed -8192..8191
		}
		else
		{
			bRecognizedEvent = FALSE;
		}

		if (bRecognizedEvent)
		{
			// See if the DXi wants to use this event, and queue up if so
			hr = m_pDXi->IsValidEvent( de );
			if (FAILED( hr ))
				break;
			if (S_OK == hr)
				m_qPending.push_back( de );
		}
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::GetBanksForPatchNames( int** panBank, int* cBank )
{
	CAutoLock lock( m_pDXi );
	return m_pDefaultInstrument->GetBanksForPatchNames( panBank, cBank );
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::GetIsDrumPatch( int nBank, int nPatch )
{
	CAutoLock lock( m_pDXi );
	return m_pDefaultInstrument->GetIsDrumPatch( nBank, nPatch );
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::GetIsDiatonicNoteNames( int nBank, int nPatch )
{
	CAutoLock lock( m_pDXi );
	return m_pDefaultInstrument->GetIsDiatonicNoteNames( nBank, nPatch );
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::GetPatchNames( int nBank, IMfxNameList2** ppMap )
{
	CAutoLock lock( m_pDXi );
	return m_pDefaultInstrument->GetPatchNames( nBank, ppMap );
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::GetNoteNames( int nBank, int nPatch, IMfxNameList2** ppMap )
{
	CAutoLock lock( m_pDXi );
	return m_pDefaultInstrument->GetNoteNames( nBank, nPatch, ppMap );
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::GetControllerNames( IMfxNameList2** ppMap )
{
	CAutoLock lock( m_pDXi );
	return m_pDefaultInstrument->GetControllerNames( ppMap );
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::GetRpnNames( IMfxNameList2** ppMap )
{
	CAutoLock lock( m_pDXi );
	return m_pDefaultInstrument->GetRpnNames( ppMap );
}

//////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::GetNrpnNames( IMfxNameList2** ppMap )
{
	CAutoLock lock( m_pDXi );
	return m_pDefaultInstrument->GetNrpnNames( ppMap );
}

//////////////////////////////////////////////////////////////////////
// IMfxSoftSynth2

STDMETHODIMP CSoftSynth::GetInstrument( int nChannel, IMfxInstrument** ppInstrument )
{
	CAutoLock lock( m_pDXi );

	if (NULL == ppInstrument)
		return E_POINTER;

	*ppInstrument = m_pDefaultInstrument;
	(*ppInstrument)->AddRef();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// IMfxNotify

STDMETHODIMP CSoftSynth::OnMfxNotify( MfxNotifyMsg* pMsg )
{
	CAutoLock lock( m_pDXi );

	switch (pMsg->m_type)
	{
		case MfxNotifyMsg::ChannelMidiChannel:
			m_mapChannelState[ pMsg->m_mfxChannel ].nMidiChannel = pMsg->m_nMidiChannel;
			break;
		case MfxNotifyMsg::ChannelMuteMask:
		{
			BYTE byMuteMask = m_mapChannelState[ pMsg->m_mfxChannel ].muteMask;
			byMuteMask &= ~pMsg->m_maskClear.byte;
			byMuteMask |= pMsg->m_maskSet.byte;
			m_mapChannelState[ pMsg->m_mfxChannel ].muteMask = byMuteMask;
			break;
		}
		case MfxNotifyMsg::ChannelVelOfs:
			m_mapChannelState[ pMsg->m_mfxChannel ].nVelOfs = pMsg->m_nOfs;
			break;
		case MfxNotifyMsg::ChannelVelTrim:
			m_mapChannelState[ pMsg->m_mfxChannel ].nVelTrim = pMsg->m_nTrim;
			break;
		case MfxNotifyMsg::ChannelKeyOfs:
			m_mapChannelState[ pMsg->m_mfxChannel ].nKeyOfs = pMsg->m_nOfs;
			break;
		case MfxNotifyMsg::ChannelKeyTrim:
			m_mapChannelState[ pMsg->m_mfxChannel ].nKeyTrim = pMsg->m_nTrim;
			break;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// IMfxInputPort

STDMETHODIMP CSoftSynth::SetInputCallback( IMfxInputCallback* pCallback )
{
	CAutoLock lock( m_pDXi );

	if (pCallback)
		pCallback->AddRef();
	if (m_pInputCallback)
		m_pInputCallback->Release();
	m_pInputCallback = pCallback;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CSoftSynth::GetInputCallback( IMfxInputCallback** ppCallback )
{
	CAutoLock lock( m_pDXi );

	if (NULL == ppCallback)
		return E_POINTER;

	if (m_pInputCallback)
	{
		m_pInputCallback->AddRef();
		*ppCallback = m_pInputCallback;
		return S_OK;
	}
	else
	{
		*ppCallback = NULL;
		return S_FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CSoftSynth::ActivatePendingEvents( long cSamp )
{
	CAutoLock lock( m_pDXi );

	LONGLONG const llSampStart = m_llSampClock;
	LONGLONG const llSampEnd = m_llSampClock + cSamp;

	// Move new items from a pending queue to the active queue
	vector<DXiEvent>::iterator itPending = m_qPending.begin();
	while (itPending != m_qPending.end())
	{
		const DXiEvent& deTop = *itPending;

		// Activate a note which starts inside this buffer
		BOOL const bInBuf = llSampStart <= deTop.llSampTimestamp && deTop.llSampTimestamp < llSampEnd;
		if (bInBuf)
		{
			// Get a writable copy of the event
			DXiEvent de( deTop );

			// Take the event off the pending queue
			itPending = m_qPending.erase( itPending );

			// Get per-channel state (mute, offsets, forced channel)
			ChannelState& state = m_mapChannelState[ de.mfxChannel ];

			// Update this channel's mute mask caused by a mute meta-event
			if (MfxEvent::MuteMask == de.me.m_eType)
			{
				BYTE bTemp = state.muteMask;
				bTemp = (BYTE)( (bTemp  & ~de.me.m_maskClear.byte) | de.me.m_maskSet.byte );
				state.muteMask = bTemp;
			}

			// Apply just-in-time changes to note events
			if (MfxEvent::Note == de.me.m_eType)
			{
				if (state.muteMask.AnySet() && ((de.dwFlags & DXiEvent::F_STREAMED) || !state.muteMask.Record))
					continue; // skip muted notes altogether
				de.me.m_byKey = (BYTE) max( 0, min( 127, state.nKeyOfs + state.nKeyTrim + de.me.m_byKey ) );
				de.me.m_byVel = (BYTE) max( 0, min( 127, state.nVelOfs + state.nVelTrim + de.me.m_byVel ) );
			}

			// Apply just-in-time channelization
			if (-1 != state.nMidiChannel)
				de.me.m_byChan = (BYTE) (state.nMidiChannel & 0xF);

			// Let the synth set any extra information
			HRESULT hr = S_OK;
			if (MfxEvent::Note == de.me.m_eType)
			{
				// Don't pass zero-length note events (note-off events) to the synth,
				// because we'll handle them specially below.
				if (de.me.m_dwDuration > 0)
				{
					hr = m_pDXi->InitializeNoteEvent( de );
					if (FAILED( hr ))
						return hr;
				}
			}

			// Save the event into the active queue, only if the synth wants it
			if (S_OK == hr)
				m_qActive.push_back( de );
		}

		// Note starts before this buffer, so keep looking
		else
			itPending++;
	}

	// Handle note-offs from "live" events
	BYTE abyNoteOff[ 128 ];
	memset( abyNoteOff, 0, sizeof abyNoteOff );
	deque<DXiEvent>::iterator itActive = m_qActive.begin();
	while (itActive != m_qActive.end())
	{
		DXiEvent& de = *itActive;

		// A zero-duration note event acts as a note off for the oldest note-on,
		// and should also be removed right away to not make a sound.
		if (MfxEvent::Note == de.me.m_eType && 0 == de.me.m_dwDuration)
		{
			abyNoteOff[ de.me.m_byKey ] = TRUE;
			itActive = m_qActive.erase( itActive );
			continue;
		}
		itActive++;
	}

	// Look for "live" note-on events, and truncate their durations if they are
	// ready to be turned off.
	for (itActive = m_qActive.begin(); itActive != m_qActive.end(); itActive++)
	{
		DXiEvent& de = *itActive;

		// Is this a note event, that was played "live", that should be turned off?
		if (MfxEvent::Note == de.me.m_eType &&
			 ULONG_MAX == de.me.m_dwDuration &&
			 abyNoteOff[ de.me.m_byKey ])
		{
			// Truncate its duration, and make sure we don't turn any more of this note off
			de.me.m_dwDuration = 0;
			abyNoteOff[ de.me.m_byKey ] = FALSE;
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CSoftSynth::ExpireEvents( BOOL bForceAllExpired )
{
	CAutoLock lock( m_pDXi );

	HRESULT hr = S_OK;

	// Scan all active events
	deque<DXiEvent>::iterator it = m_qActive.begin();
	while (it != m_qActive.end())
	{
		DXiEvent& de = *it;
		if (MfxEvent::Note == de.me.m_eType)
		{
			hr = m_pDXi->ExpireNoteEvent( de, bForceAllExpired );
			if (FAILED( hr ))
				break;
			if (S_OK == hr)
			{
				it = m_qActive.erase( it );
				continue;
			}
		}
		else
		{
			// If the clock as run past the event's start time, the event is expired
			if (m_llSampClock > de.llSampTimestamp)
			{
				it = m_qActive.erase( it );
				continue;
			}
		}
		++it;
	}

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

LONGLONG CSoftSynth::TicksToSamples( LONG lTicks ) const
{
	CAutoLock lock( m_pDXi );

	if (m_pTimeConverter)
	{
		MFX_TIME mfxTime;
		mfxTime.timeFormat = TF_TICKS;
		mfxTime.lTicks = lTicks;
		HRESULT hr = m_pTimeConverter->ConvertMfxTime( &mfxTime, TF_SAMPLES );
		ASSERT( SUCCEEDED( hr ) );
		return mfxTime.llSamples;
	}
	else if (m_pTempoMap)
	{
		long lMsec = m_pTempoMap->TicksToMsecs( lTicks );
		return static_cast<LONGLONG>(lMsec) * m_pDXi->GetInputFormat()->nSamplesPerSec / 1000;
	}
	else
	{
		ASSERT(FALSE);
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////

LONG CSoftSynth::SamplesToTicks( LONGLONG llSamples ) const
{
	CAutoLock lock( m_pDXi );

	if (m_pTimeConverter)
	{
		MFX_TIME mfxTime;
		mfxTime.timeFormat = TF_SAMPLES;
		mfxTime.llSamples = llSamples;
		HRESULT hr = m_pTimeConverter->ConvertMfxTime( &mfxTime, TF_TICKS );
		ASSERT( SUCCEEDED( hr ) );
		return mfxTime.lTicks;
	}
	else if (m_pTempoMap)
	{
		long lMsec = static_cast<LONG>( (llSamples * 1000) / m_pDXi->GetInputFormat()->nSamplesPerSec );
		return m_pTempoMap->MsecsToTicks( lMsec );
	}
	else
	{
		ASSERT(FALSE);
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
