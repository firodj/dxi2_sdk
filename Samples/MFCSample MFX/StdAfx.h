// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__00EDAF2B_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_STDAFX_H__00EDAF2B_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxtempl.h>

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define BEGIN_INTERFACE
#define END_INTERFACE
#include <basetyps.h>
#include <objbase.h>
#include <ocidl.h>

#pragma warning( disable:4100 )	// (4) unreferenced formal parameter


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__00EDAF2B_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
