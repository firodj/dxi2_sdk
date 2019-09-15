// MidiInput.cpp: implementation of the CMidiInput class.
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PlugInHost.h"
#include "MidiInput.h"
#include "DXiPlayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

CMidiInput::CMidiInput()
{
	m_hmi = NULL;
	m_id = 0;
}

CMidiInput::~CMidiInput()
{
	Terminate();
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiInput::Initialize()
{
	m_id = theApp.GetProfileInt( theApp.GetOptionsSectionName(), "MidiInId", 0 );
	HRESULT hr = SetDeviceId( m_id );
	if (FAILED( hr ))
	{
		CHECK( SetDeviceId( 0 ) );
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiInput::SetDeviceId( UINT u )
{
	CHECK( Terminate() );

	m_id = u;

	if (0 != midiInOpen( &m_hmi, m_id, (DWORD)callback, 0, CALLBACK_FUNCTION ))
		CHECK(E_FAIL);
	if (0 != midiInStart( m_hmi ))
		CHECK(E_FAIL);

	theApp.WriteProfileInt( theApp.GetOptionsSectionName(), "MidiInId", m_id );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiInput::Terminate()
{
	if (m_hmi)
	{
		midiInClose( m_hmi );
		m_hmi = NULL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

void CALLBACK CMidiInput::callback( HMIDIIN hmi, UINT wMsg, DWORD dwInstance, DWORD dw1, DWORD dw2 )
{
	if (MIM_DATA == wMsg)
		theDXiPlayer.OnMidiShortMsg( dw1, dw2 );
}

//////////////////////////////////////////////////////////////////////
