#include "stdafx.h"

#include "system/misc/StringUtil.h"
#include "wx/treelistctrl/treelistctrl.h"
#include "wx/filename.h"

#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefHelpers.h>
#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Xt/XtVisualisation.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/Level/ReferenceLevel.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Components/ReferenceDialog.h>

using namespace Editor;

//modified wxFileSelector convenience function to work with multiple selection
wxArrayString wxArrayFileSelector(const wxChar *title,
                               const wxChar *defaultDir,
                               const wxChar *defaultFileName,
                               const wxChar *defaultExtension,
                               const wxChar *filter,
                               int flags,
                               wxWindow *parent,
                               int x=wxDefaultCoord, int y=wxDefaultCoord)
{
    wxString filter2;
    if ( !wxIsEmpty(defaultExtension) && wxIsEmpty(filter) )
        filter2 = wxString(wxT("*.")) + defaultExtension;
    else if ( !wxIsEmpty(filter) )
        filter2 = filter;

    wxString defaultDirString;
    if (!wxIsEmpty(defaultDir))
        defaultDirString = defaultDir;

    wxString defaultFilenameString;
    if (!wxIsEmpty(defaultFileName))
        defaultFilenameString = defaultFileName;

    wxFileDialog fileDialog(parent, title, defaultDirString,
                            defaultFilenameString, filter2,
                            flags | wxMULTIPLE, wxPoint(x, y));

   // if filter is of form "All files (*)|*|..." set correct filter index
    if((wxStrlen(defaultExtension) != 0) && (filter2.Find(wxT('|')) != wxNOT_FOUND))
    {
        int filterIndex = 0;

        wxArrayString descriptions, filters;
        // don't care about errors, handled already by wxFileDialog
        (void)wxParseCommonDialogsFilter(filter2, descriptions, filters);
        for (size_t n=0; n<filters.GetCount(); n++)
        {
            if (filters[n].Contains(defaultExtension))
            {
                filterIndex = n;
                        break;
            }
        }

        if (filterIndex > 0)
            fileDialog.SetFilterIndex(filterIndex);
    }

    wxArrayString filenames;
    if ( fileDialog.ShowModal() == wxID_OK )
        fileDialog.GetPaths(filenames);

    return filenames;
}
static void SetTreeItemImage(wxTreeListCtrl *pTree, wxTreeItemId id, const EditorLib::C_AftkRef *pRef)
{
	U32 image = 0;
	if (pRef->IsLoaded())
		image = 1;
	else if (wxFileName::FileExists(wxString(pRef->GetOriginalFilePath().Chars())))
		image = 0;
	else
		image = 2;

	pTree->SetItemImage(id, image);
}

C_ReferenceData::C_ReferenceData(const EditorLib::C_AftkRef *pAftkRef)
: wxTreeItemData()
, m_pAftkRef(pAftkRef)
{
}

C_ReferenceData::~C_ReferenceData()
{
}

const EditorLib::C_AftkRef* C_ReferenceData::GetAftkRef() const
{
	return m_pAftkRef;
}



C_ReferencePanel::C_Slots::C_Slots(C_ReferencePanel *pDialog)
: m_pDialog(pDialog)
{
}

void C_ReferencePanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelAftkRefAdded.connect(this, &C_ReferencePanel::C_Slots::OnLevelAftkRefAddedSlot);
	s.levelAftkRefRemoved.connect(this, &C_ReferencePanel::C_Slots::OnLevelAftkRefRemovedSlot);
	s.aftkRefLoaded.connect(this, &C_ReferencePanel::C_Slots::OnAftkRefLoadedSlot);
	s.aftkRefUnloaded.connect(this, &C_ReferencePanel::C_Slots::OnAftkRefUnloadedSlot);

	s.levelEvent.connect(this, &C_ReferencePanel::C_Slots::OnLevelEventSlot);
	s.levelOpening.connect(this, &C_ReferencePanel::C_Slots::OnLevelOpeningSlot);
	s.levelOpened.connect(this, &C_ReferencePanel::C_Slots::OnLevelOpenedSlot);
	s.levelClosing.connect(this, &C_ReferencePanel::C_Slots::OnLevelClosingSlot);
	s.levelClosed.connect(this, &C_ReferencePanel::C_Slots::OnLevelClosedSlot);
}

void C_ReferencePanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelAftkRefAdded.disconnect(this);
	s.levelAftkRefRemoved.disconnect(this);
	s.aftkRefLoaded.disconnect(this);
	s.aftkRefUnloaded.disconnect(this);

	s.levelEvent.disconnect(this);
	s.levelOpening.disconnect(this);
	s.levelOpened.disconnect(this);
	s.levelClosing.disconnect(this);
	s.levelClosed.disconnect(this);
}

void C_ReferencePanel::C_Slots::OnLevelAftkRefAddedSlot(const Btl::String& originalFilePath)
{
	if (!m_pDialog->m_flags.m_pendingSync)
		m_pDialog->AddRefNode(originalFilePath.Chars());
}

void C_ReferencePanel::C_Slots::OnLevelAftkRefRemovedSlot(const Btl::String& originalFilePath)
{
	if (!m_pDialog->m_flags.m_pendingSync)
		m_pDialog->RemoveRefNode(originalFilePath.Chars());
}

void C_ReferencePanel::C_Slots::OnAftkRefLoadedSlot(const Btl::String& originalFilePath, const Aftk::C_Scene& aftkScene, const EditorLib::C_SceneNode& sceneNode, const EditorLib::AftkDgMap& aftkDgMap)
{
	if (!m_pDialog->m_flags.m_pendingSync)
		m_pDialog->AddRefNode(originalFilePath.Chars());
}

void C_ReferencePanel::C_Slots::OnAftkRefUnloadedSlot(const Btl::String& originalFilePath)
{
	if (!m_pDialog->m_flags.m_pendingSync)
		m_pDialog->AddRefNode(originalFilePath.Chars());
}

void C_ReferencePanel::C_Slots::OnLevelEventSlot(const EditorLib::I_Level* level, EditorLib::E_LevelEvent e)
{
	if (e == EditorLib::E_LevelEvent_ActiveLevelChanged)
	{
		m_pDialog->SetActiveLevel(level);
	}
}

void C_ReferencePanel::C_Slots::OnLevelOpeningSlot(const EditorLib::I_Level& level)
{
	m_pDialog->m_flags.m_pendingSync = true;
}

void C_ReferencePanel::C_Slots::OnLevelOpenedSlot(const EditorLib::I_Level& level)
{
	assert(m_pDialog->m_flags.m_pendingSync);
	m_pDialog->m_flags.m_syncOnIdle = true;
}

void C_ReferencePanel::C_Slots::OnLevelClosingSlot(const EditorLib::I_Level& level)
{
	m_pDialog->m_flags.m_pendingSync = true;
}

void C_ReferencePanel::C_Slots::OnLevelClosedSlot(const EditorLib::I_Level& level)
{
	assert(m_pDialog->m_flags.m_pendingSync );
	m_pDialog->m_flags.m_syncOnIdle = true;
}


IMPLEMENT_CLASS(C_ReferencePanel, wxPanel)

C_ReferencePanel::C_ReferencePanel(wxWindow *parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style)
, m_Slots(this)
{
	SetBackgroundColour(*wxWHITE);

	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer( wxVERTICAL );
	
	wxPanel *pPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	
	wxBoxSizer* pSizer1;
	pSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* pSizer2;
	pSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* pSizer3;
	pSizer3 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* pSizer6;
	pSizer6 = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText *pFileLabel = new wxStaticText( pPanel, wxID_ANY, wxT("Scene"), wxDefaultPosition, wxDefaultSize, 0 );
	pFileLabel->Wrap( -1 );
	pSizer3->Add( pFileLabel, 0, wxALL, 5 );
	
	wxStaticText *pCacheLabel = new wxStaticText( pPanel, wxID_ANY, wxT("Cache"), wxDefaultPosition, wxDefaultSize, 0 );
	pCacheLabel->Wrap( -1 );
	pSizer3->Add( pCacheLabel, 0, wxALL, 5 );
	
	wxStaticText *pVisIdLabel = new wxStaticText( pPanel, wxID_ANY, wxT("VisId"), wxDefaultPosition, wxDefaultSize, 0 );
	pVisIdLabel->Wrap( -1 );
	pSizer3->Add( pVisIdLabel, 0, wxALL, 5 );

	pSizer2->Add( pSizer3, 0, wxEXPAND, 5 );
	
	wxBoxSizer* pSizer4;
	pSizer4 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* pSceneEditSizer = new wxBoxSizer( wxHORIZONTAL );
	{
		m_pScenePathCtrl = new wxTextCtrl( pPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
		pSceneEditSizer->Add( m_pScenePathCtrl, 1, wxALL|wxEXPAND, 2 );
		m_pScenePathIsExtRefCb = new wxCheckBox( pPanel, wxID_ANY, "ExtActor", wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
		pSceneEditSizer->Add( m_pScenePathIsExtRefCb, 0, wxALL|wxEXPAND, 2 );
	}
	pSizer4->Add( pSceneEditSizer, 0, wxALL|wxEXPAND, 2 );
	
	m_pCachePathCtrl = new wxTextCtrl( pPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pSizer4->Add( m_pCachePathCtrl, 0, wxALL|wxEXPAND, 2 );

	m_pVisIdCtrl = new wxTextCtrl( pPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pSizer4->Add( m_pVisIdCtrl, 0, wxALL|wxEXPAND, 2 );
	
	pSizer2->Add( pSizer4, 1, wxEXPAND, 5 );
	
	wxBoxSizer* pSizer5;
	pSizer5 = new wxBoxSizer( wxVERTICAL );
	
	wxButton *pAddRefButton  = new wxButton( pPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 22,22 ), 0 );
	pSizer5->Add( pAddRefButton, 0, wxALL, 2 );

	pSizer2->Add( pSizer5, 0, wxEXPAND, 5 );
	
	pSizer1->Add( pSizer2, 0, wxEXPAND, 0 );

	wxBitmapButton *pLoadAllRefButton = new wxBitmapButton( pPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	pLoadAllRefButton->SetToolTip( wxT("Load All References") );
	pLoadAllRefButton->SetBitmapLabel(wxBitmap(wxT("./icons/references_load.png"), wxBITMAP_TYPE_ANY) );
	pLoadAllRefButton->SetBitmapSelected(wxBitmap(wxT("./icons/references_loaddisabled.png"), wxBITMAP_TYPE_ANY) );
	pLoadAllRefButton->SetBitmapDisabled(wxBitmap(wxT("./icons/references_loaddisabled.png"), wxBITMAP_TYPE_ANY) );
	pLoadAllRefButton->SetBackgroundColour(*wxWHITE);
	pSizer6->Add( pLoadAllRefButton, 0, wxALL, 2 );

	wxBitmapButton *pUnloadAllRefButton = new wxBitmapButton( pPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	pUnloadAllRefButton->SetToolTip( wxT("Unload All References") );
	pUnloadAllRefButton->SetBitmapLabel(wxBitmap(wxT("./icons/references_unload.png"), wxBITMAP_TYPE_ANY) );
	pUnloadAllRefButton->SetBitmapSelected(wxBitmap(wxT("./icons/references_unloaddisabled.png"), wxBITMAP_TYPE_ANY) );
	pUnloadAllRefButton->SetBitmapDisabled(wxBitmap(wxT("./icons/references_unloaddisabled.png"), wxBITMAP_TYPE_ANY) );
	pUnloadAllRefButton->SetBackgroundColour(*wxWHITE);
	pSizer6->Add( pUnloadAllRefButton, 0, wxALL, 2 );

	wxBitmapButton *pRefreshRefButton = new wxBitmapButton( pPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	pRefreshRefButton->SetToolTip( wxT("Refresh References") );
	pRefreshRefButton->SetBitmapLabel(wxBitmap(wxT("./icons/references_refresh.png"), wxBITMAP_TYPE_ANY) );
	pRefreshRefButton->SetBitmapSelected(wxBitmap(wxT("./icons/references_refreshdisabled.png"), wxBITMAP_TYPE_ANY) );
	pRefreshRefButton->SetBitmapDisabled(wxBitmap(wxT("./icons/references_refreshdisabled.png"), wxBITMAP_TYPE_ANY) );
	pRefreshRefButton->SetBackgroundColour(*wxWHITE);
	pSizer6->Add( pRefreshRefButton, 0, wxALL, 2 );

	wxBitmapButton *pGetRefButton = new wxBitmapButton( pPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	pGetRefButton->SetToolTip( wxT("Get Latest References") );
	pGetRefButton->SetBitmapLabel(wxBitmap(wxT("./icons/references_getlatest.png"), wxBITMAP_TYPE_ANY) );
	pGetRefButton->SetBitmapSelected(wxBitmap(wxT("./icons/references_getlatestdisabled.png"), wxBITMAP_TYPE_ANY) );
	pGetRefButton->SetBitmapDisabled(wxBitmap(wxT("./icons/references_getlatestdisabled.png"), wxBITMAP_TYPE_ANY) );
	pGetRefButton->SetBackgroundColour(*wxWHITE);
	pSizer6->Add( pGetRefButton, 0, wxALL, 2 );

	pSizer1->Add( pSizer6, 0, wxEXPAND, 0 );
	
	m_pReferenceListCtrl = new wxTreeListCtrl( pPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_EDIT_LABELS|wxTR_EXTENDED|wxTR_FULL_ROW_HIGHLIGHT|wxTR_HIDE_ROOT|wxTR_NO_LINES|wxTR_MULTIPLE|wxHSCROLL|wxNO_BORDER|wxTAB_TRAVERSAL|wxVSCROLL );
	pSizer1->Add( m_pReferenceListCtrl, 1, wxALL|wxEXPAND, 0 );
	
	pPanel->SetSizer( pSizer1 );
	pPanel->Layout();
	pSizer1->Fit( pPanel );
	pSizer0->Add( pPanel, 1, wxEXPAND | wxALL, 0 );
	
	SetSizer(pSizer0);
	Layout();

	{
		wxImageList *iconList = new wxImageList(16,16);
		wxImage image;
		image.LoadFile("./icons/scenegraph_refscene_unloaded.png");
		iconList->Add(image);
		image.LoadFile("./icons/scenegraph_refscene_loaded.png");
		iconList->Add(image);
		image.LoadFile("./icons/scenegraph_refscene_missing.png");
		iconList->Add(image);
		image.LoadFile("./icons/scenegraph_refscenes.png");
		iconList->Add(image);
		m_pReferenceListCtrl->AssignImageList(iconList);
	}

	m_pReferenceListCtrl->AddColumn("File", 150, wxALIGN_LEFT, -1, true, false);
	m_pReferenceListCtrl->AddColumn("Path", 320, wxALIGN_LEFT, -1, true, false);

	Connect(pAddRefButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_ReferencePanel::OnAddReference));
	Connect(pRefreshRefButton->GetId(),  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_ReferencePanel::OnRefreshReference));
	Connect(pLoadAllRefButton->GetId(),  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_ReferencePanel::OnLoadAllReference));
	Connect(pUnloadAllRefButton->GetId(),  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_ReferencePanel::OnUnloadAllReference));
	Connect(pGetRefButton->GetId(),  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_ReferencePanel::OnGetLatestReferences));
	Connect(wxID_ANY, wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(C_ReferencePanel::OnNodeActivate));
	Connect(wxID_ANY, wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler(C_ReferencePanel::OnNodeSelect));
	Connect(wxID_ANY, wxEVT_COMMAND_TREE_KEY_DOWN, wxTreeEventHandler(C_ReferencePanel::OnKeyDown));
	Connect(
		m_pScenePathIsExtRefCb->GetId(), 
		wxEVT_COMMAND_CHECKBOX_CLICKED, 
		wxCommandEventHandler(C_ReferencePanel::OnExtRefCbClicked));
	Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_ReferencePanel::OnIdle));
	wxTreeItemId rootId = m_pReferenceListCtrl->AddRoot("");
	m_levelRoot = m_pReferenceListCtrl->AppendItem(rootId, "Active Level References", 3);
	m_globalRoot = m_pReferenceListCtrl->AppendItem(rootId, "Global References", 3);
	m_visualisationRoot = m_pReferenceListCtrl->AppendItem(rootId, "Visualisation", 3);
	Populate();

	m_Slots.Connect();
	m_flags.m_connected = true;
	m_flags.m_pendingSync = false;
	m_flags.m_syncOnIdle = false;

	m_pScenePathCtrl->Disable();
	m_pScenePathIsExtRefCb->Disable();
	m_pCachePathCtrl->Disable();
	m_pVisIdCtrl->Disable();
}

C_ReferencePanel::~C_ReferencePanel()
{
	Shutdown();
}

void C_ReferencePanel::Shutdown()
{
	if (m_flags.m_connected)
	{
		m_Slots.Disconnect();
		m_flags.m_connected = false;
	}
}

void C_ReferencePanel::OnNodeActivate(wxTreeEvent& e)
{
	if (!e.GetItem().IsOk())
		return;

	C_ReferenceData *pData = (C_ReferenceData*) m_pReferenceListCtrl->GetItemData(e.GetItem());
	if (pData)
	{
		const EditorLib::C_AftkRef* pRef = pData->GetAftkRef();
		EditorLib::C_Status status;
		if (pRef->IsLoaded())
			status = pRef->Unload();
		else
			status = pRef->Load();

		SetTreeItemImage(m_pReferenceListCtrl, e.GetItem(), pRef);
	}
}

void C_ReferencePanel::OnNodeSelect(wxTreeEvent& e)
{
	wxTreeItemId nodeId = e.GetItem();
	if (!nodeId.IsOk())
		return;

	const EditorLib::C_AftkRef *pRef = 0;
	C_ReferenceData *pData = (C_ReferenceData*)m_pReferenceListCtrl->GetItemData(nodeId);
	if (pData)
	{
		pRef = pData->GetAftkRef();
	}

	const EditorLib::C_AftkLevelRef* pLevelRef = 0;
	if (pRef)
	{
		pLevelRef = 
			EditorLib::C_Core::Instance().GetLevelManager()->GetReferenceLevel()->GetAftkRef(pRef->GetOriginalFilePath());

		if (!pLevelRef)
		{
			Btl::Vector<EditorLib::C_LevelPack*> packs = EditorLib::C_Core::Instance().GetLevelManager()->GetPacks();
			for (Btl::Vector<EditorLib::C_LevelPack*>::Iterator packIt = packs.Begin(); packIt != packs.End(); ++packIt)
			{
				Btl::Vector<EditorLib::C_XtLevel*> levels = (*packIt)->GetLevels();

				for (Btl::Vector<EditorLib::C_XtLevel*>::Iterator levelIt = levels.Begin(); levelIt != levels.End(); ++levelIt)
				{
					const EditorLib::C_AftkLevelRef* levelRef = (*levelIt)->GetAftkRef(pRef->GetOriginalFilePath());
					if (levelRef)
						pLevelRef = levelRef;
				}
			}
		}
	}

	if (pLevelRef)
	{
		m_pScenePathCtrl->Enable();
		m_pCachePathCtrl->Enable();
		m_pVisIdCtrl->Enable();

		m_pScenePathCtrl->SetValue(pRef->GetOriginalFilePath().Chars());
		m_pCachePathCtrl->SetValue(pRef->GetCacheFilePath().Chars());

		const EditorLib::XtVisualisationDef* pDef = 0;
		if (pRef->IsVisualisation())
		{
			pDef = EditorLib::C_Core::Instance().GetXtVisualisationData().FindByFilePath(
					pRef->GetOriginalFilePath().Chars());
			m_pScenePathIsExtRefCb->SetValue(false);
			m_pScenePathIsExtRefCb->Disable();
		}
		else
		{
			m_pScenePathIsExtRefCb->SetValue(pLevelRef->isExtActor);
			m_pScenePathIsExtRefCb->Enable();
		}
		if (pDef)
			m_pVisIdCtrl->SetValue(pDef->id.Chars());
		else
			m_pVisIdCtrl->SetValue(wxEmptyString);
	}
	else
	{
		m_pScenePathCtrl->Clear();
		m_pScenePathCtrl->Disable();
		m_pScenePathIsExtRefCb->SetValue(false);
		m_pScenePathIsExtRefCb->Disable();
		m_pCachePathCtrl->Clear();
		m_pCachePathCtrl->Disable();
		m_pVisIdCtrl->Clear();
		m_pVisIdCtrl->Disable();
	}
}

void C_ReferencePanel::OnAddReference(wxCommandEvent& e)
{
	//make sure we've got a level open.
	if (!EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel())
		return;

	wxArrayString paths = wxArrayFileSelector(_T("Add Reference"), m_pScenePathCtrl->GetValue(), NULL, NULL,
		wxString::Format
		(
			_T("Aftk Supported Files (*.a3d *.ma *.mb *.xsi)|*.a3d;*.ma;*.mb;*.xsi;|Maya Scenes (*.mb *.ma)|*.mb;*.ma|A3d Scenes (*.a3d)|*.a3d|DotXSI Scenes (*.xsi)|*.xsi|All Files (%s)|%s"),
			wxFileSelectorDefaultWildcardStr,
			wxFileSelectorDefaultWildcardStr
		),
		wxFD_OPEN|wxFD_PREVIEW|wxMULTIPLE,
		this);
		
	for (U32 i=0; i<paths.Count(); ++i)
		EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->AddAftkRef(paths[i].c_str());	
}

void C_ReferencePanel::OnRefreshReference(wxCommandEvent& e)
{
	EditorLib::AftkRefHelpers::ReloadReferences(true, true, &Editor::AppYield);
}

void C_ReferencePanel::OnLoadAllReference(wxCommandEvent& e)
{
	EditorLib::AftkRefHelpers::LoadAllReferences(true, false, &Editor::AppYield);
}

void C_ReferencePanel::OnUnloadAllReference(wxCommandEvent& e)
{
	EditorLib::AftkRefHelpers::UnloadAllReferences(false, &Editor::AppYield);
}

void C_ReferencePanel::OnGetLatestReferences(wxCommandEvent& e)
{
	if (EditorLib::AftkRefHelpers::GetLatestReferences(&AppYield))
		EditorLib::AftkRefHelpers::ReloadReferences(true, true, &Editor::AppYield);
}

void C_ReferencePanel::OnKeyDown(wxTreeEvent& e)
{
	if (e.GetKeyCode() == WXK_BACK || e.GetKeyCode() == WXK_DELETE)
	{
		wxArrayTreeItemIds selection;
		if (m_pReferenceListCtrl->GetSelections(selection) > 0)
		{
			m_pReferenceListCtrl->Freeze();
			for (U32 i=0; i<selection.GetCount(); ++i)
			{
				C_ReferenceData* pData = (C_ReferenceData*) m_pReferenceListCtrl->GetItemData(selection[i]);
				if (pData && !pData->GetAftkRef()->IsVisualisation())
					EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->RemoveAftkRef(pData->GetAftkRef()->GetOriginalFilePath());
			}
			m_pReferenceListCtrl->Thaw();
			m_pReferenceListCtrl->Update();
		}
	}
	else
		e.Skip();
}

void C_ReferencePanel::OnExtRefCbClicked(wxCommandEvent& e)
{
	const wxString& filePath = m_pScenePathCtrl->GetValue();

	EditorLib::C_AftkRef* pRef = 
		EditorLib::C_Core::Instance().GetAftkRefManager().GetRef(filePath.c_str());
	if (!pRef || pRef->IsVisualisation())
		return;

	EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->UpdateAftkRef(
		filePath.c_str(), m_pScenePathIsExtRefCb->GetValue());
}

void C_ReferencePanel::OnIdle(wxIdleEvent& e)
{
	if (m_flags.m_syncOnIdle)
	{
		assert(m_flags.m_pendingSync);

		//hmm. manually clear everything prior to repopulating the tree
		//as we seem to be getting selection events while repopulating the tree :(
		SetEvtHandlerEnabled(false);
		m_pReferenceListCtrl->UnselectAll();
		m_pVisIdCtrl->SetValue(wxEmptyString);
		m_pScenePathCtrl->SetValue(wxEmptyString);
		m_pCachePathCtrl->SetValue(wxEmptyString);
		SetEvtHandlerEnabled(true);

		Populate();

		m_flags.m_pendingSync = false;
		m_flags.m_syncOnIdle = false;
	}
}

void C_ReferencePanel::Populate()
{
	wxTreeItemId rootId = m_pReferenceListCtrl->GetRootItem();
	m_pReferenceListCtrl->DeleteChildren(m_levelRoot);
	m_pReferenceListCtrl->DeleteChildren(m_globalRoot);
	m_pReferenceListCtrl->DeleteChildren(m_visualisationRoot);
	m_GlobalMap.Clear();
	m_LevelMap.Clear();
	m_VisualisationMap.Clear();

	EditorLib::ConstAftkRefList refList;
	EditorLib::C_Core::Instance().GetAftkRefManager().GetRefs(refList);

	for (EditorLib::ConstAftkRefList::Iterator it=refList.Begin(); it!=refList.End(); ++it)
		AddRefNode((*it)->GetOriginalFilePath().Chars());

	SetActiveLevel(EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel());
}

void C_ReferencePanel::AddRefNode(const char *pPath)
{
	EditorLib::ConstAftkRefList refList;
	EditorLib::C_Core::Instance().GetAftkRefManager().GetRefs(refList);

	EditorLib::ConstAftkRefList::Iterator it;
	for (it=refList.Begin(); it!=refList.End(); ++it)
	{
		if ((*it)->GetOriginalFilePath() == pPath)
			break;
	}

	if (it != refList.End())
	{
		if ((*it)->IsVisualisation())
		{
			StringTreeMap::Iterator treeItem = m_VisualisationMap.Find(pPath);

			if (treeItem != m_VisualisationMap.End())
				SetTreeItemImage(m_pReferenceListCtrl, treeItem->GetValue(), *it);
			else
			{
				wxTreeItemId id = m_pReferenceListCtrl->AppendItem(
					m_visualisationRoot,
					Misc::ExtractFilename((*it)->GetOriginalFilePath()).Chars(),
					-1,
					-1,
					new C_ReferenceData(*it));

				SetTreeItemImage(m_pReferenceListCtrl, id, *it);
				m_pReferenceListCtrl->SetItemText(id, 1, (*it)->GetOriginalFilePath().Chars());

				m_VisualisationMap.Insert(pPath, id);
			}
		} else {
			{
				StringTreeMap::Iterator treeItem = m_GlobalMap.Find(pPath);

				if (treeItem != m_GlobalMap.End())
					SetTreeItemImage(m_pReferenceListCtrl, treeItem->GetValue(), *it);
				else
				{
					wxTreeItemId id = m_pReferenceListCtrl->AppendItem(
						m_globalRoot,
						Misc::ExtractFilename((*it)->GetOriginalFilePath()).Chars(),
						-1,
						-1,
						new C_ReferenceData(*it));

					SetTreeItemImage(m_pReferenceListCtrl, id, *it);
					m_pReferenceListCtrl->SetItemText(id, 1, (*it)->GetOriginalFilePath().Chars());

					m_GlobalMap.Insert(pPath, id);
				}
			}

			{
				StringTreeMap::Iterator treeItem = m_LevelMap.Find(pPath);

				if (!m_Level)
					return;

				const EditorLib::AftkLevelRefList& references = m_Level->GetAftkRefs();

				for (EditorLib::AftkLevelRefList::ConstIterator levelRef = references.Begin(); levelRef != references.End(); ++levelRef)
				{
					if ((*levelRef)->pAftkRef->GetOriginalFilePath() == (*it)->GetOriginalFilePath())
					{
						if (treeItem != m_LevelMap.End())
							SetTreeItemImage(m_pReferenceListCtrl, treeItem->GetValue(), *it);
						else
						{
							wxTreeItemId id = m_pReferenceListCtrl->AppendItem(
								m_levelRoot,
								Misc::ExtractFilename((*it)->GetOriginalFilePath()).Chars(),
								-1,
								-1,
								new C_ReferenceData(*it));

							SetTreeItemImage(m_pReferenceListCtrl, id, *it);
							m_pReferenceListCtrl->SetItemText(id, 1, (*it)->GetOriginalFilePath().Chars());

							m_LevelMap.Insert(pPath, id);
						}
					}
				}				
			}
		}
	}
}

void C_ReferencePanel::RemoveRefNode(const char *pPath)
{
	{
		StringTreeMap::Iterator it = m_GlobalMap.Find(pPath);
		if (it != m_GlobalMap.End())
		{
			m_pReferenceListCtrl->Delete(it->GetValue());
			m_GlobalMap.Erase(it);
		}
	}

	{
		StringTreeMap::Iterator it = m_LevelMap.Find(pPath);
		if (it != m_LevelMap.End())
		{
			m_pReferenceListCtrl->Delete(it->GetValue());
			m_LevelMap.Erase(it);
		}
	}

	{
		StringTreeMap::Iterator it = m_VisualisationMap.Find(pPath);
		if (it != m_VisualisationMap.End())
		{
			m_pReferenceListCtrl->Delete(it->GetValue());
			m_VisualisationMap.Erase(it);
		}
	}
}

void C_ReferencePanel::AddRefNode(const EditorLib::C_AftkRef *pRef)
{
	AddRefNode(pRef->GetOriginalFilePath().Chars());
}

void C_ReferencePanel::RemoveRefNode(const EditorLib::C_AftkRef *pRef)
{
	RemoveRefNode(pRef->GetOriginalFilePath().Chars());
}

void C_ReferencePanel::SetActiveLevel(const EditorLib::I_Level* level)
{
	m_Level = level;

	m_pReferenceListCtrl->DeleteChildren(m_levelRoot);
	m_LevelMap.Clear();

	if (!m_Level)
		return;

	const EditorLib::AftkLevelRefList& references = level->GetAftkRefs();

	for (EditorLib::AftkLevelRefList::ConstIterator it = references.Begin(); it != references.End(); ++it)
	{
		const EditorLib::C_AftkRef* levelRef = (*it)->pAftkRef;

		wxTreeItemId itemId = m_pReferenceListCtrl->AppendItem(
			m_levelRoot, 
			Misc::ExtractFilename(levelRef->GetOriginalFilePath()).Chars(),
			-1,
			-1,
			new C_ReferenceData(levelRef));

		SetTreeItemImage(m_pReferenceListCtrl, itemId, levelRef);
		m_pReferenceListCtrl->SetItemText(itemId, 1, levelRef->GetOriginalFilePath().Chars());

		m_LevelMap.Insert(levelRef->GetOriginalFilePath(), itemId);
	}
}