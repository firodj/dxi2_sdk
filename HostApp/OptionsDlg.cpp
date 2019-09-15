// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include <dsound.h>

#include "PlugInHost.h"
#include "OptionsDlg.h"

#include "DXiPlayer.h"
#include "MidiInput.h"
#include "DirectSound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog


COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionsDlg)
	m_uBufferMsec = 10;
	//}}AFX_DATA_INIT
}


void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDlg)
	DDX_Control(pDX, IDC_MIDI_INPUT, m_comboMidi);
	DDX_Control(pDX, IDC_DSOUND_OUTPUT, m_comboDSound);
	DDX_Text(pDX, IDC_EDIT1, m_uBufferMsec);
	DDV_MinMaxUInt(pDX, m_uBufferMsec, MIN_BUFFER_MSEC, MAX_BUFFER_MSEC);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		// Update the MIDI device
		int const nNewMidiInput = m_comboMidi.GetCurSel();
		if (nNewMidiInput != theDXiPlayer.GetMidiInput()->GetDeviceId())
			theDXiPlayer.GetMidiInput()->SetDeviceId( m_comboMidi.GetCurSel() );

		// Update the DSound device
		const GUID* pNewDeviceGuid = (const GUID*) m_comboDSound.GetItemData( m_comboDSound.GetCurSel() );
		if (*pNewDeviceGuid != theDXiPlayer.GetDirectSound()->GetDeviceGuid())
			theDXiPlayer.GetDirectSound()->SetDeviceGuid( *pNewDeviceGuid );

		// Update the buffer size
		if (m_uBufferMsec != theDXiPlayer.GetBufferMsec())
			theDXiPlayer.SetBufferMsec( m_uBufferMsec );
	}
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	//{{AFX_MSG_MAP(COptionsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

BOOL COptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Populate the MIDI input combobox
	UINT const cMidiIn = midiInGetNumDevs();
	UINT const idMidiIn = theDXiPlayer.GetMidiInput()->GetDeviceId();
	for (UINT ix = 0; ix < cMidiIn; ix++)
	{
		MIDIINCAPS mic;
		if (0 == midiInGetDevCaps( ix, &mic, sizeof(mic ) ))
		{
			int nSel = m_comboMidi.AddString( mic.szPname );
			if (ix == idMidiIn)
				m_comboMidi.SetCurSel( nSel );
		}
	}

	// Populate the DirectSound combobox
	DirectSoundEnumerate( DSEnumCallback, this );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

////////////////////////////////////////////////////////////////////////////////

BOOL COptionsDlg::DestroyWindow() 
{
	// Clean up allocated GUID memory
	for (int ix = 0; ix < m_comboDSound.GetCount(); ++ix)
	{
		GUID* pGuid = (GUID*) m_comboDSound.GetItemData( ix );
		if (pGuid)
			delete pGuid;
		m_comboDSound.SetItemData( ix, 0 );
	}
	
	return CDialog::DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK COptionsDlg::DSEnumCallback( GUID* pGuid, LPCSTR pszDesc, LPCSTR pszModule, void* pCtx )
{
	COptionsDlg* pDlg = (COptionsDlg*) pCtx;
	int nSel = pDlg->m_comboDSound.AddString( pszDesc );
	if (-1 != nSel)
	{
		GUID* pDataGuid = new GUID;
		if (pDataGuid)
		{
			if (pGuid)
				*pDataGuid = *pGuid;
			else
				*pDataGuid = DSDEVID_DefaultPlayback;
		}
		pDlg->m_comboDSound.SetItemData( nSel, (DWORD)pDataGuid );

		if (*pDataGuid == theDXiPlayer.GetDirectSound()->GetDeviceGuid())
			pDlg->m_comboDSound.SetCurSel( nSel );
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

