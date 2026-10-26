#include "stdafx.h"

#include <xtwin/HyperView.h>

#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Components/IView.h>
#include <EditorApp/Components/SorceryHypergraphPanel.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>
#include <EditorApp/Config.h>
#include <EditorApp/App.h>

namespace Editor
{

C_SorceryHypergraphPanel::C_SorceryHypergraphPanel(
	wxWindow *pParent,
	const wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxWindow(pParent, id, pos, size, style),
	m_slots(this)
{
	srand(GetTickCount());

	m_pHyperViewForm = 0;
	m_flags.dirty = false;
	m_viewEnabled = 1;

	Connect(GetId(), wxEVT_SIZE, wxSizeEventHandler(C_SorceryHypergraphPanel::OnSize));
	Connect(wxID_ANY, wxEVT_SHOW, wxShowEventHandler(C_SorceryHypergraphPanel::OnShow));
	Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_SorceryHypergraphPanel::OnIdle));
	Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(C_SorceryHypergraphPanel::OnKeyDown));

	m_slots.Connect();
}

C_SorceryHypergraphPanel::~C_SorceryHypergraphPanel()
{
	Disconnect(GetId(), wxEVT_SIZE, wxSizeEventHandler(C_SorceryHypergraphPanel::OnSize));
	Disconnect(wxID_ANY, wxEVT_SHOW, wxShowEventHandler(C_SorceryHypergraphPanel::OnShow));
	Disconnect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_SorceryHypergraphPanel::OnIdle));
	Disconnect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(C_SorceryHypergraphPanel::OnKeyDown));

	delete m_pHyperViewForm;
}

void C_SorceryHypergraphPanel::Shutdown()
{
	m_slots.Disconnect();
}

bool C_SorceryHypergraphPanel::SetRecord(Xt::DomRecord* pRecord)
{
	if (m_pHyperViewForm)
		delete m_pHyperViewForm;

	m_pHyperViewForm = new HyperViewForm(EditorLib::C_Core::Instance().GetXtManager().GetDb());

	m_pHyperViewForm->Create((HWND)GetHandle());

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pHyperViewForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pHyperViewForm->SetDom(pRecord);
	return true;
}

bool C_SorceryHypergraphPanel::SetDom(Xt::DomElement* pElement, LarvaType* pLarvaType, bool enabled)
{
	if (!m_pHyperViewForm)
	{
		m_pHyperViewForm = new HyperViewForm(EditorLib::C_Core::Instance().GetXtManager().GetDb());

		m_pHyperViewForm->Create((HWND)GetHandle());
	}

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pHyperViewForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pHyperViewForm->SetDom(pElement);
	Enable(enabled);
	return true;
}

bool C_SorceryHypergraphPanel::SetDom(Xt::DomBlock* pElement, LarvaField* pLarvaField, bool enabled)
{
	if (!m_pHyperViewForm)
	{
		m_pHyperViewForm = new HyperViewForm(EditorLib::C_Core::Instance().GetXtManager().GetDb());

		m_pHyperViewForm->Create((HWND)GetHandle());
	}

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pHyperViewForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pHyperViewForm->SetDom(pElement);
	Enable(enabled);

	return true;
}

void C_SorceryHypergraphPanel::OnCoreEnableViews(bool enable)
{
	m_viewEnabled += enable ? 1 : -1;
}

void C_SorceryHypergraphPanel::OnSelectionChanged()
{
	if (!m_viewEnabled)
		return;

	m_currentType = "";
	m_currentId = EditorLib::C_NodeId();

	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	//workaround compiler error obtaining a ConstReverseIterator from a const object
	EditorLib::C_Selection::ItemArray& items = const_cast<EditorLib::C_Selection::ItemArray&>(selection.GetItems());
	EditorLib::C_Selection::ItemArray::ConstReverseIterator it = items.ReverseBegin();

	while (it != items.ReverseEnd())
	{
		EditorLib::I_DgNode *pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
		if (!pNode)
			it++;
		else
			break;
	}

	if (it == items.ReverseEnd())
	{
		//Nothing selected.
		delete m_pHyperViewForm;
		m_pHyperViewForm = 0;
		return;
	}

	//Select last item in list.
	EditorLib::I_DgNode* pNode = 
		EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
	EditorLib::C_XtObject* pXtObject;
	if (pNode)
		pXtObject = pNode->AsXtObject();
	else
		pXtObject = 0;
		
	if (!pXtObject || !pXtObject->GetXtData() || pNode->IsA(EditorLib::E_ObjType_SceneAftkNode))
	{
		//Invalid node.
		delete m_pHyperViewForm;
		m_pHyperViewForm = 0;		
		return;
	}

	bool setOkay = false;
	if (pXtObject->GetXtData()->kind == Xt::kDomBlock) 
	{
		setOkay = SetDom(pXtObject->GetXtData(), pXtObject->GetXtType(), !pNode->IsReadOnly());
	} 
	else if (pXtObject->GetXtData()->kind == Xt::kDomComponent) 
	{
		setOkay = SetDom(Xt::AsComponent(pXtObject->GetXtData()), pXtObject->GetXtType(), !pNode->IsReadOnly());
	}
	else 
	{
		if (!pNode->IsA(EditorLib::E_ObjType_DgXtPropNodeBase))
			return;

		EditorLib::C_DgXtPropNodeBase* pXtPropNode = (EditorLib::C_DgXtPropNodeBase*)pNode;
		EditorLib::C_DgXtCompNode* pXtPropNodeParent = pXtPropNode->GetParent();
		EditorLib::C_XtObject* pXtParentObject = pXtPropNodeParent->AsXtObject();
		LarvaField* pField = FindFieldByName(AsStruct(pXtParentObject->GetXtType()), pXtObject->GetXtData()->parentKey);

		setOkay = SetDom(Xt::AsBlock(pXtObject->GetXtData()->parent), pField, !pNode->IsReadOnly());
	}

	if (setOkay)
	{
		//Re-acquire node as form xt acquire methods invoked may have caused untold changes to xt data.
		EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);

		if (pNode)
		{
			m_currentId = pNode->GetId();
			m_currentType = GetRootXtNodeType(pNode);
		}
	}
	else
	{
		m_currentId = EditorLib::C_NodeId();
		m_currentType = "";
	}
}

void C_SorceryHypergraphPanel::OnNodeUpdated(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection*)
{
	if (dgNode.GetId() != m_currentId)
		return;

	if (dgNode.AsXtObject() && 
		m_currentType != GetRootXtNodeType(&dgNode))
		m_flags.dirty = true;
}

HyperViewForm* C_SorceryHypergraphPanel::GetHyperViewForm()
{
	return m_pHyperViewForm;
}

void C_SorceryHypergraphPanel::OnShow(wxShowEvent& e)
{
	if (e.GetShow())
	{
		m_flags.dirty = true;
		m_viewEnabled++;
	} else {
		m_viewEnabled--;
		SAFE_DELETE_PTR(m_pHyperViewForm);
	}
}

void C_SorceryHypergraphPanel::OnSize(wxSizeEvent& e)
{
	if (!m_pHyperViewForm)
		return;

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pHyperViewForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void C_SorceryHypergraphPanel::OnIdle(wxIdleEvent& e)
{
	if (m_flags.dirty && m_viewEnabled)
	{
		OnSelectionChanged();
		m_flags.dirty = false;
	}
}

void C_SorceryHypergraphPanel::Clear()
{
	SAFE_DELETE_PTR(m_pHyperViewForm);
	wxGetApp().Yield();
	m_flags.dirty = true;
	m_currentType = "";
	m_currentId = EditorLib::C_NodeId();
}

void C_SorceryHypergraphPanel::Refresh()
{
	//m_flags.dirty = true;
}

Btl::String C_SorceryHypergraphPanel::GetRootXtNodeType(const EditorLib::I_DgNode* pDgNode) const
{
	if (!pDgNode || !pDgNode->AsXtObject()) 
		return "";

	const EditorLib::I_DgNode* pRoot = 0;
	if (!pDgNode->IsA(EditorLib::E_ObjType_SceneXtNode))
	{
		pRoot = pDgNode;
	}
	else if (pDgNode->IsA(EditorLib::E_ObjType_DgXtCompNode))
	{
		pRoot = ((EditorLib::C_DgXtCompNode*)pDgNode)->GetParent();
	}
	else if (pDgNode->IsA(EditorLib::E_ObjType_DgXtPropNodeBase))
	{
		pRoot = ((EditorLib::C_DgXtPropNodeBase*)pDgNode)->GetParent() ?
			((EditorLib::C_DgXtPropNodeBase*)pDgNode)->GetParent()->GetParent() : 0;
	}

	if (!pRoot || !pRoot->AsXtObject())
		return "";

	return pDgNode->AsXtObject()->GetXtTypeName();
}

void C_SorceryHypergraphPanel::OnCoreDirtyViewsApplied(EditorLib::E_DirtyEventType e)
{
	if (e == EditorLib::E_DirtyEventType_Patch)
		m_flags.dirty = true;
}

C_SorceryHypergraphPanel::C_Flags& C_SorceryHypergraphPanel::GetFlags()
{
	return m_flags;
}

C_SorceryHypergraphPanel::C_Slots::C_Slots(C_SorceryHypergraphPanel* pForm)
{
	m_pWxScarabForm = pForm;
}

void C_SorceryHypergraphPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreSelectionChanged.connect(this, &Editor::C_SorceryHypergraphPanel::C_Slots::OnCoreSelectionChangedSlot);
	s.coreEnableViews.connect(this, &Editor::C_SorceryHypergraphPanel::C_Slots::OnCoreEnableViewsSlot);

	s.levelOpening.connect(this, &C_SorceryHypergraphPanel::C_Slots::OnLevelOpening);
	s.levelOpened.connect(this, &C_SorceryHypergraphPanel::C_Slots::OnLevelOpened);
	s.levelClosing.connect(this, &C_SorceryHypergraphPanel::C_Slots::OnLevelClosing);
	s.levelClosed.connect(this, &C_SorceryHypergraphPanel::C_Slots::OnLevelClosed);

	s.dgNodeUpdated.connect(this, &C_SorceryHypergraphPanel::C_Slots::OnNodeUpdated);
	s.dgNodeDestroy.connect(this, &C_SorceryHypergraphPanel::C_Slots::OnNodeDestroy);
}

void C_SorceryHypergraphPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreSelectionChanged.disconnect(this);
	s.coreEnableViews.disconnect(this);

	s.levelOpening.disconnect(this);
	s.levelOpened.disconnect(this);
	s.levelClosing.disconnect(this);
	s.levelClosed.disconnect(this);

	s.dgNodeUpdated.disconnect(this);
	s.dgNodeDestroy.disconnect(this);
}

void C_SorceryHypergraphPanel::C_Slots::OnCoreEnableViewsSlot(bool enable)
{
	m_pWxScarabForm->OnCoreEnableViews(enable);
}

void C_SorceryHypergraphPanel::C_Slots::OnCoreSelectionChangedSlot()
{
	m_pWxScarabForm->GetFlags().dirty = true;
}

void C_SorceryHypergraphPanel::OnKeyDown(wxKeyEvent& e)
{
}

bool C_SorceryHypergraphPanel::MSWShouldPreProcessMessage(WXMSG* msg)
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

bool C_SorceryHypergraphPanel::MSWTranslateMessage(WXMSG* msg)
{
	//if (wxWindowMSW::MSWTranslateMessage(msg))
	//	return true;
	//else
	//{
	//	//send a new event to parent panel.
	//	//This is because ::IsDialog() will dispatch messages to the underlying ScarabForm
	//	//so we'll never see it again in wxWidgets land
	//	::SendMessage((HWND)GetHWND(), msg->message, msg->wParam, msg->lParam);
	//	return false;
	//}

	return wxWindowMSW::MSWTranslateMessage(msg);
}

void C_SorceryHypergraphPanel::C_Slots::OnLevelOpening(const EditorLib::I_Level& level)
{
}

void C_SorceryHypergraphPanel::C_Slots::OnLevelOpened(const EditorLib::I_Level& level)
{
}

void C_SorceryHypergraphPanel::C_Slots::OnLevelClosing(const EditorLib::I_Level& level)
{
	m_pWxScarabForm->Clear();
}

void C_SorceryHypergraphPanel::C_Slots::OnLevelClosed(const EditorLib::I_Level& level)
{
}

void C_SorceryHypergraphPanel::C_Slots::OnNodeUpdated(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc)
{
	m_pWxScarabForm->OnNodeUpdated(dgNode, pHc);
}

void C_SorceryHypergraphPanel::C_Slots::OnNodeDestroy(const EditorLib::I_DgNode& dgNode)
{
}

}
