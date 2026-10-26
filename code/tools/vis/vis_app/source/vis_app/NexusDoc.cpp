// NexusDoc.cpp : implementation of the NexusDoc class
//

#include <vis_app/stdafx.h>
#include <vis_app/Nexus.h>

#include <vis_app/NexusSystem.h>
#include <vis_app/NexusRender.h>

#include <vis_app/NexusDoc.h>
#include "..\..\include\vis_app\nexusdoc.h"

#include "../../resource.h"
#include "..\..\include\vis_app\mainfrm.h"

using namespace Nexus;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// NexusDoc

IMPLEMENT_DYNCREATE(NexusDoc, CDocument)

BEGIN_MESSAGE_MAP(NexusDoc, CDocument)
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
END_MESSAGE_MAP()


// NexusDoc construction/destruction

NexusDoc::NexusDoc()
{
	// TODO: add one-time construction code here
}

NexusDoc::~NexusDoc()
{
	m_nexusSystem.Destroy();
}

BOOL NexusDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	g_pMainFrame->ClearLogText();

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	if (m_nexusSystem.IsCreated())
	{
		m_nexusSystem.Destroy();
	}

	m_nexusSystem.Create();

	m_nexusSystem.NewScene();

	return TRUE;
}

NexusSystem* NexusDoc::GetNexusSystemPtr()
{
	return &m_nexusSystem;
}

// NexusDoc serialization

void NexusDoc::Serialize(CArchive& ar)
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


// NexusDoc diagnostics

#ifdef _DEBUG
void NexusDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void NexusDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

const Nexus::NexusSystem* NexusDoc::NexusSystemPtr() const
{
	return &m_nexusSystem;
}

Nexus::NexusSystem* NexusDoc::NexusSystemPtr()
{
	return &m_nexusSystem;
}

// NexusDoc commands

BOOL NexusDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
	{
		return FALSE;
	}

	g_pMainFrame->ClearLogText();

	SetModifiedFlag(FALSE);     // start off with unmodified

	BOOL status = m_nexusSystem.OpenScene(lpszPathName);

	if (m_nexusSystem.AftkPtr()->GetRootSceneNodePtr() != 0)
	{
		//This will trigger the scene renderer to rebuild its vertex buffers, texture and shader info.
		UpdateAllViews(NULL);
	}

	//Load textures if needed.
	if (status)
	{
		if (m_nexusSystem.AftkPtr()->GetRootSceneNodePtr() != 0)
		{
			if (g_pNexusRender->Settings().UseTextures())
			{
				Nexus::g_pNexusRender->OnLoadTextures();
				//This will trigger the scene renderer to rebuild its vertex buffers, texture and shader info.
				UpdateAllViews(NULL);
			}
		}
	}


	return status;
}

BOOL NexusDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnSaveDocument(lpszPathName))
	{
		return FALSE;
	}

	if (!m_nexusSystem.SaveScene(lpszPathName))
	{
		return FALSE;
	}

	return TRUE;
}


void NexusDoc::OnFileImport()
{
	char strFilter[] = { "Abstract 3D (*.a3d)|*.a3d|SoftImage XSI (*.xsi)|*.xsi|All Files (*.*)|*.*||" };

	CFileDialog openFile(
		true,
		"A3D",
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		strFilter,
		0);

	if( openFile.DoModal() == IDOK )
		OnOpenDocument(openFile.GetPathName().GetBuffer());
	
}


void NexusDoc::OnFileExport()
{
	char strFilter[] = { "Abstract 3D Files (*.a3d)|*.a3d" };

	CFileDialog saveFile(
		false,
		"A3D",
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		strFilter,
		0);

	if( saveFile.DoModal() == IDOK )
	{
		OnSaveDocument(saveFile.GetPathName().GetBuffer());
	}
}


