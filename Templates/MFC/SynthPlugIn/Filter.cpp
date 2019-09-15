// Filter.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include <mediaerr.h>
#include <initguid.h>
#include <DeferZeroFill.h>
#include <malloc.h>
#include "Filter.h"
#include "SynthPlugInPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#pragma comment(lib, "../../../lib/strmbasd.lib")
#else
#pragma comment(lib, "../../../lib/strmbase.lib")
#endif

#include "PlugInGUIDs.h"


/////////////////////////////////////////////////////////////////////////////
// Setup data

static AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Audio,        // Major CLSID
    &MEDIASUBTYPE_NULL       // Minor type
};

static AMOVIESETUP_PIN psudPins[] =
{
    { L"Input",             // Pin's string name
      FALSE,                // Is it rendered
      FALSE,                // Is it an output
      FALSE,                // Allowed none
      FALSE,                // Allowed many
      &CLSID_NULL,          // Connects to filter
      L"Output",            // Connects to pin
      1,                    // Number of types
      &sudPinTypes },       // Pin information
    { L"Output",            // Pin's string name
      FALSE,                // Is it rendered
      TRUE,                 // Is it an output
      FALSE,                // Allowed none
      FALSE,                // Allowed many
      &CLSID_NULL,          // Connects to filter
      L"Input",             // Connects to pin
      1,                    // Number of types
      &sudPinTypes }        // Pin information
};

static AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_Filter,          // CLSID of filter
    L"SynthPlugIn Plug-In",         // Filter's name
    MERIT_DO_NOT_USE,       // Filter merit
    2,                      // Number of pins
    psudPins                // Pin information
};

/////////////////////////////////////////////////////////////////////////////

CFactoryTemplate g_Templates[ 2 ] =
{
    { L"SynthPlugIn Plug-In"
    , &CLSID_Filter
    , CFilter::CreateInstance
    , NULL
    , &sudFilter }
	 ,
	 { L"SynthPlugIn Properties"
	 , &CLSID_FilterPropPage
	 , CSynthPlugInPropPage::CreateInstance
	 , NULL
	 , NULL }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

//--------------------------------------------------------------------------------
// Helper function to notify the graph about pin changes

static void notifyGraphChanged( IFilterGraph* pGraph )
{
	if (pGraph)
	{
		IMediaEventSink* pSink = NULL;
		if (S_OK == pGraph->QueryInterface( IID_IMediaEventSink, (void**)&pSink ))
		{
			pSink->Notify( EC_GRAPH_CHANGED, 0, 0 );
			pSink->Release();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// CFilter
////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------
// Ctor

CFilter::CFilter( TCHAR* pName, LPUNKNOWN pUnk, HRESULT* phr ) :
	CBaseFilter( pName, pUnk, this, CLSID_Filter, phr ),
	CPersistStream( pUnk, phr ),
	CSoftSynth( this, phr ),
	CSynthPlugIn( this, phr ),
	m_pinInput( NAME("Input"), this, phr, L"Input" ),
	m_listPinOutput( NAME("Output List") ),
	m_cPinOutput( 0 ),
	m_llSamplePosition( 0 )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	AfxOleLockApp();

	if (SUCCEEDED( *phr ))
	{
		// Create our DirectX automation helper object
		*phr = CMediaParams::Create( &m_pMediaParams, (IUnknown*)(IBaseFilter*)this );
	}

	if (SUCCEEDED( *phr ))
	{
		// Create default output pin
		*phr = CreateOutputPin();

		// Initialize the plug-in
		if (SUCCEEDED( *phr ))
			*phr = Initialize();
	}
}


//------------------------------------------------------------------------------
// Factory-style construction

CUnknown * WINAPI CFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT* phr )
{
	return new CFilter( NAME("Filter"), pUnk, phr );
}


//--------------------------------------------------------------------------------
// Expose setup data for registration

LPAMOVIESETUP_FILTER CFilter::GetSetupData()
{
	return &sudFilter;
}


//--------------------------------------------------------------------------------
// Expose other interfaces to the world

STDMETHODIMP CFilter::NonDelegatingQueryInterface( REFIID riid, void** ppv )
{
	if (riid == IID_IDispatch) 
		return GetInterface((IDispatch*)this, ppv);
	else if (riid == IID_ISpecifyPropertyPages) 
		return GetInterface((ISpecifyPropertyPages*)this, ppv);
	else if (riid == IID_IPersistStream) 
		return GetInterface((IPersistStream*)this, ppv);
	else if (riid == IID_IMfxSoftSynth)
		return GetInterface((IMfxSoftSynth*)this, ppv);
	else if (riid == IID_IMfxSoftSynth2)
		return GetInterface((IMfxSoftSynth2*)this, ppv);
	else if (riid == IID_IMfxNotify)
		return GetInterface((IMfxNotify*)this, ppv);
	else if (riid == IID_IMfxInputPort)
		return GetInterface((IMfxInputPort*)this, ppv);
	else if (NULL != m_pMediaParams)
	{
		if (riid == IID_IMediaParams)
			return GetInterface((IMediaParams*)m_pMediaParams, ppv);
		else if (riid == IID_IMediaParamInfo)
			return GetInterface((IMediaParamInfo*)m_pMediaParams, ppv);
		else if (riid == IID_IMediaParamsSetUICallback)
			return GetInterface((IMediaParamsSetUICallback*)m_pMediaParams, ppv);
		else if (riid == IID_IMediaParamsUICallback)
			return GetInterface((IMediaParamsUICallback*)m_pMediaParams, ppv);
	}

	return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}


//--------------------------------------------------------------------------------
// CFilter dtor

CFilter::~CFilter()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	ASSERT( !m_bMfxConnected );
	DestroyAllOutputPins();

	if (m_pMediaParams)
		m_pMediaParams->Release(), m_pMediaParams = NULL;

	AfxOleUnlockApp();
}


//--------------------------------------------------------------------------------
// Get the total number of pins in our filter

int CFilter::GetPinCount()
{
	return m_cPinOutput + 1;
}


//--------------------------------------------------------------------------------
// Get the Nth pin from our filter (CBaseFilter pure virtual override)
// Pin 0 is the input pin and all others are output pins

CBasePin* CFilter::GetPin( int n )
{
	if (n < 0 || n >= m_cPinOutput+1)
		return NULL;

	// Pin zero is the one and only input pin
	if (0 == n)
		return &m_pinInput;
	// Get the head of the output pin list
	POSITION pos = m_listPinOutput.GetHeadPosition();
	
	// Count through the output list until we reach the one they want
	CFilterOutputPin* pOutputPin = m_listPinOutput.GetNext( pos );
	while (n > 1)
	{
		if (NULL == pos)
			return NULL;
		pOutputPin = m_listPinOutput.GetNext( pos );
		n--;
	}

	return pOutputPin;
}


//--------------------------------------------------------------------------------
// Create a new output pin

HRESULT CFilter::CreateOutputPin( const char* pszPinName /* = NULL */, DWORD* pidCreated /* = NULL */ )
{
	CAutoLock	cObjectLock(m_pLock);
	HRESULT		hr = S_OK;

	// Tell the host we are about to change
	hr = audioPortsChangeBegin( m_cPinOutput + 1 );
	if (FAILED( hr ))
		return hr;

	// Give each output pin a meaningful name
	char sz[ 256 ];
	if (NULL == pszPinName)
		sprintf( sz, "Output %d", m_cPinOutput+1 );
	else
		strcpy( sz, pszPinName );

	// Convert to Unicode for DShow
	WCHAR wsz[ 256 ];
	int cch = MultiByteToWideChar( CP_ACP, NULL, sz, -1, wsz, sizeof(wsz)/sizeof(WCHAR) );
	wsz[ cch ] = 0;

	// Create the next output pin
	CFilterOutputPin* pPin = new CFilterOutputPin( NAME("Output") , this, &hr, wsz );
	if (FAILED( hr ))
		return hr;
	if (NULL == pPin)
		return E_OUTOFMEMORY;
	pPin->AddRef();

	// Give the ID back to the caller for reference
	if (pidCreated)
		*pidCreated = pPin->GetId();

	// Add it to our list
	m_cPinOutput++;
	m_listPinOutput.AddTail( pPin );

	// Tell the host we are done changing
	hr = audioPortsChangeEnd( m_cPinOutput );
	ASSERT( SUCCEEDED( hr ) );

	return S_OK;
}


//--------------------------------------------------------------------------------
// Look up a pin by its ID

CBasePin* CFilter::FindOutputPin( DWORD id ) const
{
	POSITION pos = m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_listPinOutput.GetNext( pos );
		if (pOutputPin->GetId() == id)
			return pOutputPin;
	}
	return NULL; // not found
}


//--------------------------------------------------------------------------------
// Get an output pin ID from its index

DWORD CFilter::GetOutputPinId( int nIndex ) const
{
	POSITION pos = m_listPinOutput.GetHeadPosition();
	for (int ix = 0; NULL != pos && ix < nIndex; ix++)
		m_listPinOutput.GetNext( pos );

	if (NULL == pos)
	{
		ASSERT(FALSE);
		return (DWORD)-1;
	}
	else
	{
		CFilterOutputPin*	pPin = m_listPinOutput.Get( pos );
		return pPin->GetId();
	}
}


//--------------------------------------------------------------------------------
// Destroy a specific output pin

HRESULT CFilter::DestroyOutputPin( DWORD id )
{
	CAutoLock cObjectLock(m_pLock);

	HRESULT hr = E_FAIL;

	// Find the output pin to destroy
	CFilterOutputPin* pOutputPin = NULL;
	POSITION				pos = m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		pOutputPin = m_listPinOutput.Get( pos );
		if (pOutputPin->GetId() == id)
			break;
		m_listPinOutput.GetNext( pos );
	}

	if (pOutputPin)
	{
		// Tell the host we are about to change
		hr = audioPortsChangeBegin( m_cPinOutput - 1 );
		if (FAILED( hr ))
			return hr;

		// The host should have disconnected the pin, but be paranoid just in case
		if (pOutputPin->IsConnected())
		{
			ASSERT(FALSE);
			return E_UNEXPECTED;
		}

		// Destroy the pin
		m_listPinOutput.Remove( pos );
		pOutputPin->Release();
		m_cPinOutput--;

		// Tell the host we are done changing
		hr = audioPortsChangeEnd( m_cPinOutput );
		ASSERT( SUCCEEDED( hr ) );
	}

	return hr; // not found
}


//--------------------------------------------------------------------------------
// Destroy all output pins and associated data 

HRESULT CFilter::DestroyAllOutputPins()
{
	CAutoLock	cObjectLock(m_pLock);
	HRESULT		hr = S_OK;

	// Tell the host we are about to change
	hr = audioPortsChangeBegin( 0 );
	if (FAILED( hr ))
		return hr;

	// Destroy all of our output pins
	while (m_listPinOutput.GetHead())
	{
		CFilterOutputPin* pOutputPin = m_listPinOutput.GetHead();

		// The host should have disconnected all pins, but be paranoid just in case
		if (pOutputPin->IsConnected())
		{
			ASSERT(FALSE);
			return E_UNEXPECTED;
		}

		m_listPinOutput.RemoveHead();
		pOutputPin->Release();
	}
	m_cPinOutput = 0;

	// Tell the host we are done changing
	hr = audioPortsChangeEnd( 0 );
	ASSERT( SUCCEEDED( hr ) );

	return S_OK;
}


//--------------------------------------------------------------------------------
// See if the host will permit us to change our number of audio ports

HRESULT CFilter::audioPortsChangeBegin( int nNewPinCount )
{
	// If we're connected, we can't create a new output pin without first asking the host,
	// which means if there's no way to ask the host's permission, we can't create the pin.
	if (m_bMfxConnected && !m_pNotifyHost)
		return E_FAIL;

	// Tell the host that we are about to change
	if (m_bMfxConnected)
	{
		IBaseFilter* pbf = dynamic_cast<IBaseFilter*>( this );
		if (NULL == pbf)
			return E_UNEXPECTED;
		HRESULT hr = m_pNotifyHost->OnMfxNotifyHost( MH_SYNTH_AUDIO_PORTS_CHANGE_BEGIN, pbf, nNewPinCount );
		if (FAILED( hr ))
			return hr;
	}

	return S_OK;
}

HRESULT CFilter::audioPortsChangeEnd( int nNewPinCount )
{
	// If we're connected, we can't create a new output pin without first asking the host,
	// which means if there's no way to ask the host's permission, we can't create the pin.
	if (m_bMfxConnected && !m_pNotifyHost)
		return E_FAIL;

	// Notify the graph about the change
	notifyGraphChanged( GetFilterGraph() );

	// Tell the host that we are about to change
	if (m_bMfxConnected)
	{
		IBaseFilter* pbf = dynamic_cast<IBaseFilter*>( this );
		if (NULL == pbf)
			return E_UNEXPECTED;
		HRESULT hr = m_pNotifyHost->OnMfxNotifyHost( MH_SYNTH_AUDIO_PORTS_CHANGE_END, pbf, nNewPinCount );
		if (FAILED( hr ))
			return hr;
	}

	return S_OK;
}


//--------------------------------------------------------------------------------
// Helper for checking media types

HRESULT CFilter::CheckMediaType( PIN_DIRECTION pinDir, const CMediaType* pmt )
{
	if (NULL == pmt)
		return E_POINTER;

	// Must be audio
	if (pmt->majortype != MEDIATYPE_Audio)
		return VFW_E_TYPE_NOT_ACCEPTED;
	
	// Must not be compressed
	if (pmt->bTemporalCompression)
		return VFW_E_TYPE_NOT_ACCEPTED;
	
	// Must be WAVEFORMATEX
	if (pmt->cbFormat < sizeof(WAVEFORMATEX))
		return VFW_E_TYPE_NOT_ACCEPTED;

	WAVEFORMATEX* pwfx = reinterpret_cast<WAVEFORMATEX*>( pmt->Format() );

	// Must be 16-bit PCM or 32-bit float
	BOOL const bInt16 =
		(WAVE_FORMAT_PCM == pwfx->wFormatTag && 16 == pwfx->wBitsPerSample) ||
		(WAVE_FORMAT_EXTENSIBLE == pwfx->wFormatTag && 16 == pwfx->wBitsPerSample);
	BOOL const bFloat32 =
		(WAVE_FORMAT_IEEE_FLOAT == pwfx->wFormatTag && 32 == pwfx->wBitsPerSample);
	if (!bInt16 && !bFloat32)
		return VFW_E_TYPE_NOT_ACCEPTED;

	// Must be mono or stereo
	if (1 != pwfx->nChannels && 2 != pwfx->nChannels)
		return VFW_E_TYPE_NOT_ACCEPTED;

	// Let the plug-in decide the rest
	if (PINDIR_INPUT == pinDir)
		return IsValidInputFormat( pwfx );
	else
		return IsValidOutputFormat( pwfx );
}


//--------------------------------------------------------------------------------
// CheckTransform

HRESULT CFilter::CheckTransform( const CMediaType* pmtIn, const CMediaType* pmtOut )
{
	if (NULL == pmtIn)
		return E_POINTER;
	if (NULL == pmtOut)
		return E_POINTER;

	// Make sure input/output types are valid
	HRESULT hr = S_OK;
	hr = CheckMediaType( PINDIR_INPUT, pmtIn );
	if (FAILED( hr ))
		return hr;
	hr = CheckMediaType( PINDIR_OUTPUT, pmtOut );
	if (FAILED( hr ))
		return hr;

	// Make sure sample rates are the same
	WAVEFORMATEX* pwfxIn = reinterpret_cast<WAVEFORMATEX*>( pmtIn->Format() );
	WAVEFORMATEX* pwfxOut = reinterpret_cast<WAVEFORMATEX*>( pmtOut->Format() );
	if (pwfxIn->nSamplesPerSec != pwfxOut->nSamplesPerSec)
		return VFW_E_TYPE_NOT_ACCEPTED;

	// Let the plug-in decide for sure
	return IsValidTransform( pwfxIn, pwfxOut );
}


//--------------------------------------------------------------------------------
// GetMediaType

HRESULT CFilter::GetMediaType( int iPosition, CMediaType* pmt )
{
	if (!m_pinInput.IsConnected())
		return E_UNEXPECTED;
	
	if (iPosition < 0)
		return E_INVALIDARG;
	
	if (iPosition > 0)
		return VFW_S_NO_MORE_ITEMS;
	
	// Get pointers to input/ouput formats
	WAVEFORMATEX* pwfxIn = reinterpret_cast<WAVEFORMATEX*>( m_pinInput.CurrentMediaType().Format() );
	if (NULL == pwfxIn)
		return E_FAIL;
	WAVEFORMATEX* pwfxOut = reinterpret_cast<WAVEFORMATEX*>( pmt->Format() );
	if (NULL == pwfxOut)
		return E_FAIL;

	// Assume output format is the same as input format
	*pwfxOut = *pwfxIn;

	// let the plug-in suggest something different
	return SuggestOutputFormat( pwfxOut );
}


//--------------------------------------------------------------------------------
// SetMediaType

HRESULT CFilter::SetMediaType( PIN_DIRECTION pindir, const CMediaType* pmt )
{
	CAutoLock	autoLock(m_pLock);
	HRESULT		hr = NOERROR;

	// Make sure it's a valid audio type
	hr = CheckMediaType( pindir, pmt );
	if (S_OK != hr)
		return hr;

	if (PINDIR_INPUT == pindir)
		m_wfxIn = *reinterpret_cast<WAVEFORMATEX*>( pmt->Format() );
	else if (PINDIR_OUTPUT == pindir)
		m_wfxOut = *reinterpret_cast<WAVEFORMATEX*>( pmt->Format() );

	m_pMediaParams->SetSampleRate( m_wfxIn.nSamplesPerSec );

	return NOERROR;
}


//--------------------------------------------------------------------------------
// Run: Overriden to handle no input connections, and to notify the plug-in

STDMETHODIMP CFilter::Run( REFERENCE_TIME tStart )
{
	CAutoLock cObjectLock(m_pLock);

	CBaseFilter::Run( tStart );
	if (!m_pinInput.IsConnected())
		m_pinInput.EndOfStream();

	return NOERROR;
}


//--------------------------------------------------------------------------------
// Pause: Overriden to handle no input connections, and to notify the plug-in

STDMETHODIMP CFilter::Pause()
{
	CAutoLock cObjectLock(m_pLock);

	if (State_Paused == m_State)
		return S_OK; // nothing to do

	BOOL const bPreRoll = (State_Stopped == m_State);

	// Set this to something in case we never see a media sample timestamp
	m_llSamplePosition = 0;

	// Let the base-class do the real pausing
	HRESULT hr = CBaseFilter::Pause();

	// Deal with unexpected disconnect
	if (!m_pinInput.IsConnected())
		m_pinInput.EndOfStream();

	if (bPreRoll && SUCCEEDED( hr ))
		hr = AllocateResources();

	return hr;
}


//--------------------------------------------------------------------------------
// Stop: Overriden to handle no input connections, and to notify the plug-in

STDMETHODIMP CFilter::Stop()
{
	CAutoLock cObjectLock(m_pLock);

	if (State_Stopped == m_State)
		return S_OK; // nothing to do

	// Let the base-class do the real stopping
	HRESULT hr = CBaseFilter::Stop();
	if (SUCCEEDED( hr ))
		hr = FreeResources();

	return hr;
}


//--------------------------------------------------------------------------------
// Initialize an AudioBuffer and IMediaSample for an output pin.

HRESULT CFilter::getOutputBuffer( CFilterOutputPin* pOutputPin,
											 AudioBuffer* pbufOut,
											 REFERENCE_TIME* prtStart,
											 BOOL bSyncPoint, BOOL bDiscontinuity, BOOL bPreroll )
{
	if (NULL == pOutputPin)
		return E_POINTER;
	if (NULL == pbufOut)
		return E_POINTER;
	if (!pOutputPin->IsConnected())
		return E_UNEXPECTED;

	// Ask our output pin to allocate an output media sample
	HRESULT hr = pOutputPin->GetDeliveryBuffer( &pbufOut->pms, NULL, NULL, 0 );
	if (FAILED( hr ))
		return hr;

	// Set media sample properties
	pbufOut->pms->SetTime( prtStart, NULL );
	pbufOut->pms->SetSyncPoint( bSyncPoint );
	pbufOut->pms->SetDiscontinuity( bDiscontinuity );
	pbufOut->pms->SetPreroll( bPreroll );

	// Set the output size
	pbufOut->cSamp = pbufOut->pms->GetSize() / m_wfxOut.nBlockAlign;
	pbufOut->lOffset = 0;

	// Set the pin id
	pbufOut->idPin = pOutputPin->GetId();

	// Tag the output buffer as being all zeros
	pbufOut->SetZerofill( TRUE );

	return S_OK;
}


//--------------------------------------------------------------------------------
// Deliver results and cleanup

HRESULT CFilter::deliverOutputBuffer( CFilterOutputPin* pOutputPin,
												  AudioBuffer* pbufOut,
												  HRESULT hrProcess, BOOL bCleanup )
{
	if (NULL == pbufOut)
		return E_POINTER;
	if (NULL == pbufOut->pms)
		return E_POINTER;
	if (!pOutputPin->IsConnected())
		return E_UNEXPECTED;

	HRESULT	hr = S_OK;

	// Deliver only if we need to
	if (S_OK == hrProcess)
	{
		if (pbufOut->GetZerofill())
		{
			// If the output buffer support IDeferZeroFill, there is nothing more
			// that we need to do to zerofill it: the zerofill will occur when some
			// downstream filter attempts to get the media sample's buffer pointer.
			// buffer fill if possible.
			IDeferZeroFill* pdzf = NULL;
			if (S_OK == pbufOut->pms->QueryInterface( IID_IDeferZeroFill, (void**)&pdzf ))
			{
				pdzf->Release();
			}
			else
			{
				// AudioBuffer::GetPointer will handle any zerofill for us.
				pbufOut->GetPointer();
			}
		}

		// Set the final output sample size
		hr = pbufOut->pms->SetActualDataLength( pbufOut->cSamp * m_wfxOut.nBlockAlign );
		if (SUCCEEDED( hr ))
		{
			// Tell the current output pin to deliver this sample
			hr = pOutputPin->Deliver( pbufOut->pms );
		}
	}
	else if (S_FALSE == hrProcess)
	{
		hr = pOutputPin->DeliverEndOfStream();
	}

	// Release the media sample once we're done with it
	if (bCleanup)
	{
		pbufOut->pms->Release();
		pbufOut->pms = NULL;
	}

	return hr;
}


//--------------------------------------------------------------------------------
// ISpecifyPropertyPages

STDMETHODIMP CFilter::GetPages(CAUUID *pPages)
{
	if (NULL == pPages)
		return E_POINTER;

	pPages->cElems = 1;
	pPages->pElems= (GUID*) CoTaskMemAlloc( sizeof(GUID) );
	
	if (pPages->pElems==NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[0] = CLSID_FilterPropPage;

	return NOERROR;
}


//--------------------------------------------------------------------------------
// CPersistStream

STDMETHODIMP CFilter::GetClassID(CLSID* pClsid)
{
	if (NULL == pClsid)
		return E_POINTER;
	*pClsid = CLSID_Filter;
	return NOERROR;
}

int CFilter::SizeMax()
{
	return PersistGetSize();
}

HRESULT CFilter::WriteToStream(IStream* pStream)
{
	CAutoLock cObjectLock(m_pLock);

	return PersistSave( pStream );
}

HRESULT CFilter::ReadFromStream(IStream* pStream)
{
	CAutoLock cObjectLock(m_pLock);

	return PersistLoad( pStream );
}


//--------------------------------------------------------------------------------
// IDispatch is exposed only so MFC's COlePropertyPage can connect; ever single
// method is not implemented.

HRESULT CFilter::GetTypeInfoCount( UINT* )
{
	return E_NOTIMPL;
}

HRESULT CFilter::GetTypeInfo( UINT, LCID, ITypeInfo** )
{
	return E_NOTIMPL;
}

HRESULT CFilter::GetIDsOfNames( REFIID, OLECHAR**, UINT, LCID, DISPID* )
{
	return E_NOTIMPL;
}

HRESULT CFilter::Invoke( DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT* )
{
	return E_NOTIMPL;
}


//////////////////////////////////////////////////////////////////////////////////
// CFilterInputPin
//////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------
// Ctor

CFilterInputPin::CFilterInputPin( TCHAR* pObjDesc, CFilter* pFilter, HRESULT* phr, LPCWSTR pPinName ) :
	CBaseInputPin( pObjDesc, pFilter, pFilter, phr, pPinName ),
	m_pFilter( pFilter ),
	m_bInsideCheckMediaType( FALSE )
{
	ASSERT( pFilter );
}


//--------------------------------------------------------------------------------
// CheckMediaType

HRESULT CFilterInputPin::CheckMediaType( const CMediaType* pmt )
{
	CAutoLock	autoLock(m_pLock);
	HRESULT		hr = NOERROR;

	// If we are already inside checkmedia type for this pin, return NOERROR.
	// It is possble to hookup two of the these filters and some other filter
	// like the video effects sample to get into this situation. If we don't
	// detect this situation, we will carry on looping till we blow the stack.

	if (m_bInsideCheckMediaType)
		return NOERROR;

	m_bInsideCheckMediaType = TRUE;

	// See if the wave format is supported
	hr = m_pFilter->CheckMediaType( PINDIR_INPUT, pmt );
	if (S_OK != hr)
	{
		m_bInsideCheckMediaType = FALSE;
		return hr;
	}

	// Make sure all output pins agree on this type
	POSITION pos = m_pFilter->m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
		if (NULL != pOutputPin && pOutputPin->IsConnected())
		{
			// See if we can transform from input to the current output type
			const CMediaType& mtOut = pOutputPin->CurrentMediaType();
			hr = m_pFilter->CheckTransform( pmt, &mtOut );
			if (FAILED( hr ))
			{
				m_bInsideCheckMediaType = FALSE;
				return VFW_E_TYPE_NOT_ACCEPTED;
			}
		}
	}
	
	// Either all the downstream pins have accepted or there are none.
	m_bInsideCheckMediaType = FALSE;
	return NOERROR;
}


//--------------------------------------------------------------------------------
// Set the media type for this connection

HRESULT CFilterInputPin::SetMediaType( const CMediaType* pmtIn )
{
	// Set the base class media type (should always succeed)
	HRESULT hr = CBaseInputPin::SetMediaType( pmtIn );
	ASSERT( SUCCEEDED( hr ) );
	
	// Check the transform can be done (should always succeed)
	hr = m_pFilter->CheckMediaType( PINDIR_INPUT, pmtIn );
	ASSERT( SUCCEEDED( hr ) );
	
	return m_pFilter->SetMediaType( PINDIR_INPUT, pmtIn );
}


//--------------------------------------------------------------------------------
// CompleteConnect: Negotiate possible reconnection if types don't match

HRESULT CFilterInputPin::CompleteConnect( IPin* pRecievePin )
{
	HRESULT hr = S_OK;

	const CMediaType& mtIn = m_pFilter->m_pinInput.CurrentMediaType();

	// Make sure all output pins agree on this type
	POSITION pos = m_pFilter->m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
		if (NULL != pOutputPin && pOutputPin->IsConnected())
		{
			// See if we can transform from input to the current output type
			const CMediaType& mtOut = pOutputPin->CurrentMediaType();
			hr = m_pFilter->CheckTransform( &mtIn, &mtOut );
			if (FAILED( hr ))
			{
				if (NULL == m_pFilter->m_pGraph)
					hr = VFW_E_NOT_IN_GRAPH;
				else
					hr = m_pFilter->m_pGraph->Reconnect( pOutputPin );
			}
			if (FAILED( hr ))
				break;
		}
	}

	return hr;
}


//--------------------------------------------------------------------------------
// Receive: override to send message to all downstream pins

HRESULT CFilterInputPin::Receive( IMediaSample* pms )
{
	CAutoLock	autoLock(m_pLock);
	HRESULT		hr = NOERROR;

	ASSERT( pms );

	// Grab the time stamp on discontinuity samples.  We'll synthesize
	// time stamps for all the rest.
	REFERENCE_TIME		rtStart = 0;
	REFERENCE_TIME*	prtStart = NULL;
	if (S_OK == pms->IsDiscontinuity())
	{
		REFERENCE_TIME rtEnd;
		HRESULT hrGetTime = pms->GetTime( &rtStart, &rtEnd );
		if (S_OK == hrGetTime || VFW_S_NO_STOP_TIME == hrGetTime)
		{
			m_pFilter->m_llSamplePosition = (rtStart * m_pFilter->m_wfxIn.nSamplesPerSec) / UNITS;
			prtStart = &rtStart;
		}
	}

	// Determine the total number of samples to be processed
	long const cSampTotal = pms->GetActualDataLength() / m_pFilter->m_wfxIn.nBlockAlign;

	// Set up an input AudioBuffer
	AudioBuffer bufIn;
	bufIn.cSamp = cSampTotal;
	bufIn.pms = pms;

	// Check to see if the input buffer is defer zero fill
	IDeferZeroFill* pdzf = NULL;
	if (S_OK == pms->QueryInterface( IID_IDeferZeroFill, (void**)&pdzf ))
	{
		bufIn.SetZerofill( pdzf->get_NeedsZerofill() );
		pdzf->Release();
	}
	else
		bufIn.SetZerofill( FALSE );

	// Determine the set of decimation points for processing
	long const lFs = m_pFilter->m_wfxIn.nSamplesPerSec;
	int const cbSampIn = m_pFilter->m_wfxIn.nBlockAlign;
	int const cbSampOut = m_pFilter->m_wfxOut.nBlockAlign;
	std::vector<LONGLONG> samplePos;
	hr = m_pFilter->m_pMediaParams->GetDecimationTimes( m_pFilter->m_llSamplePosition, m_pFilter->m_llSamplePosition + bufIn.cSamp, &samplePos );
	if (FAILED( hr ))
		return hr;

	// Get output buffer pointers and sizes
	AudioBuffer*	abufOut = (AudioBuffer*) _alloca( m_pFilter->GetOutputPinCount() * sizeof(AudioBuffer) );
	int				cBufOut = 0;
	int				ixBufOut = 0;
	POSITION			pos = m_pFilter->m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
		if (NULL != pOutputPin && pOutputPin->IsConnected())
		{
			hr = m_pFilter->getOutputBuffer( pOutputPin,
														&abufOut[ cBufOut ],
														prtStart,
														S_OK == pms->IsSyncPoint(),
														S_OK == pms->IsDiscontinuity(),
														S_OK == pms->IsPreroll() );
			if (FAILED( hr ))
				return hr;
			++cBufOut;
		}
	}

	// Set output buffer sizes = input buffer size
	for (int ix = 0; ix < cBufOut; ++ix)
	{
		if (abufOut[ ix ].cSamp < bufIn.cSamp)
		{
			ASSERT(FALSE);
			return E_FAIL;
		}
		abufOut[ ix ].cSamp = bufIn.cSamp;
		abufOut[ ix ].pms->SetActualDataLength( pms->GetActualDataLength() );
	}

	HRESULT hrProcess = S_OK;
	LONGLONG llPrevPos = m_pFilter->m_llSamplePosition;
	LONGLONG llCurrPos = llPrevPos;
	long cSampDone = 0;
	for (std::vector<LONGLONG>::iterator it = samplePos.begin(); it != samplePos.end(); it++)
	{
		// Get the next "landmark", i.e., the next point in time where either a
		// shape starts or ends, or where we periodically decimate.
		llCurrPos = *it;

		// Position all automation parameters
		hr = m_pFilter->m_pMediaParams->UpdateValuesForSample( llPrevPos );
		if (FAILED( hr ))
			return hr;

		// Don't process if we haven't changed position
		if (llCurrPos == llPrevPos)
			continue;

		// Determine the number of samples processed in this iteration
		long const cSampIter = long( min( llCurrPos - llPrevPos, cSampTotal - cSampDone ) );
		if (0 == cSampIter)
			continue; // nothing to do this iteration

		// Set the buffer sizes and offsets
		bufIn.cSamp = cSampIter;
		for (ix = 0; ix < cBufOut; ++ix)
			abufOut[ ix ].cSamp = cSampIter;

		// Get ready for MIDI processing
		hr = m_pFilter->ActivatePendingEvents( cSampIter );
		if (FAILED( hr ))
			return hr;

		// Process the audio + MIDI
		hrProcess = m_pFilter->Process( llPrevPos, &bufIn, abufOut, cBufOut,
												  m_pFilter->m_llSampClock, m_pFilter->m_qActive );

		// Update buffer offsets
		bufIn.lOffset += (cSampIter * cbSampIn);
		ASSERT( bufIn.lOffset <= (cSampTotal * cbSampIn) );
		for (ix = 0; ix < cBufOut; ++ix)
		{
			abufOut[ ix ].lOffset += (cSampIter * cbSampOut);
			ASSERT( abufOut[ ix ].lOffset <= (cSampTotal * cbSampOut) );
		}

		// On to the next...		
		llPrevPos = llCurrPos;
		cSampDone += cSampIter;
		m_pFilter->m_llSampClock += cSampIter;

		// Clean up after MIDI processing
		hr = m_pFilter->ExpireEvents( FALSE );
		if (FAILED( hr ))
			return hr;
	}

	// Process the final buffer
	if (SUCCEEDED( hrProcess ) && cSampDone < cSampTotal)
	{
		// Position all automation parameters
		hr = m_pFilter->m_pMediaParams->UpdateValuesForSample( llPrevPos );
		if (FAILED( hr ))
			return hr;

		// Set the final buffer size
		bufIn.cSamp = cSampTotal - cSampDone;
		for (ix = 0; ix < cBufOut; ++ix)
			abufOut[ ix ].cSamp = bufIn.cSamp;

		// Get ready for MIDI processing
		hr = m_pFilter->ActivatePendingEvents( bufIn.cSamp );
		if (FAILED( hr ))
			return hr;

		// Process the audio + MIDI
		hrProcess = m_pFilter->Process( llPrevPos, &bufIn, abufOut, cBufOut,
												  m_pFilter->m_llSampClock, m_pFilter->m_qActive );
		m_pFilter->m_llSampClock += bufIn.cSamp;

		// Clean up after MIDI processing
		hr = m_pFilter->ExpireEvents( FALSE );
		if (FAILED( hr ))
			return hr;
	}

	// Reset final buffer sizes
	for (ix = 0; ix < cBufOut; ++ix)
	{
		abufOut[ ix ].lOffset = 0;
		abufOut[ ix ].cSamp = cSampTotal;
	}

	// Deliver results and cleanup
	pos = m_pFilter->m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
		if (NULL != pOutputPin && pOutputPin->IsConnected())
		{
			hr = m_pFilter->deliverOutputBuffer( pOutputPin, &abufOut[ ixBufOut ], hrProcess, TRUE );
			ixBufOut++;
		}
	}

	// Propagate any failures in processing
	if (FAILED( hrProcess ))
		hr = hrProcess;

	// Update our position
	m_pFilter->m_llSamplePosition += cSampTotal;

	// Clean up after MIDI processing
	hr = m_pFilter->ExpireEvents( FALSE );
	if (FAILED( hr ))
		return hr;

	return NOERROR;
}


//--------------------------------------------------------------------------------
// EndOfStream: override to send message to all downstream pins

HRESULT CFilterInputPin::EndOfStream()
{
	CAutoLock	autoLock(m_pLock);
	HRESULT		hr = NOERROR;

	// Get output buffer pointers and sizes
	AudioBuffer*	abufOut = (AudioBuffer*) _alloca( m_pFilter->GetOutputPinCount() * sizeof(AudioBuffer) );
	int				cBufOut = 0;
	POSITION			pos = m_pFilter->m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
		if (NULL != pOutputPin && pOutputPin->IsConnected())
		{
			hr = m_pFilter->getOutputBuffer( pOutputPin, &abufOut[ cBufOut ],
														NULL, FALSE, FALSE, FALSE );
			if (FAILED( hr ))
				return hr;
			++cBufOut;
		}
	}

	// Bail out if there's nothing to do
	if (0 == cBufOut)
		return S_OK;

	int const cSamp = abufOut[0].cSamp;

	// Set all output buffer sizes to be the same
	for (int ix = 1; ix < cBufOut; ++ix)
	{
		if (abufOut[ ix ].cSamp > cSamp)
		{
			ASSERT(FALSE);
			return E_FAIL;
		}
		abufOut[ ix ].cSamp = cSamp;
	}

	// Deliver buffers until the entire flush is complete
	for (;;)
	{
		// Get ready for MIDI processing
		hr = m_pFilter->ActivatePendingEvents( cSamp );
		if (FAILED( hr ))
			return hr;

		// Process the audio + MIDI
		HRESULT hrProcess = m_pFilter->Process( m_pFilter->m_llSamplePosition, NULL, abufOut, cBufOut,
															 m_pFilter->m_llSampClock, m_pFilter->m_qActive );

		// Deliver results, but don't cleanup yet
		int ixBufOut = 0;
		pos = m_pFilter->m_listPinOutput.GetHeadPosition();
		while (pos)
		{
			CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
			if (NULL != pOutputPin && pOutputPin->IsConnected())
			{
				hr = m_pFilter->deliverOutputBuffer( pOutputPin, &abufOut[ ixBufOut ],
																 hrProcess, FALSE );
				++ixBufOut;
			}
		}

		// Propagate any failures in processing
		if (FAILED( hrProcess ))
			hr = hrProcess;

		// Stop at end-of-stream or failure
		if (S_FALSE == hrProcess || FAILED( hr ))
			break;

		// Update our position
		m_pFilter->m_llSamplePosition += cSamp;
		m_pFilter->m_llSampClock += cSamp;

		// Clean up after MIDI processing
		hr = m_pFilter->ExpireEvents( FALSE );
		if (FAILED( hr ))
			return hr;
	}

	// Clean up output buffer memory
	int ixBufOut = 0;
	pos = m_pFilter->m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
		if (NULL != pOutputPin && pOutputPin->IsConnected())
		{
			m_pFilter->deliverOutputBuffer( pOutputPin, &abufOut[ ixBufOut ], S_FALSE, TRUE );
			ixBufOut++;
		}
	}

	return hr;
}


//--------------------------------------------------------------------------------
// BeginFlush: override to send message to all downstream pins

HRESULT CFilterInputPin::BeginFlush()
{
	CAutoLock	autoLock(m_pLock);
	HRESULT		hr = NOERROR;

	// Walk through the output pins list, sending the message downstream
	POSITION pos = m_pFilter->m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
		if (NULL != pOutputPin && pOutputPin->IsConnected())
		{
			hr = pOutputPin->DeliverBeginFlush();
			if (FAILED( hr ))
				return hr;
		}
	}

	return CBaseInputPin::BeginFlush();
}


//--------------------------------------------------------------------------------
// EndFlush: override to send message to all downstream pins

HRESULT CFilterInputPin::EndFlush()
{
	CAutoLock	autoLock(m_pLock);
	HRESULT		hr = NOERROR;

	// Walk through the output pins list, sending the message downstream
	POSITION pos = m_pFilter->m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
		if (NULL != pOutputPin && pOutputPin->IsConnected())
		{
			hr = pOutputPin->DeliverEndFlush();
			if (FAILED( hr ))
				return hr;
		}
	}

	return CBaseInputPin::EndFlush();
}


//--------------------------------------------------------------------------------
// NewSegment: override to send message to all downstream pins
                    
HRESULT CFilterInputPin::NewSegment( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate )
{
	CAutoLock	autoLock(m_pLock);
	HRESULT		hr = NOERROR;

	// Walk through the output pins list, sending the message downstream
	POSITION pos = m_pFilter->m_listPinOutput.GetHeadPosition();
	while (pos)
	{
		CFilterOutputPin* pOutputPin = m_pFilter->m_listPinOutput.GetNext( pos );
		if (NULL != pOutputPin && pOutputPin->IsConnected())
		{
			hr = pOutputPin->DeliverNewSegment( tStart, tStop, dRate );
			if (FAILED( hr ))
				return hr;
		}
	}

	return CBaseInputPin::NewSegment( tStart, tStop, dRate );
}



//////////////////////////////////////////////////////////////////////////////////
// CFilterOutputPin
//////////////////////////////////////////////////////////////////////////////////

DWORD CFilterOutputPin::m_idNext = 1;

//--------------------------------------------------------------------------------
// Ctor

CFilterOutputPin::CFilterOutputPin( TCHAR*		pObjDesc,
												CFilter*		pFilter,
												HRESULT*		phr, 
												LPCWSTR		pPinName ) :
	CBaseOutputPin( pObjDesc, pFilter, pFilter, phr, pPinName ),
	m_pFilter( pFilter ),
	m_bInsideCheckMediaType( FALSE ),
	m_id( 0 ),
	m_lRefCount( 0 ),
	m_pPosition( NULL )
{
	m_id = m_idNext;
	m_idNext++;
}

CFilterOutputPin::~CFilterOutputPin()
{
	if (m_pPosition)
		m_pPosition->Release();
	m_pPosition = NULL;
}


//--------------------------------------------------------------------------------
// NonDelegatingQueryInterface - Overriden to expose IMediaPosition and
// IMediaSeeking control interfaces

STDMETHODIMP CFilterOutputPin::NonDelegatingQueryInterface( REFIID riid, void**ppv )
{
	CheckPointer( ppv, E_POINTER );
	ValidateReadWritePtr( ppv, sizeof(PVOID) );
	*ppv = NULL;
	
	if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking)
	{
		// Create the interface now if we haven't yet
		if (m_pPosition == NULL)
		{
			HRESULT hr = CreatePosPassThru( GetOwner(), FALSE, &m_pFilter->m_pinInput, &m_pPosition );
			if (FAILED( hr ))
				return hr;
		}
		return m_pPosition->QueryInterface( riid, ppv );
	}
	else
		return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}


//--------------------------------------------------------------------------------
// NonDelegatingAddRef / NonDelegatingRelease
//
// We need override these method so that we can do proper reference counting on
// the output pins in our list, independently of the base filter that owns them.

STDMETHODIMP_(ULONG) CFilterOutputPin::NonDelegatingAddRef()
{
	CAutoLock lock(m_pLock);
	
#ifdef _DEBUG
	// Update the debug only variable maintained by the base class
	InterlockedIncrement( &m_cRef );
	ASSERT( m_cRef > 0 );
#endif
	
	// Now update our reference count
	InterlockedIncrement( &m_lRefCount );
	ASSERT( m_lRefCount > 0 );
	return m_lRefCount;
}

STDMETHODIMP_(ULONG) CFilterOutputPin::NonDelegatingRelease()
{
	CAutoLock lock(m_pLock);
	
#ifdef _DEBUG
	// Update the debug only variable in CBasePin
	InterlockedDecrement( &m_cRef );
	ASSERT( m_cRef >= 0 );
#endif
	
	// Now update our reference count
	InterlockedDecrement( &m_lRefCount );
	ASSERT( m_lRefCount >= 0 );
	
	if (0 == m_lRefCount)
	{
		ASSERT( NULL == m_pFilter->FindOutputPin( GetId() ) );
		delete this;
		return 0;
	}

	return m_lRefCount;
}


//--------------------------------------------------------------------------------
// DecideBufferSize
//
// Determine allocator properties for our output pin.
// This has to be present to override the PURE virtual class base function

HRESULT CFilterOutputPin::DecideBufferSize( IMemAllocator* pAllocator,
														  ALLOCATOR_PROPERTIES* pProp )
{
	HRESULT hr = S_OK;

	// Get properties from the input pin, if possible
	if (m_pFilter->m_pinInput.IsConnected())
	{
		IMemAllocator* pAlloc = NULL;
		if (SUCCEEDED( m_pFilter->m_pinInput.GetAllocator( &pAlloc ) ))
		{
			pAlloc->GetProperties( pProp );
			pAlloc->Release();
		}

		// Ask for a larger buffer size if necessary
		if (1 == m_pFilter->m_wfxIn.nChannels && 2 == m_pFilter->m_wfxOut.nChannels)
			pProp->cbBuffer *= 2;
	}
	
	// Set the allocator's properties
	ALLOCATOR_PROPERTIES propActual;
	hr = pAllocator->SetProperties( pProp, &propActual );
	if (SUCCEEDED( hr ))
		*pProp = propActual;
	
	return hr;
}


//--------------------------------------------------------------------------------
// EnumMediaTypes

STDMETHODIMP CFilterOutputPin::EnumMediaTypes( IEnumMediaTypes** ppEnum )
{
	CAutoLock autoLock(m_pLock);
	ASSERT( ppEnum );

	// Make sure that we are connected
	if (!m_pFilter->m_pinInput.IsConnected())
		return VFW_E_NOT_CONNECTED;

	// We will simply return the enumerator of our input pin's peer
	return m_pFilter->m_pinInput.m_Connected->EnumMediaTypes(ppEnum);
}


//--------------------------------------------------------------------------------
// CheckMediaType

HRESULT CFilterOutputPin::CheckMediaType( const CMediaType* pmt )
{
	CAutoLock autoLock(m_pLock);

	// If we are already inside checkmedia type for this pin, return NOERROR
	// It is possble to hookup two of these filters and some other filter
	// like the video effects sample to get into this situation. If we
	// do not detect this, we will loop till we blow the stack

	if (m_bInsideCheckMediaType)
		return NOERROR;

	m_bInsideCheckMediaType = TRUE;

	HRESULT hr = NOERROR;

	// See if the wave format is supported
	hr = m_pFilter->CheckMediaType( PINDIR_OUTPUT, pmt );
	if (S_OK != hr)
	{
		m_bInsideCheckMediaType = FALSE;
		return hr;
	}

	// The input needs to have been connected first
	if (!m_pFilter->m_pinInput.IsConnected())
	{
		m_bInsideCheckMediaType = FALSE;
		return VFW_E_NOT_CONNECTED;
	}

	// Make sure we can transform from one type to the other
	const CMediaType& mtIn = m_pFilter->m_pinInput.CurrentMediaType();
	hr = m_pFilter->CheckTransform( &mtIn, pmt );
	if (FAILED( hr ))
	{
		m_bInsideCheckMediaType = FALSE;
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	m_bInsideCheckMediaType = FALSE;
	return NOERROR;
}


//--------------------------------------------------------------------------------
// GetMediaType - get the media type supported by this pin

HRESULT CFilterOutputPin::GetMediaType( int iPosition, CMediaType* pmt )
{
	//  We don't have any media types if our input is not connected
	if (m_pFilter->m_pinInput.IsConnected())
	{
		return m_pFilter->GetMediaType( iPosition, pmt );
	}
	else
	{
		return VFW_S_NO_MORE_ITEMS;
	}
}


//--------------------------------------------------------------------------------
// SetMediaType

HRESULT CFilterOutputPin::SetMediaType( const CMediaType* pmt )
{
	CAutoLock autoLock(m_pLock);

	// Make sure that we have an input connected
	if (!m_pFilter->m_pinInput.IsConnected())
		return VFW_E_NOT_CONNECTED;

	// Make sure that the base class likes it
	HRESULT hr = CBaseOutputPin::SetMediaType( pmt );
	if (FAILED( hr ))
		return hr;

	// Make sure the plug-in likes it
	return m_pFilter->SetMediaType( PINDIR_OUTPUT, pmt );
}
