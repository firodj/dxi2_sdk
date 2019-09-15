// MFCSample.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <InitGuid.h>	// cause MidiFilter.h to define GUIDs in this module (vs. merely declare)
#include <MidiFilter.h>
#include "MFCSample.h"
#include "MFCSamplePropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

static long g_lComponents = 0;	// Count of active components
static long g_lServerLocks = 0;	// Count of locks

/////////////////////////////////////////////////////////////////////////////
//
// CMidiFilter
//
/////////////////////////////////////////////////////////////////////////////

CMFCMidiFilter::CMFCMidiFilter() :
	m_cRef( 0 ),
	m_nReps( 4 ),
	m_bDirty( FALSE ),
	m_pTempoMap( NULL )
{ 
	::InterlockedIncrement( &g_lComponents );
	::InitializeCriticalSection( &m_cs );
}


/////////////////////////////////////////////////////////////////////////////

CMFCMidiFilter::~CMFCMidiFilter() 
{ 
	::DeleteCriticalSection( &m_cs );
	::InterlockedDecrement( &g_lComponents );
}

/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CMFCMidiFilter::QueryInterface( REFIID riid, void** ppv )
{    
	if (IID_IUnknown == riid)
		*ppv = static_cast<IMfxEventFilter*>(this);
	else if (IID_IMfxEventFilter == riid)
		*ppv = static_cast<IMfxEventFilter*>(this);
	else if (IID_ISpecifyPropertyPages == riid)
		*ppv = static_cast<ISpecifyPropertyPages*>(this);
	else if (IID_IPersistStream == riid)
		*ppv = static_cast<IPersistStream*>(this);
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMFCMidiFilter::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}


/////////////////////////////////////////////////////////////////////////////

ULONG CMFCMidiFilter::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////
// Attributes

int CMFCMidiFilter::GetReps()
{
	::EnterCriticalSection( &m_cs );
	int const nReps = m_nReps;
	::LeaveCriticalSection( &m_cs );
	return nReps;
}

void CMFCMidiFilter::SetReps( UINT nReps )
{
	ASSERT( GetMinReps() <= nReps && nReps <= GetMaxReps() );
	::EnterCriticalSection( &m_cs );
	m_nReps = nReps;
	::LeaveCriticalSection( &m_cs );
}

/////////////////////////////////////////////////////////////////////////////
// IMfxEventFilter

HRESULT CMFCMidiFilter::Connect( IUnknown* pContext )
{
	ASSERT( m_pTempoMap == NULL );	// did we initialize and/or Disconnect() properly?

	// We're interested in the ITempoMap interface. In fact, we require it,
	// so if it's not available return E_FAIL to indicate we can't proceed.
	BOOL const bOK = SUCCEEDED( pContext->QueryInterface( IID_IMfxTempoMap, (void**)&m_pTempoMap ) );
	
	// Since we're not retaining a copy of the original IUnknown*, be sure to
	// call Release() on it.
	pContext->Release();
	
	return bOK ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMFCMidiFilter::Disconnect()
{
	// Since QueryInterface() did an implicit Add() back when we obtained
	// the ITempoMap*, we need to call Release() on it now.
	if (m_pTempoMap)
	{
		m_pTempoMap->Release();
		m_pTempoMap = NULL;
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMFCMidiFilter::OnStart( LONG lTime, IMfxEventQueue* pqOut )
{
	// NOTE: We are not allowed to retain the IMfxEventQueue* pqOut, nor
	// should we call Add() or Release() on it.

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMFCMidiFilter::OnLoop( LONG lTimeRestart, LONG lTimeStop, IMfxEventQueue* pqOut )
{
	// NOTE: We are not allowed to retain the IMfxEventQueue* pqOut, nor
	// should we call Add() or Release() on it.

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMFCMidiFilter::OnStop( LONG lTime, IMfxEventQueue* pqOut )
{
	// NOTE: We are not allowed to retain the IMfxEventQueue* pqOut, nor
	// should we call Add() or Release() on it.

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

inline long scale( long v, long a, long b, long A, long B )
{
	// Transforms v by scaling it from the range 'a..b' to the range 'A..B'.
	// If a==b or A==B, the function simply returns A.
	//
	// E.g.:
	// scale(0, 0,10, 0,10) == 1
	// scale(0, 0,10, 10,0) == 10
	// scale(5, 0,10, 0,20) == 10

	if (a==b || A==B)
		return A;
	return A + ((B - A) * (v - a) / (b - a));
}


//---------------------------------------------------------------------------

HRESULT CMFCMidiFilter::OnInput( IMfxDataQueue* pqIn, IMfxDataQueue* pqOut )
{
	// TODO: Process live MIDI input, too?
	return S_OK;
}


//---------------------------------------------------------------------------

// Set this to 1 to include example code that shows how to work with MfxEvent
// types that use an MFX_HBUFFER for their data member.
#define DEMO_BUFFER_EVENTS (1)

#if DEMO_BUFFER_EVENTS
static void demoBufferEventTypes( IMfxEventQueue* pqIn, const MfxEvent& event )
{
	switch( event.m_eType )
	{
		case MfxEvent::Sysx:
		case MfxEvent::Text:
		case MfxEvent::Lyric:
			break;

		default:
			return;
	}

	// To translate MFX_HBUFFER handles into pointers, we need the
	// IMfxBufferFactory interface that allocated the memory. For MFX_HBUFFER
	// handles in MfxEvents in an IMfxEventQueue, we need the buffer factory
	// associated with that event queue.
	//
	// One twist is that the GetBufferFactory() method was added since the
	// original IMfxEventQueue interface. It is in the new IMfxEventQueue2
	// interface. We must QueryInteface() to get it.
	//
	// IMfxEventFilter::OnEvents() still passes in ordinary IMfxEventQueue (not "2")
	// pointers. Why? For compatability. Although it takes some time/effort to QI()
	// for the new interface, that's only necessary if we're interested in the
	// relatively expensive allocated memory types that use MFX_HBUFFERs. So the
	// overhead to QueryInterface() is low in relation to the overall overhead
	// of working with these types of events.
	IMfxEventQueue2* pq2;
	if (SUCCEEDED( pqIn->QueryInterface( IID_IMfxEventQueue2, (void**)&pq2 ) ))
	{
		IMfxBufferFactory* pBufFact;
		if (SUCCEEDED( pq2->GetBufferFactory( &pBufFact ) ))
		{
			switch( event.m_eType )
			{
				case MfxEvent::Sysx:
				{
					void* pv;
					DWORD cb;
					if (SUCCEEDED( pBufFact->GetPointer( event.m_hBuffer, &pv, &cb ) ))
					{
						BYTE* pby = static_cast<BYTE*>(pv);	// handy alias
						for ( ; cb--; ++pby)
						{
							TRACE( "%x ", *pby );
						}
						TRACE0( "\n" );
					}
					break;
				}

				case MfxEvent::Text:
				case MfxEvent::Lyric:
				{
					void* pv;
					DWORD cb;
					if (SUCCEEDED( pBufFact->GetPointer( event.m_hBuffer, &pv, &cb ) ))
					{
						wchar_t* pwsz = static_cast<wchar_t*>(pv);	// handy alias
						
						char* psz = new char[ cb / 2 ];
						if (psz)
						{
							if (0 < ::WideCharToMultiByte( CP_ACP, 0, pwsz, -1, psz, cb / 2, NULL, NULL ))
							{
								TRACE0( psz );
								TRACE0( "\n" );
							}
							
							delete [] psz;
						}
					}
					break;
				}
			}
			
			pBufFact->Release();
		}
		pq2->Release();
	}
}
#endif // DEMO_BUFFER_EVENTS

//---------------------------------------------------------------------------

HRESULT CMFCMidiFilter::OnEvents( LONG lTimeFrom, LONG lTimeThru, IMfxEventQueue* pqIn, IMfxEventQueue* pqOut )
{
	// NOTE: We are not allowed to retain the IMfxEventQueue* pqOut, nor
	// should we call Add() or Release() on it.

#ifdef _DEBUG
	// Connect() should have been called!
	if (!m_pTempoMap)
	{
		ASSERT( FALSE );
		return E_UNEXPECTED;
	}
#endif

	// Apply a delay to the input note events
	int nCount;
	if (SUCCEEDED( pqIn->GetCount( &nCount ) ))
	{
		for (int ix = 0; ix < nCount; ++ix)
		{
			// Only process Note events
			MfxEvent event;
			if (SUCCEEDED( pqIn->GetAt( ix, &event ) ))
			{
				if (MfxEvent::Note == event.m_eType)
				{
					int const nReps = GetReps();

					// Hardwired delay time size of 32nd note
					long const lDelaySize = m_pTempoMap->GetTicksPerQuarterNote() / 8;
					int const nOrigVel = event.m_byVel;

					for (int n = 0; n < nReps; ++n)
					{
						// Scale the velocity for this event
						event.m_byVel = static_cast<BYTE>(scale( n, 0, nReps - 1, nOrigVel, 10 ));

						HRESULT const hr = pqOut->Add( event );
						ASSERT( hr != E_INVALIDARG );	// our fault?

						// Bump the time for next event (first event stays at its original time)
						event.m_lTime += lDelaySize;
					}
				}
				else
				{
					// Pass along unmodified
					pqOut->Add( event );

#if DEMO_BUFFER_EVENTS
					demoBufferEventTypes( pqIn, event );
#endif // DEMO_BUFFER_EVENTS
				}
			}
		}
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CMFCMidiFilter::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_MFCMidiFilterProps;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CMFCMidiFilter::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_MFCMidiFilter;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMFCMidiFilter::IsDirty( void )
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMFCMidiFilter::Load( IStream* pStm )
{
	ULONG cbRead;
	HRESULT const hr = pStm->Read( &m_nReps, sizeof m_nReps, &cbRead );

	// Note: IStream::Read() can return S_FALSE, so don't use SUCCEEDED()
	if (S_OK == hr && sizeof m_nReps == cbRead)
	{
		m_bDirty = FALSE;
		return S_OK;
	}
	else
	{
		// Valid IPersistStream::Load() errors include only E_OUTOFMEMORY and
		// E_FAIL. Don't simply return whatever IStream::Read() returned.
		return E_FAIL;
	}
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMFCMidiFilter::Save( IStream* pStm, BOOL bClearDirty )
{
	ULONG cbWritten;
	HRESULT const hr = pStm->Write( &m_nReps, sizeof m_nReps, &cbWritten );
	if (SUCCEEDED( hr ))
	{
		if (bClearDirty)
			m_bDirty = FALSE;
		return S_OK;
	}
	else
	{
		// Valid IPersistStream::Write() errors include only STG_E_MEDIUMFULL
		// and STG_E_CANTSAVE. Don't simply return whatever IStream::Write() returned.
		if (STG_E_MEDIUMFULL == hr)
			return STG_E_MEDIUMFULL;
		else
			return STG_E_CANTSAVE;
	}
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMFCMidiFilter::GetSizeMax( ULARGE_INTEGER* pcbSize )
{
	pcbSize->LowPart = sizeof m_nReps;
	pcbSize->HighPart = 0;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
//
// CMidiFilterFactory
//
/////////////////////////////////////////////////////////////////////////////

class CMidiFilterFactory : public IClassFactory
{
public:
	// IUnknown
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG) AddRef( void );
	STDMETHODIMP_(ULONG) Release( void );

	// IClassFactory
	STDMETHODIMP CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv );
	STDMETHODIMP LockServer( BOOL bLock ); 

	// Constructor
	CMidiFilterFactory() : m_cRef(0) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilterFactory::QueryInterface( REFIID riid, void** ppv )
{    
	if ((riid == IID_IUnknown) || (riid == IID_IClassFactory))
	{
		*ppv = static_cast<IClassFactory*>(this); 
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IClassFactory*>(*ppv)->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMidiFilterFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMidiFilterFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilterFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CMFCMidiFilter* pFilter = new CMFCMidiFilter;
	if (!pFilter)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pFilter->QueryInterface( riid, ppv );

	// Destroy the object if QI failed.
	if (FAILED( hr ))
		delete pFilter;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilterFactory::LockServer( BOOL bLock ) 
{
	if (bLock)
	{
		::InterlockedIncrement( &g_lServerLocks ); 
	}
	else
	{
		::InterlockedDecrement( &g_lServerLocks );
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
//
// CPropPageFactory
//
/////////////////////////////////////////////////////////////////////////////

class CPropPageFactory : public IClassFactory
{
public:
	// IUnknown
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG) AddRef( void );
	STDMETHODIMP_(ULONG) Release( void );

	// IClassFactory
	STDMETHODIMP CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv );
	STDMETHODIMP LockServer( BOOL bLock ); 

	// Constructor
	CPropPageFactory() : m_cRef(0) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropPageFactory::QueryInterface( REFIID riid, void** ppv )
{    
	if ((riid == IID_IUnknown) || (riid == IID_IClassFactory))
	{
		*ppv = static_cast<IClassFactory*>(this); 
		static_cast<IClassFactory*>(*ppv)->AddRef();
		return S_OK;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

ULONG CPropPageFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

ULONG CPropPageFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropPageFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CMFCSamplePropPage* pPropPage = new CMFCSamplePropPage;
	if (!pPropPage)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pPropPage->QueryInterface( riid, ppv );

	// Destroy the object if QI failed.
	if (FAILED( hr ))
		delete pPropPage;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropPageFactory::LockServer( BOOL bLock ) 
{
	if (bLock)
	{
		::InterlockedIncrement( &g_lServerLocks ); 
	}
	else
	{
		::InterlockedDecrement( &g_lServerLocks );
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
//
// DLL entry points
//
/////////////////////////////////////////////////////////////////////////////

STDAPI DllCanUnloadNow()
{
	//	Should return S_OK if no objects remain and there are no outstanding
	// locks due to the client calling IClassFactory::LockServer.
	
	if ((g_lComponents == 0) && (g_lServerLocks == 0))
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllGetClassObject( REFCLSID clsid, REFIID riid, void** ppv )
{
	IClassFactory* pFactory = NULL;
	if (CLSID_MFCMidiFilter == clsid)
	{
		pFactory = new CMidiFilterFactory;  // No AddRef in constructor
	}
	else if (CLSID_MFCMidiFilterProps == clsid)
	{
		pFactory = new CPropPageFactory;  // No AddRef in constructor
	}
	else
		return CLASS_E_CLASSNOTAVAILABLE;

	if (!pFactory)
		return E_OUTOFMEMORY;

	// Get requested interface.
	HRESULT const hr = pFactory->QueryInterface( riid, ppv );
	if (FAILED( hr ))
		delete pFactory;
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
//
// Server registration
//
/////////////////////////////////////////////////////////////////////////////

static void CLSIDToString( const CLSID& clsid, char* pszCLSID, int cb )
{
	LPOLESTR pwszCLSID;
	if (SUCCEEDED( StringFromCLSID( clsid, &pwszCLSID ) ))
	{
		// Covert from wide characters to non-wide.
		wcstombs( pszCLSID, pwszCLSID, cb );

		// Free memory.
		CoTaskMemFree( pwszCLSID );
	}
}


/////////////////////////////////////////////////////////////////////////////

static void recursiveDeleteKey( HKEY hKeyParent, const char* pcszKeyChild )
{
	// Open the child.
	HKEY hKeyChild;
	if (ERROR_SUCCESS == RegOpenKeyEx( hKeyParent, pcszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild ))
	{
		// Enumerate all of the decendents of this child.
		FILETIME time;
		char szBuffer[ 256 ];
		DWORD dwSize = sizeof szBuffer;
		while (S_OK == RegEnumKeyEx( hKeyChild, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time ))
		{
			// Delete the decendents of this child.
			recursiveDeleteKey( hKeyChild, szBuffer );
			dwSize = sizeof szBuffer;
		}

		// Close the child.
		RegCloseKey( hKeyChild );
	}

	// Delete this child.
	RegDeleteKey( hKeyParent, pcszKeyChild );
}


/////////////////////////////////////////////////////////////////////////////

static void setKeyAndValue( HKEY hKeyParent, const char* pcszKey, const char* pcszSubkey, const char* pcszValue, const char* pszName = NULL )
{
	// Copy keyname into buffer.
	char szKeyBuf[ 1024 ];
	strcpy( szKeyBuf, pcszKey );

	// Add subkey name to buffer.
	if (pcszSubkey != NULL)
	{
		strcat( szKeyBuf, "\\" );
		strcat( szKeyBuf, pcszSubkey );
	}

	// Create and open key and subkey.
	HKEY hKey;
	if (ERROR_SUCCESS == RegCreateKeyEx( hKeyParent, szKeyBuf,  0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hKey, NULL ))
	{
		if (pcszValue)
			RegSetValueEx( hKey, pszName, 0, REG_SZ, (BYTE*)pcszValue, strlen( pcszValue ) + 1 );
		RegCloseKey( hKey );
	}
}

/////////////////////////////////////////////////////////////////////////////

static void registerFactory( const CLSID& clsid, const char* pcszFriendlyName )
{
	// Get server location.
	char szModule[ _MAX_PATH ];
	::GetModuleFileName( theApp.m_hInstance, szModule, sizeof szModule );

	// Convert the CLSID into a char.
	char szCLSID[ 128 ];
	CLSIDToString( clsid, szCLSID, sizeof szCLSID );

	// Add the CLSID to HKEY_CLASSES_ROOT\CLSID
	char szKey[ 64 ];
	strcpy( szKey, "CLSID\\" );
	strcat( szKey, szCLSID );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, pcszFriendlyName );

	// Add the pathname subkey and threading mode under the CLSID key.
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, "InprocServer32", szModule );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, "InprocServer32", "Both", "ThreadingModel" );
}

/////////////////////////////////////////////////////////////////////////////

static void unregisterFactory( const CLSID& clsid )
{
	// Get server location.
	char szModule[ _MAX_PATH ];
	::GetModuleFileName( theApp.m_hInstance, szModule, sizeof szModule );

	// Convert the CLSID into a char.
	char szCLSID[ 128 ];
	CLSIDToString( clsid, szCLSID, sizeof szCLSID );

	char szKey[ 64 ];
	strcpy( szKey, "CLSID\\" );
	strcat( szKey, szCLSID );

	recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );
}

/////////////////////////////////////////////////////////////////////////////

// Friendly name of component
static const char s_szFriendlyName[] = "Sample MFC Midi Filter";
static const char s_szFriendlyNameProps[] = "Sample MFC Midi Filter Property Page";


STDAPI DllRegisterServer()
{
	registerFactory( CLSID_MFCMidiFilter, s_szFriendlyName );

	registerFactory( CLSID_MFCMidiFilterProps, s_szFriendlyNameProps );

	// Add CLSID_MidiFilter to HKEY_LOCAL_MACHINE\Software\Cakewalk Music Software\MIDI Filters
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_MFCMidiFilter, szCLSID, sizeof szCLSID );
	char szKey[ 256 ];
	strcpy( szKey, SZ_MIDI_FILTER_REGKEY );
	strcat( szKey, "\\" );
	strcat( szKey, szCLSID );
	setKeyAndValue( HKEY_LOCAL_MACHINE, szKey, NULL, NULL );
		
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{
	unregisterFactory( CLSID_MFCMidiFilter );

	unregisterFactory( CLSID_MFCMidiFilterProps );

	// Add the CLSID to HKEY_LOCAL_MACHINE\Software\Cakewalk Music Software\MIDI Filters
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_MFCMidiFilter, szCLSID, sizeof szCLSID );
	char szKey[ 256 ];
	strcpy( szKey, SZ_MIDI_FILTER_REGKEY );
	strcat( szKey, "\\" );
	strcat( szKey, szCLSID );
	recursiveDeleteKey( HKEY_LOCAL_MACHINE, szKey );

	return S_OK;
}

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CMFCSampleApp

BEGIN_MESSAGE_MAP(CMFCSampleApp, CWinApp)
	//{{AFX_MSG_MAP(CMFCSampleApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCSampleApp construction

CMFCSampleApp::CMFCSampleApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMFCSampleApp object

CMFCSampleApp theApp;
