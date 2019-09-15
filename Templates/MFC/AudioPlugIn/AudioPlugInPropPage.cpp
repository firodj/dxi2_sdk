// AudioPlugInPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "AudioPlugInPropPage.h"

#include <MedParam.h>
#include "CakeMedParam.h"
#include "Parameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioPlugInPropPage property page

CAudioPlugInPropPage::CAudioPlugInPropPage( IUnknown* pUnk, HRESULT* phr ) :
	COlePropertyPage( CAudioPlugInPropPage::IDD, IDS_NAME_PLUGIN ),
	CUnknown( "AudioPlugInPropPage", pUnk ),
	m_pMediaParams(NULL),
	m_pUICallback(NULL),
	m_bFirstQI(TRUE)
{	
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	//{{AFX_DATA_INIT(CAudioPlugInPropPage)
	//}}AFX_DATA_INIT

#ifdef _DEBUG
	// Turn off obnoxious "non-standard size" warnings from MFC.  At least
	// they gave us a flag to do this!
	m_bNonStandardSize = TRUE;
#endif
}

CAudioPlugInPropPage::~CAudioPlugInPropPage()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if (m_pMediaParams)
		m_pMediaParams->Release();
	m_pMediaParams = NULL;
	
	if (m_pUICallback)
		m_pUICallback->Release();
	m_pUICallback = NULL;
}

void CAudioPlugInPropPage::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	COlePropertyPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CAudioPlugInPropPage)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioPlugInPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CAudioPlugInPropPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////
// DO NOT CHANGE ANY OF THE FOLLOWING CODE!
////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CAudioPlugInPropPage::QueryInterface(REFIID riid, void **ppv)
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

ULONG CAudioPlugInPropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalAddRef();
};

ULONG CAudioPlugInPropPage::Release()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalRelease();
};

STDMETHODIMP CAudioPlugInPropPage::NonDelegatingQueryInterface(REFIID riid,void **ppv)
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

ULONG CAudioPlugInPropPage::NonDelegatingRelease()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalRelease();
}

ULONG CAudioPlugInPropPage::NonDelegatingAddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalAddRef();
}

////////////////////////////////////////////////////////////////////////////////
// CBasePropertyPage

CUnknown * WINAPI CAudioPlugInPropPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return new CAudioPlugInPropPage( lpunk, phr );
}

////////////////////////////////////////////////////////////////////////////////

void CAudioPlugInPropPage::OnObjectsChanged()
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

