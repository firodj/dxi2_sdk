// Copyright (C) 1998- by Twelve Tone Systems, Inc..  All Rights Reserved.

#include "stdafx.h"
#include "Main.h"
#include "Resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CPropPage
//
/////////////////////////////////////////////////////////////////////////////

CPropPage::CPropPage() :
	m_cRef( 0 ),
	m_hDlg( NULL ),
	m_pFilter( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
{
}

CPropPage::~CPropPage()
{
	if (m_pFilter)
		m_pFilter->Release();
	m_pFilter = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CPropPage::QueryInterface( REFIID riid, void** ppv )
{    
	if (IID_IUnknown == riid)
		*ppv = static_cast<IPropertyPage*>(this);
	else if (IID_IPropertyPage == riid)
		*ppv = static_cast<IPropertyPage*>(this);
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

ULONG CPropPage::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CPropPage::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage

HRESULT CPropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "Sample MIDI Filter";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );

	pPageInfo->size.cx      = 100;
	pPageInfo->size.cy      = 100;
	pPageInfo->pszDocString = NULL;
	pPageInfo->pszHelpFile  = NULL;
	pPageInfo->dwHelpContext= 0;

	// Create the property page in order to determine its size
	HWND const hWnd = ::CreateDialogParam( g_hModule, MAKEINTRESOURCE( IDD_PROPPAGE ), GetDesktopWindow(), (DLGPROC)StaticDialogProc, 0 );
	if (hWnd)
	{
		// Get the dialog size and destroy the window
		RECT rc;
		GetWindowRect( hWnd, &rc );
		pPageInfo->size.cx = rc.right - rc.left;
		pPageInfo->size.cy = rc.bottom - rc.top;
		DestroyWindow( hWnd );
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
	// Free existing object
	if (m_pFilter)
		m_pFilter->Release();
	m_pFilter = NULL;

	if (cObjects == 1)
	{
		// Validate arguments
		if (ppUnk == NULL || *ppUnk == NULL)
			return E_POINTER;

		// Cast this to be our type
		CMidiFilter* const pFilter = reinterpret_cast<CMidiFilter*>(*ppUnk);
		
		// Store it
		m_pFilter = pFilter;
		m_pFilter->AddRef();

		// Refresh the property page controls from the m_pFilter property values
		::SetDlgItemInt( m_hDlg, IDC_REPS_STATIC, m_pFilter->GetReps(), TRUE );

		::SendMessage( GetDlgItem( m_hDlg, IDC_REPS_SLIDER ), TBM_SETRANGE, FALSE, MAKELONG( CMidiFilter::GetMinReps(), CMidiFilter::GetMaxReps() ) );
		::SendMessage( GetDlgItem( m_hDlg, IDC_REPS_SLIDER ), TBM_SETPOS, TRUE, m_pFilter->GetReps() );
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK CPropPage::StaticDialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CPropPage* pPage;

	if (WM_INITDIALOG == uMsg)
	{
		SetWindowLong( hwnd, DWL_USER, lParam );
		pPage = reinterpret_cast<CPropPage*>(lParam);
		if (!pPage)
			return TRUE;
	}

	pPage = reinterpret_cast<CPropPage*>(GetWindowLong( hwnd, DWL_USER ));
	if (!pPage)
		return TRUE;

	return pPage->DialogProc( hwnd, uMsg, wParam, lParam );
}

////////////////////////////////////////////////////////////////////////////////

BOOL CPropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:
			m_hDlg = hwnd;
			break;

		case WM_HSCROLL:
			// Horizontal trackbar sends WM_HSCROLL messages
			onScroll( LOWORD( wParam ), HIWORD( wParam ) );
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CPropPage::onScroll( int nCode, int nPos )
{
	// Get current repetitions property from our filter
	int nReps = m_pFilter->GetReps();

	// Note: Trackbars don't send SB_LEFT/SB_RIGHT
	switch( nCode )
	{
		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			if (nReps < CMidiFilter::GetMaxReps())
				++nReps;
			else
				return;
			break;

		case SB_LINELEFT:
		case SB_PAGELEFT:
			if (CMidiFilter::GetMinReps() < nReps)
				--nReps;
			else
				return;
			break;

		case SB_THUMBTRACK:
			if (CMidiFilter::GetMinReps() <= nPos && nPos <= CMidiFilter::GetMaxReps())
				nReps = nPos;
			else
				return;
			break;

		default:
			return;
	}

	// Set the new repetitions property
	m_pFilter->SetReps( nReps );

	// Mark the filter as dirty (not saved)
	m_pFilter->SetDirty();

	// Mark our own page as dirty
	m_bDirty = TRUE;

	// Let our IPropertyPageSite know that a property has changed
	m_pPageSite->OnStatusChange( PROPPAGESTATUS_DIRTY );

	// Refresh the trackbar control position
	::SetDlgItemInt( m_hDlg, IDC_REPS_STATIC, nReps, TRUE );
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
{
	if (!pRect)
		return E_POINTER;
	if (NULL != m_hDlg)
		return E_UNEXPECTED;	// already active!

	m_hDlg = CreateDialogParam( g_hModule, MAKEINTRESOURCE( IDD_PROPPAGE ), hwndParent, (DLGPROC)StaticDialogProc, (LPARAM)this );
	if (!m_hDlg)
		return E_OUTOFMEMORY;

	// Refresh the property page controls from the m_pFilter property values
	if (m_pFilter)
	{
		::SetDlgItemInt( m_hDlg, IDC_REPS_STATIC, m_pFilter->GetReps(), TRUE );

		::SendMessage( GetDlgItem( m_hDlg, IDC_REPS_SLIDER ), TBM_SETRANGE, FALSE, MAKELONG( CMidiFilter::GetMinReps(), CMidiFilter::GetMaxReps() ) );
		::SendMessage( GetDlgItem( m_hDlg, IDC_REPS_SLIDER ), TBM_SETPOS, TRUE, m_pFilter->GetReps() );
	}

	// Move page into position and show it.
	Move( pRect );
	Show( SW_SHOWNORMAL );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hDlg)
		E_UNEXPECTED;

	MoveWindow( m_hDlg, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::Show( UINT nCmdShow )
{
	if (NULL == m_hDlg)
		E_UNEXPECTED;
	// Ignore wrong show flags
	if (nCmdShow != SW_SHOW && nCmdShow != SW_SHOWNORMAL && nCmdShow != SW_HIDE)
		return E_INVALIDARG;

	ShowWindow( m_hDlg, nCmdShow );

	// Take the focus.
	if (SW_SHOWNORMAL == nCmdShow || SW_SHOW == nCmdShow)
#if 1
		SetFocus( GetDlgItem( m_hDlg, IDC_REPS_SLIDER ) );
#else
		SetFocus( m_hDlg );
#endif

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::Deactivate()
{
	if (NULL == m_hDlg)
		return E_UNEXPECTED;

	DestroyWindow( m_hDlg );
	m_hDlg = NULL;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
{
	if (pPageSite)
	{
		if (m_pPageSite)
			return E_UNEXPECTED;
		m_pPageSite = pPageSite;
		m_pPageSite->AddRef();
	}
	else
	{
		if (m_pPageSite == NULL)
			return E_UNEXPECTED;
		m_pPageSite->Release();
		m_pPageSite = NULL;
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CPropPage::OnApplyChanges()
{
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the filter. I.e. there is no Cancel.
	m_bDirty = FALSE;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::IsPageDirty( void )
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::Help( LPCWSTR lpszHelpDir )
{
	// Get location of DLL
	char szDLL[ _MAX_PATH ];
	if (0 == ::GetModuleFileName( g_hModule, szDLL, sizeof szDLL ))
		return E_FAIL;
	
	// Convert to location of .HLP file
	char szHelp[ _MAX_PATH ];
	::strncpy( szHelp, szDLL, ::strlen( szDLL ) - 3 );
	::strcat( szHelp, "HLP" );

	// Call help 
	if (::WinHelp( m_hDlg, szHelp, HELP_CONTENTS, NULL ))
		return S_OK;

	return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
