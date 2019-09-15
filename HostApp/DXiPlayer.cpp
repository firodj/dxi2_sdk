// DXiPlayer.cpp: implementation of the CDXiPlayer class.
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <DxFilterGraph.h>
#include <MidiInput.h>
#include <MfxSeq.h>
#include <MfxHostSite.h>
#include <MfxEventQueue.h>
#include <MfxDataQueue.h>

#include "PlugInHost.h"
#include "DXiPlayer.h"
#include "DirectSound.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDXiPlayer theDXiPlayer;

//////////////////////////////////////////////////////////////////////

CDXiPlayer::CDXiPlayer()
{
	m_pDirectSound = NULL;
	m_pFilterGraph = NULL;
	m_pMidiInput = NULL;
	m_pFilter = NULL;
	m_pSeq = NULL;
	m_pMfxSynth = NULL;
	m_pMfxHostSite = NULL;
	m_bPlaying = FALSE;
	m_bLooping = FALSE;
	m_bRunning = FALSE;
	m_tLoopStart = 0;
	m_tLoopEnd = 0;
	m_llSampLoopStart = 0;
	m_llSampLoopEnd = 0;
	m_tNow = 0;
	m_pqData = NULL;
	m_nBufferMsec = 100;
	m_nCurrentTrack = -1;
	InitializeCriticalSection( &m_csData );
}

CDXiPlayer::~CDXiPlayer()
{
	Terminate();
	DeleteCriticalSection( &m_csData );
}

//////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::Initialize( HWND hWnd )
{
	if (m_pDirectSound || m_pFilterGraph)
		CHECK(E_FAIL); // already initialized

	m_nBufferMsec = theApp.GetProfileInt( theApp.GetOptionsSectionName(), "BufferMsec", 100 );

	m_pDirectSound = new CDirectSound;
	if (NULL == m_pDirectSound)
		CHECK(E_OUTOFMEMORY);

	m_pMidiInput = new CMidiInput;
	if (NULL == m_pMidiInput)
		CHECK(E_OUTOFMEMORY);

	m_pqData = CMfxDataQueue::Create();
	if (NULL == m_pqData)
		CHECK(E_OUTOFMEMORY);

	HRESULT hr = S_OK;
	m_pFilterGraph = new CDxFilterGraph( &hr );
	CHECK( hr );
	if (NULL == m_pFilterGraph)
		CHECK(E_OUTOFMEMORY);

	CHECK( CMfxHostSite::Create( &m_pMfxHostSite ) );

	CHECK( GetMidiInput()->Initialize() );
	CHECK( GetDirectSound()->Initialize( hWnd ) );

	return S_OK;
}

///////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::SetBufferMsec( UINT n )
{
	if (IsPlaying())
		CHECK(E_FAIL);
	if (n < MIN_BUFFER_MSEC || n > MAX_BUFFER_MSEC)
		CHECK(E_INVALIDARG);

	// Don't pull the rug out from under playback
	BOOL const bWasRunning = m_bRunning;
	BOOL const bWasPlaying = IsPlaying();
	Stop( TRUE );

	m_nBufferMsec = n;

	theApp.WriteProfileInt( theApp.GetOptionsSectionName(), "BufferMsec", m_nBufferMsec );

	// Restart if necessary
	if (bWasRunning)
		Play( bWasPlaying );

	return S_OK;
}

///////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::Terminate()
{
	SAFE_RELEASE( m_pMfxHostSite );

	if (GetFilterGraph())
	{
		CHECK( SetFilter( NULL ) );
		SAFE_DELETE( m_pFilterGraph );
	}

	if (GetDirectSound())
	{
		CHECK( GetDirectSound()->Terminate() );
		SAFE_DELETE( m_pDirectSound );
	}

	if (GetMidiInput())
	{
		CHECK( GetMidiInput()->Terminate() );
		SAFE_DELETE( m_pMidiInput );
	}

	SAFE_RELEASE( m_pqData );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::SetSeq( CMfxSeq* pSeq )
{
	// We can't change CMfxSeq's while playing
	CHECK( Stop( TRUE ) );

	// Disconnect the old DXi before changing the seq
	if (m_pMfxSynth)
		m_pMfxSynth->Disconnect();
	
	// Set the new seq
	m_pSeq = pSeq;

	// Do we have something to reconnect?
	if (GetSeq() && m_pMfxSynth)
	{
		// Reconnect the DXi
		GetHostSite()->SetSeq( GetSeq() );
		GetHostSite()->AddRef();
		CHECK( m_pMfxSynth->Connect( reinterpret_cast<IMfxTempoMap*>( GetHostSite() ) ) );

		// Restart live thruing of MIDI
		CHECK( SetPosition( 0 ) );
		CHECK( Play( FALSE ) );
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::SetFilter( IBaseFilter* pFilter )
{
	if (pFilter == m_pFilter)
		return S_OK; // nothing to do

	// We can't change filters while playing
	CHECK( Stop( TRUE ) );

	// Disconnect and release the DXi first
	if (m_pMfxSynth)
	{
		CHECK( m_pMfxSynth->Disconnect() );
		SAFE_RELEASE( m_pMfxSynth );
	}

	// Set the new filter in its graph
	CHECK( GetFilterGraph()->SetFilter( pFilter ) );

	// Store the filter pointer
	if (m_pFilter)
		m_pFilter->Release();
	m_pFilter = pFilter;
	if (m_pFilter)
		m_pFilter->AddRef();

	// Query for a DXi
	if (m_pFilter && SUCCEEDED( m_pFilter->QueryInterface( IID_IMfxSoftSynth, (void**)&m_pMfxSynth ) ))
	{
		ASSERT( GetSeq() );
		if (NULL == GetSeq())
			CHECK(E_FAIL);

		// Connect the DXi if it was found
		GetHostSite()->SetSeq( GetSeq() );
		GetHostSite()->AddRef();
		CHECK( m_pMfxSynth->Connect( reinterpret_cast<IMfxTempoMap*>( GetHostSite() ) ) );

		// Restart live thruing of MIDI
		CHECK( Play( FALSE ) );
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::Play( BOOL bPlaySeq )
{
	if (NULL == m_pDirectSound || NULL == m_pFilterGraph)
		CHECK(E_FAIL); // not initialized
	if (m_bRunning)
		CHECK(E_FAIL); // already playing

	// Make sure our loop sample times are up to date
	CHECK( updateLoopSampleTimes() );

	// Initialize the filter graph
	CHECK( GetFilterGraph()->SetSampleRate( SAMPLE_RATE ) );
	CHECK( GetFilterGraph()->SetBufferSize( (GetBufferMsec() * SAMPLE_RATE) / 1000 ) );

	// Create the output buffer
	CHECK( GetDirectSound()->CreateOutputBuffer( SAMPLE_RATE, (GetBufferMsec() * SAMPLE_RATE) / 1000 ) );

	// Position the graph et al
	CHECK( SetPosition( m_tNow ) );

	// Start the DXi
	if (m_pMfxSynth)
		CHECK( m_pMfxSynth->OnStart( m_tNow ) );

	// Start the filter graph
	CHECK( GetFilterGraph()->Pause() );
	CHECK( GetFilterGraph()->Play() );

	m_bRunning = TRUE;
	m_bPlaying = bPlaySeq;

	// Start streaming through DirectSound
	CHECK( GetDirectSound()->Play() );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::Stop( BOOL bFullStop )
{
	if (NULL == m_pDirectSound || NULL == m_pFilterGraph)
		CHECK(E_FAIL); // not initialized
	if (!m_bRunning)
		return S_OK; // already stopped

	// Stop streaming through DirectSound
	if (bFullStop)
	{
		CHECK( GetDirectSound()->Stop() );
		CHECK( GetDirectSound()->DestroyOutputBuffer() );
	}

	// Stop the filter graph
	CHECK( GetFilterGraph()->Stop() );

	// Remember where we stopped
	if (m_bPlaying)
		m_tNow = GetPosition();

	// Stop the DXi
	if (m_pMfxSynth)
		m_pMfxSynth->OnStop( m_tNow );

	m_bPlaying = m_bRunning = FALSE;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::Rewind()
{
	CHECK( Stop( TRUE ) );
	CHECK( SetPosition( 0 ) );
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

LONG CDXiPlayer::GetPosition()
{
	if (NULL != GetSeq())
	{
		MFX_TIME t;
		t.timeFormat = TF_SAMPLES;
		t.llSamples = GetFilterGraph()->GetPosition();
		GetHostSite()->ConvertMfxTime( &t, TF_TICKS );
		const_cast<CDXiPlayer*>(this)->m_tNow = t.lTicks;
	}
	return m_tNow;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::SetPosition( LONG lTicks )
{
	if (NULL == GetSeq())
		return E_FAIL;

	// Convert tick time to samples
	MFX_TIME t;
	t.timeFormat = TF_TICKS;
	t.lTicks = lTicks;
	CHECK( GetHostSite()->ConvertMfxTime( &t, TF_SAMPLES ) );

	// Reposition the graph
	CHECK( GetFilterGraph()->SetPosition( t.llSamples ) );

	// Reposition each track in the sequence
	GetSeq()->SetPosition( lTicks );

	// Remember our position
	m_tNow = lTicks;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::SetLoopStart( LONG t )
{
	m_tLoopStart = t;
	if (m_tLoopStart > m_tLoopEnd)
		m_tLoopEnd = m_tLoopStart;

	CHECK( updateLoopSampleTimes() );
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::SetLoopEnd( LONG t )
{
	m_tLoopEnd = t;
	if (m_tLoopStart > m_tLoopEnd)
		m_tLoopEnd = m_tLoopStart;

	CHECK( updateLoopSampleTimes() );
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

void CDXiPlayer::SetLooping( BOOL b )
{
	if (b == m_bLooping)
		return; // nothing to do
	
	m_bLooping = b;

	if (IsLooping())
	{
		// Turn zero length loops into something reasonable
		if (m_tLoopStart == m_tLoopEnd)
			m_tLoopEnd = m_tLoopStart + (GetSeq() ? GetSeq()->m_tempoMap.GetPPQ() : 120) * 4;

		// Make sure the sample times are in sync
		updateLoopSampleTimes();
		
		// If they've enabled looping and we're already past the loop end time,
		// reposition to the start of the loop
		if (GetPosition() > m_tLoopEnd && IsPlaying())
		{
			Stop( TRUE );
			SetPosition( GetLoopStart() );
			Play( TRUE );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::updateLoopSampleTimes()
{
	if (NULL == GetSeq())
		CHECK(E_FAIL);

	MFX_TIME t;

	t.timeFormat = TF_TICKS;
	t.lTicks = GetLoopStart();
	CHECK( GetHostSite()->ConvertMfxTime( &t, TF_SAMPLES ) );
	m_llSampLoopStart = t.llSamples;

	t.timeFormat = TF_TICKS;
	t.lTicks = GetLoopEnd();
	CHECK( GetHostSite()->ConvertMfxTime( &t, TF_SAMPLES ) );
	m_llSampLoopEnd = t.llSamples;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::FillBuffer( void* pvData, DWORD cbData )
{	
	if (NULL == GetFilterGraph() ||
		 NULL == GetSoftSynth() ||
		 NULL == GetHostSite() ||
		 NULL == GetSeq())
		return S_OK; // nothing to do

	short* pnOutput = (short*)pvData;

	// Determine loop times
	LONGLONG	llLoopFrom = IsLooping() ? m_llSampLoopStart : _I64_MIN;
	LONGLONG llLoopThru = IsLooping() ? m_llSampLoopEnd : _I64_MAX;

	// Loopback may require repeated MIDI/audio streaming
	LONG cSampToGo = GetFilterGraph()->GetBufferSize();
	while (cSampToGo > 0)
	{
		LONGLONG llPosition = GetFilterGraph()->GetPosition();

		// See if we need to loop back
		BOOL bLoopedBack = FALSE;
		if (llPosition >= llLoopThru)
		{
			llPosition = llLoopFrom;
			bLoopedBack = TRUE;
		}
	
		// Determine how many samples to process this time
		LONG cSampProcess = cSampToGo;
		if (llPosition + cSampProcess > llLoopThru)
			cSampProcess = static_cast<LONG>( llLoopThru - llPosition );

		// Convert audio buffer times to MIDI ticks
		MFX_TIME t;
		t.timeFormat = TF_SAMPLES;
		t.llSamples = llPosition;
		CHECK( GetHostSite()->ConvertMfxTime( &t, TF_TICKS ) );
		LONG const lTickFrom = t.lTicks;
		t.timeFormat = TF_SAMPLES;
		t.llSamples = llPosition + cSampProcess;
		CHECK( GetHostSite()->ConvertMfxTime( &t, TF_TICKS ) );
		LONG const lTickUpTo = t.lTicks + 1;
		
		// Deal with loop back
		if (bLoopedBack)
		{
			// Tell the DXi about it
			CHECK( m_pMfxSynth->OnLoop( GetLoopStart(), GetLoopEnd() ) );
				
			// Reposition the playing seq
			GetSeq()->SetPosition( GetLoopStart() );
		}

		// Feed any input data to the DXi
		if (NULL != m_pqData && !m_pqData->IsEmpty())
		{
			EnterCriticalSection( &m_csData );

			// Apply the current track's properties, just in time
			CMfxTrack* pCurrentTrack = GetSeq()->GetTrack( GetCurrentTrack() );
			int cData = 0;
			m_pqData->GetCount( &cData );
			for (int ix = 0; ix < cData; ix++)
			{
				MfxData& mfxData = (*m_pqData)[ ix ];
				if (pCurrentTrack)
					pCurrentTrack->ApplyTrackProperties( &mfxData.m_dwData );
				mfxData.m_lTime = lTickFrom;
			}

			m_pMfxSynth->OnInput( GetCurrentTrack(), m_pqData );
			m_pqData->Reset();

			LeaveCriticalSection( &m_csData );
		}
		
		if (m_bPlaying)
		{
			// Feed data from each track
			map<int,CMfxTrack>::iterator it;
			for (it = GetSeq()->GetBeginTrack(); it != GetSeq()->GetEndTrack(); it++)
			{
				int		nTrack = it->first;
				CMfxTrack&	trk = it->second;

				// Get the index of the next event to be fed to the DXi
				int ixFrom = trk.GetPlayIndex();
				int ixUpTo = ixFrom;
				
				if (ixFrom < trk.size())
				{
					// Do we have an event to feed?
					LONG const tCurr = trk[ ixFrom ].GetTime();
					if (tCurr < lTickUpTo)
					{
						for (ixUpTo = ixFrom + 1; ixUpTo < trk.size(); ixUpTo++)
						{
							if (trk[ ixUpTo ].GetTime() > lTickUpTo)
								break;
						}
					}
				}
				
				// Create an IMfxEventQueue for the next range of events to feed
				if (ixUpTo > ixFrom)
				{
					CMfxEventQueue queue( trk, ixFrom, ixUpTo );

					// Stream events to the DXi
					CHECK( m_pMfxSynth->OnEvents( lTickFrom, lTickUpTo, nTrack, &queue ) );
					
					// Update the track's play index
					trk.SetPlayIndex( ixUpTo );
				}
			}
		}
		
		// Loop back the filter graph if necessary
		if (bLoopedBack)
			CHECK( GetFilterGraph()->SetPosition( llPosition ) );

		// Pump another buffer through the filter graph
		float* pfOutput = NULL;
		CHECK( GetFilterGraph()->Process( &pfOutput, cSampProcess ) );
		
		// Fill the data.  Note we assume stereo!
		for (int ix = 0; ix < cSampProcess * 2; ix++)
		{
			int n = pfOutput[ix] * 32767;
			if (n < -32768)
				n = -32768;
			else if (n > 32767)
				n = 32767;

			*pnOutput = static_cast<short>( n );
			pnOutput++;
		}

		// Tally up what we processed so far
		cSampToGo -= cSampProcess;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::queueData( const MfxData& mfxd )
{
	EnterCriticalSection( &m_csData );
	HRESULT hr = m_pqData->Add( mfxd );
	LeaveCriticalSection( &m_csData );
	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::OnMidiShortMsg( DWORD dwData, DWORD dwTimestamp )
{
	if (0x000000FE == dwData)
		return S_OK; // ignore active sensing

	if (m_pqData)
	{
		MfxData data;
		data.m_lTime = 0; // TODO: Convert dwTimestamp to musical time
		data.m_dwData = dwData;
		queueData( data );
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::mfxNotify( MfxNotifyMsg& msg )
{
	IMfxSoftSynth* pSynth = GetSoftSynth();
	if (pSynth)
	{
		IMfxNotify* pNotify = NULL;
		if (SUCCEEDED( pSynth->QueryInterface( IID_IMfxNotify, (void**)&pNotify ) ))
		{
			pNotify->OnMfxNotify( &msg );
			pNotify->Release();
		}
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::sendShortMsg( int data1, int data2, int data3 )
{
	if (NULL == m_pqData)
		CHECK(E_FAIL);

	MfxData mfxData;
	mfxData.m_lTime = 0;
	mfxData.m_dwData = data1;
	((BYTE*)&mfxData.m_dwData)[ 1 ] = data2;
	((BYTE*)&mfxData.m_dwData)[ 2 ] = data3;

	return queueData( mfxData );
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDXiPlayer::sendShortMsg( int data1, int data2 )
{
	if (NULL == m_pqData)
		CHECK(E_FAIL);

	MfxData mfxData;
	mfxData.m_lTime = 0;
	mfxData.m_dwData = data1;
	((BYTE*)&mfxData.m_dwData)[ 1 ] = data2;

	return queueData( mfxData );
}

//////////////////////////////////////////////////////////////////////

void CDXiPlayer::SendMuteState( MFX_CHANNEL mfxCh, MfxMuteMask muteSetBits, MfxMuteMask muteClearBits )
{
	MfxNotifyMsg msg;
	msg.m_type = MfxNotifyMsg::ChannelMuteMask;
	msg.m_mfxChannel = mfxCh;
	msg.m_maskSet = muteSetBits;
	msg.m_maskClear = muteClearBits;

	mfxNotify( msg );
}

/////////////////////////////////////////////////////////////////////

void CDXiPlayer::SendKeyOfs( MFX_CHANNEL mfxCh, int nKeyOfs )
{
	MfxNotifyMsg msg;
	msg.m_type = MfxNotifyMsg::ChannelKeyOfs;
	msg.m_mfxChannel = mfxCh;
	msg.m_nOfs = nKeyOfs;

	mfxNotify( msg );
}

/////////////////////////////////////////////////////////////////////

void CDXiPlayer::SendVelOfs( MFX_CHANNEL mfxCh, int nVelOfs )
{
	MfxNotifyMsg msg;
	msg.m_type = MfxNotifyMsg::ChannelVelOfs;
	msg.m_mfxChannel = mfxCh;
	msg.m_nOfs = nVelOfs;

	mfxNotify( msg );
}

/////////////////////////////////////////////////////////////////////

void CDXiPlayer::SendVol( int nMidiCh, int nVol )
{
	// Send a CC7 to our channel, if we have one
	if (-1 != nMidiCh)
		sendShortMsg( CONTROL | nMidiCh, 7, nVol );
}

/////////////////////////////////////////////////////////////////////

void CDXiPlayer::SendPan( int nMidiCh, int nPan )
{
	// Send a CC10 to our channel, if we have one
	if (-1 != nMidiCh)
		sendShortMsg( CONTROL | nMidiCh, 10, nPan );
}

//////////////////////////////////////////////////////////////////////

void CDXiPlayer::SendBank( int nChan, int nBank, BYTE bsm )
{
	if (-1 == nChan)
		return; // no channel to send it to

	if (nBank != BANK_NONE)
	{
		switch (bsm)
		{
			case Normal:
			{
				if (!( 0 <= nBank && nBank <= MAX_BANK ))
					return;

				BYTE msb, lsb;
				WORD14_BYTES( static_cast<WORD>(nBank), &lsb, &msb );

				sendShortMsg( CONTROL | nChan, 0, msb );
				sendShortMsg( CONTROL | nChan, 32, lsb );
				break;
			}

			case Ctrl0:
				sendShortMsg( CONTROL | nChan, 0, nBank & 0x7F );
				break;

			case Ctrl32:
				if (nBank <= 127)
					sendShortMsg( CONTROL | nChan, 32, nBank );
				else
					sendShortMsg( CONTROL | nChan, 32, (nBank >> 7) & 0x7F );
				break;

			case Patch100:
				sendShortMsg( PATCH | nChan, min( 127, nBank + 100 ) );
				break;

			default:
				ASSERT( FALSE );
				return;
		}
	}
}

//////////////////////////////////////////////////////////////////////

void CDXiPlayer::SendPatch( int nChan, int nPatch )
{
	if (-1 == nChan)
		return; // no channel to send it to

	if (nPatch != PATCH_NONE)
	{
		ASSERT( 0 <= nPatch && nPatch <= 127 );

		sendShortMsg( PATCH | nChan, nPatch & 0x7F );
	}
}

//////////////////////////////////////////////////////////////////////
