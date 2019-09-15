// TwonarPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TwonarPropPage.h"
#include "ITwonar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTwonarPropPage property page

CTwonarPropPage::CTwonarPropPage( IUnknown* pUnk, HRESULT* phr ) :
	COlePropertyPage( CTwonarPropPage::IDD, IDS_NAME_PLUGIN ),
	CUnknown( "TwonarPropPage", pUnk ),
	m_pITwonar(NULL),
	m_bFirstQI(TRUE),
	m_pInputPort(NULL),
	m_pInputCallback(NULL),
	m_btnPiano( this )
{	
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	//{{AFX_DATA_INIT(CTwonarPropPage)
	m_bMultiOut = FALSE;
	m_nMode = -1;
	m_bEnableMetro = FALSE;
	m_nNoteMeasTop = 0;
	m_nVelMeasTop = 0;
	m_nNoteBeat = 0;
	m_nVelBeat = 0;
	//}}AFX_DATA_INIT

	memset( m_abKeyDown, 0, sizeof(m_abKeyDown) );

#if _DEBUG
	// Turn off obnoxious "non-standard size" warnings from MFC.  At least
	// they gave us a flag to do this!
	m_bNonStandardSize = TRUE;
#endif
}

CTwonarPropPage::~CTwonarPropPage()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if (m_pITwonar)
		m_pITwonar->Release();
	m_pITwonar = NULL;
	if (m_pInputPort)
		m_pInputPort->Release();
	m_pInputPort = NULL;
	if (m_pInputCallback)
		m_pInputCallback->Release();
	m_pInputCallback = NULL;
}

void CTwonarPropPage::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	COlePropertyPage::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate && NULL != m_pITwonar)
	{
		// TODO: Load dialog box data from ITwonar, i.e.,
		//		if (FAILED( m_pITwonar->get_Foo( &m_nFoo ) ))
		//			pDX->Fail();
		if (FAILED( m_pITwonar->get_IsMultiOut( &m_bMultiOut ) ))
			pDX->Fail();
		if (FAILED( m_pITwonar->get_Mode( &m_nMode ) ))
			pDX->Fail();
		if (FAILED( m_pITwonar->get_EnableMetro( &m_bEnableMetro ) ))
			pDX->Fail();
		if (FAILED( m_pITwonar->get_MetroInfo( &m_nNoteMeasTop, &m_nVelMeasTop, &m_nNoteBeat, &m_nVelBeat ) ))
			pDX->Fail();
	}

	//{{AFX_DATA_MAP(CTwonarPropPage)
	DDX_Check(pDX, IDC_CHECK1, m_bMultiOut);
	DDX_Radio(pDX, IDC_RADIO1, m_nMode);
	DDX_Check(pDX, IDC_CHECK2, m_bEnableMetro);
	DDX_Text(pDX, IDC_EDIT1, m_nNoteMeasTop);
	DDV_MinMaxInt(pDX, m_nNoteMeasTop, 0, 127);
	DDX_Text(pDX, IDC_EDIT2, m_nVelMeasTop);
	DDV_MinMaxInt(pDX, m_nVelMeasTop, 0, 127);
	DDX_Text(pDX, IDC_EDIT3, m_nNoteBeat);
	DDV_MinMaxInt(pDX, m_nNoteBeat, 0, 127);
	DDX_Text(pDX, IDC_EDIT4, m_nVelBeat);
	DDV_MinMaxInt(pDX, m_nVelBeat, 0, 127);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && NULL != m_pITwonar)
	{
		// TODO: Save dialog box data to ITwonar, i.e.,
		//		if (FAILED( m_pITwonar->put_Foo( m_nFoo ) ))
		//			pDX->Fail();
		if (FAILED( m_pITwonar->put_IsMultiOut( m_bMultiOut ) ))
			pDX->Fail();
		if (FAILED( m_pITwonar->put_Mode( m_nMode ) ))
			pDX->Fail();
		if (FAILED( m_pITwonar->put_EnableMetro( m_bEnableMetro ) ))
			pDX->Fail();
		if (FAILED( m_pITwonar->put_MetroInfo( m_nNoteMeasTop, m_nVelMeasTop, m_nNoteBeat, m_nVelBeat ) ))
			pDX->Fail();
	}
}


BEGIN_MESSAGE_MAP(CTwonarPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CTwonarPropPage)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusNoteMeasTop)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusVelMeasTop)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnKillfocusNoteBeat)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnKillfocusVelBeat)
	ON_BN_CLICKED(IDC_CHECK2, OnEnableMetro)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////
// DO NOT CHANGE ANY OF THE FOLLOWING CODE!
////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CTwonarPropPage::QueryInterface(REFIID riid, void **ppv)
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

ULONG CTwonarPropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalAddRef();
};

ULONG CTwonarPropPage::Release()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ExternalRelease();
};

STDMETHODIMP CTwonarPropPage::NonDelegatingQueryInterface(REFIID riid,void **ppv)
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

ULONG CTwonarPropPage::NonDelegatingRelease()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalRelease();
}

ULONG CTwonarPropPage::NonDelegatingAddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return InternalAddRef();
}

////////////////////////////////////////////////////////////////////////////////
// CBasePropertyPage

CUnknown * WINAPI CTwonarPropPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return new CTwonarPropPage( lpunk, phr );
}

////////////////////////////////////////////////////////////////////////////////

void CTwonarPropPage::OnObjectsChanged()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Release old IFilter
	if (m_pITwonar)
		m_pITwonar->Release();
	m_pITwonar = NULL;

	// Look for a new IFilter
	ULONG cObj = 0;
	IDispatch** apIDisp = GetObjectArray( &cObj );
	for (ULONG i = 0; i < cObj; ++i)
	{
		if (S_OK == apIDisp[i]->QueryInterface( IID_ITwonar, (void**)&m_pITwonar ))
			break;
	}

	// Release old input port and callback
	if (m_pInputPort)
		m_pInputPort->Release();
	m_pInputPort = NULL;
	if (m_pInputCallback)
		m_pInputCallback->Release();
	m_pInputCallback = NULL;

	// Query for a new one
	if (m_pITwonar)
	{
		if (SUCCEEDED( m_pITwonar->QueryInterface( IID_IMfxInputPort, (void**)&m_pInputPort ) ))
		{
			if (S_OK != m_pInputPort->GetInputCallback( &m_pInputCallback ))
				m_pInputCallback = NULL;
		}
	}

	// Update controls if we've got a new object and we're activated
	if (m_pITwonar && ::IsWindow( m_hWnd ))
		UpdateData( FALSE );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL CTwonarPropPage::OnInitDialog() 
{
	COlePropertyPage::OnInitDialog();
	
	m_btnPiano.AutoLoad( IDC_PIANO, this );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

////////////////////////////////////////////////////////////////////////////////

BOOL CTwonarPropPage::fillMfxEventFromKey( MfxEvent& me, UINT nChar, BOOL bKeyDown )
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

void CTwonarPropPage::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	COlePropertyPage::OnKeyDown(nChar, nRepCnt, nFlags);
	if (NULL != m_pInputPort && NULL != m_pInputCallback && 1 == nRepCnt)
	{
		MfxEvent me;
		if (fillMfxEventFromKey( me, nChar, TRUE ))
			m_pInputCallback->OnEvent( m_pInputPort, me );
	}
}

void CTwonarPropPage::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	COlePropertyPage::OnKeyUp(nChar, nRepCnt, nFlags);
	if (NULL != m_pInputPort && NULL != m_pInputCallback && 1 == nRepCnt)
	{
		MfxEvent me;
		if (fillMfxEventFromKey( me, nChar, FALSE ))
			m_pInputCallback->OnEvent( m_pInputPort, me );
	}
}

void CTwonarPropPage::OnKillfocusNoteMeasTop() 
{
	CString str;
	GetDlgItem( IDC_EDIT1 )->GetWindowText( str );
	int n = atoi( str );
	if (0 <= n && n <= 127)
	{
		m_nNoteMeasTop = n;
		m_pITwonar->put_MetroInfo( m_nNoteMeasTop, m_nVelMeasTop, m_nNoteBeat, m_nVelBeat );
	}
}

void CTwonarPropPage::OnKillfocusVelMeasTop() 
{
	CString str;
	GetDlgItem( IDC_EDIT2 )->GetWindowText( str );
	int n = atoi( str );
	if (0 <= n && n <= 127)
	{
		m_nVelMeasTop = n;
		m_pITwonar->put_MetroInfo( m_nNoteMeasTop, m_nVelMeasTop, m_nNoteBeat, m_nVelBeat );
	}
}

void CTwonarPropPage::OnKillfocusNoteBeat() 
{
	CString str;
	GetDlgItem( IDC_EDIT3 )->GetWindowText( str );
	int n = atoi( str );
	if (0 <= n && n <= 127)
	{
		m_nNoteBeat = n;
		m_pITwonar->put_MetroInfo( m_nNoteMeasTop, m_nVelMeasTop, m_nNoteBeat, m_nVelBeat );
	}
}

void CTwonarPropPage::OnKillfocusVelBeat() 
{
	CString str;
	GetDlgItem( IDC_EDIT4 )->GetWindowText( str );
	int n = atoi( str );
	if (0 <= n && n <= 127)
	{
		m_nVelBeat = n;
		m_pITwonar->put_MetroInfo( m_nNoteMeasTop, m_nVelMeasTop, m_nNoteBeat, m_nVelBeat );
	}
}

void CTwonarPropPage::OnEnableMetro() 
{
	m_bEnableMetro = IsDlgButtonChecked( IDC_CHECK2 );
	m_pITwonar->put_EnableMetro( m_bEnableMetro );
}
