// AutoSynth.h: interface for the CAutoSynth class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOSYNTH_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
#define AFX_AUTOSYNTH_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct DXiEvent;
struct MfxEvent;

#include "DXi.h"

class CAutoSynth :
	public CDXi
{
public:
	CAutoSynth( CDXiSynthContext* pDXi, HRESULT* phr );
	virtual ~CAutoSynth();

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

// Implementation
private:

	HRESULT	configurePins();
	HRESULT	initTables();
	void		panToLRGain( int nPan, float* pfGainL, float* pfGainR );
	HRESULT	synthNote( float* pf, DXiEvent& de, long lSampToPlay, BOOL bFadeTail );

	enum EWaveShape { SineWave, SquareWave, SawWave, MAX_WAVESHAPE };

	struct NoteState
	{
		EWaveShape	eWaveShape;
		long			lSampDuration;
		long			lSampCurrent;
		double		dTheta;
		double		dStep;
	};

	struct MidiChan
	{
		BYTE	abyCC[ 128 ];
		BYTE	byPatch;
		float	fWheelPitch;
	};

	MidiChan m_aMidiChan[ 16 ];
	DWORD		m_aidOutPin[ MAX_WAVESHAPE ];
	DWORD		m_idMixOutPin;
	double	m_in[2][4];
	double	m_out[2][4];

	static double	m_adFreq[ 128 ];
	static float	m_afPanTable[ 65 ][ 2 ];
};

#endif // !defined(AFX_AUTOSYNTH_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
