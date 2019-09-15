#if !defined(AFX_MFCSAMPLEPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_MFCSAMPLEPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CMFCSamplePropPage dialog

class CMFCSamplePropPage : public CDialog,
									public IPropertyPage
{
// Construction
public:
	CMFCSamplePropPage(CWnd* pParent = NULL);   // standard constructor
	~CMFCSamplePropPage();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CMFCSamplePropPage)
	enum { IDD = IDD_PROPPAGE };
	CSliderCtrl	m_ctlRepsSlider;
	//}}AFX_DATA

	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)		AddRef( void );
	STDMETHODIMP_(ULONG)		Release( void );

	// *** IPropertyPage methods ***
	STDMETHODIMP_(HRESULT)	SetPageSite(LPPROPERTYPAGESITE pPageSite);
	STDMETHODIMP_(HRESULT)	Activate(HWND hwndParent, LPCRECT prect, BOOL fModal);
	STDMETHODIMP_(HRESULT)	Deactivate(void);
	STDMETHODIMP_(HRESULT)	GetPageInfo(LPPROPPAGEINFO pPageInfo);
	STDMETHODIMP_(HRESULT)	SetObjects(ULONG cObjects, LPUNKNOWN *ppUnk);
	STDMETHODIMP_(HRESULT)	Show(UINT nCmdShow);
	STDMETHODIMP_(HRESULT)	Move(LPCRECT prect);
	STDMETHODIMP_(HRESULT)	IsPageDirty(void);
	STDMETHODIMP_(HRESULT)	Apply(void);
	STDMETHODIMP_(HRESULT)	Help(LPCWSTR lpszHelpDir);
	STDMETHODIMP_(HRESULT)	TranslateAccelerator(LPMSG lpMsg);

	// *** IPlugInPropPageMFC methods ***
	STDMETHODIMP_(BOOL)	CreateMFCDialog( HWND hwndParent );
	STDMETHODIMP_(BOOL)	DestroyMFCDialog();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFCSamplePropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Implementation
private:
	LONG						m_cRef;
	CMFCMidiFilter*		m_pFilter;
	IPropertyPageSite*	m_pPageSite;
	BOOL						m_bDirty;

	static BOOL CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	//{{AFX_MSG(CMFCSamplePropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCSAMPLEPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
