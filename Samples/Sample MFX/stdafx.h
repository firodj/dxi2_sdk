//---------------------------------------------------------------------------
// Microsoft Visual C++
#ifdef _MSC_VER

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define OEMRESOURCE		// allow us to include Windows OBM_ Bitmaps
#include <afxwin.h>        // MFC core and standard components
#include <afxext.h>        // MFC extensions
#include <afxole.h>        // MFC OLE classes
#include <afxodlgs.h>      // MFC OLE dialog classes
#include <afxdisp.h>       // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxtempl.h>		// MFC templates
#include <afxpriv.h>			// MFC customized help processing

#include <windowsx.h>

#pragma warning( disable: 4100 )	// unreferenced formal parameter

//---------------------------------------------------------------------------
// Other compilers
#else // _MSC_VER

#define  WIN32_EXTRA_LEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#endif // _MSC_VER
//---------------------------------------------------------------------------
