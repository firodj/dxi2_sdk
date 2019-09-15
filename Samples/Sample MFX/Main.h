#ifndef _MAIN_H_
#define _MAIN_H_

#include <MidiFilter.h>
#include <olectl.h>	// for IPropertyPage	(or use ocidl.h)

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif

/////////////////////////////////////////////////////////////////////////////

// WARNING: Do not copy these into another project.
// You MUST run UUIGDEN to generate your own GUIDs!!!
const GUID CLSID_MidiFilter = { /* 166ff9a0-ea77-11d1-a8e0-0000a0090daf */
	0x166ff9a0,
	0xea77,
	0x11d1,
	{0xa8, 0xe0, 0x00, 0x00, 0xa0, 0x09, 0x0d, 0xaf}
};

const GUID CLSID_MidiFilterProps = { /* 166ff9a1-ea77-11d1-a8e0-0000a0090daf */
	0x166ff9a1,
	0xea77,
	0x11d1,
	{0xa8, 0xe0, 0x00, 0x00, 0xa0, 0x09, 0x0d, 0xaf}
};


/////////////////////////////////////////////////////////////////////////////
// CMidiFilter

class CMidiFilter:	public IMfxEventFilter,
							public IPersistStream,
							public ISpecifyPropertyPages
{
public:
	// Ctors
	CMidiFilter();
	virtual ~CMidiFilter();

	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)		AddRef();
	STDMETHODIMP_(ULONG)		Release();

	// *** IMfxEventFilter methods ***
	STDMETHODIMP Connect( IUnknown* pContext );
	STDMETHODIMP Disconnect();
	STDMETHODIMP OnStart( LONG lTime, IMfxEventQueue* pqOut );
	STDMETHODIMP OnLoop( LONG lTimeRestart, LONG lTimeStop, IMfxEventQueue* pqOut );
	STDMETHODIMP OnStop( LONG lTime, IMfxEventQueue* pqOut );
	STDMETHODIMP OnEvents( LONG lTimeFrom, LONG lTimeThru, IMfxEventQueue* pqIn, IMfxEventQueue* pqOut );
	STDMETHODIMP OnInput( IMfxDataQueue* pqIn, IMfxDataQueue* pqOut );

	// *** IPersistStream methods ***
	STDMETHODIMP GetClassID( CLSID* pClsid );
	STDMETHODIMP IsDirty();
	STDMETHODIMP Load( IStream* pStm );
	STDMETHODIMP Save( IStream* pStm, BOOL bClearDirty );
	STDMETHODIMP GetSizeMax( ULARGE_INTEGER* pcbSize );

	// *** ISpecifyPropertyPages methods ***
	STDMETHODIMP GetPages( CAUUID* pPages );

// Attributes
	int GetReps();
	void SetReps( int nReps );
	static int GetMinReps() { return 1; }
	static int GetMaxReps() { return 10; }

	void SetDirty() { m_bDirty = TRUE; }
	BOOL GetDirty() const { return m_bDirty; }

// Implementation
private:
	LONG					m_cRef;
	CRITICAL_SECTION	m_cs;		// critical section to guard properties
	int					m_nReps;
	BOOL					m_bDirty;
	IMfxTempoMap*		m_pTempoMap;
	IMfxInputPulse*	m_pInputPulse;
	LONG					m_lInputPulseMsec;
};

/////////////////////////////////////////////////////////////////////////////
// CPropPage

class CPropPage : public IPropertyPage
{
public:
	CPropPage();
	~CPropPage();

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

	BOOL DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static BOOL CALLBACK StaticDialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	BOOL OnApplyChanges();

// Implementation
private:
	LONG						m_cRef;
	HWND						m_hDlg;
	CMidiFilter*			m_pFilter;
	IPropertyPageSite*	m_pPageSite;
	BOOL						m_bDirty;

// Private methods
	void onScroll( int nCode, int nPos );
};

/////////////////////////////////////////////////////////////////////////////

extern HMODULE g_hModule;

/////////////////////////////////////////////////////////////////////////////

#endif
