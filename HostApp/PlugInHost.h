// PlugInHost.h : main header file for the PlugInHost application
//

#if !defined(AFX_PLUGINHOST_H__706CDCAA_9C7A_491E_AACA_2841605BEC8D__INCLUDED_)
#define AFX_PLUGINHOST_H__706CDCAA_9C7A_491E_AACA_2841605BEC8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "PlugInInventory.h"

/////////////////////////////////////////////////////////////////////////////
// CPlugInHostApp:
// See PlugInHost.cpp for the implementation of this class
//

class CPlugInHostApp : public CWinApp
{
public:
	CPlugInHostApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlugInHostApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CPlugInHostApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CFont& GetFont() { return m_font; }
	const CPlugInInventory& GetPlugInInventory() const { return m_inventory; }

	static const char* GetOptionsSectionName() { return "Options"; }

private:
	BOOL createFont();

private:
	CFont					m_font;
	CPlugInInventory	m_inventory;
};

extern CPlugInHostApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLUGINHOST_H__706CDCAA_9C7A_491E_AACA_2841605BEC8D__INCLUDED_)
