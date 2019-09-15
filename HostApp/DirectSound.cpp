// DirectSound.cpp: implementation of the CDirectSound class.
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlugInHost.h"
#include "DirectSound.h"
#include "DXiPlayer.h"

#include <InitGuid.h>
#include <dsound.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

CDirectSound::CDirectSound()
{
	m_hWnd = NULL;
	m_pDS = NULL;
	m_pDSBOutput = NULL;
	m_pDSNotify = NULL;
	m_aPosNotify = NULL;
	m_hEventNotify = NULL;
	m_guidDevice = DSDEVID_DefaultPlayback;
}

CDirectSound::~CDirectSound()
{
	Terminate();
}

//////////////////////////////////////////////////////////////////////

HRESULT CDirectSound::Initialize( HWND hWnd )
{
	ASSERT( NULL == m_pDS );
	ASSERT( NULL == m_pDSBOutput );
	ASSERT( NULL == m_pDSNotify );
	ASSERT( NULL == m_hEventNotify );

	m_hWnd = hWnd;

	CString strGuid = theApp.GetProfileString( theApp.GetOptionsSectionName(), "DSoundDevice", "" );
	if (0 != strGuid.GetLength())
	{
		wchar_t wszGuid[ 128 ];
		MultiByteToWideChar( CP_ACP, 0, strGuid, -1, wszGuid, sizeof(wszGuid ) );
		CHECK( CLSIDFromString( wszGuid, &m_guidDevice ) );
	}

	// Create IDirectSound using the selected device
	HRESULT hr = DirectSoundCreate( &GetDeviceGuid(), &m_pDS, NULL );
	if (FAILED( hr ))
	{
		m_guidDevice = DSDEVID_DefaultPlayback;
		CHECK( DirectSoundCreate( &GetDeviceGuid(), &m_pDS, NULL ) );
	}
	
	// Set cooperative level to DSSCL_PRIORITY
	CHECK( m_pDS->SetCooperativeLevel( hWnd, DSSCL_PRIORITY ) );

	// Create an event for notifications
	m_hEventNotify = CreateEvent( NULL, FALSE, FALSE, NULL );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDirectSound::Terminate()
{
	if (m_pDSBOutput)
	{
		CHECK( Stop() );
		CHECK( DestroyOutputBuffer() );
	}

	SAFE_RELEASE( m_pDS ); 

	if (m_hEventNotify)
	{
		CloseHandle( m_hEventNotify );
		m_hEventNotify = NULL;
	}

	SAFE_ARRAY_DELETE( m_aPosNotify );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////	

HRESULT CDirectSound::SetDeviceGuid( const GUID& guid )
{
	DWORD dwStatus = 0;
	if (m_pDSBOutput)
		CHECK( m_pDSBOutput->GetStatus( &dwStatus ) );

	CHECK( Terminate() );

	m_guidDevice = guid;

	LPOLESTR pwsz = NULL;
	CHECK( StringFromCLSID( m_guidDevice, &pwsz ) );
	CString strGuid( pwsz );
	CoTaskMemFree( pwsz );

	theApp.WriteProfileString( theApp.GetOptionsSectionName(), "DSoundDevice", strGuid );

	CHECK( Initialize( m_hWnd ) );

	if (dwStatus & DSBSTATUS_PLAYING)
	{
		CHECK( CreateOutputBuffer( m_wfx.nSamplesPerSec, m_nBufferSamples ) );
		CHECK( Play() );
	}	

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////	

HRESULT CDirectSound::CreateOutputBuffer( int nSampleRate, int nBufferSamples )
{
	if (NULL != m_pDSBOutput && NULL != m_pDSNotify)
		return S_OK; // already created

	m_nBufferSamples = nBufferSamples;

	// Initialize a 16-bit, stereo wave format
	m_wfx.wFormatTag			= WAVE_FORMAT_PCM; 
	m_wfx.nChannels			= 2; 
	m_wfx.nSamplesPerSec		= nSampleRate; 
	m_wfx.nBlockAlign			= 4; 
	m_wfx.nAvgBytesPerSec	=  m_wfx.nSamplesPerSec * m_wfx.nBlockAlign; 
	m_wfx.wBitsPerSample		= 16; 
	m_wfx.cbSize				= 0;
	
	// Set up DSBUFFERDESC structure.  We use 2 buffers in a ping-pong system
	DSBUFFERDESC dsbdesc;
	memset( &dsbdesc, 0, sizeof(DSBUFFERDESC) ); 
	dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
	dsbdesc.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
	dsbdesc.dwBufferBytes = NUM_BUFFERS * m_nBufferSamples * m_wfx.nBlockAlign;
	dsbdesc.lpwfxFormat = &m_wfx;
	
	// Create the output buffer
	CHECK( m_pDS->CreateSoundBuffer( &dsbdesc, &m_pDSBOutput, NULL ) );

	// Get a notification interface
	CHECK( m_pDSBOutput->QueryInterface( IID_IDirectSoundNotify, (void**)&m_pDSNotify ) );

	// Create notify structure
	if (NULL == m_aPosNotify)
	{
		m_aPosNotify = new DSBPOSITIONNOTIFY [ NUM_BUFFERS ];
		if (NULL == m_aPosNotify)
			CHECK(E_OUTOFMEMORY);
		memset( m_aPosNotify, 0, NUM_BUFFERS * sizeof(DSBPOSITIONNOTIFY) );
	}

	// Set up the notification positions
	DWORD const dwNotifySize = m_nBufferSamples * m_wfx.nBlockAlign;
	for (int ix = 0; ix < NUM_BUFFERS; ix++)
	{
		m_aPosNotify[ ix ].dwOffset = (dwNotifySize * ix) + dwNotifySize - 1;
		m_aPosNotify[ ix ].hEventNotify = m_hEventNotify;             
	}
	CHECK( m_pDSNotify->SetNotificationPositions( NUM_BUFFERS, m_aPosNotify ) );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////	

HRESULT CDirectSound::DestroyOutputBuffer()
{
	SAFE_RELEASE( m_pDSNotify );
	SAFE_RELEASE( m_pDSBOutput );
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDirectSound::Play()
{
	if (NULL == m_pDSBOutput)
		return E_FAIL; // not initialized

	DWORD dwStatus = 0;
	CHECK( m_pDSBOutput->GetStatus( &dwStatus ) );
	if (dwStatus & DSBSTATUS_PLAYING)
		return S_FALSE; // already playing

	// Kick off our helper thread
	m_bExitThread = FALSE;
	m_pThread = AfxBeginThread( threadProc, this, THREAD_PRIORITY_TIME_CRITICAL, 0, CREATE_SUSPENDED );
	if (NULL == m_pThread)
		CHECK(E_FAIL);
	m_pThread->m_bAutoDelete = FALSE;

	// Zerofill the output buffer
	void*	pvData = NULL;
	DWORD	cbTotal = NUM_BUFFERS * m_nBufferSamples * m_wfx.nBlockAlign;
	DWORD	cbData = cbTotal;
	CHECK( m_pDSBOutput->Lock( 0, cbTotal, &pvData, &cbData, NULL, NULL, 0 ) );
	ZeroMemory( pvData, cbData );
	CHECK( m_pDSBOutput->Unlock( pvData, cbData, NULL, 0 ) );

	// Specify the first buffer index to back-fill
	m_nBufferIndex = 0;

	// Start playback
	CHECK( m_pDSBOutput->Play( 0, 0, DSBPLAY_LOOPING ) );

	// Resume our thread, allowing it process DSound updates
	m_pThread->ResumeThread();

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDirectSound::Stop()
{
	if (NULL == m_pDSBOutput)
		return E_FAIL; // not initialized

	DWORD dwStatus = 0;
	CHECK( m_pDSBOutput->GetStatus( &dwStatus ) );
	if (0 == (dwStatus & DSBSTATUS_PLAYING))
		return S_FALSE; // already stopped

	// Stop our thread
	if (NULL != m_pThread)
	{
		InterlockedIncrement( &m_bExitThread );
		DWORD dw = WaitForSingleObject( m_pThread->m_hThread, 10000 );
		ASSERT( WAIT_OBJECT_0 == dw );
		SAFE_DELETE( m_pThread );
	}

	// Stop DSound
	CHECK( m_pDSBOutput->Stop() );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CDirectSound::onDirectSoundNotify()
{
	// Determine which segment to fill this time
	DWORD cbBuf = m_nBufferSamples * m_wfx.nBlockAlign;
	DWORD dwStart = m_nBufferIndex * cbBuf;
	void*	pvData = NULL;
	DWORD	cbData = cbBuf;

	// Get the buffer pointer
	CHECK( m_pDSBOutput->Lock( dwStart, cbBuf, &pvData, &cbData, NULL, NULL, 0 ) );
	ASSERT( cbData == cbBuf );

	// Fill it from the transport
	HRESULT hr = theDXiPlayer.FillBuffer( pvData, cbData );

	// Release the buffer pointer
	CHECK( m_pDSBOutput->Unlock( pvData, cbData, NULL, 0 ) );

	// Point to the next buffer
	++m_nBufferIndex;
	if (NUM_BUFFERS == m_nBufferIndex)
		m_nBufferIndex = 0;

	CHECK( hr );
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

UINT CDirectSound::threadProc( void* pv )
{
	CDirectSound* pThis = (CDirectSound*)pv;

	while (!pThis->m_bExitThread)
	{
		DWORD dw = WaitForSingleObject( pThis->m_hEventNotify, 1000 );
		if (WAIT_OBJECT_0 == dw)
		{
			pThis->onDirectSoundNotify();
		}
		else
		{
			TRACE("WaitForSingleObject did not return WAIT_OBJECT_0!\r\n");
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
