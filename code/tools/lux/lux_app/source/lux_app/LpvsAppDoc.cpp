// lpvsappdoc.cpp : implementation of the LpvsAppDoc class
//

#include "stdafx.h"
#include <lux_app/LpvsApp.h>

#include <lux_app/LpvsAppDoc.h>
#include "lux_app/LpvsFileProgressDialog.h"
#include "..\..\include\lux_app\lpvsappdoc.h"
#include "lux_app/LayerDlg.h"
#include "lux_app/LpvsMainFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// LpvsAppDoc

IMPLEMENT_DYNCREATE(LpvsAppDoc, CDocument)

BEGIN_MESSAGE_MAP(LpvsAppDoc, CDocument)
END_MESSAGE_MAP()


// LpvsAppDoc construction/destruction

LpvsAppDoc::LpvsAppDoc()
{
	// TODO: add one-time construction code here
}

LpvsAppDoc::~LpvsAppDoc()
{
}

BOOL LpvsAppDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	// TODO: remove current scene - reinitialize with empty scene

	return TRUE;
}


// check what layers there are in this new scene and stick them in the layer dialog
void LpvsAppDoc::UpdateLayerDialog()
{
	Aftk::C_Scene *pScene = &LPVS::LPVSCORE.GetScene();
	LpvsMainFrame *pFrame = (LpvsMainFrame*)theApp.GetMainWnd();
	CLayerDlg* pDlg = pFrame->m_pLayerDlg;

	pDlg->DeleteControls();

	int n = pScene->GetNumLayers();
	for (int i=0; i<n; i++)
		pDlg->AddLayer(pScene->GetLayerPtr(i)->GetName().c_str());

	pDlg->CreateControls();
}

// open document
BOOL LpvsAppDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	bool bResult;

	LpvsFileProgressDialog	progressWindow;

	progressWindow.Create(IDD_FILE_PROGRESS, theApp.GetMainWnd());
	progressWindow.ShowWindow(SW_SHOW);

	bResult = LPVS::LPVSCORE.ReadProject(lpszPathName, (LPVS::MultiProgressNotification*)&progressWindow);

	if (!bResult)
		return FALSE;

	SetModifiedFlag(FALSE);     // start off with unmodified
	UpdateAllViews(NULL);

	UpdateLayerDialog();

	return TRUE;
}

// save document
BOOL LpvsAppDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	bool bResult;

	LpvsFileProgressDialog	progressWindow;

	progressWindow.Create(IDD_FILE_PROGRESS, theApp.GetMainWnd());
	progressWindow.ShowWindow(SW_SHOW);

	bResult = LPVS::LPVSCORE.WriteProject(lpszPathName, (LPVS::MultiProgressNotification*)&progressWindow);

	if (!bResult)
		return FALSE;

	UpdateAllViews(NULL);

	return TRUE;
}


// LpvsAppDoc serialization

void LpvsAppDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// LpvsAppDoc diagnostics

#ifdef _DEBUG
void LpvsAppDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void LpvsAppDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// LpvsAppDoc commands

void LpvsAppDoc::SetTitle(LPCTSTR lpszTitle)
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::SetTitle(lpszTitle);
}
