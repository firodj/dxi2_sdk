#ifndef _PLUGIN_PROP_PAGE_H_
#define _PLUGIN_PROP_PAGE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct IMediaParams;
struct IMediaParamsUICallback;

#include "CaptureSlider.h"
#include "PianoButton.h"

/////////////////////////////////////////////////////////////////////////////
// CAutoSynthPropPage dialog

class CAutoSynthPropPage :
	public COlePropertyPage,
	public CUnknown,
	public CCaptureNotify
{
// Construction
public:
	CAutoSynthPropPage( IUnknown* pUnk, HRESULT* phr );
	virtual ~CAutoSynthPropPage();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);
	STDMETHODIMP_(ULONG) NonDelegatingRelease();
	STDMETHODIMP_(ULONG) NonDelegatingAddRef();

// Dialog Data
	//{{AFX_DATA(CAutoSynthPropPage)
	enum { IDD = IDD_PROPPAGE };
	CCaptureSlider	m_sliderCutoff;
	CCaptureSlider	m_sliderResonance;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAutoSynthPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void OnObjectsChanged();

	// CCaptureNotify
	virtual void OnBeginCapture( int idCtrl, UINT nFlags, CPoint point );
	virtual void OnEndCapture( int idCtrl, UINT nFlags, CPoint point );

public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

	IMediaParams*				m_pMediaParams;
	IMediaParamsUICallback*	m_pUICallback;
	BOOL							m_bFirstQI;

	IMfxInputPort*				m_pInputPort;
	IMfxInputCallback*		m_pInputCallback;
	CPianoButton				m_btnPiano;

private:
	BOOL fillMfxEventFromKey( MfxEvent& me, UINT nChar, BOOL bKeyDown );
	BOOL m_abKeyDown[ 12 ];

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAutoSynthPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnEnable();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _PLUGIN_PROP_PAGE_H_
