// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__6FC9206C_13AA_46C4_BBCA_14AED721BC90__INCLUDED_)
#define AFX_MAINFRM_H__6FC9206C_13AA_46C4_BBCA_14AED721BC90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTrackPropertiesView;
class CPropertyPagesView;
class CPlugInHostDoc;

#include "TransportBar.h"

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;

public:

	CTrackPropertiesView* GetTrackPropertiesView();
	CPropertyPagesView* GetPropertyPagesView();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar		m_wndStatusBar;
	CToolBar			m_wndToolBar;
	CDXiPlayerBar	m_wndTransportBar;
	CReBar			m_wndReBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnDestroy();
	afx_msg void OnUpdateOptions(CCmdUI* pCmdUI);
	afx_msg void OnOptions();
	afx_msg void OnViewTrackProperties();
	//}}AFX_MSG
	afx_msg void OnUpdateDXi( CCmdUI* pCmdUI );
	afx_msg void OnDXi( UINT id );
	DECLARE_MESSAGE_MAP()

private:
	BOOL		m_bFilledDXiMenu;
	ULONG		m_ixDXi;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__6FC9206C_13AA_46C4_BBCA_14AED721BC90__INCLUDED_)
