// MainFrm.cpp : implementation of the CMainFrame class
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//

#include "stdafx.h"
#include "PlugInHost.h"
#include "PlugInHostDoc.h"

#include "MainFrm.h"
#include "PropertyPagesView.h"
#include "TrackPropertiesView.h"
#include "DXiPlayer.h"
#include "OptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_INITMENUPOPUP()
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_OPTIONS, OnUpdateOptions)
	ON_COMMAND(ID_OPTIONS, OnOptions)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(ID_DXI_MIN, ID_DXI_MAX, OnUpdateDXi)
	ON_COMMAND_RANGE(ID_DXI_MIN, ID_DXI_MAX, OnDXi)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_bFilledDXiMenu = FALSE;
	m_ixDXi = 0;
}

CMainFrame::~CMainFrame()
{
}

/////////////////////////////////////////////////////////////////////////////

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndTransportBar.Create(this, IDD_DIALOGBAR, CBRS_TOP, 666))
	{
		TRACE0("Failed to create dialog bar\n");
		return -1;
	}

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar) ||
		!m_wndReBar.AddBar(&m_wndTransportBar))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	// Initialize the transport
	HRESULT hr = theDXiPlayer.Initialize( GetSafeHwnd() );
	ASSERT( SUCCEEDED( hr ) );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	// Set up the splitter
	m_wndSplitter.CreateStatic( this, 1, 2 );
	
	// Allow the splitter window to receive tooltip messages from its childen
	m_wndSplitter.EnableToolTips( TRUE );

	// Create each column
	if (!m_wndSplitter.CreateView( 0, 0, RUNTIME_CLASS( CTrackPropertiesView ), CSize( 10, 10 ), pContext ))
		return FALSE;
	if (!m_wndSplitter.CreateView( 0, 1, RUNTIME_CLASS( CPropertyPagesView ), CSize( 10, 10 ), pContext ))
		return FALSE;

	// Set each column's ideal and minimum width.
	m_wndSplitter.SetColumnInfo( 0, CTrackPropertiesView::GetIdealWidth(), 0 );
	m_wndSplitter.SetColumnInfo( 1, CPropertyPagesView::GetIdealWidth(), 0 );

	// Shared scrollbars
	m_wndSplitter.SetScrollStyle( 0 );
	m_wndSplitter.RecalcLayout();

	// Set the active pane explicitly: To row 0 column 1, which is design pane.
	// This helps the MDI framework by having the active pane be a CView as opposed
	// to a mere CWnd (e.g. window title is non-blank).
	m_wndSplitter.SetActivePane( 0, 1 );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// Avoid CS_HREDRAW and CS_VREDRAW styles
	cs.lpszClass = AfxRegisterWndClass( 0, NULL, reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1), NULL );
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CTrackPropertiesView* CMainFrame::GetTrackPropertiesView()
{
	CWnd* pWnd = m_wndSplitter.GetPane( 0, 0 );
	return DYNAMIC_DOWNCAST( CTrackPropertiesView, pWnd );
}

/////////////////////////////////////////////////////////////////////////////

CPropertyPagesView* CMainFrame::GetPropertyPagesView()
{
	CWnd* pWnd = m_wndSplitter.GetPane( 0, 1 );
	return DYNAMIC_DOWNCAST( CPropertyPagesView, pWnd );
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	CMenu* const pMenuBar = GetMenu();
	if (NULL == pMenuBar)
		return;

	enum
	{
		FILEMENU,
		DXIMENU,
		VIEWMENU,
		HELPMENU,
	};
	
	// Populate the DXi plugin menu
	CMenu* const pmDXi = pMenuBar->GetSubMenu( DXIMENU );
	if (NULL != pmDXi && pmDXi == pPopupMenu && !m_bFilledDXiMenu)
	{
		UINT idCmd = ID_DXI_MIN + 1;
		ULONG const cPlugIn = theApp.GetPlugInInventory().GetCount();
		for (ULONG ix = 0; ix < cPlugIn; ix++)
		{
			CLSID	clsid;
			char	szName[ 128 ];
			if (SUCCEEDED( theApp.GetPlugInInventory().GetInfo( ix, &clsid, szName, sizeof(szName) ) ))
			{
				pmDXi->AppendMenu( MF_ENABLED, idCmd, szName );
				idCmd++;
			}
		}
		m_bFilledDXiMenu = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateDXi( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( TRUE );
	pCmdUI->SetCheck( pCmdUI->m_nID == (m_ixDXi + ID_DXI_MIN) );
}

////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnDXi( UINT id )
{
	ASSERT( id >= ID_DXI_MIN && id <= ID_DXI_MAX );
	UINT const ixNew = id - ID_DXI_MIN;
	if (ixNew != m_ixDXi)
	{
		// Release old DXi
		GetPropertyPagesView()->Terminate();
		GetTrackPropertiesView()->SetFilter( NULL );
		theDXiPlayer.SetFilter( NULL );

		// Create new DXi if necessary
		if (ixNew > 0)
		{
			IBaseFilter* pFilter = NULL;
			if (FAILED( theApp.GetPlugInInventory().CreatePlugIn( ixNew - 1, &pFilter ) ))
				return;
			if (FAILED( theDXiPlayer.SetFilter( pFilter ) ))
				return;
			pFilter->Release(); // transport owns the DXi now.  We can safely free it.

			if (FAILED( GetPropertyPagesView()->Initialize( pFilter ) ))
			{
				GetPropertyPagesView()->Terminate();
				GetTrackPropertiesView()->SetFilter( NULL );
				theDXiPlayer.SetFilter( NULL );
				return;
			}

			GetTrackPropertiesView()->SetFilter( pFilter );
		}
		m_ixDXi = ixNew;
	}
}

////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnDestroy() 
{
	theDXiPlayer.Terminate();

	CFrameWnd::OnDestroy();
}

////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateOptions(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( !theDXiPlayer.IsPlaying() );
}

void CMainFrame::OnOptions() 
{
	COptionsDlg dlg;
	dlg.m_uBufferMsec = theDXiPlayer.GetBufferMsec();
	dlg.DoModal();
}
