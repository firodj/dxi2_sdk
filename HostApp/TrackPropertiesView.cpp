// TrackPropertiesView.cpp : implementation file
//

#include "stdafx.h"
#include "PlugInHost.h"
#include "PlugInHostDoc.h"
#include "TrackPropertiesView.h"
#include "MfxSeq.h"
#include "DXiPlayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrackPropertiesView

IMPLEMENT_DYNCREATE(CTrackPropertiesView, CFormView)

CTrackPropertiesView::CTrackPropertiesView()
	: CFormView(CTrackPropertiesView::IDD)
{
	//{{AFX_DATA_INIT(CTrackPropertiesView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nTrkNum = 0;
	m_pSeq = NULL;
	m_pFilter = NULL;
}

CTrackPropertiesView::~CTrackPropertiesView()
{
}

void CTrackPropertiesView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTrackPropertiesView)
	DDX_Control(pDX, IDC_VOL, m_editVol);
	DDX_Control(pDX, IDC_VELPLUS, m_editVelPlus);
	DDX_Control(pDX, IDC_TRACK, m_comboTrack);
	DDX_Control(pDX, IDC_PATCH, m_comboPatch);
	DDX_Control(pDX, IDC_PAN, m_editPan);
	DDX_Control(pDX, IDC_MUTE, m_btnMute);
	DDX_Control(pDX, IDC_KEYPLUS, m_editKeyPlus);
	DDX_Control(pDX, IDC_CHANNEL, m_editChannel);
	DDX_Control(pDX, IDC_BANK, m_comboBank);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTrackPropertiesView, CFormView)
	//{{AFX_MSG_MAP(CTrackPropertiesView)
	ON_CBN_SELCHANGE(IDC_TRACK, OnSelchangeTrack)
	ON_CBN_SELCHANGE(IDC_BANK, OnSelchangeBank)
	ON_CBN_SELCHANGE(IDC_PATCH, OnSelchangePatch)
 	ON_BN_CLICKED(IDC_MUTE, OnMute)
	ON_EN_KILLFOCUS(IDC_PAN, OnKillfocusPan)
	ON_EN_KILLFOCUS(IDC_KEYPLUS, OnKillfocusKeyplus)
	ON_EN_KILLFOCUS(IDC_VELPLUS, OnKillfocusVelplus)
	ON_EN_KILLFOCUS(IDC_VOL, OnKillfocusVol)
	ON_EN_KILLFOCUS(IDC_CHANNEL, OnKillfocusChannel)
	ON_CBN_KILLFOCUS(IDC_PATCH, OnKillfocusPatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

int CTrackPropertiesView::GetIdealWidth()
{
	return 185;
}

/////////////////////////////////////////////////////////////////////////////
// CTrackPropertiesView diagnostics

#ifdef _DEBUG
void CTrackPropertiesView::AssertValid() const
{
	CFormView::AssertValid();
}

void CTrackPropertiesView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Helpers

static int getEditInt( CWnd& wnd )
{
	CString str; /* = */ wnd.GetWindowText( str );
	int n = -1;
	if (1 != sscanf( str, "%d", &n ))
		return -1;
	else
		return n;
}

/////////////////////////////////////////////////////////////////////////////
// CTrackPropertiesView message handlers

void CTrackPropertiesView::SetSeq( CMfxSeq* pSeq )
{
	m_pSeq = pSeq;

	m_comboTrack.ResetContent();

	m_editVol.EnableWindow( NULL != m_pSeq );
	m_editVelPlus.EnableWindow( NULL != m_pSeq );
	m_comboTrack.EnableWindow( NULL != m_pSeq );
	m_editPan.EnableWindow( NULL != m_pSeq );
	m_btnMute.EnableWindow( NULL != m_pSeq );
	m_editKeyPlus.EnableWindow( NULL != m_pSeq );
	m_editChannel.EnableWindow( NULL != m_pSeq );

	if (m_pSeq)
	{
		// Repopulate the track list
		BOOL bFirst = TRUE;
		for (map<int,CMfxTrack>::iterator it = m_pSeq->GetBeginTrack();
			  it != m_pSeq->GetEndTrack();
			  it++)
	  {
			CMfxTrack& trk = it->second;
			int nChan = trk.GetChannel();

			CString strLabel;
			if (strlen( trk.GetName() ))
				strLabel.Format("%d: %s", it->first+1, trk.GetName());
			else
				strLabel.Format("%d", it->first+1);

			int const nIndex = m_comboTrack.AddString( strLabel );
			m_comboTrack.SetItemData( nIndex, it->first );
			if (bFirst)
			{
				m_nTrkNum = it->first;
				m_comboTrack.SetCurSel( nIndex );
				bFirst = FALSE;
			}
		}

		// Update all controls
		OnSelchangeTrack();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::SetFilter( IBaseFilter* pFilter )
{
	m_pFilter = pFilter;

	if (m_pFilter)
	{
		// Update all controls
		OnSelchangeTrack();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnKillfocusVol() 
{
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	if (!pTrk)
		return;

	int const nVol = getEditInt( m_editVol );
	if (nVol < 0 || nVol > 127)
		return;

	theDXiPlayer.SendVol( pTrk->GetChannel(), nVol );
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnKillfocusPan() 
{
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	if (!pTrk)
		return;

	int const nPan = getEditInt( m_editPan );
	if (nPan < 0 || nPan > 127)
		return;

	theDXiPlayer.SendPan( pTrk->GetChannel(), nPan );
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnSelchangeTrack() 
{
	int const nIndex = m_comboTrack.GetCurSel();
	m_nTrkNum = m_comboTrack.GetItemData( nIndex );
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	if (NULL == pTrk)
		return;

	// Set the current track so that the right just-in-time properties are applied
	theDXiPlayer.SetCurrentTrack( m_nTrkNum );

	CString str;

	str.Format("%d", pTrk->GetChannel() + 1);
	m_editChannel.SetWindowText( str );

	str.Format("%d", pTrk->GetKeyOfs());
	m_editKeyPlus.SetWindowText( str );

	str.Format("%d", pTrk->GetVelOfs());
	m_editVelPlus.SetWindowText( str );

	m_btnMute.SetCheck( 1 == pTrk->GetMuteState().Manual );

	fillBankList();
	fillPatchList();
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnSelchangeBank() 
{
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	if (!pTrk)
		return;

	int nIndex = m_comboBank.GetCurSel();
	int nBank = (-1 == nIndex) ? -1 : m_comboBank.GetItemData( nIndex );
	
	pTrk->SetBank( nBank );
	fillPatchList();
	theDXiPlayer.SendBank( pTrk->GetChannel(), pTrk->GetBank(), pTrk->GetBankSelectMethod() );
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnSelchangePatch() 
{
	OnKillfocusPatch();
}


////////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnKillfocusPatch() 
{
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	if (!pTrk)
		return;

	int nIndex = m_comboPatch.GetCurSel();
	int nPatch = -1;
	if (-1 == nIndex)
		nPatch = getEditInt( m_comboPatch );
	else
		nPatch = m_comboPatch.GetItemData( nIndex );

	if (0 <= nPatch && nPatch <= 127)
	{
		pTrk->SetPatch( nPatch );
		theDXiPlayer.SendPatch( pTrk->GetChannel(), pTrk->GetPatch() );
	}
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnKillfocusChannel() 
{
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	if (!pTrk)
		return;

	int const nVal = getEditInt( m_editChannel );
	if (nVal < 0 || nVal > 16)
		return;

	if (nVal-1 != pTrk->GetChannel())
	{
		pTrk->SetChannel( nVal-1 );
		fillBankList();
		fillPatchList();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnMute() 
{
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	if (!pTrk)
		return;

	MfxMuteMask maskSet = 0;
	MfxMuteMask maskClear = 0;
	if (m_btnMute.GetCheck())
		maskSet.Manual = 1;
	else
		maskClear.Manual = 1;

	pTrk->SetMuteState( maskSet, maskClear );
	theDXiPlayer.SendMuteState( pTrk->GetMfxChannel(), maskSet, maskClear );
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnKillfocusKeyplus() 
{
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	if (!pTrk)
		return;

	int const nOfs = getEditInt( m_editKeyPlus );
	if (nOfs < -127 || nOfs > 127)
		return;

	pTrk->SetKeyOfs( nOfs );
	theDXiPlayer.SendKeyOfs( pTrk->GetMfxChannel(), nOfs );
}

/////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnKillfocusVelplus() 
{
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	if (!pTrk)
		return;

	int const nOfs = getEditInt( m_editVelPlus );
	if (nOfs < -127 || nOfs > 127)
		return;

	pTrk->SetVelOfs( nOfs );
	theDXiPlayer.SendVelOfs( pTrk->GetMfxChannel(), nOfs );
}

////////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	CPlugInHostDoc* pDoc = DYNAMIC_DOWNCAST( CPlugInHostDoc, GetDocument() );
	if (pDoc)
		SetSeq( pDoc->GetSeq() );
	else
		SetSeq( NULL );
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
static void fillBankCombo( CComboBox& combo, int nCurBank, T* pInstr )
{
	int*		pnBanks = NULL;
	int		cBanks = 0;
	if (SUCCEEDED( pInstr->GetBanksForPatchNames( &pnBanks, &cBanks ) ))
	{
		// Populate combo box with bank names
		for (int ix = 0; ix < cBanks; ix++)
		{
			int const nBank = pnBanks[ ix ];
			IMfxNameList2* pNames = NULL;
			if (SUCCEEDED( pInstr->GetPatchNames( nBank, &pNames ) ) && NULL != pNames)
			{
				// Format the bank name
				char szTitle[ 256 ];
				pNames->GetTitle( szTitle, sizeof(szTitle) );
				CString str;
				str.Format("%d: %s", nBank, szTitle);
				pNames->Release();

				// Add the bank to the combo box, selecting it if's the current one
				int const nIndex = combo.AddString( str );
				combo.SetItemData( nIndex, nBank );
				if (nBank == nCurBank)
					combo.SetCurSel( nIndex );
			}
		}

		// COM rules: we must free pointers allocated for us
		CoTaskMemFree( (void*) pnBanks );
	}
}

////////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::fillBankList()
{
	m_comboBank.ResetContent();

	CString strNone;
	strNone.LoadString( IDS_NONE );
	int nIndex;

	// Add "--None--" to the bank list, make it our default in case we can't find one
	nIndex = m_comboBank.AddString( strNone );
	m_comboBank.SetItemData( nIndex, -1 );
	m_comboBank.SetCurSel( nIndex );

	// Enable the list only if we can change their value
	m_comboBank.EnableWindow( NULL != m_pFilter );

	// Get the track's MIDI channel, to determine its bank names
	int nChan = getEditInt( m_editChannel );

	if (NULL == m_pFilter || nChan < 1 || nChan > 16)
		return; // nothing to fill with

	// Convert channel to 0-based
	nChan--;
	
	// Get the current track's bank
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	int const nBank = (pTrk) ? pTrk->GetBank() : -1;
	
	// Check for IMfxSoftSynth2, which provides per-channel banks and patches
	IMfxSoftSynth2* pSynth2 = NULL;
	if (SUCCEEDED( m_pFilter->QueryInterface( IID_IMfxSoftSynth2, (void**)&pSynth2 ) ))
	{
		// Get the instrument for this channel
		IMfxInstrument* pInstr = NULL;
		if (SUCCEEDED( pSynth2->GetInstrument( nChan, &pInstr ) ))
		{
			fillBankCombo<IMfxInstrument>( m_comboBank, nBank, pInstr );
			pInstr->Release();
		}
		pSynth2->Release();
	}
	else
	{
		// Check for IMfxSoftSynth, which provides global banks and patches
		IMfxSoftSynth* pSynth = NULL;
		if (SUCCEEDED( m_pFilter->QueryInterface( IID_IMfxSoftSynth, (void**)&pSynth ) ))
		{
			fillBankCombo<IMfxSoftSynth>( m_comboBank, nBank, pSynth );
			pSynth->Release();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
static void fillPatchCombo( CComboBox& combo, int nCurBank, int nCurPatch, T* pInstr )
{
	IMfxNameList2* pNames = NULL;
	if (SUCCEEDED( pInstr->GetPatchNames( nCurBank, &pNames ) ) && NULL != pNames)
	{
		MFX_POSITION pos;
		if (SUCCEEDED( pNames->GetStartPosition( &pos ) ))
		{
			int	nPatch;
			char	szPatch[ 256 ];
			while (S_OK == pNames->GetNextAssoc( &pos, &nPatch, szPatch, sizeof(szPatch) ))
			{
				int const nIndex = combo.AddString( szPatch );
				combo.SetItemData( nIndex, nPatch );
				if (nPatch == nCurPatch)
					combo.SetCurSel( nIndex );
			}
		}
		pNames->Release();
	}
}

////////////////////////////////////////////////////////////////////////////////

void CTrackPropertiesView::fillPatchList()
{
	m_comboPatch.ResetContent();

	CString strNone;
	strNone.LoadString( IDS_NONE );
	int nIndex;

	// Add "--None--" to the patch list, make it our default in case we can't find one
	nIndex = m_comboPatch.AddString( strNone );
	m_comboPatch.SetItemData( nIndex, -1 );
	m_comboPatch.SetCurSel( nIndex );

	// Enable the list only if we can change their value
	m_comboPatch.EnableWindow( NULL != m_pFilter );

	// Get the track's MIDI channel, to determine its bank names
	int nChan = getEditInt( m_editChannel );

	// Get the current track's bank and patch
	CMfxTrack* pTrk = GetSeq()->GetTrack( m_nTrkNum );
	int const nBank = (pTrk) ? pTrk->GetBank() : -1;
	int const nPatch = (pTrk) ? pTrk->GetPatch() : -1;
	
	if (NULL == m_pFilter || nChan < 1 || nChan > 16)
		return; // nothing to fill with

	// Convert channel to 0-based
	nChan--;
	
	// Check for IMfxSoftSynth2, which provides per-channel banks and patches
	IMfxSoftSynth2* pSynth2 = NULL;
	if (SUCCEEDED( m_pFilter->QueryInterface( IID_IMfxSoftSynth2, (void**)&pSynth2 ) ))
	{
		// Get the instrument for this channel
		IMfxInstrument* pInstr = NULL;
		if (SUCCEEDED( pSynth2->GetInstrument( nChan, &pInstr ) ))
		{
			fillPatchCombo<IMfxInstrument>( m_comboPatch, nBank, nPatch, pInstr );
			pInstr->Release();
		}
		pSynth2->Release();
	}
	else
	{
		// Check for IMfxSoftSynth, which provides global banks and patches
		IMfxSoftSynth* pSynth = NULL;
		if (SUCCEEDED( m_pFilter->QueryInterface( IID_IMfxSoftSynth, (void**)&pSynth ) ))
		{
			fillPatchCombo<IMfxSoftSynth>( m_comboPatch, nBank, nPatch, pSynth );
			pSynth->Release();
		}
	}

	// If we couldn't select a real patch, and we have a valid patch number, use it
	nIndex = m_comboPatch.GetCurSel();
	if (nIndex < 1 && -1 != nPatch)
	{
		CString str;
		str.Format("%d", nPatch);
		m_comboPatch.SetWindowText( str );
	}
}

////////////////////////////////////////////////////////////////////////////////
