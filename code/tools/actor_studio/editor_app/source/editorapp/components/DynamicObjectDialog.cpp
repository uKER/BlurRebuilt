#include "stdafx.h"

#include <Tools_Shared/Utility/MacroExpander.h>

#include <EditorApp/Components/DynamicObjectDialog.h>
#include <EditorApp/Editor/Manipulators/ActorCreateManipulator.h>

#include <EditorLib/Command/Dg_Cmd/CreateXtNodeCmd.h>
#include <EditorLib/Command/CommandFactory.h>
#include <EditorLib/Context/EditorContext.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Level/LevelHelpers.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/CoreConfig.h>
#include <EditorLib/GlobalSignals.h>

#include <wx/notebook.h>

using namespace Editor;

class wxFlowSizer: public wxSizer
{
public:
    wxFlowSizer( int orient );

    void RecalcSizes();
    wxSize CalcMin();

    int GetOrientation() const
        { return m_orient; }

    void SetOrientation(int orient)
        { m_orient = orient; }

protected:
    int m_orient;
	   
	int m_stretchable;
    int m_minWidth;
    int m_minHeight;
    int m_fixedWidth;
    int m_fixedHeight;

private:
    DECLARE_CLASS(wxFlowSizer)
};

IMPLEMENT_CLASS(wxFlowSizer, wxSizer)

wxFlowSizer::wxFlowSizer( int orient )
    : m_orient( orient )
{
}

void wxFlowSizer::RecalcSizes()
{
    if (m_children.GetCount() == 0)
        return;

	int width = GetSize().GetX();
	int currentOffsetX = 0;
	int currentOffsetY = 0;
	int maxRowSize = 0;

	wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->ShouldAccountFor())
        {
            wxSize size( item->GetMinSizeWithBorder() );

			if (size.GetY() > maxRowSize)
				maxRowSize = size.GetY();

			if (currentOffsetX + size.GetX() > width)
			{
				currentOffsetX = 0;
				currentOffsetY += maxRowSize;
			}

			wxPoint pos(currentOffsetX, currentOffsetY);
			item->SetDimension( pos, size );

			currentOffsetX += size.GetX();
		}

		node = node->GetNext();
	}
}

wxSize wxFlowSizer::CalcMin()
{
    if (m_children.GetCount() == 0)
        return wxSize();

    m_minWidth = 0;
	m_minHeight = 0;

	int width = GetSize().GetX();
    
	int currentOffsetX = 0;
	int currentOffsetY = 0;

	int maxColumnSize = 0;
	int maxRowSize = 0;

	wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->ShouldAccountFor())
        {
            wxSize size( item->GetMinSizeWithBorder() );

			if (size.GetX() > maxColumnSize)
				maxColumnSize = size.GetX();

			if (size.GetY() > maxRowSize)
				maxRowSize = size.GetY();

			if (currentOffsetX + size.GetX() > width)
			{
				currentOffsetX = 0;
				currentOffsetY += maxRowSize;
				maxRowSize = size.GetY();
			}

			wxPoint pos(currentOffsetX, currentOffsetY);
			item->SetDimension( pos, size );

			currentOffsetX += size.GetX();
		}

		node = node->GetNext();
	}

	m_minWidth = maxColumnSize;
	m_minHeight = currentOffsetY + maxRowSize;

    return wxSize( m_minWidth, m_minHeight );
}

C_DynamicObjectPanel::C_DynamicObjectPanel(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
: wxPanel(parent, winid, pos, size, style, name),
  m_Slots(this)
{
	wxSizer* pSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(pSizer);

	wxSizer* pComboRefreshSizer = new wxBoxSizer(wxHORIZONTAL);
	pSizer->Add(pComboRefreshSizer);

	m_pLevel = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT);
	pComboRefreshSizer->Add(m_pLevel, 0, 0, 5);

	m_pRefresh = new wxButton(this, wxID_ANY, "Refresh Objects");
	pComboRefreshSizer->Add(m_pRefresh, 0, 0, 5);

	m_pTabs = new wxNotebook(this, wxID_ANY);

	pSizer->Add(m_pTabs, 1, wxEXPAND|wxALL);

	Layout();

	m_pRefresh->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_DynamicObjectPanel::OnRefresh), NULL, this);
	m_pLevel->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(C_DynamicObjectPanel::OnLevelComboChanged), NULL, this);
	Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_DynamicObjectPanel::OnIdle), NULL, this);

	m_Slots.Connect();

	m_Flags.connected = true;
	m_Flags.dataDirty = false;

	EditorLib::C_CreateXtNodeCmd::GetContext()->RegisterPostCreate("dynamicObject", Ut::Delegate<void(EditorLib::I_DgNode*)>(this, &C_DynamicObjectPanel::OnCreateObject));
}

C_DynamicObjectPanel::~C_DynamicObjectPanel()
{
	
}

void C_DynamicObjectPanel::Shutdown()
{
	if (m_Flags.connected)
	{
		m_Flags.connected = false;
		m_Slots.Disconnect();
		m_pLevel->Disconnect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(C_DynamicObjectPanel::OnLevelComboChanged));
		Disconnect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_DynamicObjectPanel::OnIdle));
	}
}

void C_DynamicObjectPanel::OnRefresh(wxCommandEvent& e)
{
	Populate();
}

void C_DynamicObjectPanel::OnSelectObject(wxCommandEvent& e)
{
	m_ObjectId = m_WxXtMap[e.GetId()];
	m_ObjectName = ((wxButton*)e.GetEventObject())->GetLabelText().ToAscii();

	if (!EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel())
		return;

	Xt::DomRecord* pRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(m_ObjectId);

	if (!pRecord)
		return;

	LOGDEBUG(EditorLib::C_Core::Instance().GetLogger(), "Clicked dynamic object: %s (%X)", pRecord->name.Text(), m_ObjectId);

	C_ActorCreateManipulatorContext actorCreateContext;
	actorCreateContext.typeName = "PhysicalObjectActor";
	actorCreateContext.postCreate = "dynamicObject";

	EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Create, &actorCreateContext);
}

void C_DynamicObjectPanel::OnLevelComboChanged(wxCommandEvent& e)
{
	Btl::String level = m_pLevel->GetValue().ToAscii();
	
	PopulateTabs(level);
}

void C_DynamicObjectPanel::OnIdle(wxIdleEvent& e)
{
	if (m_Flags.dataDirty)
		Populate();
}

void C_DynamicObjectPanel::OnCreateObject(EditorLib::I_DgNode* dgNode)
{
	if (!m_ObjectId)
		return;

	EditorLib::C_XtObject* pXtObject = dgNode->AsXtObject();

	Xt::DomElement* pElement = pXtObject->GetXtData();

	Xt::DomComponent* pComponent = Xt::AsComponent(pElement);

	if (!pComponent)
		return;

	LarvaType* type = FindTypeByName(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pComponent->type);

	LtStruct* typeStruct = AsStruct(type);

	if (!typeStruct)
		return;

	Xt::FieldDom definition;

	Xt::FieldDom object = AcquireBySemantic(typeStruct, &pComponent->data, "SceneView", "dynamicobject", "object");
	if (object)
	{
		definition = AcquireBySemantic(AsStruct(object.field->Type()), Xt::AsBlock(object.dom), "SceneView", "dynamicobject", "definition");
	} else {
		definition = AcquireBySemantic(typeStruct, &pComponent->data, "SceneView", "dynamicobject", "definition");
	}

	if (!definition)
		return;

	Xt::DomField* pField = Xt::AsField(definition.dom);

	if (!pField)
		return;

	Xt::Set(pField, Btl::String::Format("%X", m_ObjectId).Chars());

	definition = AcquireBySemantic(typeStruct, &pComponent->data, "SceneView", "semantic", "Name");

	if (!definition)
		return;

	pField = Xt::AsField(definition.dom);

	if (!pField)
		return;

	Xt::Set(pField, EditorLib::LevelHelpers::GetNextFreeName(m_ObjectName).Chars());
}

void C_DynamicObjectPanel::RefreshVisualisationLocations(Btl::String location, U32 depth)
{
	if (depth > 2)
		return;

	Sys::C_DirectoryInfo info;
	Sys::GetDefaultFileSystem()->DirectoryInfo(location, &info);
	for (U32 i=0; i<info.NumFiles(); i++)
	{
		if (info.FileInfo(i).m_FileType == Sys::E_FileType_Directory)
		{
			Btl::Vector<Sys::C_FilePath> filePaths;
			info.FileInfo(i).m_FileName.TokeniseByDir(filePaths);
			if (filePaths[filePaths.Size()-1] == "Instance_References")
				PopulateVisualisations(info.FileInfo(i).m_FileName.String(), filePaths[filePaths.Size()-2].String());
			else
				RefreshVisualisationLocations(info.FileInfo(i).m_FileName.String(), depth+1);
		}
	}
}

void C_DynamicObjectPanel::PopulateVisualisations(Btl::String filePath, Btl::String visualisation)
{
	Sys::C_DirectoryInfo category;
	Sys::GetDefaultFileSystem()->DirectoryInfo(filePath, &category);
	for (U32 i=0; i<category.NumFiles(); i++)
	{
		if (category.FileInfo(i).m_FileType == Sys::E_FileType_Directory)
		{
			Sys::C_DirectoryInfo instances;
			Sys::GetDefaultFileSystem()->DirectoryInfo(category.FileInfo(i).m_FileName, &instances);

			for (U32 j=0; j<instances.NumFiles(); j++)
			{
				if (instances.FileInfo(j).m_FileType == Sys::E_FileType_Directory)
				{
					Btl::Vector<Sys::C_FilePath> filePaths;
					instances.FileInfo(j).m_FileName.TokeniseByDir(filePaths);

					if (filePaths[filePaths.Size()-1] == "thumbnails")
					{
						Sys::C_DirectoryInfo thumbs;
						Sys::GetDefaultFileSystem()->DirectoryInfo(instances.FileInfo(j).m_FileName, &thumbs);

						for (U32 k=0; k<thumbs.NumFiles(); k++)
						{
							if (thumbs.FileInfo(k).m_FileType == Sys::E_FileType_File)
							{
								Btl::String visLocation = visualisation + "." + Sys::ExtractFilename(category.FileInfo(i).m_FileName).String() + "." + Sys::ExtractFilename(thumbs.FileInfo(k).m_FileName).String();
								visLocation = visLocation(0, visLocation.Length()-1-Sys::ExtractFileExtension(thumbs.FileInfo(k).m_FileName).Length());
								visLocation = visLocation.Lower();
								m_Visualisations[visLocation] = thumbs.FileInfo(k).m_FileName.String();
							}
						}
					}
				}
			}
		}
	}
}

void C_DynamicObjectPanel::Populate()
{
	m_WxXtMap.Clear();
	m_DynamicObjects.Clear();
	m_Visualisations.Clear();

	Btl::String visualisationLocation = Btl::String::Format("%%BIZARRE_ROOT%%\\%s\\assets", EditorLib::C_Core::Instance().GetConfig().projectName.Chars());
	Tools::C_MacroExpander::Instance().Expand(visualisationLocation);
	RefreshVisualisationLocations(visualisationLocation);
	
	for (XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&EditorLib::C_Core::Instance().GetXtManager().GetDb()->packs); packIt; ++packIt)
	{
		for (XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator recordIt(&packIt->records); recordIt; ++recordIt)
		{
			LtStruct* pType = Xt::TypeOf(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, &(*recordIt));
			if (!pType)	continue;
			
			LarvaParam* param = pType->FindParam("SceneView", "dynamicobject");
			if (param && param->AsString() == "definition")
			{
				m_DynamicObjects.PushBack(Btl::Pair<LtStruct*, Xt::DomRecord*>(pType, &(*recordIt)));
			}
		}
	}

	m_pLevel->Clear();

	Btl::Set<Btl::String> comboSet;
	for (Btl::Vector<Btl::Pair<LtStruct*,Xt::DomRecord*> >::Iterator it = m_DynamicObjects.Begin(); it != m_DynamicObjects.End(); ++it)
	{
		Btl::Vector<Btl::String> splitName;
		Btl::String(it->second->name.Text()).Tokenise(splitName, '.');

		if (splitName.Size() < 3)
			continue;

		if (comboSet.Find(splitName[0]) != comboSet.End())
			continue;

		comboSet.Insert(splitName[0]);

		wxString item = splitName[0].Chars();

		m_pLevel->Append(item);
	}

	m_pLevel->Select(0);

	PopulateTabs(m_pLevel->GetValue().ToAscii());
	
	Layout();
	Refresh();

	m_Flags.dataDirty = false;
}

wxWindow* C_DynamicObjectPanel::GetTab(const Btl::String& tabName)
{
	if (m_Tabs.Find(tabName) != m_Tabs.End())
		return m_Tabs[tabName];

	/*wxPanel* pPanel = new wxPanel(m_pTabs);*/
	wxScrolledWindow* pWindow = new wxScrolledWindow(m_pTabs);
	pWindow->SetScrollRate(10,10);

	m_pTabs->AddPage(pWindow, tabName.Chars());

	wxSizer* labelSizer = new wxFlowSizer(wxHORIZONTAL);
	pWindow->SetSizer(labelSizer);

	m_Tabs[tabName] = pWindow;

	return pWindow;
}

void C_DynamicObjectPanel::PopulateTabs(const Btl::String& level)
{
	m_Tabs.Clear();
	m_pTabs->DeleteAllPages();

	for (Btl::Vector<Btl::Pair<LtStruct*,Xt::DomRecord*> >::Iterator it = m_DynamicObjects.Begin(); it != m_DynamicObjects.End(); ++it)
	{
		Btl::Vector<Btl::String> splitName;
		Btl::String(it->second->name.Text()).Tokenise(splitName, '.');

		if (splitName.Size() < 3)
			continue;

		if (splitName[0] != level)
			continue;

		wxWindow* pPanel = GetTab(splitName[1]);

		wxButton* button;

		Btl::String name;

		for (XtUt::List<LarvaField>::Iterator modelField(&it->first->fields); modelField; ++modelField)
		{
			LarvaParam* param = modelField->attrs.FindParam("SceneView", "dynamicobject");

			if (param && param->AsString() == "model")
			{
				Xt::DomElement* pElement = Xt::At(&it->second->root.data, modelField);
				Xt::DomField* pField = Xt::AsField(pElement);
				if (!pField)
					continue;

				XtId modelId = Xt::ParseId(pField->value);

				Xt::DomRecord* pModelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(modelId);
				
				LtStruct* pType = Xt::TypeOf(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pModelRecord);
				if (!pType) continue;

				for (XtUt::List<LarvaField>::Iterator filenameField(&pType->fields); filenameField; ++filenameField)
				{
					LarvaParam* param = filenameField->attrs.FindParam("SceneView", "dynamicobject");

					if (param && param->AsString() == "modelfilename")
					{
						Xt::DomElement* pElement = Xt::At(&pModelRecord->root.data, filenameField);
						Xt::DomField* pField = Xt::AsField(pElement);
						
						if (!pField)
							continue;

						name = pField->value.Text();
					}
				}

				break;
			}
		}

		// HACK : Replace when we have our own proper vis images.
		if (name.Length() > (S32)(strlen("models/") + strlen(".model")))
		{
			name = name(strlen("models/"), name.Length());
			name = name(0, name.Length()-strlen(".model"));
			name = name.Replace("\\", ".");
			name = name.Lower();
		}

		if (m_Visualisations.Find(name) != m_Visualisations.End())
		{
			wxImage image;
			image.LoadFile(m_Visualisations[name].Chars());
			image.Rescale(64, 64, wxIMAGE_QUALITY_NORMAL);
			button = new wxBitmapButton(pPanel, wxID_ANY, wxBitmap(image), wxDefaultPosition, wxSize(64,64), 4, wxDefaultValidator, splitName[2].Chars());
			button->SetLabel(splitName[2].Chars());
		} else {
			button = new wxButton(pPanel, wxID_ANY, splitName[2].Chars(), wxDefaultPosition, wxSize(64, 64));
		}

		button->SetToolTip(it->second->name.Text());

		button->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_DynamicObjectPanel::OnSelectObject), NULL, this);

		pPanel->GetSizer()->Add(button);

		m_WxXtMap[button->GetId()] = it->second->id;
	}

	for (Btl::OpenHashMap<Btl::String, wxWindow*, Btl::HashKeyOpBtlString>::Iterator it = m_Tabs.Begin(); it != m_Tabs.End(); ++it)
	{
		it->GetValue()->InvalidateBestSize();
		wxSize size = it->GetValue()->GetBestVirtualSize();
        it->GetValue()->SetVirtualSize(size);

		it->GetValue()->Refresh();
	}
}

C_DynamicObjectPanel::C_Slots::C_Slots(C_DynamicObjectPanel* pPanel)
{
	m_pDynamicObjectPanel = pPanel;
}

void C_DynamicObjectPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.connect(this, &C_DynamicObjectPanel::C_Slots::OnDatabaseEventSlot);
}

void C_DynamicObjectPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.disconnect(this);
}

void C_DynamicObjectPanel::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent e)
{
	if (e == EditorLib::E_DatabaseEvent_Opened)
	{
		m_pDynamicObjectPanel->m_Flags.dataDirty = true;
	}
}