// TransportBar.cpp : implementation file
//

#include "stdafx.h"
#include "PlugInHost.h"
#include "TransportBar.h"
#include "DXiPlayer.h"
#include "MfxSeq.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDXiPlayerBar dialog

CDXiPlayerBar::CDXiPlayerBar() : m_lPosition(0)
{
	//{{AFX_DATA_INIT(CDXiPlayerBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDXiPlayerBar, CDialogBar)
	//{{AFX_MSG_MAP(CDXiPlayerBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_EN_KILLFOCUS(IDC_NOW, OnKillfocusNow)
	ON_EN_KILLFOCUS(IDC_FROM, OnKillfocusFrom)
	ON_EN_KILLFOCUS(IDC_THRU, OnKillfocusThru)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Edit control data conversion

HRESULT CDXiPlayerBar::editToTick( UINT id, LONG* plTick )
{
	if (NULL == theDXiPlayer.GetSeq())
		return E_FAIL;
	CString str;
	CEdit* pEdit = (CEdit*) GetDlgItem( id );
	pEdit->GetWindowText( str );
	return theDXiPlayer.GetSeq()->m_meterKeySigMap.MbtStr2Tick( str, plTick );
}

HRESULT CDXiPlayerBar::tickToEdit( UINT id, LONG lTick )
{
	if (NULL == theDXiPlayer.GetSeq())
		return E_FAIL;
	char sz[ 32 ] = { 0 };
	theDXiPlayer.GetSeq()->m_meterKeySigMap.Tick2MbtStr( lTick, sz );
	CEdit* pEdit = (CEdit*) GetDlgItem( id );
	pEdit->SetWindowText( sz );
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CDXiPlayerBar message handlers

int CDXiPlayerBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Set up a timer to update the "now" time
	SetTimer( 0, 50, NULL );
	
	return 0;
}

void CDXiPlayerBar::OnDestroy() 
{
	KillTimer( 0 );
	CDialogBar::OnDestroy();
}

void CDXiPlayerBar::OnTimer(UINT nIDEvent) 
{
	if (!theDXiPlayer.IsPlaying())
		return;
	LONG lNewPos = theDXiPlayer.GetPosition();
	if (lNewPos != m_lPosition)
	{
		m_lPosition = lNewPos;
		tickToEdit( IDC_NOW, m_lPosition );
	}

	CDialogBar::OnTimer(nIDEvent);
}

void CDXiPlayerBar::OnKillfocusNow() 
{
	if (theDXiPlayer.IsPlaying())
		return;
	LONG lTick;
	if (SUCCEEDED( editToTick( IDC_NOW, &lTick ) ))
	{
		theDXiPlayer.SetPosition( lTick );
		tickToEdit( IDC_NOW, lTick );
	}
}

void CDXiPlayerBar::OnKillfocusFrom() 
{
	LONG lTick;
	if (SUCCEEDED( editToTick( IDC_FROM, &lTick ) ))
	{
		theDXiPlayer.SetLoopStart( lTick );
		tickToEdit( IDC_FROM, theDXiPlayer.GetLoopStart() );
		tickToEdit( IDC_THRU, theDXiPlayer.GetLoopEnd() );
	}
}

void CDXiPlayerBar::OnKillfocusThru() 
{
	LONG lTick;
	if (SUCCEEDED( editToTick( IDC_THRU, &lTick ) ))
	{
		theDXiPlayer.SetLoopEnd( lTick );
		tickToEdit( IDC_FROM, theDXiPlayer.GetLoopStart() );
		tickToEdit( IDC_THRU, theDXiPlayer.GetLoopEnd() );
	}
}
