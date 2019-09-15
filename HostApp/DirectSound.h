// DirectSound.h: interface for the CDirectSound class.
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTSOUND_H__DF0A6A59_42AF_4884_8CBC_AB0A058868F1__INCLUDED_)
#define AFX_DIRECTSOUND_H__DF0A6A59_42AF_4884_8CBC_AB0A058868F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDxFilterGraph;

interface IDirectSound;
interface IDirectSoundBuffer;
interface IDirectSoundNotify;

typedef struct _DSBPOSITIONNOTIFY DSBPOSITIONNOTIFY;

class CDirectSound  
{
public:

	CDirectSound();
	virtual ~CDirectSound();

	HRESULT Initialize( HWND hWnd );
	HRESULT Terminate();
	HRESULT CreateOutputBuffer( int nSampleRate, int nBufferSamples );
	HRESULT DestroyOutputBuffer();

	HRESULT Play();
	HRESULT Stop();

	const WAVEFORMATEX& GetWaveFormat() const { return m_wfx; }

	const GUID& GetDeviceGuid() const { return m_guidDevice; }
	HRESULT SetDeviceGuid( const GUID& guid );

private:

	HRESULT onDirectSoundNotify();

	static UINT AFX_CDECL threadProc( void* pv );

private:

	enum { NUM_BUFFERS = 2 };

	HWND						m_hWnd;
	GUID						m_guidDevice;
	IDirectSound*			m_pDS;
	IDirectSoundBuffer*	m_pDSBOutput;
	IDirectSoundNotify*	m_pDSNotify;

	DSBPOSITIONNOTIFY*	m_aPosNotify;
	HANDLE					m_hEventNotify; 

	CWinThread*				m_pThread;
	LONG						m_bExitThread;
	int						m_nBufferSamples;
	int						m_nBufferIndex;
	WAVEFORMATEX			m_wfx;
};

#endif // !defined(AFX_DIRECTSOUND_H__DF0A6A59_42AF_4884_8CBC_AB0A058868F1__INCLUDED_)
