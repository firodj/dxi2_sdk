// SoftSynth.h: interface for the SoftSynth class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOFTSYNTH_H__FA4E145B_71E1_4F44_AA21_7814A7F8375D__INCLUDED_)
#define AFX_SOFTSYNTH_H__FA4E145B_71E1_4F44_AA21_7814A7F8375D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CInstrument;

#include "DXi.h"

////////////////////////////////////////////////////////////////////////////////

class CSoftSynth :
	public CDXiSynthContext,
	public IMfxSoftSynth2,
	public IMfxNotify,
	public IMfxInputPort
{
public:
	CSoftSynth( CDXi* pDXi, HRESULT* phr );
	virtual ~CSoftSynth();

	// IMfxSoftSynth
	STDMETHODIMP Connect( IUnknown* pContext );
	STDMETHODIMP Disconnect();
	STDMETHODIMP OnStart( LONG lTime );
	STDMETHODIMP OnLoop( LONG lTimeRestart, LONG lTimeStop );
	STDMETHODIMP OnStop( LONG lTime );
	STDMETHODIMP OnEvents( LONG lTimeFrom, LONG lTimeThru, MFX_CHANNEL mfxChannel, IMfxEventQueue* pqIn );
	STDMETHODIMP OnInput( MFX_CHANNEL mfxChannel, IMfxDataQueue* pqIn );
	STDMETHODIMP GetBanksForPatchNames( int** panBank, int* cBank );
	STDMETHODIMP GetIsDrumPatch( int nBank, int nPatch );
	STDMETHODIMP GetIsDiatonicNoteNames( int nBank, int nPatch );
	STDMETHODIMP GetPatchNames( int nBank, IMfxNameList2** ppMap );
	STDMETHODIMP GetNoteNames( int nBank, int nPatch, IMfxNameList2** ppMap );
	STDMETHODIMP GetControllerNames( IMfxNameList2** ppMap );
	STDMETHODIMP GetRpnNames( IMfxNameList2** ppMap );
	STDMETHODIMP GetNrpnNames( IMfxNameList2** ppMap );

	// IMfxSoftSynth2
	STDMETHODIMP GetInstrument( int nChannel, IMfxInstrument** ppInstrument );

	// IMfxNotify
	STDMETHODIMP OnMfxNotify( MfxNotifyMsg* pMsg );

	// IMfxInputPort
	STDMETHODIMP SetInputCallback( IMfxInputCallback* pCallback );
	STDMETHODIMP GetInputCallback( IMfxInputCallback** ppCallback );

	// CDXiSynthContext
	LONGLONG		TicksToSamples( LONG lTicks ) const;
	LONG			SamplesToTicks( LONGLONG llSamples ) const;

	// Get buffered MIDI data ready for rendering
	HRESULT ActivatePendingEvents( long cSamp );

	// Remove expired old MIDI data
	HRESULT ExpireEvents( BOOL bForceAllExpired );

protected:

	void releaseInterfaces();

	CDXi*						m_pDXi;

	IMfxTempoMap*			m_pTempoMap;
	IMfxTimeConverter*	m_pTimeConverter;
	IMfxInputCallback*	m_pInputCallback;
	IMfxNotifyHost*		m_pNotifyHost;
	BOOL						m_bMfxConnected;

	CInstrument*			m_pDefaultInstrument;

	struct ChannelState
	{
		ChannelState() : nMidiChannel(-1), muteMask(0), nVelOfs(0), nVelTrim(0), nKeyOfs(0), nKeyTrim(0) {}
		char			nMidiChannel;
		MfxMuteMask	muteMask;
		char			nVelOfs;
		char			nVelTrim;
		char			nKeyOfs;
		char			nKeyTrim;
	};

	vector<DXiEvent>					m_qPending;
	deque<DXiEvent>					m_qActive;
	map<MFX_CHANNEL,ChannelState>	m_mapChannelState;
	LONGLONG								m_llSampClock;
	LONGLONG								m_llSampStart;
	LONGLONG								m_llSampLoopUnroll;
};

#endif // !defined(AFX_SOFTSYNTH_H__FA4E145B_71E1_4F44_AA21_7814A7F8375D__INCLUDED_)
