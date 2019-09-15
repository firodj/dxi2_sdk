// MFCSamplePropPage.cpp : implementation file
//

#include "stdafx.h"
#include <MidiFilter.h>
#include "MFCSample.h"
#include "MFCSamplePropPage.h"

/////////////////////////////////////////////////////////////////////////////
// CMFCSamplePropPage dialog


CMFCSamplePropPage::CMFCSamplePropPage(CWnd* pParent /*=NULL*/)
: CDialog(CMFCSamplePropPage::IDD, pParent),
	m_cRef( 0 ),
	m_pFilter( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
{
	//{{AFX_DATA_INIT(CMFCSamplePropPage)
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

void CMFCSamplePropPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCSamplePropPage)
	DDX_Control(pDX, IDC_REPS_SLIDER, m_ctlRepsSlider);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMFCSamplePropPage, CDialog)
	//{{AFX_MSG_MAP(CMFCSamplePropPage)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CMFCSamplePropPage::~CMFCSamplePropPage()
{
	if (m_pFilter)
		m_pFilter->Release();
	m_pFilter = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CMFCSamplePropPage::QueryInterface( REFIID riid, void** ppv )
{    
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

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

ULONG CMFCSamplePropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMFCSamplePropPage::Release() 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage

HRESULT CMFCSamplePropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "Sample MFC MIDI Filter";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );

	// Populate the page info structure
	pPageInfo->cb					= sizeof(PROPPAGEINFO);
	pPageInfo->size.cx      = 100;
	pPageInfo->size.cy      = 100;
	pPageInfo->pszDocString = NULL;
	pPageInfo->pszHelpFile  = NULL;
	pPageInfo->dwHelpContext= 0;

	// Create the property page in order to determine its size
	HWND const hWnd = CreateDialogParam( theApp.m_hInstance, MAKEINTRESOURCE( IDD_PROPPAGE ), 
													 ::GetDesktopWindow(), DialogProc, 0 );
	if (hWnd)
	{
		// Get the dialog size and destroy the window
		RECT rc;
		::GetWindowRect( hWnd, &rc );
		pPageInfo->size.cx = rc.right - rc.left;
		pPageInfo->size.cy = rc.bottom - rc.top;
		::DestroyWindow( hWnd );
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK CMFCSamplePropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// This proc is only a dummy. It's necessary to to get the size of the dialog
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMFCSamplePropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
	// Release existing object
	if (m_pFilter)
		m_pFilter->Release();
	m_pFilter = NULL;

	if (cObjects == 1)
	{
		// Validate arguments
		if (ppUnk == NULL || *ppUnk == NULL)
			return E_POINTER;

		// Cast this to be our type
		CMFCMidiFilter* const pFilter = reinterpret_cast<CMFCMidiFilter*>(*ppUnk);
		
		// Store it
		m_pFilter = pFilter;
		m_pFilter->AddRef();

		return S_OK;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CMFCSamplePropPage::CreateMFCDialog( HWND hwndParent )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return Create( CMFCSamplePropPage::IDD, CWnd::FromHandle( hwndParent ) );
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMFCSamplePropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
{
	if (!pRect)
		return E_POINTER;
	if (NULL != m_hWnd)
		return E_UNEXPECTED;	// already active!

	if (!CreateMFCDialog( hwndParent ))
		return E_OUTOFMEMORY;

	// Parent should control us so the user can tab out of property page
	DWORD dwStyle = GetWindowLong( m_hWnd, GWL_EXSTYLE );
	dwStyle = dwStyle | WS_EX_CONTROLPARENT;
	SetWindowLong( m_hWnd, GWL_EXSTYLE, dwStyle );

	// Move page into position and show it.
	Move( pRect );
	Show( SW_SHOWNORMAL );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMFCSamplePropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMFCSamplePropPage::Show( UINT nCmdShow )
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;
	// Ignore wrong show flags
	if (nCmdShow != SW_SHOW && nCmdShow != SW_SHOWNORMAL && nCmdShow != SW_HIDE)
		return E_INVALIDARG;

	ShowWindow( nCmdShow );

	// Take the focus.
	if (SW_SHOWNORMAL == nCmdShow || SW_SHOW == nCmdShow)
		m_ctlRepsSlider.SetFocus();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMFCSamplePropPage::DestroyMFCDialog()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMFCSamplePropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	DestroyMFCDialog();
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMFCSamplePropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
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

HRESULT CMFCSamplePropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the filter. I.e. there is no Cancel.
	m_bDirty = FALSE;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMFCSamplePropPage::IsPageDirty()
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMFCSamplePropPage::Help( LPCWSTR lpszHelpDir )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Get location of DLL
	char szDLL[ _MAX_PATH ];
	if (0 == ::GetModuleFileName( AfxGetInstanceHandle(), szDLL, sizeof szDLL ))
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

HRESULT CMFCSamplePropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMFCSamplePropPage::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Refresh the property page controls from the m_pFilter property values
	if (m_pFilter)
	{
		m_ctlRepsSlider.SetRange( CMFCMidiFilter::GetMinReps(), CMFCMidiFilter::GetMaxReps() );
		m_ctlRepsSlider.SetPos( m_pFilter->GetReps()  );
		SetDlgItemInt( IDC_REPS_STATIC, m_pFilter->GetReps(), TRUE );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////

void CMFCSamplePropPage::OnHScroll( UINT nCode, UINT nPos, CScrollBar* pScrollBar ) 
{
	if (!m_pFilter)
		return;

	// Get current repetitions property from our filter
	UINT nReps = m_pFilter->GetReps();

	// Note: Trackbars don't send SB_LEFT/SB_RIGHT
	switch( nCode )
	{
		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			if (nReps < CMFCMidiFilter::GetMaxReps())
				++nReps;
			else
				return;
			break;

		case SB_LINELEFT:
		case SB_PAGELEFT:
			if (CMFCMidiFilter::GetMinReps() < nReps)
				--nReps;
			else
				return;
			break;

		case SB_THUMBTRACK:
			if (CMFCMidiFilter::GetMinReps() <= nPos && nPos <= CMFCMidiFilter::GetMaxReps())
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
	SetDlgItemInt( IDC_REPS_STATIC, nReps, TRUE );
}

/////////////////////////////////////////////////////////////////////////////
