#include "stdafx.h"

#include <wx/clipbrd.h>

#include <xtwin/WndCore.h>
#include <xtwin/Scarab.h>

#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Components/LevelExplorer.h>
#include <EditorApp/Components/SorceryDialog.h>
#include <EditorApp/Components/SplitterControl.h>

#include <EditorApp/Editor/Manipulators/DropperSelectManipulator.h>

using namespace Editor;

class C_SorceryContext : public ScarabFormContext
{
public:
	C_SorceryContext(C_SorceryPanel* pPanel);
	virtual ~C_SorceryContext();

public:
	virtual void OpenRecord(Xt::DomRecord* record);
	virtual void BeginDropper(LarvaField* field, Xt::DomElement* element);

	C_SorceryPanel* m_pSorceryPanel;
};

C_SorceryExplorer::C_SorceryExplorer(C_SorceryPanel* pPanel)
{
	m_pSorceryPanel = pPanel;
}

void C_SorceryExplorer::OnActivateItem(ExplorerData* pData)
{
	if (pData->GetType() == kExItemTypeRecord)
		m_pSorceryPanel->OpenRecord(pData->GetDomRecord());
}

HMENU C_SorceryExplorer::AppendNewMenu(HMENU rootMenu, Explorer::ExplorerData* item, Btl::OpenHashMap<UINT, Btl::String>& typeHash)
{
	HMENU hMenuItemNew = CreatePopupMenu();

	AppendMenu(hMenuItemNew, MF_STRING, kExMenuItemNewFolder, "New Folder");
	AppendMenu(hMenuItemNew, MF_SEPARATOR, 0, "");

	int recordCount = 0;

	Btl::Map<Btl::String, HMENU> categoryMenus;

	LarvaSchema* pSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;
	for (XtUt::Dictionary<XtString, LarvaType>::Iterator type(&pSchema->types); type; ++type, ++recordCount)
	{
		Btl::String category = type->category.Text();

		if (category == "")
			category = "(Misc)";
		
		HMENU categoryMenu = 0;
		if (categoryMenus.Find(category) == categoryMenus.End())
		{
			HMENU hMenuItemCategory = CreatePopupMenu();
			categoryMenus[category] = hMenuItemCategory;
		}

		categoryMenu = categoryMenus[category];

		if (!type->hidden && type->attrs.FindAttr("Record"))
		{
			int flags = MF_STRING;
			if (GetMenuItemCount(categoryMenu) != 0 && (GetMenuItemCount(categoryMenu)%40)==0)
				flags |= MF_MENUBARBREAK;
			AppendMenu(categoryMenu, flags, kExMenuItemNewRecord + recordCount, type->label.Text());

			typeHash[kExMenuItemNewRecord + recordCount] = type->name.Text();
		}
	}

	for (Btl::Map<Btl::String, HMENU>::Iterator it = categoryMenus.Begin(); it != categoryMenus.End(); ++it)
	{
		AppendMenu(hMenuItemNew, MF_POPUP, (UINT)it->GetValue(), it->GetKey().Chars());
	}

	AppendMenu(rootMenu, MF_POPUP, (UINT)hMenuItemNew, "New");

	return hMenuItemNew;
}

void C_SorceryExplorer::OnContextMenu(Explorer::ExplorerData* item, const WinPoint& pt)
{
	HMENU hContextMenu = CreatePopupMenu();

	Btl::OpenHashMap<UINT, Btl::String> typeHash;

	switch (item->GetType())
	{
		case (kExItemTypeRoot):
		{
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewPack, "New Pack");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewFolder, "New Folder");
			break;
		}
		case (kExItemTypeFilterPack):
		{
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewPack, "New Pack");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemNewFolder, "New Folder");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemRename, "Rename");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDelete, "Delete");
			break;
		}
		case (kExItemTypePack):
		{
			AppendNewMenu(hContextMenu, item, typeHash);
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemRename, "Rename");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDelete, "Delete");
			break;
		}
		case (kExItemTypeFilterRecord):
		{
			AppendNewMenu(hContextMenu, item, typeHash);
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemRename, "Rename");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDelete, "Delete");
			//AppendMenu(hContextMenu, MF_STRING, kExMenuItemDuplicate, "Duplicate");
			break;
		}
		case (kExItemTypeRecord):
		{
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemOpen, "Open");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemRename, "Rename");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDelete, "Delete");
			AppendMenu(hContextMenu, MF_STRING, kExMenuItemDuplicate, "Duplicate");
			AppendMenu(hContextMenu, MF_SEPARATOR, 0, "");

			AppendMenu(hContextMenu, MF_STRING, kExMenuItemId, Format("Id = %X", item->GetDomRecord()->id).Text());
			break;
		}
	}

	POINT winPt;
	winPt.x = pt.x;
	winPt.y = pt.y;
	ClientToScreen(_hwnd, &winPt);

	int menuItem = TrackPopupMenuEx(hContextMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD, winPt.x, winPt.y, _hwnd, 0);
	switch (menuItem)
	{
	case kExMenuItemNewPack:
		{
			CreateNewPack(item);
			break;
		}
	case kExMenuItemNewFolder:
		{
			CreateNewFolder(item);
			break;
		}
	case kExMenuItemNewRecord:
		{
			CreateNewRecord(item, "");
			break;
		}
	case kExMenuItemRename:
		{
			StartRename(item);
			break;
		}
	case kExMenuItemDelete:
		{
			Delete(item);
			break;
		}
	case kExMenuItemDuplicate:
		{
			DuplicateItem(item);
			break;
		}
	case kExMenuItemOpen:
		{
			OnActivateItem(item);
			break;
		}
	case kExMenuItemId:
		{
			if (wxTheClipboard->Open()) {	
				wxTheClipboard->SetData( new wxTextDataObject(Format("%X", item->GetDomRecord()->id).Text()) );
				wxTheClipboard->Close();
			}
			break;
		}
	default:
		{
			if (menuItem >= kExMenuItemNewRecord)
			{
				CreateNewRecord(item, typeHash[menuItem]);
			}
		}
	}	

	DestroyMenu(hContextMenu);
}

C_SorceryContext::C_SorceryContext(C_SorceryPanel* pPanel) : ScarabFormContext()
{
	m_pSorceryPanel = pPanel;
}

C_SorceryContext::~C_SorceryContext()
{
}

void C_SorceryContext::OpenRecord(Xt::DomRecord* pRecord)
{
	m_pSorceryPanel->OpenRecord(pRecord);
}

void C_SorceryContext::BeginDropper(LarvaField* field, Xt::DomElement* element)
{
	if (field->kind == kLfWeakPtr)
	{
		DropperUserData data = {field, element};
		EditorLib::C_Core::Instance().SetContext("dropperselect", &data);
	}
}

C_SorceryRecordWindow::C_SorceryRecordWindow(C_SorceryPanel* pPanel, wxMDIParentFrame* pParent, Xt::DomRecord* pRecord, long style)
: wxMDIChildFrame(pParent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX|wxTHICK_FRAME|wxSYSTEM_MENU|wxCAPTION),
  m_Slots(this)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer( wxVERTICAL );

	m_pScarabFormPanel = new wxPanel(this);

	m_pScarabForm = new ScarabForm(new C_SorceryContext(pPanel));
	m_pScarabForm->Create((HWND)m_pScarabFormPanel->GetHandle());
	m_pScarabForm->SetRecord(pRecord);

	SetLabel(pRecord->name.Text());

	pSizer0->Add(m_pScarabFormPanel, 1, wxALL|wxEXPAND, 0);

	SetSizer(pSizer0);
	Layout();

	WinRect rcClient((HWND)m_pScarabFormPanel->GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pScarabFormPanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_SorceryRecordWindow::OnSize), NULL, this);

	m_Slots.Connect();

	m_Connected = true;
}

C_SorceryRecordWindow::~C_SorceryRecordWindow()
{
	Shutdown();
}

void C_SorceryRecordWindow::Shutdown()
{
	if (m_Connected)
	{
		m_Slots.Disconnect();

		m_pScarabFormPanel->Disconnect(wxEVT_SIZE, wxSizeEventHandler(C_SorceryRecordWindow::OnSize), NULL, this);

		SAFE_DELETE_PTR(m_pScarabForm);

		m_Connected = false;
	}
}

void C_SorceryRecordWindow::OnSize(wxSizeEvent& e)
{
	WinRect rcClient((HWND)m_pScarabFormPanel->GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

C_SorceryRecordWindow::C_Slots::C_Slots(C_SorceryRecordWindow* pForm)
{
	m_pSorceryWindow = pForm;
}

void C_SorceryRecordWindow::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.connect(this, &C_SorceryRecordWindow::C_Slots::OnDatabaseEventSlot);
}

void C_SorceryRecordWindow::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.disconnect(this);
}

void C_SorceryRecordWindow::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent e)
{
	if (e == EditorLib::E_DatabaseEvent_Closing)
	{
		//m_pSorceryWindow->Shutdown(); // Why does this cause a crash?
		m_pSorceryWindow->Destroy();
	}
}

C_SorceryPanel::C_SorceryPanel(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
: wxPanel(parent, winid, pos, size, style, name),
  m_Slots(this)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer( wxVERTICAL );
	
	m_pSplitter = new wxSplitterCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxSP_SASH_BORDERS);

	m_pExplorerPanel = new wxPanel(this);

	m_pExplorer = new C_SorceryExplorer(this);
	m_pExplorer->Create((HWND)m_pExplorerPanel->GetHandle());

	m_pMDIFrame = new wxMDIParentFrame(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);

	m_pSplitter->Split(E_SplitterLayout_2Vertical, m_pExplorerPanel, m_pMDIFrame);
	m_pSplitter->SetSplitterProportion(2500);

	pSizer0->Add(m_pSplitter, 1, wxALL|wxEXPAND, 0);

	SetSizer(pSizer0);
	Layout();

	WinRect rcClient((HWND)m_pExplorerPanel->GetHandle());
	SetWindowPos(*m_pExplorer, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pExplorerPanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_SorceryPanel::OnSize), NULL, this);

	m_Slots.Connect();

	m_Flags.Connected = true;
}

C_SorceryPanel::~C_SorceryPanel()
{
	Shutdown();
}

void C_SorceryPanel::Shutdown()
{
	if (m_Flags.Connected)
	{
		m_Flags.Connected = false;

		m_Slots.Disconnect();

		m_pExplorerPanel->Disconnect(wxEVT_SIZE, wxSizeEventHandler(C_SorceryPanel::OnSize), NULL, this);

		wxWindowList& children = m_pMDIFrame->GetChildren();
		for (wxWindowList::const_iterator it = children.begin(); it != children.end(); ++it)
		{
			((C_SorceryRecordWindow*)(*it))->Shutdown();
		}
		
		delete m_pExplorer;
	}
}

void C_SorceryPanel::OpenRecord(Xt::DomRecord* pRecord)
{
	C_SorceryRecordWindow* recordWindow = new C_SorceryRecordWindow(this, m_pMDIFrame, pRecord);
}

void C_SorceryPanel::OnSize(wxSizeEvent& e)
{
	WinRect rcClient((HWND)m_pExplorerPanel->GetHandle());
	SetWindowPos(*m_pExplorer, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

C_SorceryPanel::C_Slots::C_Slots(C_SorceryPanel* pForm)
{
	m_pSorceryPanel = pForm;
}

void C_SorceryPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.connect(this, &C_SorceryPanel::C_Slots::OnDatabaseEventSlot);
}

void C_SorceryPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.disconnect(this);
}

void C_SorceryPanel::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent e)
{
	if (e == EditorLib::E_DatabaseEvent_Closing)
		m_pSorceryPanel->m_pExplorer->SetDb(0);
	else if (e == EditorLib::E_DatabaseEvent_Opened)
		m_pSorceryPanel->m_pExplorer->SetDb(&db);
}

