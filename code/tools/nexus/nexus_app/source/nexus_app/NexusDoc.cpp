// NexusDoc.cpp : implementation of the NexusDoc class
//

#include <nexus_app/stdafx.h>
#include <nexus_app/Nexus.h>

#include <nexus_app/NexusSystem.h>
#include <nexus_app/DisplayManager.h>

#include <nexus_app/NexusDoc.h>
#include "..\..\include\nexus_app\nexusdoc.h"

#include "../../resource.h"
#include "..\..\include\nexus_app\mainfrm.h"
#include <nexus_app/SceneGraphTree.h>
#include <nexus_app/CommandEdit.h>
#include <mfc_extras/ChatRichEd.h>
#include <nexus_app/CommandView.h>
#include <nexus_app/DockingPanelBottom.h>
#include <nexus_app/DockingPanelRight.h>
#include <nexus_app/ViewportParent.h>

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


Nexus::NexusDoc* Nexus::g_pDocument = 0;
// NexusDoc construction/destruction

NexusDoc::NexusDoc()
{
	// TODO: add one-time construction code here
	g_pDocument = this;
	m_nexusSystem.Create();
}

NexusDoc::~NexusDoc()
{
	m_nexusSystem.Destroy();
}

BOOL NexusDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	g_pMainFrame->GetDockingPanelBottomPtr()->Clear();
	g_pMainFrame->GetDockingPanelRightPtr()->Clear();
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
BOOL NexusDoc::OnImportDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
	{
		return FALSE;
	}

	g_pMainFrame->GetDockingPanelBottomPtr()->Clear();
	g_pMainFrame->GetDockingPanelRightPtr()->Clear();

	SetModifiedFlag(FALSE);     // start off with unmodified

	bool cancelled;
	Tu::Status status = m_nexusSystem.OpenScene(lpszPathName, true, cancelled);
	if (cancelled)
	{
		return FALSE;
	}
	if (!status)
	{
		g_pMainFrame->OnError(status);
		return FALSE;
	}

	if (m_nexusSystem.GetCorePtr()->GetAftkScenePtr()->GetRootSceneNodePtr() != 0)
	{
		//This will trigger the scene renderer to rebuild its vertex buffers, texture and shader info.
		UpdateAllViews(NULL);
	}

	g_pMainFrame->GetDockingPanelRightPtr()->Update();

	if (status && m_nexusSystem.GetCorePtr()->GetAftkScenePtr()->GetRootSceneNodePtr())
	{
		if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useTextures)
		{
			Nexus::g_pViewportParent->GetDisplayManagerPtr()->OnLoadTextures();
			UpdateAllViews(NULL);
		}
		if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useCropSheet)
		{
			Nexus::g_pViewportParent->GetDisplayManagerPtr()->OnLoadCropSheetTextures();
			UpdateAllViews(NULL);
		}
		Nexus::g_pViewportParent->GetDisplayManagerPtr()->FrameAll();
	}

	return status;
}

BOOL NexusDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
	{
		return FALSE;
	}

	g_pMainFrame->GetDockingPanelBottomPtr()->Clear();
	g_pMainFrame->GetDockingPanelRightPtr()->Clear();

	SetModifiedFlag(FALSE);     // start off with unmodified

	if (m_nexusSystem.IsCreated())
	{
		m_nexusSystem.Clear();
	}

	bool cancelled;
	Tu::Status status = m_nexusSystem.OpenScene(lpszPathName, false, cancelled);
	if (cancelled)
	{
		return FALSE;
	}
	if (!status)
	{
		g_pMainFrame->OnError(status);
		return FALSE;
	}

	if (m_nexusSystem.GetCorePtr()->GetAftkScenePtr()->GetRootSceneNodePtr() != 0)
	{
		//This will trigger the scene renderer to rebuild its vertex buffers, texture and shader info.
		UpdateAllViews(NULL);
	}

	g_pMainFrame->Update();

	//Load textures if needed.
	if (status)
	{
		if (m_nexusSystem.GetCorePtr()->GetAftkScenePtr()->GetRootSceneNodePtr() != 0)
		{
			if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useTextures)
			{
				Nexus::g_pViewportParent->GetDisplayManagerPtr()->OnLoadTextures();
				UpdateAllViews(NULL);
			}
			if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useCropSheet)
			{
				Nexus::g_pViewportParent->GetDisplayManagerPtr()->OnLoadCropSheetTextures();
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

	bool cancelled;
	if (!m_nexusSystem.SaveScene(lpszPathName, cancelled))
	{
		return FALSE;
	}

	return TRUE;
}


void NexusDoc::OnFileImport()
{
	std::string stringFilter;

	Aftk::StringArray extArray = Aftk::C_Global::GetReadSupportedFileExtensions();
	Aftk::StringArray descArray = Aftk::C_Global::GetReadSupportedFileDescriptions();

	for (U32 i = 0; i < (U32)extArray.Size(); i++)
	{
		stringFilter += descArray[i];
		stringFilter += " (*.";
		stringFilter += extArray[i];
		stringFilter += ")|*.";
		stringFilter += extArray[i];
		stringFilter += "|";
	}

	stringFilter += "All Files (*.*)|*.*||";

	CFileDialog openFile(
		true,
		"A3D",
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		stringFilter.c_str(),
		0);

	if( openFile.DoModal() == IDOK )
		OnImportDocument(openFile.GetPathName().GetBuffer());
}

void NexusDoc::OnFileExport()
{
	std::string stringFilter;

	Aftk::StringArray extArray = 
		Aftk::C_Global::GetWriteSupportedFileExtensions();
	Aftk::StringArray descArray = 
		Aftk::C_Global::GetWriteSupportedFileDescriptions();

	for (U32 i = 0; i < (U32)extArray.Size(); i++)
	{
		stringFilter += descArray[i];
		stringFilter += " (*.";
		stringFilter += extArray[i];
		stringFilter += ")|*.";
		stringFilter += extArray[i];
		stringFilter += "|";
	}

	CFileDialog saveFile(
		false,
		"A3D",
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		stringFilter.c_str(),
		0);

	if( saveFile.DoModal() == IDOK )
	{
		OnSaveDocument(saveFile.GetPathName().GetBuffer());
	}
}


