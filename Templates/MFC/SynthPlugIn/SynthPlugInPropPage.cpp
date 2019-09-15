// SynthPlugInPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SynthPlugInPropPage.h"

#include <MedParam.h>
#include "CakeMedParam.h"
#include "Parameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSynthPlugInPropPage property page

CSynthPlugInPropPage::CSynthPlugInPropPage( IUnknown* pUnk, HRESULT* phr ) :
	COlePropertyPage( CSynthPlugInPropPage::IDD, IDS_NAME_PLUGIN ),
	CUnknown( "SynthPlugInPropPage", pUnk ),
	m_pMediaParams(NULL),
	m_pUICallback(NULL),
	m_bFirstQI(TRUE)
{	
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	//{{AFX_DATA_INIT(CSynthPlugInPropPage)
	//}}AFX_DATA_INIT

#ifdef _DEBUG
	// Turn off obnoxious "non-standard size" warnings from MFC.  At least
	// they gave us a flag to do this!
	m_bNonStandardSize = TRUE;
#endif
}

CSynthPlugInPropPage::~CSynthPlugInPropPage()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if (m_pMediaParams)
		m_pMediaParams->Release();
	m_pMediaParams = NULL;
	
	if (m_pUICallback)
		m_pUICallback->Release();
	m_pUICallback = NULL;
}

void CSynthPlugInPropPage::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	COlePropertyPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CSynthPlugInPropPage)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSynthPlugInPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CSynthPlugInPropPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////
// DO NOT CHANGE ANY OF THE FOLLOWING CODE!
////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CSynthPlugInPropPage::QueryInterface(REFIID riid, void **ppv)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// COlePropertyPage objects are constructed with a refcount = 1.
	// This conflicts with DirectShow CUnknown, which wants to contstruct
	// its objects with a refcount = 0.  To keep the 2 in sync, we'll
	// notice the first successful QI call, and decrement MFC's refcount
	// upon success.
	HRESULT hr = ExternalQueryInterface( &riid, ppv );
	if (S_OK == hr && m_bFirstQI)
	{
		m_bFirstQI = FALSE;
		ExternalRelease();
	}
	return hr;
};

ULONG CSynthPlugInPropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalAddRef();
};

ULONG CSynthPlugInPropPage::Release()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalRelease();
};

STDMETHODIMP CSynthPlugInPropPage::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// COlePropertyPage objects are constructed with a refcount = 1.
	// This conflicts with DirectShow CUnknown, which wants to contstruct
	// its objects with a refcount = 0.  To keep the 2 in sync, we'll
	// notice the first successful QI call, and decrement MFC's refcount
	// upon success.
	HRESULT hr = InternalQueryInterface( &riid, ppv );
	if (S_OK == hr && m_bFirstQI)
	{
		// Work around interaction w/ Cubase, which asks for the dialog's
		// size setting its site.
		OnSetPageSite();

		m_bFirstQI = FALSE;
		InternalRelease();
	}
	return hr;
}

ULONG CSynthPlugInPropPage::NonDelegatingRelease()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalRelease();
}

ULONG CSynthPlugInPropPage::NonDelegatingAddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalAddRef();
}

////////////////////////////////////////////////////////////////////////////////
// CBasePropertyPage

CUnknown * WINAPI CSynthPlugInPropPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return new CSynthPlugInPropPage( lpunk, phr );
}

////////////////////////////////////////////////////////////////////////////////

void CSynthPlugInPropPage::OnObjectsChanged()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Release old interfaces
	if (m_pMediaParams)
		m_pMediaParams->Release();
	m_pMediaParams = NULL;
	if (m_pUICallback)
		m_pUICallback->Release();
	m_pUICallback = NULL;

	// Look for a new IFilter
	ULONG cObj = 0;
	IDispatch** apIDisp = GetObjectArray( &cObj );
	for (ULONG i = 0; i < cObj; ++i)
	{
		if (S_OK == apIDisp[i]->QueryInterface( IID_IMediaParams, (void**)&m_pMediaParams ))
		{
			apIDisp[i]->QueryInterface( IID_IMediaParamsUICallback, (void**)&m_pUICallback );
			break;
		}
	}

	// Update controls if we've got a new object and we're activated
	if (m_pMediaParams && ::IsWindow( m_hWnd ))
		UpdateData( FALSE );
}

////////////////////////////////////////////////////////////////////////////////

