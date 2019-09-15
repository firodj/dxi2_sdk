#ifndef _PLUGIN_PROP_PAGE_H_
#define _PLUGIN_PROP_PAGE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FilterPropPage.h : header file
//

struct ITwonar;

#include "PianoButton.h"

/////////////////////////////////////////////////////////////////////////////
// CTwonarPropPage dialog

class CTwonarPropPage :
	public COlePropertyPage,
	public CUnknown
{
// Construction
public:
	CTwonarPropPage( IUnknown* pUnk, HRESULT* phr );
	virtual ~CTwonarPropPage();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);
	STDMETHODIMP_(ULONG) NonDelegatingRelease();
	STDMETHODIMP_(ULONG) NonDelegatingAddRef();

// Dialog Data
	//{{AFX_DATA(CTwonarPropPage)
	enum { IDD = IDD_PROPPAGE };
	BOOL	m_bMultiOut;
	int		m_nMode;
	BOOL	m_bEnableMetro;
	int		m_nNoteMeasTop;
	int		m_nVelMeasTop;
	int		m_nNoteBeat;
	int		m_nVelBeat;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTwonarPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void OnObjectsChanged();

public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

	ITwonar*					m_pITwonar;
	BOOL						m_bFirstQI;
	IMfxInputPort*			m_pInputPort;
	IMfxInputCallback*	m_pInputCallback;

	CPianoButton	m_btnPiano;

public:
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

private:
	BOOL fillMfxEventFromKey( MfxEvent& me, UINT nChar, BOOL bKeyDown );
	BOOL m_abKeyDown[ 12 ];

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTwonarPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusNoteMeasTop();
	afx_msg void OnKillfocusVelMeasTop();
	afx_msg void OnKillfocusNoteBeat();
	afx_msg void OnKillfocusVelBeat();
	afx_msg void OnEnableMetro();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _PLUGIN_PROP_PAGE_H_
