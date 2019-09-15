// StringMap.cpp: implementation of the CStringMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////////////
// CStringMap
////////////////////////////////////////////////////////////////////////////////

CStringMap::CStringMap( const char* pszTitle, int nMaxNames ) :
	m_nRefCount(0),
	m_nMaxNames(nMaxNames)
{
	ASSERT( NULL != pszTitle );
	if (NULL != pszTitle)
		m_strTitle = pszTitle;
}

CStringMap::~CStringMap()
{
	ASSERT( 0 == m_nRefCount );
}

////////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CStringMap::QueryInterface( REFIID riid, void** ppvObject )
{
	if (IsEqualIID( riid, IID_IUnknown ))
		*ppvObject = (IUnknown*) this;
	else if (IsEqualIID( riid, IID_IMfxNameList ))
		*ppvObject = (IMfxNameList*) this;
	else if (IsEqualIID( riid, IID_IMfxNameList2 ))
		*ppvObject = (IMfxNameList2*) this;
	else
		return E_NOINTERFACE;
		
	AddRef();
	return S_OK;
}

//------------------------------------------------------------------------------

ULONG CStringMap::AddRef( void )
{
	return ++m_nRefCount;
}

//------------------------------------------------------------------------------

ULONG CStringMap::Release( void )
{
	if (0 == --m_nRefCount)
	{
		delete this;
		return 0;
	}
	return m_nRefCount;
}


////////////////////////////////////////////////////////////////////////////////
// IMfxNameList

HRESULT CStringMap::GetTitle( char* pszString, int cbString )
{
	if (NULL == pszString)
		return E_POINTER;
	if (cbString > 0)
	{
		strncpy( pszString, m_strTitle.c_str(), cbString );
		pszString[ cbString - 1 ] = 0;
	}
	return S_OK;
}

//------------------------------------------------------------------------------

HRESULT CStringMap::GetMaxNames( int* pnMaxNames )
{
	if (NULL == pnMaxNames)
		return E_POINTER;

	*pnMaxNames = m_nMaxNames;
	return S_OK;
}

//------------------------------------------------------------------------------

HRESULT CStringMap::GetAt( int ix, char* pszName, int cbName )
{
	if (NULL == pszName || IsBadWritePtr( pszName, cbName ))
		return E_POINTER;
	if (ix < 0 || ix >= m_nMaxNames)
		return E_INVALIDARG;

	map<int,string>::iterator it = m_map.find( ix );
	if (it != m_map.end())
	{
		strncpy( pszName, (*it).second.c_str(), cbName );
		pszName[ cbName - 1 ] = 0;
	}
	else
	{
		*pszName = 0;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// IMfxNameList2

HRESULT CStringMap::GetCount( int* pnCount )
{
	if (NULL == pnCount)
		return E_POINTER;

	*pnCount = m_map.size();
	return S_OK;
}

//------------------------------------------------------------------------------

HRESULT CStringMap::GetStartPosition( MFX_POSITION* pPos )
{
	// This code assumes MFX_POSITION is the same size as our string map
	// iterator, true on MSVC 6.  The following code may complain if you
	// switch compilers or STL implementation.
	ASSERT( sizeof(MFX_POSITION) == sizeof( map<int,string>::iterator ) );
	if ( sizeof(MFX_POSITION) != sizeof( map<int,string>::iterator ) )
		return E_FAIL;

	if (NULL == pPos)
		return E_POINTER;
	*pPos = *reinterpret_cast<MFX_POSITION*>( &m_map.begin() );
	return S_OK;
}

//------------------------------------------------------------------------------

HRESULT CStringMap::GetNextAssoc( MFX_POSITION* pPos, int* pnKey, char* pszString, int cbString )
{
	// This code assumes MFX_POSITION is the same size as our string map
	// iterator, true on MSVC 6.  The following code may complain if you
	// switch compilers or STL implementation.
	ASSERT( sizeof(MFX_POSITION) == sizeof( map<int,string>::iterator ) );
	if ( sizeof(MFX_POSITION) != sizeof( map<int,string>::iterator ) )
		return E_FAIL;

	if (NULL == pszString || IsBadWritePtr( pszString, cbString ))
		return E_POINTER;
	if (NULL == pPos)
		return E_POINTER;

	// Are we at the end of the list?
	if (NULL == *pPos)
		return S_FALSE;

	// Get the current position
	map<int,string>::iterator it = *reinterpret_cast<map<int,string>::iterator*>( pPos );

	// Are we at the end of the list?
	if (m_map.end() == it)
		return S_FALSE;

	// Retrieve key and/or value
	if (NULL != pnKey)
	{
		*pnKey = (*it).first;
	}
	if (NULL != pszString && cbString > 0)
	{
		strncpy( pszString, (*it).second.c_str(), cbString );
		pszString[ cbString - 1 ] = 0;
	}

	// Increment the position
	it++;
	*pPos = *reinterpret_cast<MFX_POSITION*>( &it );

	return S_OK;
}

//------------------------------------------------------------------------------

HRESULT CStringMap::Lookup( int nKey, char* pszString, int cbString )
{
	if (NULL == pszString || IsBadWritePtr( pszString, cbString ))
		return E_POINTER;

	map<int,string>::iterator it = m_map.find( nKey );
	if (it != m_map.end())
	{
		if (NULL != pszString && cbString > 0)
		{
			strncpy( pszString, (*it).second.c_str(), cbString );
			pszString[ cbString - 1 ] = 0;
		}
		return S_OK;
	}
	else
		return S_FALSE;
}


////////////////////////////////////////////////////////////////////////////////
// Operations

HRESULT CStringMap::SetAt( int nKey, const char* pszString )
{
	string str( pszString );
	m_map[ nKey ] = str;
	return S_OK;
}

//------------------------------------------------------------------------------

HRESULT CStringMap::RemoveKey( int nKey )
{
	m_map.erase( nKey );
	return S_OK;
}

//------------------------------------------------------------------------------

HRESULT CStringMap::DeleteAll()
{
	m_map.clear();
	return S_OK;
}

//------------------------------------------------------------------------------

