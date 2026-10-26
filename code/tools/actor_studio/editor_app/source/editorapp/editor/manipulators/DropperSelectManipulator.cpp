#include "stdafx.h"

#include <EditorLib/Context/EditorContext.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>

#include <xtwin/WndCore.h>
#include <xtwin/WndClasses.h>
#include <xtwin/Scarab.h>

#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/DropperSelectManipulator.h>
#include <EditorApp/Components/IView.h>
#include <EditorApp/Components/Scarab_Form/WxScarabForm.h>
#include <EditorApp/components/scarab_form/wxScarabFormContext.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/DgNodeSelection.h>

using namespace Editor;

BEGIN_EVENT_TABLE(C_DropperSelectManipulator, I_Manipulator)
  EVT_MOUSE_EVENTS(C_DropperSelectManipulator::OnMouseEvent)
END_EVENT_TABLE()

C_DropperSelectManipulator::C_DropperSelectManipulator()
: I_Manipulator()
{
}

C_DropperSelectManipulator::~C_DropperSelectManipulator()
{
}

E_ManipulatorType C_DropperSelectManipulator::GetType() const
{
	return E_ManipulatorType_DropperSelect;
}

bool C_DropperSelectManipulator::IsA(E_ManipulatorType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == C_DropperSelectManipulator::GetType()) 
		return true;

	return false;
}

void C_DropperSelectManipulator::SetData(const void* userData)
{
	LarvaSchema *pSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;

	DropperUserData *pDropperData = reinterpret_cast<DropperUserData*>(const_cast<void*>(userData));
	m_pDomElement = pDropperData->pDomElement;
	m_pLvField = pDropperData->pLvField;

	assert(m_pDomElement && m_pLvField);
}

void C_DropperSelectManipulator::OnMouseEvent(wxMouseEvent& e)
{
	if (e.ControlDown())
		wxSetCursor(wxCursor("\"IDC_CURSOR_DROPPERSELECTLINK\""));
	else
		wxSetCursor(wxCursor("\"IDC_CURSOR_DROPPERSELECT\""));

	if (e.LeftDown())
	{
		C_DgNodeSelection selectionCandidates;
		wxPoint pt = e.GetPosition();
		RECT r = {pt.x, pt.y, pt.x+10, pt.y+10};	//matches cursor dimensions

		Display::Device()->SetView(NULL, false);
		Display::Device()->BeginScene();

		m_pViewport->GetViewContext().BeginRender();			
		C_RenderCore::Instance().GetRenderPipeline().Select(r, selectionCandidates);

		Display::Device()->SetView(NULL, false);
		Display::Device()->EndScene();
		m_pViewport->GetViewContext().EndRender(false);

		C_DgNodeSelection::ItemArray& items = const_cast<C_DgNodeSelection::ItemArray&>(selectionCandidates.GetItems());
		for (C_DgNodeSelection::ItemArray::ConstReverseIterator it=items.ReverseBegin(); it!=items.ReverseEnd(); ++it)
		{
			EditorLib::C_SceneXtNodeBase* pSceneXtNodeBase = it->pDgNode->IsA(EditorLib::E_ObjType_SceneXtNodeBase) ? 
				(EditorLib::C_SceneXtNodeBase*)it->pDgNode : 0;
			if (!pSceneXtNodeBase)
				continue;

			Xt::DomComponent* pDomComponent = Xt::AsComponent(pSceneXtNodeBase->GetXtData());
			LtStruct *pLtStruct = static_cast<LtStruct*>(pSceneXtNodeBase->GetXtType());

			if (!IsBaseOf(static_cast<LfWeakPtr*>(m_pLvField)->baseType, pLtStruct))
				continue;

			XtId id = pDomComponent->id;
			if (!id)
				continue;

			//set data directly...
			Xt::Set(AsRef(m_pDomElement), id);
			
			if (e.ControlDown())
			{
				//add node to selection 
				Btl::String args;
				args.Printf("%s -a", it->pDgNode->GetId().AsString());
				EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", args);

				m_pDomElement = Xt::At(&pDomComponent->data, m_pLvField);
			}
			else
				EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);

			break;
		}
	}
	else if (e.MiddleDown() || e.RightDown())
		EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);
}
