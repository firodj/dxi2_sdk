#if !defined(EFFECT_PROPS_VIEW_H)
#define EFFECT_PROPS_VIEW_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Copyright (C) 1991-1998 by Twelve Tone Systems, Inc. All rights reserved.

/////////////////////////////////////////////////////////////////////////////
// CPropertyPagesView

class CPropertyPagesView : public CView,
								 public IPropertyPageSite
{
	DECLARE_DYNCREATE(CPropertyPagesView)

public:
	CPropertyPagesView();           // protected constructor used by dynamic creation
	virtual ~CPropertyPagesView();

	HRESULT Initialize( IUnknown* pUnknown );
	HRESULT Terminate();

	static int GetIdealWidth();

// Operations
public:
// *** IUnknown methods ***
	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, LPVOID* ppvObj );
	STDMETHODIMP_(ULONG)		AddRef();
	STDMETHODIMP_(ULONG)		Release();

// *** IPropertyPageSite methods ***
	STDMETHODIMP_(HRESULT) OnStatusChange( DWORD dwFlags );
	STDMETHODIMP_(HRESULT) GetLocaleID( LCID* pLocaleID );
	STDMETHODIMP_(HRESULT) GetPageContainer( IUnknown** ) { return E_NOTIMPL; }
	STDMETHODIMP_(HRESULT) TranslateAccelerator( LPMSG lpMsg );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyPagesView)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPropertyPagesView)
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelChangePluginTab(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	enum { WM_SETMODIFIED = WM_APP+100 };
	afx_msg LRESULT OnSetModified( WPARAM, LPARAM );

	DECLARE_MESSAGE_MAP()

private:

	enum
	{
		IDW_TAB = 1000
	};

	CTabCtrl*	m_pTabCtrl;
	CRect			m_rcTabCtrl;

	DWORD	m_dwRefCount;
	BOOL	m_bInitialized;

	struct PropPage
	{
		IPropertyPage*	pPropPage;
		PROPPAGEINFO	ppi;
	};

	CAUUID				m_caGUID;	// collection of GUIDs for property pages
	PropPage*			m_aPages;	// array of PropPages, one for each element in m_caGUID
	int					m_ixPage;	// currently displayed page
};

#endif // !defined(EFFECT_PROPS_VIEW_H)
