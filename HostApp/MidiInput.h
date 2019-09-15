// MidiInput.h: interface for the CMidiInput class.
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MIDIINPUT_H__97695117_B986_42C8_A0AA_C187509FA7D6__INCLUDED_)
#define AFX_MIDIINPUT_H__97695117_B986_42C8_A0AA_C187509FA7D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMidiInput  
{
public:
	CMidiInput();
	virtual ~CMidiInput();

	HRESULT Initialize();
	HRESULT Terminate();

	UINT GetDeviceId() const { return m_id; }
	HRESULT SetDeviceId( UINT u );

private:

	static void CALLBACK callback( HMIDIIN hmi, UINT wMsg, DWORD dwInstance, DWORD dw1, DWORD dw2 );

	HMIDIIN	m_hmi;
	UINT		m_id;
};

#endif // !defined(AFX_MIDIINPUT_H__97695117_B986_42C8_A0AA_C187509FA7D6__INCLUDED_)
