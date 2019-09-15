#if !defined(AFX_OPTIONSDLG_H__B429C645_C8D2_4C67_9793_4F2AEA13A778__INCLUDED_)
#define AFX_OPTIONSDLG_H__B429C645_C8D2_4C67_9793_4F2AEA13A778__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

class COptionsDlg : public CDialog
{
// Construction
public:
	COptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptionsDlg)
	enum { IDD = IDD_OPTIONS };
	CComboBox	m_comboMidi;
	CComboBox	m_comboDSound;
	UINT	m_uBufferMsec;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptionsDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static BOOL CALLBACK DSEnumCallback( GUID* pGuid, LPCSTR pszDesc, LPCSTR pszModule, void* pCtx );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDLG_H__B429C645_C8D2_4C67_9793_4F2AEA13A778__INCLUDED_)
