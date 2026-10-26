#include "stdafx.h"
#include <wx/aui/aui.h>

#include <xtwin/WndCore.h>
#include <xtwin/WndClasses.h>
#include <xtwin/Scarab.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Components/IView.h>
#include <EditorApp/Components/Scarab_Form/WxScarabForm.h>
#include <EditorApp/Components/Scarab_Form/WxScarabFormContext.h>

#include <EditorApp/Editor/Manipulators/DropperSelectManipulator.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>
#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Config.h>
#include <EditorApp/App.h>

namespace Editor
{
class C_WxChildScarabForm;

class C_WxChildScarabFormContext : public ScarabFormContext
{
public:
	C_WxChildScarabFormContext (C_WxChildScarabForm* pForm);
	~C_WxChildScarabFormContext();

	void OpenRecord(Xt::DomRecord* record);
	void BeginDropper(LarvaField* field, Xt::DomElement* element);

	C_WxChildScarabForm* m_pWxChildScarabForm;
};


class C_WxChildScarabForm : public wxWindow
{
public:
	C_WxChildScarabForm(wxWindow* pParent, 
						wxWindowID id=wxID_ANY,
						const wxPoint& pos = wxDefaultPosition,
						const wxSize size = wxDefaultSize,
						long style = wxDEFAULT_FRAME_STYLE);

	~C_WxChildScarabForm();

	bool SetRecord(Xt::DomRecord* pRecord);
	void OnSize(wxSizeEvent& e);
	bool MSWShouldPreProcessMessage(WXMSG* msg);

private:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		C_Slots(C_WxChildScarabForm *pForm);

		void Connect();
		void Disconnect();
		void OnLevelClosing(const EditorLib::I_Level& level);

	private:
		C_WxChildScarabForm	*m_pWxChildScarabForm;
	};


	ScarabForm *m_pScarabForm;
	C_Slots		m_slots;
};






C_WxChildScarabFormContext::C_WxChildScarabFormContext(C_WxChildScarabForm* pForm)
: m_pWxChildScarabForm(pForm)
{
}

C_WxChildScarabFormContext::~C_WxChildScarabFormContext()
{
}

void C_WxChildScarabFormContext::OpenRecord(Xt::DomRecord* record)
{
	wxString name = wxString::Format("%d", record->id);

	C_Frame *pFrame = wxGetApp().GetFrame();
	assert(pFrame);
	wxAuiManager *pManager = pFrame->GetAuiManager();
	assert(pManager);

	wxAuiPaneInfo& info = pManager->GetPane(name);
	if (!info.window)
	{
		C_WxChildScarabForm *pChildForm = new C_WxChildScarabForm(pFrame);

		pManager->AddPane(pChildForm, wxAuiPaneInfo().
			Name(name).Caption(record->name.Text()).DestroyOnClose(true).
			CloseButton(true).Dockable(false).Float().Show(true).BestSize(m_pWxChildScarabForm->GetSize()));

		pChildForm->SetRecord(record);
		pManager->Update();
	}
}

void C_WxChildScarabFormContext::BeginDropper(LarvaField* field, Xt::DomElement* element)
{
}





C_WxChildScarabForm::C_Slots::C_Slots(C_WxChildScarabForm *pForm)
: m_pWxChildScarabForm(pForm)
{
}

void C_WxChildScarabForm::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
	s.levelClosed.connect(this, &C_WxChildScarabForm::C_Slots::OnLevelClosing);
}

void C_WxChildScarabForm::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
	s.levelClosed.disconnect(this);
}

void C_WxChildScarabForm::C_Slots::OnLevelClosing(const EditorLib::I_Level& level)
{
	C_Frame *pFrame = wxGetApp().GetFrame();
	assert(pFrame);
	wxAuiManager *pManager = pFrame->GetAuiManager();
	assert(pManager);

	pManager->DetachPane(m_pWxChildScarabForm);
	m_pWxChildScarabForm->Destroy();
	pManager->Update();
}


C_WxChildScarabForm::C_WxChildScarabForm(wxWindow* pParent, wxWindowID id, const wxPoint& pos, const wxSize size, long style)
: wxWindow(pParent, id, pos, size, style)
, m_pScarabForm(0)
, m_slots(this)
{
	Connect(GetId(), wxEVT_SIZE, wxSizeEventHandler(C_WxChildScarabForm::OnSize));
	m_slots.Connect();
}

C_WxChildScarabForm::~C_WxChildScarabForm()
{
	delete m_pScarabForm;
	m_slots.Disconnect();
}

bool C_WxChildScarabForm::SetRecord(Xt::DomRecord* pRecord)
{
	if (m_pScarabForm)
		delete m_pScarabForm;

	if ((m_pScarabForm = new ScarabForm(new C_WxChildScarabFormContext(this))) == 0)
		return false;

	m_pScarabForm->Create((HWND)GetHandle());

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pScarabForm->SetRecord(pRecord);

	return true;
}

void C_WxChildScarabForm::OnSize(wxSizeEvent& e)
{
	if (!m_pScarabForm)
		return;

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

bool C_WxChildScarabForm::MSWShouldPreProcessMessage(WXMSG* msg)
{
	if (msg->hwnd!=GetHWND() && msg->message == WM_KEYDOWN)	//keydown events from a non-wx child, ie. the scarabform
	{
		char className[256];
		if (::GetClassName(::GetFocus(), className, sizeof(className)))
		{
			//only allow msg pre-processing if event is from one of the scarabform container windows
			if (strnicmp(className,"WindowClassImpl",15)==0 ||
				strnicmp(className,"FrameWindowClassImpl",20)==0 ||
				strnicmp(className,"MdiWindowClassImpl",18)==0)
				return true;
		}
		return false;
	}
	return wxWindow::MSWShouldPreProcessMessage(msg);
}





C_WxScarabFormContext::C_WxScarabFormContext(C_WxScarabForm* pForm) : ScarabFormContext()
{
	m_pWxScarabForm = pForm;
}

C_WxScarabFormContext::~C_WxScarabFormContext()
{
}

void C_WxScarabFormContext::OpenRecord(Xt::DomRecord* record)
{
	wxString name = wxString::Format("%d", record->id);

	C_Frame *pFrame = wxGetApp().GetFrame();
	assert(pFrame);
	wxAuiManager *pManager = pFrame->GetAuiManager();
	assert(pManager);

	wxAuiPaneInfo& info = pManager->GetPane(name);
	if (!info.window)
	{
		C_WxChildScarabForm *pChildForm = new C_WxChildScarabForm(pFrame);

		pManager->AddPane(pChildForm, wxAuiPaneInfo().
			Name(name).Caption(record->name.Text()).DestroyOnClose(true).
			CloseButton(true).Dockable(false).Float().Show(true).BestSize(m_pWxScarabForm->GetSize()));

		pChildForm->SetRecord(record);
		pManager->Update();
	}
}

void C_WxScarabFormContext::BeginDropper(LarvaField* field, Xt::DomElement* element)
{
	if (field->kind == kLfWeakPtr)
	{
		DropperUserData data = {field, element};
		EditorLib::C_Core::Instance().SetContext("dropperselect", &data);
	}
}

}
