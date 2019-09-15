#if !defined(AFX_TRACKPROPERTIESVIEW_H__0C46041B_5978_4A6D_BB0B_CF9B1B901561__INCLUDED_)
#define AFX_TRACKPROPERTIESVIEW_H__0C46041B_5978_4A6D_BB0B_CF9B1B901561__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TrackPropertiesView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTrackPropertiesView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CMfxSeq;
struct IBaseFilter;

class CTrackPropertiesView : public CFormView
{
protected:
	CTrackPropertiesView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTrackPropertiesView)

// Form Data
public:
	//{{AFX_DATA(CTrackPropertiesView)
	enum { IDD = IDD_TRACK_PROPERTIES };
	CEdit	m_editVol;
	CEdit	m_editVelPlus;
	CComboBox	m_comboTrack;
	CComboBox	m_comboPatch;
	CEdit	m_editPan;
	CButton	m_btnMute;
	CEdit	m_editKeyPlus;
	CEdit	m_editChannel;
	CComboBox	m_comboBank;
	//}}AFX_DATA

// Attributes
public:

	static int GetIdealWidth();

	CMfxSeq* GetSeq() { return m_pSeq; }
	void SetSeq( CMfxSeq* pSeq );

	IBaseFilter* GetFilter() { return m_pFilter; }
	void SetFilter( IBaseFilter* pFilter );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrackPropertiesView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTrackPropertiesView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CTrackPropertiesView)
	afx_msg void OnSelchangeTrack();
	afx_msg void OnSelchangeBank();
	afx_msg void OnSelchangePatch();
	afx_msg void OnMute();
	afx_msg void OnKillfocusPan();
	afx_msg void OnKillfocusKeyplus();
	afx_msg void OnKillfocusVelplus();
	afx_msg void OnKillfocusVol();
	afx_msg void OnKillfocusChannel();
	afx_msg void OnKillfocusPatch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void fillBankList();
	void fillPatchList();

private:
	
	int				m_nTrkNum;
	CMfxSeq*			m_pSeq;
	IBaseFilter*	m_pFilter;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRACKPROPERTIESVIEW_H__0C46041B_5978_4A6D_BB0B_CF9B1B901561__INCLUDED_)
