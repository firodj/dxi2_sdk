// MFCSample.h : main header file for the MFCSAMPLE DLL
//

#if !defined(AFX_MFCSAMPLE_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_MFCSAMPLE_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

// {3DE6F480-01FF-11d2-AFFF-00A0C90DA071}
static const GUID CLSID_MFCMidiFilter = 
{ 0x3de6f480, 0x1ff, 0x11d2, { 0xaf, 0xff, 0x0, 0xa0, 0xc9, 0xd, 0xa0, 0x71 } };

// {3DE6F481-01FF-11d2-AFFF-00A0C90DA071}
static const GUID CLSID_MFCMidiFilterProps = 
{ 0x3de6f481, 0x1ff, 0x11d2, { 0xaf, 0xff, 0x0, 0xa0, 0xc9, 0xd, 0xa0, 0x71 } };

/////////////////////////////////////////////////////////////////////////////
// CMidiFilter

class CMFCMidiFilter:	public IMfxEventFilter,
								public IPersistStream,
								public ISpecifyPropertyPages
{
public:
	// Ctors
	CMFCMidiFilter();
	virtual ~CMFCMidiFilter();

	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)		AddRef();
	STDMETHODIMP_(ULONG)		Release();

	// *** IMfxEventFilter methods ***
	STDMETHODIMP_(HRESULT) Connect( IUnknown* pContext );
	STDMETHODIMP_(HRESULT) Disconnect();
	STDMETHODIMP_(HRESULT) OnStart( LONG lTime, IMfxEventQueue* pqOut );
	STDMETHODIMP_(HRESULT) OnLoop( LONG lTimeRestart, LONG lTimeStop, IMfxEventQueue* pqOut );
	STDMETHODIMP_(HRESULT) OnStop( LONG lTime, IMfxEventQueue* pqOut );
	STDMETHODIMP_(HRESULT) OnEvents( LONG lTimeFrom, LONG lTimeThru, IMfxEventQueue* pqIn, IMfxEventQueue* pqOut );
	STDMETHODIMP_(HRESULT) OnInput( IMfxDataQueue* pqIn, IMfxDataQueue* pqOut );

	// *** IPersistStream methods ***
	STDMETHODIMP_(HRESULT)	GetClassID( CLSID* pClsid );
	STDMETHODIMP_(HRESULT)	IsDirty();
	STDMETHODIMP_(HRESULT)	Load( IStream* pStm );
	STDMETHODIMP_(HRESULT)	Save( IStream* pStm, BOOL bClearDirty );
	STDMETHODIMP_(HRESULT)	GetSizeMax( ULARGE_INTEGER* pcbSize );

	// *** ISpecifyPropertyPages methods ***
	STDMETHODIMP_(HRESULT)	GetPages( CAUUID* pPages );

// Attributes
	int GetReps();
	void SetReps( UINT nReps );
	static UINT GetMinReps() { return 1; }
	static UINT GetMaxReps() { return 10; }

	void SetDirty() { m_bDirty = TRUE; }
	BOOL GetDirty() const { return m_bDirty; }

// Implementation
private:
	LONG					m_cRef;
	CRITICAL_SECTION	m_cs;		// critical section to guard properties
	int					m_nReps;
	BOOL					m_bDirty;
	IMfxTempoMap*		m_pTempoMap;
};

/////////////////////////////////////////////////////////////////////////////
// CMFCSampleApp
// See MFCSample.cpp for the implementation of this class
//

class CMFCSampleApp : public CWinApp
{
public:
	CMFCSampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFCSampleApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMFCSampleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CMFCSampleApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCSAMPLE_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
