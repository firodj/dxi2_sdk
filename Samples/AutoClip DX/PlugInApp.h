// PlugInApp.h : main header file for PlugIn.ax
//

#ifndef _PLUGIN_APP_H_
#define _PLUGIN_APP_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPlugInApp
// See PlugInApp.cpp for the implementation of this class
//

class CPlugInApp : public CWinApp
{
public:
	CPlugInApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlugInApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPlugInApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////


#endif // _PLUGIN_APP_H_
