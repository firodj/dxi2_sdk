// CaptureSlider.cpp : implementation file
//

#include "stdafx.h"
#include "AutoSynth.h"
#include "CaptureSlider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCaptureSlider

BEGIN_MESSAGE_MAP(CCaptureSlider, CSliderCtrl)
	//{{AFX_MSG_MAP(CCaptureSlider)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCaptureSlider message handlers

void CCaptureSlider::OnLButtonDown( UINT nFlags, CPoint point )
{
	m_bCaptured = TRUE;
	if (m_pNotify)
		m_pNotify->OnBeginCapture( GetDlgCtrlID(), nFlags, point );
	CSliderCtrl::OnLButtonDown( nFlags, point );
}

void CCaptureSlider::OnLButtonUp( UINT nFlags, CPoint point )
{
	m_bCaptured = FALSE;
	if (m_pNotify)
		m_pNotify->OnEndCapture( GetDlgCtrlID(), nFlags, point );
	CSliderCtrl::OnLButtonUp( nFlags, point );
}

