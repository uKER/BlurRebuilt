#include "stdafx.h"

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Context/EditorContext.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Core.h>

#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/ToolBars/ManipulatorToolBar.h>

#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/CameraConnection.h>
#include <EditorApp/Editor/RenderViewport.h>

#include <EditorApp/Input_Handlers/ShortcutHandler.h>

#include <EditorApp/Config.h>
#include <EditorApp/App.h>

namespace Editor
{
namespace ToolBars
{
//helper class for updating the "move to game camera" tool to avoid 
//having to verify the selection on every updateui event
class C_MoveToGameCameraHelper : public sigslot::has_slots<>
{
public:

	void Connect()
	{
		EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
		s.levelOpened.connect(this, &C_MoveToGameCameraHelper::OnLevelOpenedSlot);
		s.levelClosed.connect(this, &C_MoveToGameCameraHelper::OnLevelClosedSlot);
		s.coreSelectionChanged.connect(this, &C_MoveToGameCameraHelper::OnCoreSelectionChangedSlot);
	}

	void Disconnect()
	{
		if (m_flags.connected)
		{
			EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
			s.levelOpened.disconnect(this);
			s.levelClosed.disconnect(this);
			s.coreSelectionChanged.disconnect(this);
		}
	}

	void OnMoveToGameCamera(wxCommandEvent& e)
	{
		C_CameraConnection *pConnection = C_RenderCore::Instance().GetCameraConnection();
		CameraMessage msg;

		EditorLib::Matrix44 worldMatrix;
		bool set = false;
		if (pConnection && pConnection->Recieve(msg))
		{			
			Maths::C_Quaternion q0(msg.a, msg.b, msg.c, msg.d);
			q0.Normalise();
			Maths::HMat33 rot(q0);		
			Maths::HMat44 camWorldH(rot);
			camWorldH[3] = Maths::HVec4(msg.x, msg.y, msg.z, 1.f);

			Maths::Store(camWorldH, (MathsStorage::Float44*) &worldMatrix);
			set = true;
		}
		else
		{
			//use current persp camera position
			const C_RenderViewport* pViewport = wxGetApp().GetShortcutHandler()->GetLastFocusedViewport();
			if (pViewport && pViewport->GetCamera().GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
			{
				const AftkRender::C_Camera& camera = pViewport->GetCamera();				
				const Maths::HVec3& position = camera.GetPosition();
				const Maths::HVec3& direction = camera.GetFocusPosition() - camera.GetPosition();

				Maths::HVec3 toX, toY, toZ;
				Maths::HMat44 lookMatrix;
				toZ = direction;
				toZ.Normalise();

				toX = camera.GetUpVector()^toZ;
				toX.Normalise();

				toY = toZ^toX;
				toY.Normalise();

				lookMatrix.AX(Maths::HVec4(toX));
				lookMatrix.AY(Maths::HVec4(toY));
				lookMatrix.AZ(Maths::HVec4(toZ));
				lookMatrix.AW(Maths::HVec4(position.X(), position.Y(), position.Z(), 1.f));

				Maths::Store(lookMatrix, (MathsStorage::Float44*) &worldMatrix);
				set = true;
			}
		}

		if (set)
		{
			EditorLib::C_Selection selection;
			EditorLib::C_Core::Instance().GetSelection(selection);

			const EditorLib::C_Selection::ItemArray& items = selection.GetItems();

			for (S32 i=0; i<items.Size(); ++i)
			{
				EditorLib::I_DgNode *pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(items[i].id);
				if (pNode && pNode->IsA(EditorLib::E_ObjType_SceneXtActorNode) &&
					!pNode->IsReadOnly())
				{
					EditorLib::C_SceneNode *pSceneNode = (EditorLib::C_SceneNode*)pNode;
					pSceneNode->SetWorldMatrix(worldMatrix);
				}
			}
		}
		else
			LOGERROR(g_Log, "Failed to obtain game/viewport camera position");
	}

	void OnUpdateMoveToGameCamera(wxUpdateUIEvent& e)
	{
		if (m_flags.selectionChanged)
		{
			EditorLib::C_Selection selection;
			EditorLib::C_Core::Instance().GetSelection(selection);

			m_flags.selectionValid = false;
			const EditorLib::C_Selection::ItemArray& items = selection.GetItems();

			for (S32 i=0; i<items.Size(); ++i)
			{
				EditorLib::I_DgNode *pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(items[i].id);
				if (pNode && pNode->IsA(EditorLib::E_ObjType_SceneXtActorNode) &&
					!pNode->IsReadOnly())
				{
					m_flags.selectionValid = true;
					break;
				}
			}

			m_flags.selectionChanged = false;
		}

		bool gameConnected = false;
		{
			C_CameraConnection *pConnection = C_RenderCore::Instance().GetCameraConnection();
			gameConnected = pConnection && pConnection->GetConnectionState() == C_CameraConnection::E_ConnectionState_Connected;
		}

		bool perspViewportFocus = false;
		{
			const C_RenderViewport* pViewport = wxGetApp().GetShortcutHandler()->GetLastFocusedViewport();
			if (pViewport && pViewport->GetCamera().GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
				perspViewportFocus = true;
		}

		e.Enable(m_flags.selectionValid && (gameConnected | perspViewportFocus));
	}

private:

	void OnLevelOpenedSlot(const EditorLib::I_Level& level)
	{
		m_flags.selectionChanged = false;
		m_flags.selectionValid = false;
	}

	void OnLevelClosedSlot(const EditorLib::I_Level& level)
	{
		m_flags.selectionChanged = false;
		m_flags.selectionValid = false;
	}

	void OnCoreSelectionChangedSlot()
	{
		m_flags.selectionChanged = true;
	}

	struct Flags
	{
		bool connected;
		bool selectionChanged;
		bool selectionValid;
	};

	Flags m_flags;
};

static C_MoveToGameCameraHelper g_MoveToGameCamHelper;

}
}

wxToolBar* Editor::ToolBars::CreateManipulatorToolBar(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
	wxToolBar *pManipulatorToolBar = new wxToolBar(parent, id, pos, size, style);
	pManipulatorToolBar->SetToolBitmapSize(wxSize(16,16));

	wxToolBarToolBase* pTool = NULL;
	wxImage toolIcon;

	toolIcon.LoadFile(_T("./icons/manipulator_select.png"));
	pTool = pManipulatorToolBar->AddTool(wxID_ANY, "select", toolIcon, wxNullBitmap, wxITEM_RADIO, _T("Select"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pManipulatorToolBar, &Editor::ToolBars::OnSetActiveManipulator, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pManipulatorToolBar, &Editor::ToolBars::OnUpdateManipulatorToolBar, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/manipulator_move.png"));
	pTool = pManipulatorToolBar->AddTool(wxID_ANY, "move", toolIcon, wxNullBitmap, wxITEM_RADIO, _T("Move"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pManipulatorToolBar, &Editor::ToolBars::OnSetActiveManipulator, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pManipulatorToolBar, &Editor::ToolBars::OnUpdateManipulatorToolBar, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/manipulator_rotate.png"));
	pTool = pManipulatorToolBar->AddTool(wxID_ANY, "rotate", toolIcon, wxNullBitmap, wxITEM_RADIO, _T("Rotate"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pManipulatorToolBar, &Editor::ToolBars::OnSetActiveManipulator, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pManipulatorToolBar, &Editor::ToolBars::OnUpdateManipulatorToolBar, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/manipulator_scale.png"));
	pTool = pManipulatorToolBar->AddTool(wxID_ANY, "scale", toolIcon, wxNullBitmap, wxITEM_RADIO, _T("Scale"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pManipulatorToolBar, &Editor::ToolBars::OnSetActiveManipulator, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pManipulatorToolBar, &Editor::ToolBars::OnUpdateManipulatorToolBar, pTool->GetId());

	pManipulatorToolBar->AddSeparator();

	toolIcon.LoadFile(_T("./icons/manipulator_gamecampos.png"));
	pTool = pManipulatorToolBar->AddTool(wxID_ANY, "movetocam", toolIcon, wxNullBitmap, wxITEM_NORMAL, _T("Move to Camera"));
	toolIcon.LoadFile(_T("./icons/manipulator_gamecamposdisabled.png"));
	pTool->SetDisabledBitmap(toolIcon);

	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pManipulatorToolBar, &Editor::ToolBars::C_MoveToGameCameraHelper::OnMoveToGameCamera, &g_MoveToGameCamHelper, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pManipulatorToolBar, &Editor::ToolBars::C_MoveToGameCameraHelper::OnUpdateMoveToGameCamera, &g_MoveToGameCamHelper, pTool->GetId());

	pManipulatorToolBar->Realize();

	g_MoveToGameCamHelper.Connect();

	return pManipulatorToolBar;
}

void Editor::ToolBars::OnSetActiveManipulator(wxCommandEvent& e)
{
	if (e.GetEventObject()->IsKindOf(CLASSINFO(wxToolBar)))
	{
		wxToolBar *pToolBar = (wxToolBar*)e.GetEventObject();
		wxToolBarToolBase *pTool = pToolBar->FindById(e.GetId());
		if (pTool)
			//C_RenderManager::Instance().GetManipulatorManager()->SetActiveManipulator(Util::C_String(pTool->GetLabel().c_str()));
			EditorLib::C_Core::Instance().SetContext(pTool->GetLabel().c_str());
	}
}

void Editor::ToolBars::OnUpdateManipulatorToolBar(wxUpdateUIEvent& e)
{
	if (e.GetEventObject()->IsKindOf(CLASSINFO(wxToolBar)))
	{
		wxToolBar *pToolBar = (wxToolBar*)e.GetEventObject();
		wxToolBarToolBase *pTool = pToolBar->FindById(e.GetId());
		if (pTool)
			//e.Check(pTool->GetLabel() == C_RenderManager::Instance().GetManipulatorManager()->GetActiveManipulatorName());
			e.Check(pTool->GetLabel() == EditorLib::C_Core::Instance().GetContext().Chars());
	}
	else
		e.Skip();
}
