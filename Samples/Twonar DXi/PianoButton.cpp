// PianoButton.cpp : implementation file
//

#include "stdafx.h"
#include "twonar.h"
#include "resource.h"
#include "PianoButton.h"
#include "TwonarPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPianoButton

CPianoButton::CPianoButton( CTwonarPropPage* pParent ) : m_pParent(pParent)
{
}

CPianoButton::~CPianoButton()
{
}


BEGIN_MESSAGE_MAP(CPianoButton, CBitmapButton)
	//{{AFX_MSG_MAP(CPianoButton)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPianoButton message handlers

void CPianoButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CBitmapButton::OnKeyDown(nChar, nRepCnt, nFlags);
	m_pParent->OnKeyDown( nChar, nRepCnt, nFlags );
}

void CPianoButton::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CBitmapButton::OnKeyUp(nChar, nRepCnt, nFlags);
	m_pParent->OnKeyUp( nChar, nRepCnt, nFlags );
}

UINT CPianoButton::OnGetDlgCode() 
{
	return DLGC_STATIC | DLGC_WANTALLKEYS;
}
