// PlugInHost.cpp : Defines the class behaviors for the application.
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//

#include "stdafx.h"
#include "PlugInHost.h"

#include "MainFrm.h"
#include "PlugInHostDoc.h"
#include "TrackPropertiesView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlugInHostApp

BEGIN_MESSAGE_MAP(CPlugInHostApp, CWinApp)
	//{{AFX_MSG_MAP(CPlugInHostApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlugInHostApp construction

CPlugInHostApp::CPlugInHostApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPlugInHostApp object

CPlugInHostApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPlugInHostApp initialization

BOOL CPlugInHostApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Initialize OLE
	CoInitialize( NULL );

	// Create our font
	if (!createFont())
		return FALSE;

	// Get the list of registered plug-ins
	if (FAILED( m_inventory.EnumPlugIns() ))
		return FALSE;

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("Cakewalk DXi SDK"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CPlugInHostDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CTrackPropertiesView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CPlugInHostApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// Create the font for the application

BOOL CPlugInHostApp::createFont()
{
	CDC dc;
	dc.CreateDC( "DISPLAY", NULL, NULL, NULL );

	// Create standard font
	LOGFONT lf;
	memset( &lf, 0, sizeof lf );
	lf.lfHeight				= -MulDiv( 8, dc.GetDeviceCaps( LOGPIXELSY ), 72 );
	lf.lfWeight				= FW_NORMAL;
	lf.lfCharSet			= ANSI_CHARSET;
	lf.lfOutPrecision		= OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	lf.lfQuality			= DEFAULT_QUALITY;
	lf.lfPitchAndFamily	= FF_SWISS | VARIABLE_PITCH;
	::strcpy( lf.lfFaceName, "Lucida Console" );

	return m_font.CreateFontIndirect( &lf );
}

/////////////////////////////////////////////////////////////////////////////
// CPlugInHostApp message handlers

int CPlugInHostApp::ExitInstance() 
{
	int const nRet = CWinApp::ExitInstance();
	CoUninitialize();
	return nRet;
}
