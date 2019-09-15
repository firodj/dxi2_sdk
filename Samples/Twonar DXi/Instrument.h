// Instrument.h: interface for the CInstrument class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INSTRUMENT_H__92DF1062_7347_4DC0_9B3A_9DF7BAA286E0__INCLUDED_)
#define AFX_INSTRUMENT_H__92DF1062_7347_4DC0_9B3A_9DF7BAA286E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStringMap;

class CInstrument : public IMfxInstrument  
{
public:

	CInstrument( const char* pszName );
	virtual ~CInstrument();

	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppvObject );
  	STDMETHODIMP_(ULONG)		AddRef( void );
	STDMETHODIMP_(ULONG)		Release( void );

	// *** IMfxInstrument ***
	STDMETHODIMP_(HRESULT)	GetInstrumentName( char* pszName, int cbName );
	STDMETHODIMP_(HRESULT)	GetBanksForPatchNames( int** panBank, int* cBank );
	STDMETHODIMP_(HRESULT)	GetIsDrumPatch( int nBank, int nPatch );
	STDMETHODIMP_(HRESULT)	GetIsDiatonicNoteNames( int nBank, int nPatch );
	STDMETHODIMP_(HRESULT)	GetPatchNames( int nBank, IMfxNameList2** ppMap );
	STDMETHODIMP_(HRESULT)	GetNoteNames( int nBank, int nPatch, IMfxNameList2** ppMap );
	STDMETHODIMP_(HRESULT)	GetControllerNames( IMfxNameList2** ppMap );
	STDMETHODIMP_(HRESULT)	GetRpnNames( IMfxNameList2** ppMap );
	STDMETHODIMP_(HRESULT)	GetNrpnNames( IMfxNameList2** ppMap );

protected:

	inline int BankPatch( int nBank, int nPatch ) const
	{
		return (nBank << 16) | (nPatch & 0xFFFF);
	}
	
	// Derived classes can override these to create more than 1 instrument
	virtual HRESULT createPatchNames( int nBank );
	virtual HRESULT createNoteNames( int nBank, int nPatch );
	virtual HRESULT createControllerNames();
	virtual HRESULT createRpnNames();
	virtual HRESULT createNrpnNames();

	static HRESULT createStringMap( CStringMap** ppMap, const char* pszName, int nMax );

private:

	CCritSec					m_csState;
	int						m_nRefCount;
	CString					m_strName;
	CStringMap*				m_pMapController;
	CStringMap*				m_pMapRpn;
	CStringMap*				m_pMapNrpn;
	map<int,CStringMap*>	m_mapBank2PatchNames;
	map<int,CStringMap*>	m_mapBankPatch2NoteNames;
};

#endif // !defined(AFX_INSTRUMENT_H__92DF1062_7347_4DC0_9B3A_9DF7BAA286E0__INCLUDED_)
