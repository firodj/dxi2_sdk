// PropertyPagesView.cpp - Implementation of an MFC CView class to host
// OLE property pages.
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlugInHost.h"
#include "PropertyPagesView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPropertyPagesView, CView)

/////////////////////////////////////////////////////////////////////////////

int CPropertyPagesView::GetIdealWidth()
{
	return 600;
}

/////////////////////////////////////////////////////////////////////////////

CPropertyPagesView::CPropertyPagesView() :
	m_bInitialized( FALSE ),
	m_dwRefCount( 0 ),
	m_aPages( NULL ),
	m_ixPage( 0 ),
	m_pTabCtrl( NULL )
{
	::memset( &m_caGUID, 0, sizeof m_caGUID );
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropertyPagesView::Initialize( IUnknown* pUnknown )
{
	ASSERT( !m_bInitialized );
	if (m_bInitialized)
		return E_FAIL;
	ASSERT( NULL == m_aPages );
	ASSERT( 0 == m_caGUID.cElems );

	HRESULT hr = S_OK;

	ASSERT_POINTER( pUnknown, IUnknown );

	// See if they've got property pages
	ISpecifyPropertyPages* pISPP;
	if (FAILED( hr = pUnknown->QueryInterface( IID_ISpecifyPropertyPages, (void**)&pISPP ) ))
		return hr;

	// Get the array of property page GUIDs
	hr = pISPP->GetPages( &m_caGUID );
	pISPP->Release();
	if (FAILED( hr ))
		return hr;
	if (m_caGUID.cElems <= 0)
		return E_FAIL;

	// Create each property page
	m_aPages = new PropPage[ m_caGUID.cElems ];
	if (NULL == m_aPages)
		return E_OUTOFMEMORY;
	::memset( m_aPages, 0, m_caGUID.cElems * sizeof(PropPage) );
	for (UINT ix = 0; ix < m_caGUID.cElems; ++ix)
	{
		PropPage& pp = m_aPages[ ix ];	// handy alias
		if (FAILED( hr = CoCreateInstance( m_caGUID.pElems[ ix ], NULL, CLSCTX_INPROC_SERVER, IID_IPropertyPage, (void**)&pp.pPropPage ) ))
			break;
		if (FAILED( hr = pp.pPropPage->SetPageSite( this ) ))
			break;
		if (FAILED( hr = pp.pPropPage->SetObjects( 1, &pUnknown ) ))
			break;
		pp.ppi.cb = sizeof PROPPAGEINFO;
		if (FAILED( hr = pp.pPropPage->GetPageInfo( &pp.ppi ) ))
			break;
	}

	// Cleanup upon failure to create property pages
	if (FAILED( hr ))
	{
		delete[] m_aPages;
		m_aPages = NULL;
		CoTaskMemFree( m_caGUID.pElems );
		m_caGUID.cElems = 0;
		m_caGUID.pElems = NULL;
		return hr;
	}

	CRect rcClient(0, 0, 0, 0);	// computed total client area

	// Create the tab control
	m_rcTabCtrl.SetRect( 0, 0, 0, 0 );
	m_pTabCtrl = new CTabCtrl;
	if (NULL == m_pTabCtrl)
		return E_OUTOFMEMORY;
	m_pTabCtrl->Create( WS_CHILD | TCS_TABS | WS_VISIBLE | WS_GROUP | WS_TABSTOP, m_rcTabCtrl, this, IDW_TAB );
	m_pTabCtrl->SetFont( &theApp.GetFont() );

	int xLabel = 0;
	CString labelStr;

	// Add a tab for each property page
	for (ix = 0; ix < m_caGUID.cElems; ++ix)
	{
		PropPage& pp = m_aPages[ ix ];	// handy alias

		// Keep track of the maximum size
		m_rcTabCtrl.right = max( m_rcTabCtrl.right, pp.ppi.size.cx );
		m_rcTabCtrl.bottom = max( m_rcTabCtrl.bottom, pp.ppi.size.cy );

		// If we've got more than one tab, set the title
		if (1 < m_caGUID.cElems)
		{
			// Convert OLE text to ANSI
			char sz[ 256 ];
			if (0 != WideCharToMultiByte( CP_ACP, 0, pp.ppi.pszTitle, -1, sz, sizeof sz, NULL, NULL ))
			{
				// Set the tab title
				TC_ITEM ti;
				memset( &ti, 0, sizeof ti );
				ti.mask = TCIF_TEXT | TCIF_IMAGE;
				ti.iImage = -1;
				ti.pszText = sz;
				m_pTabCtrl->InsertItem(ix, &ti );
			}
		}
	}

	// Map the inner contents rectangle of the tab control to an outer window
	// rectangle.  This becomes our initial client rectangle.
	rcClient = m_rcTabCtrl;
	m_pTabCtrl->AdjustRect( TRUE, rcClient );
	rcClient.OffsetRect( -rcClient.left, -rcClient.top );
	
	// Compute the client coordinates of the contents rectangle of the tab control
	m_rcTabCtrl = rcClient;
	m_pTabCtrl->AdjustRect( FALSE, m_rcTabCtrl );

	// Set the tab control's final position
	{
		CRect rcTab( m_rcTabCtrl );
		m_pTabCtrl->AdjustRect( TRUE, rcTab );
		rcTab.OffsetRect( -rcTab.left, -rcTab.top );
		m_pTabCtrl->SetWindowPos( &wndTop, rcTab.left, rcTab.top,
										rcTab.Width(), rcTab.Height(), SWP_NOZORDER );
	}

	// Activate the FIRST page in property pages
	m_ixPage = 0;
	m_aPages[ m_ixPage ].pPropPage->Activate( m_hWnd, &m_rcTabCtrl, /* bModal = */ FALSE );

	// Show the active property page
	m_aPages[ m_ixPage ].pPropPage->Show( SW_SHOW );

	// Force a repaint
	Invalidate( FALSE );

	m_bInitialized = TRUE;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

CPropertyPagesView::~CPropertyPagesView()
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropertyPagesView::Terminate()
{
	if (m_aPages)
	{
		// Deactivate the current page
		IPropertyPage*	const pPage = m_aPages[ m_ixPage ].pPropPage;
		if (pPage)
		{
			HRESULT hr = S_OK;
			hr = pPage->Deactivate();
			ASSERT( SUCCEEDED( hr ) );
		}

		// Free all pages
		for (UINT ix = 0; ix < m_caGUID.cElems; ++ix)
		{
			PropPage& pp = m_aPages[ ix ];	// handy alias
			if (pp.pPropPage)
				pp.pPropPage->Release();
			::CoTaskMemFree( pp.ppi.pszTitle );
			::CoTaskMemFree( pp.ppi.pszDocString );
			::CoTaskMemFree( pp.ppi.pszHelpFile );
		}

		SAFE_ARRAY_DELETE( m_aPages );
	}

	if (m_caGUID.pElems)
	{
		::CoTaskMemFree( m_caGUID.pElems );
		m_caGUID.pElems = NULL;
	}

	m_caGUID.cElems = 0;

	SAFE_DELETE( m_pTabCtrl );

	m_bInitialized = FALSE;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CPropertyPagesView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// [1] Use NULL for window class cursor. Prevents arrow cursor flickering in
	// before our OnMouseMove() handler can set the correct one.

	// [2] Use COLOR_BTNFACE as our background color.  This gives us a modeless
	// dialog box feel.

	cs.lpszClass = AfxRegisterWndClass( 0, NULL, reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1), NULL );
	if (CView::PreCreateWindow( cs ))
	{
		cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CPropertyPagesView::PreTranslateMessage(MSG* pMsg) 
{
	// If not any CONTROL+TAB keystroke -- which are MDI next/prev window
	// shortcuts -- give this to IsDialogMessage(). Enables TAB and SHIFT+TAB
	// and other dialog box navigation.  Note: We could call TranslateMDIAccelerator()
	// here, but that's too broad. Other than tab keystrokes, we do let users
	// override MDI shortcuts with their own key bindings.
   if ((pMsg->message != WM_KEYDOWN) || 
       (pMsg->wParam != VK_TAB && pMsg->wParam != VK_F4 ) ||
       (0 == (GetKeyState( VK_CONTROL ) & 0x8000)))
	{

		if (IsDialogMessage( pMsg ))
			return TRUE;
	}

	return CView::PreTranslateMessage( pMsg );
}

////////////////////////////////////////////////////////////////////////////////

void CPropertyPagesView::OnDraw( CDC* pDC )
{
	// No-op
}

/////////////////////////////////////////////////////////////////////////////
// IUnknown
/////////////////////////////////////////////////////////////////////////////

HRESULT CPropertyPagesView::QueryInterface( REFIID riid, LPVOID* ppvObj )
{
	if (IsEqualIID( riid, IID_IUnknown ))
		*ppvObj = this;
	else if (IsEqualIID( riid, IID_IPropertyPageSite ))
		*ppvObj = this;
	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

ULONG CPropertyPagesView::AddRef()
{
	return ++m_dwRefCount;
}
/////////////////////////////////////////////////////////////////////////////

ULONG CPropertyPagesView::Release()
{
	ASSERT( 0 < m_dwRefCount );
	return --m_dwRefCount;
}
 
/////////////////////////////////////////////////////////////////////////////
// IPropertyPageSite
/////////////////////////////////////////////////////////////////////////////

HRESULT CPropertyPagesView::OnStatusChange( DWORD dwFlags )
{
	AFX_MANAGE_STATE( AfxGetAppModuleState() );

	if (PROPPAGESTATUS_DIRTY & dwFlags)
	{
		// We would like to do what OnSetModified() does, directly.
		// However, the caller of this function may be another MFC module.
		// In this case, MFC's current "thread state" points to the plug-in's
		// state, not our state. So when SetModifiedFlag() attempts to set the
		// title of the frame window, MFC's HWND-to-CWnd* map will fail to
		// cough up the correct pointers (since each map is kept per-thread),
		// and we crash.  To avoid this mess, we simply post a message to
		// ourselves, ensuring the proper thread state when we are ready to
		// modify the frame window.
		PostMessage( WM_SETMODIFIED, 0, 0 );
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropertyPagesView::GetLocaleID( LCID* pLocaleID )
{
	AFX_MANAGE_STATE( AfxGetAppModuleState() );

	if (!pLocaleID)
		return E_INVALIDARG;
	*pLocaleID = GetUserDefaultLCID();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropertyPagesView::TranslateAccelerator( LPMSG lpMsg )
{
	AFX_MANAGE_STATE( AfxGetAppModuleState() );

#if 0	// This never seems to be called, at least not by our AM filters
	// CTRL+TAB and CTRL+SHIFT+TAB should do the usual next/prev MDI child
	// thing. Plain TAB should go through to the property page.
	if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_TAB && (0x8000 & GetKeyState( VK_CONTROL )))
	{
		FORWARD_WM_MDINEXT( hwndMDIClient, hwndActive, (0x8000 & GetKeyState( VK_SHIFT )), PostMessage );
		return S_OK;
	}
#endif
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyPagesView message handlers

BEGIN_MESSAGE_MAP(CPropertyPagesView, CView)
	//{{AFX_MSG_MAP(CPropertyPagesView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(TCN_SELCHANGE, IDW_TAB, OnSelChangePluginTab)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETMODIFIED, OnSetModified)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

int CPropertyPagesView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CPropertyPagesView::OnDestroy()
{
	Terminate();
	CView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CPropertyPagesView::OnSetModified( WPARAM, LPARAM )
{
	// Set the document's dirty flag
	GetDocument()->SetModifiedFlag();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CPropertyPagesView::OnSelChangePluginTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Get the tab which was selected
	int const nSel = m_pTabCtrl->GetCurSel();

	// If we're not already there
	if (nSel != m_ixPage)
	{
		if (0 <= nSel && nSel < m_pTabCtrl->GetItemCount())
		{
			// Switch to another tab only if Deactivate() succeeds
			if (S_OK == m_aPages[ m_ixPage ].pPropPage->Deactivate())
			{
				m_ixPage = nSel;
				m_aPages[ m_ixPage ].pPropPage->Activate( this->m_hWnd, &m_rcTabCtrl, /* bModal = */ FALSE );
			}
			// Validation failed, so stay put in this tab
			else
				m_pTabCtrl->SetCurSel( m_ixPage );
		}
	}
	
	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////

void CPropertyPagesView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	if (m_aPages && m_aPages[ m_ixPage ].pPropPage)
	{
		m_aPages[ m_ixPage ].pPropPage->Show( SW_SHOW );
	}
}

/////////////////////////////////////////////////////////////////////////////

void CPropertyPagesView::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CView::OnShowWindow(bShow, nStatus);
	if (m_aPages && m_aPages[ m_ixPage ].pPropPage)
	{
		m_aPages[ m_ixPage ].pPropPage->Show( bShow ? SW_SHOW : SW_HIDE );
	}
}
	
/////////////////////////////////////////////////////////////////////////////
