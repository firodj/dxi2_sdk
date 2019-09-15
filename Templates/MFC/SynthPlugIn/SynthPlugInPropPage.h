#ifndef _PLUGIN_PROP_PAGE_H_
#define _PLUGIN_PROP_PAGE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct IMediaParams;
struct IMediaParamsUICallback;

/////////////////////////////////////////////////////////////////////////////
// CSynthPlugInPropPage dialog

class CSynthPlugInPropPage :
	public COlePropertyPage,
	public CUnknown
{
// Construction
public:
	CSynthPlugInPropPage( IUnknown* pUnk, HRESULT* phr );
	virtual ~CSynthPlugInPropPage();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);
	STDMETHODIMP_(ULONG) NonDelegatingRelease();
	STDMETHODIMP_(ULONG) NonDelegatingAddRef();

// Dialog Data
	//{{AFX_DATA(CSynthPlugInPropPage)
	enum { IDD = IDD_PROPPAGE };
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSynthPlugInPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void OnObjectsChanged();

public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

	IMediaParams*				m_pMediaParams;
	IMediaParamsUICallback*	m_pUICallback;
	BOOL							m_bFirstQI;

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSynthPlugInPropPage)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _PLUGIN_PROP_PAGE_H_
