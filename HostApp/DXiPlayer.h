// DXiPlayer.h: interface for the CDXiPlayer class.
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DXIPLAYER_H__49E279E0_1EFC_466B_BC0B_F34D0396D537__INCLUDED_)
#define AFX_DXIPLAYER_H__49E279E0_1EFC_466B_BC0B_F34D0396D537__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDirectSound;
class CDxFilterGraph;
class CMidiInput;
class CMfxDataQueue;
class CMfxSeq;
class CMfxHostSite;
class CCritSec;
struct IBaseFilter;

////////////////////////////////////////////////////////////////////////////////

static const UINT MIN_BUFFER_MSEC = 1;
static const UINT MAX_BUFFER_MSEC = 2000;

class CDXiPlayer
{
public:

	CDXiPlayer();
	virtual ~CDXiPlayer();

	HRESULT Initialize( HWND hWnd );
	HRESULT Terminate();

	UINT GetBufferMsec() const { return m_nBufferMsec; }
	HRESULT SetBufferMsec( UINT n );

	CDirectSound*		GetDirectSound() const { return m_pDirectSound; }
	CDxFilterGraph*	GetFilterGraph() const { return m_pFilterGraph; }
	CMidiInput*			GetMidiInput() const { return m_pMidiInput; }
	IMfxSoftSynth*		GetSoftSynth() const { return m_pMfxSynth; }
	CMfxHostSite*		GetHostSite() const { return m_pMfxHostSite; }

	CMfxSeq* GetSeq() const { return m_pSeq; }
	HRESULT SetSeq( CMfxSeq* pSeq );
	
	int GetCurrentTrack() const { return m_nCurrentTrack; }
	void SetCurrentTrack( int n ) { m_nCurrentTrack = n; }

	HRESULT SetFilter( IBaseFilter* pFilter );

	HRESULT Play( BOOL bPlaySeq );
	HRESULT Stop( BOOL bFullStop );
	HRESULT Rewind();

	LONG GetPosition();
	HRESULT SetPosition( LONG lTicks );

	HRESULT FillBuffer( void* pv, DWORD cb );

	HRESULT OnMidiShortMsg( DWORD dwData, DWORD dwTimestamp );

	BOOL IsPlaying() const { return m_bPlaying; }

	BOOL IsLooping() const { return m_bLooping; }
	void SetLooping( BOOL b );

	LONG GetLoopStart() const { return m_tLoopStart; }	
	HRESULT SetLoopStart( LONG t );

	LONG GetLoopEnd() const { return m_tLoopEnd; }	
	HRESULT SetLoopEnd( LONG t );

	void SendMuteState( MFX_CHANNEL mfxCh, MfxMuteMask muteSetBits, MfxMuteMask muteClearBits );
	void SendKeyOfs( MFX_CHANNEL mfxCh, int nKeyOfs );
	void SendVelOfs( MFX_CHANNEL mfxCh, int nVelOfs );
	void SendVol( int nMidiCh, int nVol );
	void SendPan( int nMidiCh, int nPan );
	void SendPatch( int nChan, int nPatch );
	void SendBank( int nChan, int nBank, BYTE bsm );

private:

	HRESULT updateLoopSampleTimes();
	HRESULT mfxNotify( MfxNotifyMsg& msg );
	HRESULT sendShortMsg( int data1, int data2, int data3 );
	HRESULT sendShortMsg( int data1, int data2 );
	HRESULT queueData( const MfxData& mfxd );

private:

	CDirectSound*		m_pDirectSound;
	CDxFilterGraph*	m_pFilterGraph;
	CMidiInput*			m_pMidiInput;
	IBaseFilter*		m_pFilter;
	IMfxSoftSynth*		m_pMfxSynth;
	CMfxHostSite*		m_pMfxHostSite;

	CMfxSeq*				m_pSeq;

	BOOL					m_bRunning;
	BOOL					m_bPlaying;
	BOOL					m_bLooping;
	LONG					m_tLoopStart;
	LONG					m_tLoopEnd;
	LONGLONG				m_llSampLoopStart;
	LONGLONG				m_llSampLoopEnd;
	LONG					m_tNow;
	UINT					m_nBufferMsec;
	int					m_nCurrentTrack;

	CMfxDataQueue*		m_pqData;
	CRITICAL_SECTION	m_csData;
};

extern CDXiPlayer theDXiPlayer;

////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_DXIPLAYER_H__49E279E0_1EFC_466B_BC0B_F34D0396D537__INCLUDED_)
