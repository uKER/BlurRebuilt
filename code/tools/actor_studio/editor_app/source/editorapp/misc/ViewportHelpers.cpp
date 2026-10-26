#include "stdafx.h"

#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Misc/ViewportHelpers.h>
#include <EditorApp/Editor/RenderViewport.h>

namespace Editor
{

void C_ViewportHelpers::SetViewportCamera(C_RenderViewport* pViewport, E_ViewportView view)
{
	switch (view)
	{
	case(E_ViewportView_Perspective):
		{
			pViewport->SetLabel("persp");
			pViewport->GetCamera().SetPosition(Maths::HVec3(20.f, 20.f, 20.f));
			{
				Maths::HVec3 up(-1.f, 1.f, -1.f);
				up.Normalise();
				pViewport->GetCamera().SetUpVector(up);
			}
			pViewport->GetCamera().SetFocusPosition(Maths::HVec3(0.f, 0.f, 0.f));
			pViewport->GetCamera().SetType(AftkRender::C_Camera::E_CameraType_Perspective);
			pViewport->SetCameraDefault(pViewport->GetCamera());
			break;
		}
	case(E_ViewportView_Front):
		{
			pViewport->SetLabel("front");
			pViewport->GetCamera().SetPosition(Maths::HVec3(0.f, 0.f, -100.f));
			pViewport->GetCamera().SetHeightExtent(20.f);
			pViewport->GetCamera().SetFocusPosition(Maths::HVec3(0.f, 0.f, 0.f));
			pViewport->GetCamera().SetType(AftkRender::C_Camera::E_CameraType_Orthographic);
			pViewport->GetCamera().SetUpVector(Maths::HVec3(0.f, 1.f, 0.f));
			pViewport->SetCameraDefault(pViewport->GetCamera());
			break;
		}
	case(E_ViewportView_Side):
		{
			pViewport->SetLabel("side");
			pViewport->GetCamera().SetPosition(Maths::HVec3(100.f, 0.f, 0.f));
			pViewport->GetCamera().SetHeightExtent(20.f);
			pViewport->GetCamera().SetFocusPosition(Maths::HVec3(0.f, 0.f, 0.f));
			pViewport->GetCamera().SetType(AftkRender::C_Camera::E_CameraType_Orthographic);
			pViewport->GetCamera().SetUpVector(Maths::HVec3(0.f, 1.f, 0.f));
			pViewport->SetCameraDefault(pViewport->GetCamera());
			break;
		}
	case(E_ViewportView_Top):
		{
			pViewport->SetLabel("top");
			pViewport->GetCamera().SetPosition(Maths::HVec3(0.f, 100.f, 0.f));
			pViewport->GetCamera().SetUpVector(Maths::HVec3(0.f, 0.f, 1.f));
			pViewport->GetCamera().SetHeightExtent(20.f);
			pViewport->GetCamera().SetFocusPosition(Maths::HVec3(0.f, 0.f, 0.f)); 
			pViewport->GetCamera().SetType(AftkRender::C_Camera::E_CameraType_Orthographic);
			pViewport->SetCameraDefault(pViewport->GetCamera());
			break;
		}
	}

	pViewport->SetDirty(true);
}

void C_ViewportHelpers::SetViewportObject(C_RenderViewport* pViewport, EditorLib::C_NodeId nodeId)
{
	EditorLib::I_DgNode* pNode;
	
	pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(nodeId);

	if (!pNode)
		return;

	if (!pNode->IsA(EditorLib::E_ObjType_SceneNode))
		return;
	
	EditorLib::C_SceneNode* pSceneNode = (EditorLib::C_SceneNode*)pNode;

	const EditorLib::Matrix44& worldMatrix = pSceneNode->GetWorldMatrix();

	pViewport->SetLabel(pSceneNode->GetName().Chars());
	pViewport->GetCamera().SetPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2]));
	
	Maths::HVec3 focus(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]);
	pViewport->GetCamera().SetFocusPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2])+focus);

	pViewport->GetCamera().SetUpVector(Maths::HVec3(worldMatrix[1][0], worldMatrix[1][1], worldMatrix[1][2]));

	pViewport->GetCamera().SetType(AftkRender::C_Camera::E_CameraType_Perspective);
	pViewport->SetCameraDefault(pViewport->GetCamera());

	pViewport->SetDirty(true);
}

}