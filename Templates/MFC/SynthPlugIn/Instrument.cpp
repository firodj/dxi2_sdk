// Instrument.cpp: implementation of the CInstrument class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Instrument.h"
#include "StringMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CInstrument
//////////////////////////////////////////////////////////////////////

CInstrument::CInstrument( const char* pszName ) :
	m_pMapController(NULL),
	m_pMapRpn(NULL),
	m_pMapNrpn(NULL),
	m_nRefCount(0)
{
	ASSERT( pszName );
	if (pszName)
		m_strName = pszName;
}

CInstrument::~CInstrument()
{
	// Release our own name lists
	if (m_pMapController)
		m_pMapController->Release(), m_pMapController = NULL;
	if (m_pMapRpn)
		m_pMapRpn->Release(), m_pMapRpn = NULL;
	if (m_pMapNrpn)
		m_pMapNrpn->Release(), m_pMapNrpn = NULL;

	map<int,CStringMap*>::iterator it;
	for (it = m_mapBank2PatchNames.begin(); it != m_mapBank2PatchNames.end(); ++it)
		it->second->Release();
	for (it = m_mapBankPatch2NoteNames.begin(); it != m_mapBankPatch2NoteNames.end(); ++it)
		it->second->Release();
	m_mapBank2PatchNames.clear();
	m_mapBankPatch2NoteNames.clear();
}

////////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CInstrument::QueryInterface( REFIID riid, void** ppvObject )
{
	if (IsEqualIID( riid, IID_IUnknown ))
		*ppvObject = (IUnknown*) this;
	else if (IsEqualIID( riid, IID_IMfxInstrument ))
		*ppvObject = (IMfxInstrument*) this;
	else
		return E_NOINTERFACE;
		
	AddRef();
	return S_OK;
}

ULONG CInstrument::AddRef( void )
{
	return ++m_nRefCount;
}

ULONG CInstrument::Release( void )
{
	if (0 == --m_nRefCount)
	{
		delete this;
		return 0;
	}
	return m_nRefCount;
}


////////////////////////////////////////////////////////////////////////////////
// IMfxInstrument

HRESULT CInstrument::GetInstrumentName( char* pszName, int cbName )
{
	if (NULL == pszName)
		return E_POINTER;
	if (unsigned(cbName) < m_strName.size() + 1)
		return E_POINTER;

	strcpy( pszName, m_strName.c_str() );
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::GetBanksForPatchNames( int** panBank, int* pcBank )
{
	CAutoLock cs( &m_csState );

	if (NULL == panBank || NULL == pcBank)
		return E_POINTER;

	// TODO: Allocate a larger array of int's if this synth supplies more
	// than one bank of patches.

	*panBank = (int*) CoTaskMemAlloc( 1 * sizeof(int) );
	if (NULL == *panBank)
		return E_OUTOFMEMORY;

	*pcBank = 1;
	(*panBank)[ 0 ] = 0;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::GetIsDrumPatch( int nBank, int nPatch )
{
	CAutoLock cs( &m_csState );

	if (nBank < -1 || nBank > 16383)
		return E_INVALIDARG;
	if (nPatch < -1 || nPatch > 127)
		return E_INVALIDARG;

	// TODO: Return S_OK if this synth provides a drum-kit patch
	// on the specified bank#/patch#.

	return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::GetIsDiatonicNoteNames( int nBank, int nPatch )
{
	CAutoLock cs( &m_csState );

	if (nBank < -1 || nBank > 16383)
		return E_INVALIDARG;
	if (nPatch < -1 || nPatch > 127)
		return E_INVALIDARG;

	// TODO: Return S_FALSE if this synth uses special custom note names
	// on the specified bank#/patch#.  Typically a sampling synth would
	// do so, in cases where notes are named after the sample data file(s).

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::GetPatchNames( int nBank, IMfxNameList2** ppMap )
{
	CAutoLock cs( &m_csState );

	if (nBank < -1 || nBank > 16383)
		return E_INVALIDARG;
	if (NULL == ppMap)
		return E_POINTER;

	HRESULT hr = createPatchNames( nBank );
	if (FAILED( hr ))
		return hr;

	map<int,CStringMap*>::iterator it = m_mapBank2PatchNames.find( nBank );
	if (it == m_mapBank2PatchNames.end())
	{
		ASSERT(FALSE); // should have been created by above!
		return E_FAIL;
	}

	*ppMap = it->second;
	(*ppMap)->AddRef();

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::GetNoteNames( int nBank, int nPatch, IMfxNameList2** ppMap )
{
	CAutoLock cs( &m_csState );

	if (nBank < -1 || nBank > 16383)
		return E_INVALIDARG;
	if (NULL == ppMap)
		return E_POINTER;

	HRESULT hr = createNoteNames( nBank, nPatch );
	if (FAILED( hr ))
		return hr;

	map<int,CStringMap*>::iterator it = m_mapBankPatch2NoteNames.find( BankPatch( nBank, nPatch ) );
	if (it == m_mapBankPatch2NoteNames.end())
	{
		ASSERT(FALSE); // should have been created by above!
		return E_FAIL;
	}

	*ppMap = it->second;
	(*ppMap)->AddRef();

	return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::GetControllerNames( IMfxNameList2** ppMap )
{
	CAutoLock cs( &m_csState );

	if (NULL == ppMap)
		return E_POINTER;

	HRESULT hr = createControllerNames();
	if (FAILED( hr ))
		return hr;

	m_pMapController->AddRef();
	*ppMap = m_pMapController;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::GetRpnNames( IMfxNameList2** ppMap )
{
	CAutoLock cs( &m_csState );

	if (NULL == ppMap)
		return E_POINTER;

	HRESULT hr = createRpnNames();
	if (FAILED( hr ))
		return hr;

	m_pMapRpn->AddRef();
	*ppMap = m_pMapRpn;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::GetNrpnNames( IMfxNameList2** ppMap )
{
	CAutoLock cs( &m_csState );

	if (NULL == ppMap)
		return E_POINTER;

	HRESULT hr = createNrpnNames();
	if (FAILED( hr ))
		return hr;

	m_pMapNrpn->AddRef();
	*ppMap = m_pMapNrpn;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::createPatchNames( int nBank )
{
	CAutoLock cs( &m_csState );

	// See if the name list already exists
	map<int,CStringMap*>::iterator it = m_mapBank2PatchNames.find( nBank );
	if (it != m_mapBank2PatchNames.end())
		return S_OK;

	// Create a new string map
	string		str = m_strName + " Patches";
	CStringMap*	pStringMap = NULL;
	HRESULT hr = createStringMap( &pStringMap, str.c_str(), 128 );
	if (FAILED( hr ))
		return hr;
	m_mapBank2PatchNames[ nBank ] = pStringMap;

	// TODO: add patch names to the map

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::createNoteNames( int nBank, int nPatch )
{
	CAutoLock cs( &m_csState );

	// See if the name list already exists
	map<int,CStringMap*>::iterator it = m_mapBankPatch2NoteNames.find( BankPatch( nBank, nPatch ) );
	if (it != m_mapBankPatch2NoteNames.end())
		return S_OK;

	// Create a new string map
	string		str = m_strName + " Notes";
	CStringMap*	pStringMap = NULL;
	HRESULT hr = createStringMap( &pStringMap, str.c_str(), 128 );
	if (FAILED( hr ))
		return hr;
	m_mapBankPatch2NoteNames[ BankPatch( nBank, nPatch ) ] = pStringMap;

	// TODO: add note names to the map

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::createControllerNames()
{
	CAutoLock cs( &m_csState );

	string str = m_strName + " Controllers";
	HRESULT hr = createStringMap( &m_pMapController, str.c_str(), 128 );
	if (FAILED( hr ))
		return hr;

	// TODO: add controller names to the map

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::createRpnNames()
{
	CAutoLock cs( &m_csState );

	string str = m_strName + " RPNs";
	HRESULT hr = createStringMap( &m_pMapRpn, str.c_str(), 16383 );
	if (FAILED( hr ))
		return hr;

	// TODO: add RPN names to the map

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::createNrpnNames()
{
	CAutoLock cs( &m_csState );

	string str = m_strName + " NRPNs";
	HRESULT hr = createStringMap( &m_pMapNrpn, str.c_str(), 16383 );
	if (FAILED( hr ))
		return hr;

	// TODO: add NPRN names to the map

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CInstrument::createStringMap( CStringMap** ppMap, const char* pszName, int nMax ) // static
{
	if (NULL == ppMap)
		return E_POINTER;
	if (NULL == *ppMap)
	{
		*ppMap = new CStringMap( pszName, nMax );
		if (NULL == *ppMap)
			return E_OUTOFMEMORY;
		(*ppMap)->AddRef();
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
