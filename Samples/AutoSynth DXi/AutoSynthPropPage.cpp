// AutoSynthPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "AutoSynthPropPage.h"

#include <MedParam.h>
#include "CakeMedParam.h"
#include "Parameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoSynthPropPage property page

CAutoSynthPropPage::CAutoSynthPropPage( IUnknown* pUnk, HRESULT* phr ) :
	COlePropertyPage( CAutoSynthPropPage::IDD, IDS_NAME_PLUGIN ),
	CUnknown( "AutoSynthPropPage", pUnk ),
	m_pMediaParams(NULL),
	m_pUICallback(NULL),
	m_bFirstQI(TRUE),
	m_pInputPort(NULL),
	m_pInputCallback(NULL),
	m_sliderCutoff( this ),
	m_sliderResonance( this ),
	m_btnPiano( this )
{	
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	//{{AFX_DATA_INIT(CAutoSynthPropPage)
	//}}AFX_DATA_INIT

#ifdef _DEBUG
	// Turn off obnoxious "non-standard size" warnings from MFC.  At least
	// they gave us a flag to do this!
	m_bNonStandardSize = TRUE;
#endif
}

CAutoSynthPropPage::~CAutoSynthPropPage()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if (m_pMediaParams)
		m_pMediaParams->Release();
	m_pMediaParams = NULL;
	
	if (m_pUICallback)
		m_pUICallback->Release();
	m_pUICallback = NULL;

	if (m_pInputPort)
		m_pInputPort->Release();
	m_pInputPort = NULL;

	if (m_pInputCallback)
		m_pInputCallback->Release();
	m_pInputCallback = NULL;
}

void CAutoSynthPropPage::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	COlePropertyPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CAutoSynthPropPage)
	DDX_Control(pDX, IDC_CUTOFF, m_sliderCutoff);
	DDX_Control(pDX, IDC_RESONANCE, m_sliderResonance);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutoSynthPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CAutoSynthPropPage)
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ENABLE, OnEnable)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////
// DO NOT CHANGE ANY OF THE FOLLOWING CODE!
////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CAutoSynthPropPage::QueryInterface(REFIID riid, void **ppv)
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

ULONG CAutoSynthPropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalAddRef();
};

ULONG CAutoSynthPropPage::Release()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalRelease();
};

STDMETHODIMP CAutoSynthPropPage::NonDelegatingQueryInterface(REFIID riid,void **ppv)
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

ULONG CAutoSynthPropPage::NonDelegatingRelease()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalRelease();
}

ULONG CAutoSynthPropPage::NonDelegatingAddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalAddRef();
}

////////////////////////////////////////////////////////////////////////////////
// CBasePropertyPage

CUnknown * WINAPI CAutoSynthPropPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return new CAutoSynthPropPage( lpunk, phr );
}

////////////////////////////////////////////////////////////////////////////////

void CAutoSynthPropPage::OnObjectsChanged()
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

	// Release old input port and callback
	if (m_pInputPort)
		m_pInputPort->Release();
	m_pInputPort = NULL;
	if (m_pInputCallback)
		m_pInputCallback->Release();
	m_pInputCallback = NULL;

	// Query for new ones
	if (m_pMediaParams)
	{
		if (SUCCEEDED( m_pMediaParams->QueryInterface( IID_IMfxInputPort, (void**)&m_pInputPort ) ))
		{
			if (S_OK != m_pInputPort->GetInputCallback( &m_pInputCallback ))
				m_pInputCallback = NULL;
		}
	}

	// Update controls if we've got a new object and we're activated
	if (m_pMediaParams && ::IsWindow( m_hWnd ))
		UpdateData( FALSE );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL CAutoSynthPropPage::OnInitDialog() 
{
	COlePropertyPage::OnInitDialog();
	
	m_btnPiano.AutoLoad( IDC_PIANO, this );
	
	m_sliderCutoff.SetRange( 0, 100, TRUE );
	m_sliderResonance.SetRange( 0, 400, TRUE );

	// Set up a timer to periodically update the slider every 50 msec
	SetTimer( 0, 50, NULL );

	// Initialize buttons
	if (m_pMediaParams)
	{
		float fVal;
		m_pMediaParams->GetParam( _PARAM_MULTI_OUT, &fVal );
		CheckDlgButton( IDC_CHECK1, fVal != 0 );
		m_pMediaParams->GetParam( _PARAM_MODE, &fVal );
		CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1 + int(fVal) );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

////////////////////////////////////////////////////////////////////////////////

void CAutoSynthPropPage::OnTimer(UINT nIDEvent) 
{
	if (NULL == m_pMediaParams)
		return;

	float fVal;

	if (!m_sliderCutoff.IsCaptured())
	{
		m_pMediaParams->GetParam( PARAM_FILTER_CUTOFF, &fVal );
		m_sliderCutoff.SetPos( int(fVal * 100) );
	}
	
	if (!m_sliderResonance.IsCaptured())
	{
		m_pMediaParams->GetParam( PARAM_FILTER_RESONANCE, &fVal );
		m_sliderResonance.SetPos( int(fVal * 100) );
	}
	
	m_pMediaParams->GetParam( PARAM_ENABLE, &fVal );
	CheckDlgButton( IDC_ENABLE, fVal != 0 );

	COlePropertyPage::OnTimer(nIDEvent);
}

////////////////////////////////////////////////////////////////////////////////

void CAutoSynthPropPage::OnDestroy() 
{
	COlePropertyPage::OnDestroy();
	
	KillTimer( 0 );
}


////////////////////////////////////////////////////////////////////////////////
// CCaptureNotify

void CAutoSynthPropPage::OnBeginCapture( int idCtrl, UINT nFlags, CPoint point )
{
	if (m_pUICallback)
	{
		DWORD dwIndex = 0;
		if (IDC_CUTOFF == idCtrl)
			dwIndex = PARAM_FILTER_CUTOFF;
		else if (IDC_RESONANCE == idCtrl)
			dwIndex = PARAM_FILTER_RESONANCE;
		else
		{
			ASSERT(FALSE);
			return;
		}
		m_pUICallback->ParamsBeginCapture( &dwIndex, 1 );
	}	
}

////////////////////////////////////////////////////////////////////////////////

void CAutoSynthPropPage::OnEndCapture( int idCtrl, UINT nFlags, CPoint point )
{
	if (m_pUICallback)
	{
		DWORD dwIndex = 0;
		if (IDC_CUTOFF == idCtrl)
			dwIndex = PARAM_FILTER_CUTOFF;
		else if (IDC_RESONANCE == idCtrl)
			dwIndex = PARAM_FILTER_RESONANCE;
		else
		{
			ASSERT(FALSE);
			return;
		}
		m_pUICallback->ParamsEndCapture( &dwIndex, 1 );
	}	
}

////////////////////////////////////////////////////////////////////////////////

void CAutoSynthPropPage::OnEnable() 
{
	SetModifiedFlag( TRUE );

	float fValue = IsDlgButtonChecked( IDC_ENABLE );
	if (m_pUICallback)
	{
		DWORD dwIndex = PARAM_ENABLE;
		m_pUICallback->ParamsBeginCapture( &dwIndex, 1 );
		m_pUICallback->ParamsChanged( &dwIndex, 1, &fValue );
		m_pUICallback->ParamsEndCapture( &dwIndex, 1 );
	}
}

////////////////////////////////////////////////////////////////////////////////

void CAutoSynthPropPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	COlePropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);

	SetModifiedFlag( TRUE );

	if (m_pUICallback)
	{
		float fVal = 0;
		DWORD dwIndex = 0;
		if (IDC_CUTOFF == pScrollBar->GetDlgCtrlID())
		{
			fVal = static_cast<float>( m_sliderCutoff.GetPos() ) / 100;
			dwIndex = PARAM_FILTER_CUTOFF;
		}
		else if (IDC_RESONANCE == pScrollBar->GetDlgCtrlID())
		{
			fVal = static_cast<float>( m_sliderResonance.GetPos() ) / 100;
			dwIndex = PARAM_FILTER_RESONANCE;
		}
		else
		{
			ASSERT(FALSE);
			return;
		}
		m_pUICallback->ParamsChanged( &dwIndex, 1, &fVal );
	}
}

////////////////////////////////////////////////////////////////////////////////

BOOL CAutoSynthPropPage::fillMfxEventFromKey( MfxEvent& me, UINT nChar, BOOL bKeyDown )
{
	int nNote = -1;
	switch (toupper( nChar ))
	{
		case 'Q':	// C
			nNote = 60; break;
		case '2':	// C#
			nNote = 61; break;
		case 'W':	// D
			nNote = 62; break;
		case '3':	// D#
			nNote = 63; break;
		case 'E':	// E
			nNote = 64; break;
		case 'R':	// F
			nNote = 65; break;
		case '5':	// F#
			nNote = 66; break;
		case 'T':	// G
			nNote = 67; break;
		case '6':	// G#
			nNote = 68; break;
		case 'Y':	// A
			nNote = 69; break;
		case '7':	// A#
			nNote = 70; break;
		case 'U':	// B
			nNote = 71; break;
	}
	
	if (-1 != nNote)
	{
		if (m_abKeyDown[ nNote - 60 ] != bKeyDown)
		{
			m_abKeyDown[ nNote - 60 ] = bKeyDown;
			me.m_byPort = 0;
			me.m_byChan = 0;
			me.m_eType = MfxEvent::Note;
			me.m_byKey = static_cast<BYTE>( nNote );
			me.m_byVel = 64;
			me.m_byVelOff = 64;
			me.m_dwDuration = (bKeyDown) ? ULONG_MAX : 0;
			return TRUE;
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void CAutoSynthPropPage::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	COlePropertyPage::OnKeyDown(nChar, nRepCnt, nFlags);
	if (NULL != m_pInputPort && NULL != m_pInputCallback && 1 == nRepCnt)
	{
		MfxEvent me;
		if (fillMfxEventFromKey( me, nChar, TRUE ))
			m_pInputCallback->OnEvent( m_pInputPort, me );
	}
}

void CAutoSynthPropPage::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	COlePropertyPage::OnKeyUp(nChar, nRepCnt, nFlags);
	if (NULL != m_pInputPort && NULL != m_pInputCallback && 1 == nRepCnt)
	{
		MfxEvent me;
		if (fillMfxEventFromKey( me, nChar, FALSE ))
			m_pInputCallback->OnEvent( m_pInputPort, me );
	}
}

////////////////////////////////////////////////////////////////////////////////
