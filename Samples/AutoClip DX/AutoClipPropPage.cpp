// AutoClipPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "AutoClipPropPage.h"

#include <MedParam.h>
#include "CakeMedParam.h"
#include "Parameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoClipPropPage property page

CAutoClipPropPage::CAutoClipPropPage( IUnknown* pUnk, HRESULT* phr ) :
	COlePropertyPage( CAutoClipPropPage::IDD, IDS_NAME_PLUGIN ),
	CUnknown( "AutoClipPropPage", pUnk ),
	m_sliderThreshold(this),
	m_sliderOutputGain(this),
	m_pMediaParams(NULL),
	m_pUICallback(NULL),
	m_bFirstQI(TRUE)
{	
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	//{{AFX_DATA_INIT(CAutoClipPropPage)
	//}}AFX_DATA_INIT

#ifdef _DEBUG
	// Turn off obnoxious "non-standard size" warnings from MFC.  At least
	// they gave us a flag to do this!
	m_bNonStandardSize = TRUE;
#endif
}

CAutoClipPropPage::~CAutoClipPropPage()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if (m_pMediaParams)
		m_pMediaParams->Release();
	m_pMediaParams = NULL;
	
	if (m_pUICallback)
		m_pUICallback->Release();
	m_pUICallback = NULL;
}

void CAutoClipPropPage::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	COlePropertyPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CAutoClipPropPage)
	DDX_Control(pDX, IDC_THRESHOLD, m_sliderThreshold);
	DDX_Control(pDX, IDC_OUTPUTGAIN, m_sliderOutputGain);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutoClipPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CAutoClipPropPage)
	ON_BN_CLICKED(IDC_ENABLED, OnEnabled)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////
// DO NOT CHANGE ANY OF THE FOLLOWING CODE!
////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CAutoClipPropPage::QueryInterface(REFIID riid, void **ppv)
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

ULONG CAutoClipPropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalAddRef();
};

ULONG CAutoClipPropPage::Release()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalRelease();
};

STDMETHODIMP CAutoClipPropPage::NonDelegatingQueryInterface(REFIID riid,void **ppv)
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

ULONG CAutoClipPropPage::NonDelegatingRelease()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalRelease();
}

ULONG CAutoClipPropPage::NonDelegatingAddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalAddRef();
}

////////////////////////////////////////////////////////////////////////////////
// CBasePropertyPage

CUnknown * WINAPI CAutoClipPropPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return new CAutoClipPropPage( lpunk, phr );
}

////////////////////////////////////////////////////////////////////////////////

void CAutoClipPropPage::OnObjectsChanged()
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

void CAutoClipPropPage::OnEnabled() 
{
	SetModifiedFlag( TRUE );

	float fValue = IsDlgButtonChecked( IDC_ENABLED );
	if (m_pUICallback)
	{
		DWORD dwIndex = PARAM_ENABLE;
		m_pUICallback->ParamsBeginCapture( &dwIndex, 1 );
		m_pUICallback->ParamsChanged( &dwIndex, 1, &fValue );
		m_pUICallback->ParamsEndCapture( &dwIndex, 1 );
	}
}

BOOL CAutoClipPropPage::OnInitDialog() 
{
	COlePropertyPage::OnInitDialog();
	
	// Set ranges of slider controls to match parameters
	m_sliderThreshold.SetRange( 0, 100, TRUE );
	m_sliderOutputGain.SetRange( 0, 200, TRUE );

	// Set up a timer to periodically update every 50 msec
	SetTimer( 0, 50, NULL );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAutoClipPropPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModifiedFlag( TRUE );

	if (m_pUICallback)
	{
		float fVal = 0;
		DWORD dwIndex = 0;
		if (IDC_THRESHOLD == pScrollBar->GetDlgCtrlID())
		{
			fVal = static_cast<float>( m_sliderThreshold.GetPos() );
			dwIndex = PARAM_THRESHOLD;
		}
		else if (IDC_OUTPUTGAIN == pScrollBar->GetDlgCtrlID())
		{
			fVal = static_cast<float>( m_sliderOutputGain.GetPos() );
			dwIndex = PARAM_OUTPUTGAIN;
		}
		else
		{
			ASSERT(FALSE);
			return;
		}
		m_pUICallback->ParamsChanged( &dwIndex, 1, &fVal );
	}
	
	COlePropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CAutoClipPropPage::OnDestroy() 
{
	COlePropertyPage::OnDestroy();
	
	KillTimer( 0 );
}

void CAutoClipPropPage::OnTimer(UINT nIDEvent) 
{
	if (m_pMediaParams)
	{
		float fVal;
		
		m_pMediaParams->GetParam( PARAM_THRESHOLD, &fVal );
		m_sliderThreshold.SetPos( int(fVal) );
		
		m_pMediaParams->GetParam( PARAM_OUTPUTGAIN, &fVal );
		m_sliderOutputGain.SetPos( int(fVal) );
		
		m_pMediaParams->GetParam( PARAM_ENABLE, &fVal );
		CheckDlgButton( IDC_ENABLED, fVal != 0 );
	}
	
	COlePropertyPage::OnTimer(nIDEvent);
}

void CAutoClipPropPage::OnBeginCapture( int idCtrl, UINT nFlags, CPoint point )
{
	if (m_pUICallback)
	{
		DWORD dwIndex = 0;
		if (IDC_THRESHOLD == idCtrl)
			dwIndex = PARAM_THRESHOLD;
		else if (IDC_OUTPUTGAIN == idCtrl)
			dwIndex = PARAM_OUTPUTGAIN;
		else
		{
			ASSERT(FALSE);
			return;
		}
		m_pUICallback->ParamsBeginCapture( &dwIndex, 1 );
	}	
}

void CAutoClipPropPage::OnEndCapture( int idCtrl, UINT nFlags, CPoint point )
{
	if (m_pUICallback)
	{
		DWORD dwIndex = 0;
		if (IDC_THRESHOLD == idCtrl)
			dwIndex = PARAM_THRESHOLD;
		else if (IDC_OUTPUTGAIN == idCtrl)
			dwIndex = PARAM_OUTPUTGAIN;
		else
		{
			ASSERT(FALSE);
			return;
		}
		m_pUICallback->ParamsEndCapture( &dwIndex, 1 );
	}	
}

/////////////////////////////////////////////////////////////////////////////
// CCaptureSlider

BEGIN_MESSAGE_MAP(CCaptureSlider, CSliderCtrl)
	//{{AFX_MSG_MAP(CCaptureSlider)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

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
