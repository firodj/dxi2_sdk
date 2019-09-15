#if !defined(AFX_PIANOBUTTON_H__B033193D_36FB_4178_B36B_2451C25DCB1A__INCLUDED_)
#define AFX_PIANOBUTTON_H__B033193D_36FB_4178_B36B_2451C25DCB1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PianoButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPianoButton window

class CPianoButton : public CBitmapButton
{
// Construction
public:
	CPianoButton( CWnd* pParent );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPianoButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPianoButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPianoButton)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CWnd* m_pParent;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PIANOBUTTON_H__B033193D_36FB_4178_B36B_2451C25DCB1A__INCLUDED_)
