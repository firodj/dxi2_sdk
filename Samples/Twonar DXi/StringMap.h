// StringMap.h: interface for the CStringMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGMAP_H__D4E51B8D_B432_4ED9_B5E3_45C87FDABE12__INCLUDED_)
#define AFX_STRINGMAP_H__D4E51B8D_B432_4ED9_B5E3_45C87FDABE12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class CStringMap : public IMfxNameList2
{
public:
	CStringMap( const char* pszTitle, int nMaxNames );
	virtual ~CStringMap();

	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppvObject );
  	STDMETHODIMP_(ULONG)		AddRef( void );
	STDMETHODIMP_(ULONG)		Release( void );

	// *** IMfxNameList methods ***
	STDMETHODIMP_(HRESULT)	GetTitle( char* pszString, int cbString );
	STDMETHODIMP_(HRESULT)	GetMaxNames( int* pnCount );
	STDMETHODIMP_(HRESULT)	GetAt( int ix, char* pszName, int cbName );

	// *** IMfxNameList2 methods ***
	STDMETHODIMP_(HRESULT)	GetCount( int* pnCount );
	STDMETHODIMP_(HRESULT)	GetStartPosition( MFX_POSITION* pPos );
	STDMETHODIMP_(HRESULT)	GetNextAssoc( MFX_POSITION* pPos, int* pnKey, char* pszString, int cbString );
	STDMETHODIMP_(HRESULT)	Lookup( int nKey, char* pszString, int cbString );

	// Operations
	HRESULT SetAt( int nKey, const char* pszString );
	HRESULT RemoveKey( int nKey );
	HRESULT DeleteAll();

private:

	int					m_nRefCount;
	map<int,CString>	m_map;
	CString				m_strTitle;
	int					m_nMaxNames;
};

#endif // !defined(AFX_STRINGMAP_H__D4E51B8D_B432_4ED9_B5E3_45C87FDABE12__INCLUDED_)
