// PlugInApp.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "PlugInApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CPlugInApp

BEGIN_MESSAGE_MAP(CPlugInApp, CWinApp)
	//{{AFX_MSG_MAP(CPlugInApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlugInApp construction

CPlugInApp::CPlugInApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPlugInApp object

CPlugInApp theApp;


/////////////////////////////////////////////////////////////////////////////
// DO NOT EDIT OF THE FOLLOWING CODE!
// This code provides the magic glue to allow a DLL created with the DShow
// class library to use MFC in a DLL + the MSVC runtimes in a DLL.
/////////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);	// DShow
extern "C" BOOL WINAPI DllMain(HINSTANCE, ULONG, LPVOID);			// MFC
extern "C" BOOL WINAPI RawDllMain(HINSTANCE, ULONG, LPVOID);		// MFC/MSVCRT hook
extern "C" BOOL WINAPI _CRT_INIT(HANDLE, DWORD, LPVOID);				// MSVCRT

extern "C" BOOL WINAPI PluginDllMain(HINSTANCE hInst, ULONG dwReason, LPVOID pv)
{
    if (DLL_PROCESS_ATTACH == dwReason || DLL_THREAD_ATTACH == dwReason)
    {
        RawDllMain( hInst, dwReason, pv );
        _CRT_INIT( hInst, dwReason, pv );
        DllMain( hInst, dwReason, pv );
        DllEntryPoint( hInst, dwReason, pv );
    }
    else // detach in reverse order
    {
        DllEntryPoint( hInst, dwReason, pv );
        DllMain( hInst, dwReason, pv );
        _CRT_INIT( hInst, dwReason, pv );
        RawDllMain( hInst, dwReason, pv );
    }
    
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

LONG recursiveDeleteKey( HKEY hKeyParent,					// Parent of key to delete
                         const char* lpszKeyChild )	// Key to delete
{
	// Open the child.
	HKEY hKeyChild ;
	LONG lRes = RegOpenKeyEx( hKeyParent, lpszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild );
	if (lRes != ERROR_SUCCESS)
	{
		return lRes;
	}

	// Enumerate all of the decendents of this child.
	FILETIME time;
	char szBuffer[ 256 ];
	DWORD dwSize = 256;
	while (RegEnumKeyEx( hKeyChild, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time ) == S_OK)
	{
		// Delete the decendents of this child.
		lRes = recursiveDeleteKey( hKeyChild, szBuffer );
		if (lRes != ERROR_SUCCESS)
		{
			// Cleanup before exiting.
			RegCloseKey( hKeyChild );
			return lRes;
		}
		dwSize = 256;
	}

	// Close the child.
	RegCloseKey( hKeyChild );

	// Delete this child.
	return RegDeleteKey( hKeyParent, lpszKeyChild );
}

////////////////////////////////////////////////////////////////////////////////

static const char* s_pszReg = "CakewalkPlugIns\\";

extern CFactoryTemplate g_Templates[];
extern int g_cTemplates;

////////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
	HKEY					hKey = 0;
	char					sz[ _MAX_PATH ];
	OLECHAR				wsz[ _MAX_PATH ];
	char					szCLSID[ 64 ];
	ITypeLib*			pTypeLib = 0;
	int					i = 0;
	HRESULT				hr = E_FAIL;

	// Do DirectShow registration
	hr = AMovieDllRegisterServer2( TRUE );
	if (FAILED( hr ))
		goto DONE;

	// Get our full pathname, converting to multibyte
	GetModuleFileName( g_hInst, sz, sizeof sz );
	if (0 == MultiByteToWideChar( CP_ACP, 0, sz, _MAX_PATH, wsz, _MAX_PATH ))
		goto DONE;
		
	// Iterate over all exported CLSIDs
	for (i = 0; i < g_cTemplates; i++)
	{
		CFactoryTemplate* pT = &g_Templates[ i ];

		if (NULL != pT->m_pAMovieSetup_Filter)
		{
			// For backwards compatability, instantiate all servers and get hold of
			// IAMovieSetup (if implemented) and call IAMovieSetup.Register() method
			if (NULL != pT->m_lpfnNew)
			{
				IAMovieSetup* pSetup = 0;
				if (SUCCEEDED( CoCreateInstance( *(pT->m_ClsID), 0, CLSCTX_INPROC_SERVER,
					IID_IAMovieSetup, (void**)&pSetup ) ))
				{
					pSetup->Register();
					pSetup->Release();
				}
			}

			// Convert the CLSID to an ANSI string
			StringFromGUID2( *(pT->m_ClsID), wsz, sizeof wsz );
			if (0 == WideCharToMultiByte( CP_ACP, 0, wsz, -1, szCLSID, sizeof szCLSID, NULL, NULL ))
				goto DONE;
		
			// Add {...} to HKEY_CLASSES_ROOT\<s_pszReg>
			strcpy( sz, s_pszReg );
			strcat( sz, szCLSID );
			if (ERROR_SUCCESS != RegCreateKey( HKEY_CLASSES_ROOT, sz, &hKey ))
				goto DONE;

			// {...}\Description = <description text>
			if (0 == WideCharToMultiByte( CP_ACP, 0, pT->m_Name, -1, sz, sizeof sz, NULL, NULL ))
				goto DONE;
			RegSetValueEx( hKey, "Description", 0, REG_SZ, (BYTE*)sz, strlen(sz) );

			// Written for backwards compatability with SONAR 1.x and Pro Audio:
			// {...}\HelpFilePath = ""
			// {...}\HelpFileTopic = ""
			*sz = 0;	
			RegSetValueEx( hKey, "HelpFilePath", 0, REG_SZ, (BYTE*)sz, 1 );
			RegSetValueEx( hKey, "HelpFileTopic", 0, REG_SZ, (BYTE*)sz, 1 );

			RegCloseKey( hKey );
			hKey = 0;
		}
	}

	hr = S_OK;

DONE:


	if (hKey)
		RegCloseKey( hKey );

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{	
	char					sz[ _MAX_PATH ];
	OLECHAR				wsz[ _MAX_PATH ];
	char					szCLSID[ 64 ];
	int					i = 0;
	HRESULT				hr = E_FAIL;

	// Do DirectShow unregistration
	hr = AMovieDllRegisterServer2( FALSE );
	if (FAILED( hr ))
		goto DONE;

	// Iterate over all exported CLSIDs
	for (i = 0; i < g_cTemplates; i++)
	{
		CFactoryTemplate* pT = &g_Templates[ i ];

		// For backwards compatability, instantiate all servers and get hold of
		// IAMovieSetup (if implemented) and call IAMovieSetup.Register() method
		if (NULL != pT->m_lpfnNew)
		{
			IAMovieSetup* pSetup = 0;
			if (SUCCEEDED( CoCreateInstance( *(pT->m_ClsID), 0, CLSCTX_INPROC_SERVER,
														IID_IAMovieSetup, (void**)&pSetup ) ))
			{
				pSetup->Unregister();
				pSetup->Release();
			}
		}

		// Convert the CLSID to an ANSI string
		StringFromGUID2( *(pT->m_ClsID), wsz, sizeof wsz );
		if (0 == WideCharToMultiByte( CP_ACP, 0, wsz, -1, szCLSID, sizeof szCLSID, NULL, NULL ))
			goto DONE;
		
		// Delete HKEY_CLASSES_ROOT\<s_pszReg>
		strcpy( sz, s_pszReg );
		strcat( sz, szCLSID );
		recursiveDeleteKey( HKEY_CLASSES_ROOT, sz );
	}

	hr = S_OK;

DONE:

	return hr;
}

////////////////////////////////////////////////////////////////////////////////
