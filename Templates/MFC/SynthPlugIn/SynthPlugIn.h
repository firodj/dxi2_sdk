// SynthPlugIn.h: interface for the CSynthPlugIn class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYNTHPLUGIN_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
#define AFX_SYNTHPLUGIN_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct DXiEvent;
struct MfxEvent;

#include "DXi.h"

class CSynthPlugIn :
	public CDXi
{
public:
	CSynthPlugIn( CDXiSynthContext* pDXi, HRESULT* phr );
	virtual ~CSynthPlugIn();

	HRESULT Initialize();

	HRESULT IsValidInputFormat( const WAVEFORMATEX* pwfx ) const;
	HRESULT IsValidOutputFormat( const WAVEFORMATEX* pwfx ) const;
	HRESULT IsValidTransform( const WAVEFORMATEX* pwfxIn, const WAVEFORMATEX* pwfxOut ) const;
	HRESULT SuggestOutputFormat( WAVEFORMATEX* pwfx ) const;

	HRESULT Process( LONGLONG llSampAudioTimestamp,
						  AudioBuffer* pbufIn, 
						  AudioBuffer* abufOut, long cBufOut,
						  LONGLONG llSampMidiClock, deque<DXiEvent>& qMidi );

	HRESULT	IsValidEvent( DXiEvent& de );
	HRESULT	InitializeNoteEvent( DXiEvent& de );
	HRESULT	ExpireNoteEvent( DXiEvent& de, BOOL bForce );

	HRESULT AllocateResources();
	HRESULT FreeResources();

	int	  PersistGetSize() const;
	HRESULT PersistLoad( IStream* pStream );
	HRESULT PersistSave( IStream* pStream );
};

#endif // !defined(AFX_SYNTHPLUGIN_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
