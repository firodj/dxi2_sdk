// PlugInHostDoc.cpp : implementation of the CPlugInHostDoc class
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//

#include "stdafx.h"
#include "PlugInHost.h"

#include "PlugInHostDoc.h"
#include "MfxSeq.h"
#include "DXiPlayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlugInHostDoc

IMPLEMENT_DYNCREATE(CPlugInHostDoc, CDocument)

BEGIN_MESSAGE_MAP(CPlugInHostDoc, CDocument)
	//{{AFX_MSG_MAP(CPlugInHostDoc)
	ON_UPDATE_COMMAND_UI(ID_PLAY, OnUpdatePlay)
	ON_COMMAND(ID_PLAY, OnPlay)
	ON_UPDATE_COMMAND_UI(ID_STOP, OnUpdateStop)
	ON_COMMAND(ID_STOP, OnStop)
	ON_UPDATE_COMMAND_UI(ID_REW, OnUpdateRew)
	ON_COMMAND(ID_REW, OnRew)
	ON_UPDATE_COMMAND_UI(ID_LOOP, OnUpdateLoop)
	ON_COMMAND(ID_LOOP, OnLoop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlugInHostDoc construction/destruction

CPlugInHostDoc::CPlugInHostDoc()
{
	m_pSeq = NULL;
}

CPlugInHostDoc::~CPlugInHostDoc()
{
	theDXiPlayer.SetSeq( NULL );
	SAFE_DELETE( m_pSeq );
}

BOOL CPlugInHostDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Destroy the current seq
	theDXiPlayer.SetSeq( NULL );
	SAFE_DELETE( m_pSeq );

	// Create a new empty CMfxSeq
	m_pSeq = new CMfxSeq;
	if (NULL == m_pSeq)
		return FALSE;

	// Point the transport to our CMfxSeq
	theDXiPlayer.SetSeq( m_pSeq );

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CPlugInHostDoc serialization

void CPlugInHostDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// Try loading the specified MID file
		CMfxSeq* pSeqNew = new CMfxSeq;
		if (NULL == pSeqNew)
			throw new CFileException;

		if (FAILED( LoadMid( *pSeqNew, *ar.GetFile() ) ))
		{
			SAFE_DELETE( pSeqNew );
			throw new CFileException( CFileException::invalidFile );
		}

		// Set our new CMfxSeq
		theDXiPlayer.SetSeq( NULL );
		SAFE_DELETE( m_pSeq );

		// Point the transport to our CMfxSeq
		m_pSeq = pSeqNew;
		theDXiPlayer.SetSeq( m_pSeq );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPlugInHostDoc diagnostics

#ifdef _DEBUG
void CPlugInHostDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPlugInHostDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CPlugInHostDoc commands

void CPlugInHostDoc::OnUpdatePlay(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( !theDXiPlayer.IsPlaying() );
}

void CPlugInHostDoc::OnPlay() 
{
	if (!theDXiPlayer.IsPlaying())
	{
		theDXiPlayer.Stop( TRUE );
		theDXiPlayer.Play( TRUE );
	}
}

////////////////////////////////////////////////////////////////////////////////

void CPlugInHostDoc::OnUpdateStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();
}

void CPlugInHostDoc::OnStop() 
{
	if (theDXiPlayer.IsPlaying())
	{
		theDXiPlayer.Stop( FALSE );
		theDXiPlayer.Play( FALSE );
	}
}

////////////////////////////////////////////////////////////////////////////////

void CPlugInHostDoc::OnUpdateRew(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();
}

void CPlugInHostDoc::OnRew() 
{
	theDXiPlayer.Rewind();
}

////////////////////////////////////////////////////////////////////////////////

void CPlugInHostDoc::OnUpdateLoop(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( theDXiPlayer.IsLooping() );
}

void CPlugInHostDoc::OnLoop() 
{
	theDXiPlayer.SetLooping( !theDXiPlayer.IsLooping() );
}

////////////////////////////////////////////////////////////////////////////////
