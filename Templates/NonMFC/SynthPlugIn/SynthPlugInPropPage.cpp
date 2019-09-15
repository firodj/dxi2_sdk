// SynthPlugInPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SynthPlugInPropPage.h"

#include <MedParam.h>
#include "CakeMedParam.h"
#include "Parameters.h"

/////////////////////////////////////////////////////////////////////////////
// CSynthPlugInPropPage property page

extern HMODULE g_hInst;

CSynthPlugInPropPage::CSynthPlugInPropPage( IUnknown* pUnk, HRESULT* phr ) :
	CUnknown( "SynthPlugInPropPage", pUnk ),
	m_hWnd( NULL ),
	m_pMediaParams( NULL ),
	m_pUICallback( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
{
}

/////////////////////////////////////////////////////////////////////////////

CUnknown * WINAPI CSynthPlugInPropPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	return new CSynthPlugInPropPage( lpunk, phr );
}

/////////////////////////////////////////////////////////////////////////////

CSynthPlugInPropPage::~CSynthPlugInPropPage()
{
	if (m_pMediaParams)
		m_pMediaParams->Release();
	m_pMediaParams = NULL;
	
	if (m_pUICallback)
		m_pUICallback->Release();
	m_pUICallback = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CUnknown

HRESULT CSynthPlugInPropPage::NonDelegatingQueryInterface( REFIID riid, void** ppv )
{    
	if (IID_IUnknown == riid)
		return GetInterface( (IUnknown*)this, ppv );
	else if (IID_IPropertyPage == riid)
		return GetInterface( (IPropertyPage*)this, ppv );
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage

HRESULT CSynthPlugInPropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "SynthPlugIn";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );

	pPageInfo->size.cx      = 100;
	pPageInfo->size.cy      = 100;
	pPageInfo->pszDocString = NULL;
	pPageInfo->pszHelpFile  = NULL;
	pPageInfo->dwHelpContext= 0;

	// Create the property page in order to determine its size
	HWND const hWnd = ::CreateDialogParam( g_hInst, MAKEINTRESOURCE( IDD_PROPPAGE ), GetDesktopWindow(), (DLGPROC)StaticDialogProc, 0 );
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

HRESULT CSynthPlugInPropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
	// Release old interfaces
	if (m_pMediaParams)
		m_pMediaParams->Release();
	m_pMediaParams = NULL;
	if (m_pUICallback)
		m_pUICallback->Release();
	m_pUICallback = NULL;

	// Look for a new IFilter
	ULONG cObj = 0;
	for (ULONG i = 0; i < cObjects; ++i)
	{
		if (S_OK == ppUnk[i]->QueryInterface( IID_IMediaParams, (void**)&m_pMediaParams ))
		{
			ppUnk[i]->QueryInterface( IID_IMediaParamsUICallback, (void**)&m_pUICallback );
			break;
		}
	}

	// Update controls if we've got a new object and we're activated
	if (m_pMediaParams && ::IsWindow( m_hWnd ))
		UpdateControls();

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK CSynthPlugInPropPage::StaticDialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CSynthPlugInPropPage* pPage;

	if (WM_INITDIALOG == uMsg)
	{
		SetWindowLong( hwnd, DWL_USER, lParam );
		pPage = reinterpret_cast<CSynthPlugInPropPage*>(lParam);
		if (!pPage)
			return TRUE;
	}

	pPage = reinterpret_cast<CSynthPlugInPropPage*>(GetWindowLong( hwnd, DWL_USER ));
	if (!pPage)
		return TRUE;

	return pPage->DialogProc( hwnd, uMsg, wParam, lParam );
}

////////////////////////////////////////////////////////////////////////////////

BOOL CSynthPlugInPropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:
			m_hWnd = hwnd;
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void CSynthPlugInPropPage::UpdateControls()
{
	// TODO: update all UI elements to reflect new control state
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CSynthPlugInPropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
{
	if (!pRect)
		return E_POINTER;
	if (NULL != m_hWnd)
		return E_UNEXPECTED;	// already active!

	m_hWnd = CreateDialogParam( g_hInst, MAKEINTRESOURCE( IDD_PROPPAGE ), hwndParent, (DLGPROC)StaticDialogProc, (LPARAM)this );
	if (!m_hWnd)
		return E_OUTOFMEMORY;

	// Refresh the property page controls
	UpdateControls();

	// Move page into position and show it
	Move( pRect );
	Show( SW_SHOWNORMAL );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CSynthPlugInPropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( m_hWnd, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CSynthPlugInPropPage::Show( UINT nCmdShow )
{
	if (NULL == m_hWnd)
		E_UNEXPECTED;
	// Ignore wrong show flags
	if (nCmdShow != SW_SHOW && nCmdShow != SW_SHOWNORMAL && nCmdShow != SW_HIDE)
		return E_INVALIDARG;

	ShowWindow( m_hWnd, nCmdShow );

	if (SW_SHOWNORMAL == nCmdShow || SW_SHOW == nCmdShow)
	{
		// TODO: set the focus to which control needs it
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CSynthPlugInPropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	DestroyWindow( m_hWnd );
	m_hWnd = NULL;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CSynthPlugInPropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
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

HRESULT CSynthPlugInPropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the filter. I.e. there is no Cancel.
	m_bDirty = FALSE;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CSynthPlugInPropPage::IsPageDirty( void )
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSynthPlugInPropPage::Help( LPCWSTR lpszHelpDir )
{
	// Get location of DLL
	char szDLL[ _MAX_PATH ];
	if (0 == ::GetModuleFileName( g_hInst, szDLL, sizeof szDLL ))
		return E_FAIL;
	
	// Convert to location of .HLP file
	char szHelp[ _MAX_PATH ];
	::strncpy( szHelp, szDLL, ::strlen( szDLL ) - 3 );
	::strcat( szHelp, "HLP" );

	// Call help 
	if (::WinHelp( m_hWnd, szHelp, HELP_CONTENTS, NULL ))
		return S_OK;

	return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSynthPlugInPropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
