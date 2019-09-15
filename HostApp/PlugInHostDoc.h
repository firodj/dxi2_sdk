// PlugInHostDoc.h : interface of the CPlugInHostDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLUGINHOSTDOC_H__B8776C1A_4B36_4514_841A_96DF5F7A7136__INCLUDED_)
#define AFX_PLUGINHOSTDOC_H__B8776C1A_4B36_4514_841A_96DF5F7A7136__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMfxSeq;

class CPlugInHostDoc : public CDocument
{
protected: // create from serialization only
	CPlugInHostDoc();
	DECLARE_DYNCREATE(CPlugInHostDoc)

// Attributes
public:

	CMfxSeq* GetSeq() const { return m_pSeq; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlugInHostDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPlugInHostDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	CMfxSeq* m_pSeq;

// Generated message map functions
protected:
	//{{AFX_MSG(CPlugInHostDoc)
	afx_msg void OnUpdatePlay(CCmdUI* pCmdUI);
	afx_msg void OnPlay();
	afx_msg void OnUpdateStop(CCmdUI* pCmdUI);
	afx_msg void OnStop();
	afx_msg void OnUpdateRew(CCmdUI* pCmdUI);
	afx_msg void OnRew();
	afx_msg void OnUpdateLoop(CCmdUI* pCmdUI);
	afx_msg void OnLoop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// Implemented in SMF.CPP
HRESULT LoadMid( CMfxSeq& rSeq, CFile& rFile );

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLUGINHOSTDOC_H__B8776C1A_4B36_4514_841A_96DF5F7A7136__INCLUDED_)
