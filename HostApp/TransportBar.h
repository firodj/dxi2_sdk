#if !defined(AFX_TRANSPORTBAR_H__6CE91123_AC55_4C50_ADC9_3E150ECE5CD4__INCLUDED_)
#define AFX_TRANSPORTBAR_H__6CE91123_AC55_4C50_ADC9_3E150ECE5CD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransportBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDXiPlayerBar dialog

class CDXiPlayerBar : public CDialogBar
{
// Construction
public:
	CDXiPlayerBar();

// Dialog Data
	//{{AFX_DATA(CDXiPlayerBar)
	enum { IDD = IDD_DIALOGBAR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDXiPlayerBar)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDXiPlayerBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKillfocusNow();
	afx_msg void OnKillfocusFrom();
	afx_msg void OnKillfocusThru();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	HRESULT editToTick( UINT id, LONG* plTick );
	HRESULT tickToEdit( UINT id, LONG lTick );

private:
	LONG	m_lPosition;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSPORTBAR_H__6CE91123_AC55_4C50_ADC9_3E150ECE5CD4__INCLUDED_)
