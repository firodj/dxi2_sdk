// Copyright (C) 1998- by Twelve Tone Systems, Inc..  All Rights Reserved.

#include "stdafx.h"
#include "Main.h"
#include "Resource.h"
#include <InitGuid.h>	// cause MidiFilter.h to define GUIDs in this module (vs. merely declare)
#include <MidiFilter.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

HMODULE g_hModule = NULL;			// DLL module handle

static long g_lComponents = 0;	// Count of active components
static long g_lServerLocks = 0;	// Count of locks

/////////////////////////////////////////////////////////////////////////////
//
// CMidiFilter
//
/////////////////////////////////////////////////////////////////////////////

CMidiFilter::CMidiFilter() :
	m_cRef( 0 ),
	m_nReps( 4 ),
	m_bDirty( FALSE ),
	m_pTempoMap( NULL ),
	m_pInputPulse( NULL ),
	m_lInputPulseMsec( 0 )
{ 
	::InterlockedIncrement( &g_lComponents );
	::InitializeCriticalSection( &m_cs );
}


/////////////////////////////////////////////////////////////////////////////

CMidiFilter::~CMidiFilter() 
{ 
	::DeleteCriticalSection( &m_cs );
	::InterlockedDecrement( &g_lComponents );
}

/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CMidiFilter::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMidiFilter::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}


/////////////////////////////////////////////////////////////////////////////

ULONG CMidiFilter::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////
// Attributes

int CMidiFilter::GetReps()
{
	::EnterCriticalSection( &m_cs );
	int const nReps = m_nReps;
	::LeaveCriticalSection( &m_cs );
	return nReps;
}

void CMidiFilter::SetReps( int nReps )
{
	ASSERT( GetMinReps() <= nReps && nReps <= GetMaxReps() );
	::EnterCriticalSection( &m_cs );
	m_nReps = nReps;
	::LeaveCriticalSection( &m_cs );
}

/////////////////////////////////////////////////////////////////////////////
// IMfxEventFilter

#define USE_INPUT_PULSE (0)

HRESULT CMidiFilter::Connect( IUnknown* pContext )
{
	ASSERT( m_pTempoMap == NULL );	// did we initialize and/or Disconnect() properly?
	ASSERT( m_pInputPulse == NULL );	// did we initialize and/or Disconnect() properly?

	// We're interested in the ITempoMap interface. In fact, we require it,
	// so if it's not available return E_FAIL to indicate we can't proceed.
	if (SUCCEEDED( pContext->QueryInterface( IID_IMfxTempoMap, (void**)&m_pTempoMap ) ))
	{
		if (SUCCEEDED( pContext->QueryInterface( IID_IMfxInputPulse, (void**)&m_pInputPulse ) ))
		{
#if USE_INPUT_PULSE
			// Example of using IMfxInputPulse
			VERIFY( SUCCEEDED( m_pInputPulse->GetPulseInterval( &m_lInputPulseMsec ) ) );
			TRACE1( "GetPulseInterval() returned %d\n", m_lInputPulseMsec );

			// NOTE: The first call to OnInput() may occur BEFORE this call to BeginPulse() returns!
			VERIFY( SUCCEEDED( m_pInputPulse->BeginPulse() ) );
#endif

			// Since we're not retaining a copy of the original IUnknown*, be sure to
			// call Release() on it.
			pContext->Release();
			return S_OK;
		}
		m_pTempoMap->Release();
	}
	
	// Since we're not retaining a copy of the original IUnknown*, be sure to
	// call Release() on it.
	pContext->Release();
	return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilter::Disconnect()
{
	// Since QueryInterface() did an implicit Add() back when we obtained
	// the ITempoMap*, we need to call Release() on it now.
	if (m_pTempoMap)
	{
		m_pTempoMap->Release();
		m_pTempoMap = NULL;
	}
	// Ditto for m_pInputPulse:
	if (m_pInputPulse)
	{
#if USE_INPUT_PULSE
		m_pInputPulse->EndPulse();
#endif
		m_pInputPulse->Release();
		m_pInputPulse = NULL;
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilter::OnStart( LONG lTime, IMfxEventQueue* pqOut )
{
	// NOTE: We are not allowed to retain the IMfxEventQueue* pqOut, nor
	// should we call Add() or Release() on it.

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilter::OnLoop( LONG lTimeRestart, LONG lTimeStop, IMfxEventQueue* pqOut )
{
	// NOTE: We are not allowed to retain the IMfxEventQueue* pqOut, nor
	// should we call Add() or Release() on it.

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilter::OnStop( LONG lTime, IMfxEventQueue* pqOut )
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

HRESULT CMidiFilter::OnEvents( LONG lTimeFrom, LONG lTimeThru, IMfxEventQueue* pqIn, IMfxEventQueue* pqOut )
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

	// Apply a delay to the note events
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

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilter::OnInput( IMfxDataQueue* pqIn, IMfxDataQueue* pqOut )
{
	// Apply a delay to the note events
	int nCount;
	if (SUCCEEDED( pqIn->GetCount( &nCount ) ))
	{
		for (int ix = 0; ix < nCount; ++ix)
		{
			// Only process Note events
			MfxData data;
			if (SUCCEEDED( pqIn->GetAt( ix, &data ) ))
			{
				// Is this a note on (0x90) or a note off (0x80)?
				BYTE const byKind = data.m_byStatus & 0xF0;
				if (0x90 == byKind || 0x80 == byKind)
				{
					int const nReps = GetReps();

					// Hardwired delay time size of 32nd note
					long const lDelaySize = m_pTempoMap->GetTicksPerQuarterNote() / 8;

					// Get the velocity, which is the second data byte.
					int const nOrigVel = data.m_byData2;

					for (int n = 0; n < nReps; ++n)
					{
						// Scale the velocity for this event, unless it's a note off
						if (nOrigVel != 0 && byKind != 0x80)
							data.m_byData2 = static_cast<BYTE>(scale( n, 0, nReps - 1, nOrigVel, 10 ));

						HRESULT const hr = pqOut->Add( data );
						ASSERT( hr != E_INVALIDARG );	// our fault?

						// Bump the time for next event (first event stays at its original time)
						data.m_lTime += lDelaySize;
					}
				}
				else
				{
					// Pass along unmodified
					pqOut->Add( data );
				}
			}
		}
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CMidiFilter::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (NULL == pPages->pElems)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_MidiFilterProps;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CMidiFilter::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_MidiFilter;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilter::IsDirty( void )
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiFilter::Load( IStream* pStm )
{
	int nReps;
	ULONG cbRead;
	HRESULT const hr = pStm->Read( &nReps, sizeof nReps, &cbRead );
	// Note: IStream::Read() can return S_FALSE, so don't use SUCCEEDED()
	if (S_OK == hr && sizeof nReps == cbRead)
	{
		SetReps( nReps );	// use thread-safe SetReps()
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

HRESULT CMidiFilter::Save( IStream* pStm, BOOL bClearDirty )
{
	int const nReps = GetReps();	// use thread-safe GetReps()
	ULONG cbWritten;
	HRESULT const hr = pStm->Write( &nReps, sizeof nReps, &cbWritten );
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

HRESULT CMidiFilter::GetSizeMax( ULARGE_INTEGER* pcbSize )
{
	pcbSize->LowPart = sizeof(int);
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
  	STDMETHODIMP			QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

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
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CMidiFilter* pFilter = new CMidiFilter;
	if (!pFilter)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pFilter->QueryInterface( riid, ppv );

	// Delete component if QI failed.
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
  	STDMETHODIMP			QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

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
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IClassFactory*>(*ppv)->AddRef();
	return S_OK;
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
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CPropPage* const pPropPage = new CPropPage;
	if (!pPropPage)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pPropPage->QueryInterface( riid, ppv );

	// Delete component if QI failed.
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
	if (CLSID_MidiFilter == clsid)
	{
		pFactory = new CMidiFilterFactory;  // No AddRef in constructor
	}
	else if (CLSID_MidiFilterProps == clsid)
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

static void setKeyAndValue( HKEY hKeyParent, const char* pcszKey, const char* pcszSubkey, const char* pcszValue, const char* pcszValueName = NULL )
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
			RegSetValueEx( hKey, pcszValueName, 0, REG_SZ, (BYTE*)pcszValue, strlen( pcszValue ) + 1 );
		RegCloseKey( hKey );
	}
}

/////////////////////////////////////////////////////////////////////////////

static void registerFactory( const CLSID& clsid, const char* pcszFriendlyName )
{
	// Get server location.
	char szModule[ _MAX_PATH ];
	::GetModuleFileName( g_hModule, szModule, sizeof szModule );

	// Convert the CLSID into a char.
	char szCLSID[ 128 ];
	CLSIDToString( clsid, szCLSID, sizeof szCLSID );

	// Add the CLSID to HKEY_CLASSES_ROOT\CLSID
	char szKey[ 64 ];
	strcpy( szKey, "CLSID\\" );
	strcat( szKey, szCLSID );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, pcszFriendlyName );

	// Add the pathname subkey under the CLSID key.
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, "InprocServer32", szModule );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, "InprocServer32", "Both", "ThreadingModel" );
}

/////////////////////////////////////////////////////////////////////////////

static void unregisterFactory( const CLSID& clsid )
{
	// Get server location.
	char szModule[ _MAX_PATH ];
	::GetModuleFileName( g_hModule, szModule, sizeof szModule );

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
static const char s_szFriendlyName[] = "Sample Midi Filter";
static const char s_szFriendlyNameProps[] = "Sample Midi Filter Property Page";


STDAPI DllRegisterServer()
{
	registerFactory( CLSID_MidiFilter, s_szFriendlyName );

	registerFactory( CLSID_MidiFilterProps, s_szFriendlyNameProps );

	// Add CLSID_MidiFilter to HKEY_LOCAL_MACHINE\Software\Cakewalk Music Software\MIDI Filters
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_MidiFilter, szCLSID, sizeof szCLSID );
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
	unregisterFactory( CLSID_MidiFilter );

	unregisterFactory( CLSID_MidiFilterProps );

	// Add the CLSID to HKEY_LOCAL_MACHINE\Software\Cakewalk Music Software\MIDI Filters
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_MidiFilter, szCLSID, sizeof szCLSID );
	char szKey[ 256 ];
	strcpy( szKey, SZ_MIDI_FILTER_REGKEY );
	strcat( szKey, "\\" );
	strcat( szKey, szCLSID );
	recursiveDeleteKey( HKEY_LOCAL_MACHINE, szKey );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
//
// DllMain()
//
/////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		g_hModule = hModule;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
