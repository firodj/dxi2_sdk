#ifndef _PLUGIN_PROP_PAGE_H_
#define _PLUGIN_PROP_PAGE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct IMediaParams;
struct IMediaParamsUICallback;

/////////////////////////////////////////////////////////////////////////////
// Custom slider control, to track mouse capture/release

class CCaptureSlider;

class CCaptureNotify
{
public:
	virtual void OnBeginCapture( int idCtrl, UINT nFlags, CPoint point ) = 0;
	virtual void OnEndCapture( int idCtrl, UINT nFlags, CPoint point ) = 0;
};

class CCaptureSlider : public CSliderCtrl
{
public:
	CCaptureSlider( CCaptureNotify* pNotify ) :
		m_pNotify(pNotify), m_bCaptured(FALSE) {}

	BOOL IsCaptured() const { return m_bCaptured; }

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCaptureSlider)
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP();

private:
	CCaptureNotify*	m_pNotify;
	BOOL			m_bCaptured;
};


/////////////////////////////////////////////////////////////////////////////
// CAutoClipPropPage dialog

class CAutoClipPropPage :
	public COlePropertyPage,
	public CUnknown,
	public CCaptureNotify
{
// Construction
public:
	CAutoClipPropPage( IUnknown* pUnk, HRESULT* phr );
	virtual ~CAutoClipPropPage();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);
	STDMETHODIMP_(ULONG) NonDelegatingRelease();
	STDMETHODIMP_(ULONG) NonDelegatingAddRef();

// Dialog Data
	//{{AFX_DATA(CAutoClipPropPage)
	enum { IDD = IDD_PROPPAGE };
	CCaptureSlider m_sliderThreshold;
	CCaptureSlider m_sliderOutputGain;
	//}}AFX_DATA

	// CCaptureNotify
	virtual void OnBeginCapture( int idCtrl, UINT nFlags, CPoint point );
	virtual void OnEndCapture( int idCtrl, UINT nFlags, CPoint point );

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAutoClipPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void OnObjectsChanged();

public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

	IMediaParams*				m_pMediaParams;
	IMediaParamsUICallback*	m_pUICallback;
	BOOL							m_bFirstQI;

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAutoClipPropPage)
	afx_msg void OnEnabled();
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _PLUGIN_PROP_PAGE_H_
