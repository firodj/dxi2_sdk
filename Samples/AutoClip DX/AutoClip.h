// AutoClip.h: interface for the CAutoClip class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOCLIP_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
#define AFX_AUTOCLIP_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct DXiEvent;
struct MfxEvent;

#include "DXi.h"

// TODO: #define PROCESS_IN_PLACE to FALSE if necessary, i.e., for plug-ins
// which convert mono to stereo.
#define PROCESS_IN_PLACE (TRUE)

class CAutoClip :
	public CDXi
{
public:
	CAutoClip( HRESULT* phr );
	virtual ~CAutoClip();

	HRESULT Initialize();

	HRESULT IsValidInputFormat( const WAVEFORMATEX* pwfx ) const;
	HRESULT IsValidOutputFormat( const WAVEFORMATEX* pwfx ) const;
	HRESULT IsValidTransform( const WAVEFORMATEX* pwfxIn, const WAVEFORMATEX* pwfxOut ) const;
	HRESULT SuggestOutputFormat( WAVEFORMATEX* pwfx ) const;

	HRESULT Process( LONGLONG llSampAudioTimestamp,
						  AudioBuffer* pbufIn,
						  AudioBuffer* pbufOut );

	HRESULT AllocateResources();
	HRESULT FreeResources();

	int	  PersistGetSize() const;
	HRESULT PersistLoad( IStream* pStream );
	HRESULT PersistSave( IStream* pStream );
};

#endif // !defined(AFX_AUTOCLIP_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
