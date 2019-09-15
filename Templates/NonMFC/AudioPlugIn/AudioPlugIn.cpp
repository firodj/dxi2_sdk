// AudioPlugIn.cpp: implementation of the CAudioPlugIn class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioPlugIn.h"

// Note: see AudioPlugIn.h to redefine PROCESS_IN_PLACE
#if PROCESS_IN_PLACE
#pragma message("***** Compiling an IN-PLACE audio plug-in *****")
#else
#pragma message("***** Compiling an NON-IN-PLACE audio plug-in *****")
#endif

//////////////////////////////////////////////////////////////////////
// Ctors

CAudioPlugIn::CAudioPlugIn( HRESULT* phr )
{
	// TODO: put all initialization code in Initialize(), below.
}

CAudioPlugIn::~CAudioPlugIn()
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::Initialize()
{
	// TODO: put all initialization code here

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::IsValidInputFormat( const WAVEFORMATEX* pwfx ) const
{
	// The plug-in base class will have already validated pwfx to ensure that
	// it is 16-bit PCM or 32-bit float, 1 or 2 channels.

	// TODO: Add any additional checks here, such as sample rate, etc.

	// By default, only 32-bit float buffers are supported.
	if (WAVE_FORMAT_IEEE_FLOAT != pwfx->wFormatTag)
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::IsValidOutputFormat( const WAVEFORMATEX* pwfx ) const
{
	// The plug-in base class will have already validated pwfx to ensure that
	// it is 16-bit PCM or 32-bit float, 1 or 2 channels.

	// TODO: Add any additional checks here, such as sample rate, etc.

	// By default, only 32-bit float buffers are supported.
	if (WAVE_FORMAT_IEEE_FLOAT != pwfx->wFormatTag)
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::IsValidTransform( const WAVEFORMATEX* pwfxIn, const WAVEFORMATEX* pwfxOut ) const
{
	// The plug-in base class will have already validated pwfxIn/pwfxOut to ensure that
	// it is 16-bit PCM or 32-bit float, 1 or 2 channels, and that both have the same
	// sample rate.

	// TODO: Add any additional checks here, such as sample rate, etc.

	// By default, only 32-bit float buffers are supported.
	if (WAVE_FORMAT_IEEE_FLOAT != pwfxIn->wFormatTag)
		return VFW_E_TYPE_NOT_ACCEPTED;
	if (WAVE_FORMAT_IEEE_FLOAT != pwfxOut->wFormatTag)
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::SuggestOutputFormat( WAVEFORMATEX* pwfx ) const
{
	// The plug-in base class will have already validated pwfx to ensure that
	// it is 16-bit PCM or 32-bit float, 1 or 2 channels, and that both have the same
	// sample rate.
	// TODO: Add any additional checks here, such as sample rate, etc.

	// pwfx is initially set to the input format.  If your plug-in doesn't need
	// to change the output format, simply return S_OK.
	// TODO: change pwfx if necessary.

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::Process( LONGLONG llSampAudioTimestamp,
										   AudioBuffer* pbufIn, 
											AudioBuffer* pbufOut )
{
	BOOL const bGenerateTail = (NULL == pbufIn);
	BOOL const bIsInPlace = (pbufIn == pbufOut);

	// Note about deferred zero filling:
	//
	// AudioBuffer will automatically take advantage of IDeferZeroFill,
	// if the host app supports it.  To avoid unnecessary and wasteful buffer
	// fills, always check the 'bZero' flag in AudioBuffer before calling
	// the GetPointer() method.  This is because calling GetPointer() will
	// trigger a zero-fill if the underlying data buffer was marked as "defer
	// zero fill."
	//
	// Similarly, to allow downstream filters to benefit from deferred
	// zero filling, be sure to set the 'bZero' flag in an AudioBuffer, if
	// your DSP code is producing a completely silent buffer.

	if (bGenerateTail)
	{
		// TODO: Add code to generate a tail if required by your plug-in.
		// Return S_OK if more effect tail data remains.  Return S_FALSE
		// if no more tail data remains.

		// Default implementation generates no tail
		return S_FALSE;
	}

	// TODO: Put your DSP code here

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::AllocateResources()
{
	// TODO: add code to here to prepare the for the start of streaming
	
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::FreeResources()
{
	// TODO: add code to here to clean up after streaming

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

int CAudioPlugIn::PersistGetSize() const
{
	int const cb
		= sizeof(DWORD)											// # of persisted parameters
		+ NUM_PARAMS * (sizeof(DWORD) + sizeof(float));	// (index,value), for each parameter

	return cb;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::PersistLoad( IStream* pStream )
{
	ULONG		cb = 0;
	HRESULT	hr = S_OK;
	
	// Get the number of persisted parameters 
	DWORD cParams = 0;
	hr = pStream->Read( &cParams, sizeof(cParams), &cb );
	if (FAILED( hr ) || cb != sizeof(cParams))
		return E_FAIL;

	// Restore each parameter
	for (DWORD ix = 0; ix < cParams; ix++)
	{
		// Get the parameter index
		DWORD dwParam = 0;
		hr = pStream->Read( &dwParam, sizeof(dwParam), &cb );
		if (FAILED( hr ) || cb != sizeof(dwParam))
			return E_FAIL;

		// Get the parameter value
		float fValue = 0;
		hr = pStream->Read( &fValue, sizeof(fValue), &cb );
		if (FAILED( hr ) || cb != sizeof(fValue))
			return E_FAIL;

		// Set the parameter value
		if (m_pMediaParams)
			m_pMediaParams->SetParam( dwParam, fValue );
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioPlugIn::PersistSave( IStream* pStream )
{
	ULONG		cb = 0;
	HRESULT	hr = S_OK;
	
	// Put the number of persisted parameters 
	DWORD cParams = NUM_PARAMS;
	hr = pStream->Write( &cParams, sizeof(cParams), &cb );
	if (FAILED( hr ) || cb != sizeof(cParams))
		return E_FAIL;

	// Save each parameter
	for (DWORD dwParam = 0; dwParam < cParams; dwParam++)
	{
		float fValue = 0;

		// Get the parameter value
		if (m_pMediaParams)
			m_pMediaParams->GetParam( dwParam, &fValue );

		// Write the parameter index
		hr = pStream->Write( &dwParam, sizeof(dwParam), &cb );
		if (FAILED( hr ) || cb != sizeof(dwParam))
			return E_FAIL;

		// Write the parameter value
		hr = pStream->Write( &fValue, sizeof(fValue), &cb );
		if (FAILED( hr ) || cb != sizeof(fValue))
			return E_FAIL;
	}

	return S_OK;
}
