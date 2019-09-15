#if !defined(AFX_CAPTURESLIDER_H__7DF549A9_E18B_4369_B26B_07022A752255__INCLUDED_)
#define AFX_CAPTURESLIDER_H__7DF549A9_E18B_4369_B26B_07022A752255__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CaptureSlider.h : header file
//

/////////////////////////////////////////////////////////////////////////////

class CCaptureNotify
{
public:
	virtual void OnBeginCapture( int idCtrl, UINT nFlags, CPoint point ) = 0;
	virtual void OnEndCapture( int idCtrl, UINT nFlags, CPoint point ) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// CCaptureSlider window

class CCaptureSlider : public CSliderCtrl
{
// Construction
public:
	CCaptureSlider( CCaptureNotify* pNotify ) : m_pNotify(pNotify), m_bCaptured(FALSE) {}

// Attributes
public:
	BOOL IsCaptured() const { return m_bCaptured; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCaptureSlider)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCaptureSlider() {}

	// Generated message map functions
protected:
	//{{AFX_MSG(CCaptureSlider)
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CCaptureNotify*	m_pNotify;
	BOOL					m_bCaptured;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAPTURESLIDER_H__7DF549A9_E18B_4369_B26B_07022A752255__INCLUDED_)
