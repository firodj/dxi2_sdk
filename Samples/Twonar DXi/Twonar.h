// Twonar.h: interface for the CTwonar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TWONAR_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
#define AFX_TWONAR_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ITwonar.h"

struct DXiEvent;
struct MfxEvent;

#include "DXi.h"

class CTwonar :
	public CDXi,
	public ITwonar
{
public:
	CTwonar( CDXiSynthContext* pDXi, HRESULT* phr );
	virtual ~CTwonar();

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

	struct Params1
	{
		int	nMode;
		BOOL	bMultiOut;
	};

	struct Params2 : public Params1
	{
		BOOL	bEnableMetro;
		int	nMetroNoteMeasTop;
		int	nMetroVelMeasTop;
		int	nMetroNoteBeat;
		int	nMetroVelBeat;
	}
	m_p;

// Parameter get/put methods
public:
	STDMETHOD(get_MetroInfo)(int* pnNoteMeasTop, int* pnVelMeasTop, int* pnNoteBeat, int* pnVelBeat);
	STDMETHOD(put_MetroInfo)(int nNoteMeasTop, int nVelMeasTop, int nNoteBeat, int nVelBeat);
	STDMETHOD(get_EnableMetro)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_EnableMetro)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_IsMultiOut)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_IsMultiOut)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_Mode)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_Mode)(/*[in]*/ int newVal);

	// This little code fragment tricks DevStudio into thinking this is
	// an ATL class.  By so doing, you can easily add new parameter get
	// and put methods by right clicking the class in the class wizard.

	#if !_WINDLL // only defined at compile time
		BEGIN_COM_MAP(CTwonar)
			COM_INTERFACE_ENTRY(ITwonar)
		END_COM_MAP()
	#endif // !_WINDLL

// Implementation
private:

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

	static double	m_adFreq[ 128 ];
	static float	m_afPanTable[ 65 ][ 2 ];
};

#endif // !defined(AFX_TWONAR_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
