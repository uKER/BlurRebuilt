#include "stdafx.h"

#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Editor/CameraController.h>
#include <EditorApp/Editor/CameraConnection.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <System/Time/Time.h>

#include <UserNode/Props/Animation/DgAnimationPropNode.h>

#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/ComplexRenderer.h>
#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>
#include <UserNode/Props/Animation/RenderAnimationPropNode.h>

namespace Editor
{

C_SyncCamera::C_SyncCamera(C_RenderViewport *pViewport)
: I_CameraController()
, m_pViewport(pViewport)
, m_lastMessageTime(0)
{
	assert(pViewport);
	m_camState.camera = m_pViewport->GetCamera();
	m_camState.label = m_pViewport->GetLabel();
	
	m_pViewport->SetLabel(m_camState.label);

	m_pLastMessage = new CameraMessage;
	memset(m_pLastMessage, 0, sizeof(CameraMessage));
	m_pLastMessage->fov = Maths::Constants<F32>::Pi()/3.f;
}

C_SyncCamera::~C_SyncCamera()
{
	m_pViewport->SetLabel(m_camState.label);
	m_pViewport->GetCamera() = m_camState.camera;
	m_pViewport->SetDirty(true);
	delete m_pLastMessage;
}

void C_SyncCamera::Recieve(const CameraMessage* msg)
{
	//ignore if the camera is currently being manipulated
	if (m_pViewport->IsCameraActive())
		return;

	m_pViewport->SetCameraPosition(Maths::HVec3(msg->x, msg->y, msg->z));
	m_pViewport->SetCameraOrientation(Maths::C_Quaternion(msg->a, msg->b, msg->c, msg->d));
	//ignore fov

	m_pViewport->SetDirty(true);

	m_lastMessageTime = Time::GetCurrentTime();

	AftkRender::C_Camera& camera = m_pViewport->GetViewContext().GetView().camera;
	const Maths::HVec3& p = camera.GetPosition();
	Maths::HVec3 x, y=camera.GetUpVector(), z=camera.GetFocusPosition()-camera.GetPosition();
	z.Normalise();
	x = y^z;
	x.Normalise();
	y = z^x;
	y.Normalise();
	Maths::C_Quaternion q(Maths::HMat33(x,y,z));

	CameraMessage camMsg = {
							p.X(), p.Y(), p.Z(), 
							q.Imag().X(), q.Imag().Y(), q.Imag().Z(), q.Real().W(), 
							m_pLastMessage->fov
							};

	*m_pLastMessage = camMsg;
}


bool C_SyncCamera::Send(CameraMessage* msg)
{
	AftkRender::C_Camera& camera = m_pViewport->GetViewContext().GetView().camera;

	const Maths::HVec3& p = camera.GetPosition();
	Maths::HVec3 x, y=camera.GetUpVector(), z=camera.GetFocusPosition()-camera.GetPosition();
	z.Normalise();
	x = y^z;
	x.Normalise();
	y = z^x;
	y.Normalise();

	Maths::C_Quaternion q(Maths::HMat33(x,y,z));

	//ignore fov
	CameraMessage camMsg = {
							p.X(), p.Y(), p.Z(), 
							q.Imag().X(), q.Imag().Y(), q.Imag().Z(), q.Real().W(), 
							m_pLastMessage->fov
							};
	*msg = camMsg;
	bool changed = !MSG_EQUALITY_CHECK(*msg, *m_pLastMessage, 1024, 65536);
	*m_pLastMessage = camMsg;

	//only send message given sufficient time since the last update message
	Time::C_Time timeDelta = Time::GetCurrentTime() - m_lastMessageTime;
	if (changed && timeDelta > 500)
		return true;
	else
		return false;
}

void C_SyncCamera::Update()
{
	//do nothing.
}


C_DgNodeCamera::C_Slots::C_Slots(C_DgNodeCamera* pController)
: m_pController(pController)
{
}

void C_DgNodeCamera::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
	s.dgNodeUpdated.connect(this, &C_DgNodeCamera::C_Slots::OnDgNodeUpdatedSlot);
	s.dgNodeDestroy.connect(this, &C_DgNodeCamera::C_Slots::OnDgNodeDestroySlot);
}


void C_DgNodeCamera::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
	s.dgNodeUpdated.disconnect(this);
	s.dgNodeDestroy.disconnect(this);
}

void C_DgNodeCamera::C_Slots::OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc)
{
	m_pController->OnDgNodeUpdated(dgNode);
}

void C_DgNodeCamera::C_Slots::OnDgNodeDestroySlot(const EditorLib::I_DgNode& dgNode)
{
	m_pController->OnDgNodeDestroy(dgNode);
}

C_DgNodeCamera::C_DgNodeCamera(C_RenderViewport *pViewport, const EditorLib::I_DgNode *pDgNode)
: I_CameraController()
, m_pViewport(pViewport)
, m_lastMessageTime(0)
, m_pDgNode(pDgNode)
, m_slots(this)
{
	assert(pViewport && m_pDgNode);
	m_camState.camera = m_pViewport->GetCamera();
	m_camState.label = m_pViewport->GetLabel();
	
	m_pLastMessage = new CameraMessage;
	memset(m_pLastMessage, 0, sizeof(CameraMessage));
	m_pLastMessage->fov = Maths::Constants<F32>::Pi()/3.f;

	m_slots.Connect();
}

C_DgNodeCamera::~C_DgNodeCamera()
{
	m_pViewport->SetLabel(m_camState.label);
	m_pViewport->GetCamera() = m_camState.camera;
	m_pViewport->SetDirty(true);
	delete m_pLastMessage;

	m_slots.Disconnect();
}


void C_DgNodeCamera::Recieve(const CameraMessage* msg)
{
	//do nothing
}

bool C_DgNodeCamera::Send(CameraMessage* msg)
{
	AftkRender::C_Camera& camera = m_pViewport->GetViewContext().GetView().camera;
	const Maths::HVec3& p = camera.GetPosition();
	Maths::HVec3 x, y=camera.GetUpVector(), z=camera.GetFocusPosition()-camera.GetPosition();
	z.Normalise();
	x = y^z;
	x.Normalise();
	y = z^x;
	y.Normalise();

	Maths::C_Quaternion q(Maths::HMat33(x,y,z));

	//ignore fov
	CameraMessage camMsg = {
							p.X(), p.Y(), p.Z(), 
							q.Imag().X(), q.Imag().Y(), q.Imag().Z(), q.Real().W(), 
							m_pLastMessage->fov,
							};
	*msg = camMsg;
	bool changed = !MSG_EQUALITY_CHECK(*msg, *m_pLastMessage, 1024, 65536);
	*m_pLastMessage = camMsg;

	//only send message given sufficient time since the last update message
	Time::C_Time timeDelta = Time::GetCurrentTime() - m_lastMessageTime;
	if (changed && timeDelta > 500)
		return true;
	else
		return false;
}

void C_DgNodeCamera::Update()
{
	AftkRender::C_Camera& camera = m_pViewport->GetViewContext().GetView().camera;

	//use front end cache node to position camera to allow interactive editing
	AftkRender::C_SceneGraphNode *pRenderSgNode = C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(m_pDgNode);
	if (!pRenderSgNode)
		DgNodeDeserialised(*m_pDgNode);
	else
	{
		Maths::HMat44 worldMatrix = pRenderSgNode->GetWorldMatrix();
		camera.SetPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2]));
		
		Maths::HVec3 focus(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]);
		camera.SetFocusPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2])+focus);

		camera.SetUpVector(Maths::HVec3(worldMatrix[1][0], worldMatrix[1][1], worldMatrix[1][2]));
	}
}

void C_DgNodeCamera::OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode)
{
	DgNodeDeserialised(dgNode);
}

void C_DgNodeCamera::DgNodeDeserialised(const EditorLib::I_DgNode& dgNode)
{
	if (&dgNode == m_pDgNode)
	{
		m_pViewport->SetLabel(m_pDgNode->GetName().Chars());

		assert(dgNode.IsA(EditorLib::E_ObjType_SceneXtNodeBase));

		const EditorLib::Matrix44& worldMatrix = static_cast<const EditorLib::C_SceneXtNodeBase&>(dgNode).GetWorldMatrix();
		AftkRender::C_Camera& camera = m_pViewport->GetCamera();
		camera.SetPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2]));
		
		Maths::HVec3 focus(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]);
		camera.SetFocusPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2])+focus);

		camera.SetUpVector(Maths::HVec3(worldMatrix[1][0], worldMatrix[1][1], worldMatrix[1][2]));

		camera.SetType(AftkRender::C_Camera::E_CameraType_Perspective);
		m_pViewport->SetDirty(true);
	}
}

void C_DgNodeCamera::OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode)
{
	DgNodeDestroy(dgNode);
}

void C_DgNodeCamera::DgNodeDestroy(const EditorLib::I_DgNode& dgNode)
{
	if (&dgNode == m_pDgNode)
		C_RenderCore::Instance().DestroyCameraController(this);
}







C_DgAnimationPropNodeCamera::C_DgAnimationPropNodeCamera(C_RenderViewport *pViewport, const EditorLib::C_DgAnimationPropNode* pDgNode, U32 component)
: C_DgNodeCamera(pViewport, pDgNode)
, m_componentId(component)
{
}

void C_DgAnimationPropNodeCamera::Update()
{
	AftkRender::C_Camera& camera = m_pViewport->GetViewContext().GetView().camera;

	//use front end cache node to position camera to allow interactive editing
	AftkRender::C_SceneGraphNode *pRenderSgNode = C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(m_pDgNode);
	if (!pRenderSgNode)
		DgNodeDeserialised(*m_pDgNode);
	else
	{
		Editor::C_RenderAnimationPropNode *pPropNode = static_cast<Editor::C_RenderAnimationPropNode*>(pRenderSgNode);

		Maths::HMat44 worldMatrix = pPropNode->GetLocalTransform(m_componentId) * pPropNode->GetWorldMatrix();
		camera.SetPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2]));
		
		Maths::HVec3 focus(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]);
		camera.SetFocusPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2])+focus);

		camera.SetUpVector(Maths::HVec3(worldMatrix[1][0], worldMatrix[1][1], worldMatrix[1][2]));
	}
}

void C_DgAnimationPropNodeCamera::DgNodeDeserialised(const EditorLib::I_DgNode& dgNode)
{
	if (&dgNode == m_pDgNode)
	{
		m_pViewport->SetLabel(m_pDgNode->GetName().Chars());

		assert(dgNode.IsA(EditorLib::E_ObjType_DgAnimationPropNode));

		//use front end render node
		AftkRender::C_SceneGraphNode *pRenderSgNode = C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(m_pDgNode);

		if (pRenderSgNode)
		{
			AftkRender::C_Camera& camera = m_pViewport->GetCamera();
			Maths::HMat44 worldMatrix = pRenderSgNode->GetWorldMatrix();
			camera.SetPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2]));
			
			Maths::HVec3 focus(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]);
			camera.SetFocusPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2])+focus);

			camera.SetUpVector(Maths::HVec3(worldMatrix[1][0], worldMatrix[1][1], worldMatrix[1][2]));

			camera.SetType(AftkRender::C_Camera::E_CameraType_Perspective);
			m_pViewport->SetDirty(true);
		}
		else
		{
			//use parent actor position
			const EditorLib::C_DgXtCompNode *pComponent = static_cast<const EditorLib::C_DgXtPropNodeBase*>(m_pDgNode)->GetParent();
			const EditorLib::I_DgNode *pActor = pComponent ? pComponent->GetParent() : 0;

			if (pActor && pActor->IsA(EditorLib::E_ObjType_SceneXtNodeBase))
			{
				const EditorLib::Matrix44& worldMatrix = static_cast<const EditorLib::C_SceneXtNodeBase*>(pActor)->GetWorldMatrix();
				AftkRender::C_Camera& camera = m_pViewport->GetCamera();
				camera.SetPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2]));
				
				Maths::HVec3 focus(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]);
				camera.SetFocusPosition(Maths::HVec3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2])+focus);

				camera.SetUpVector(Maths::HVec3(worldMatrix[1][0], worldMatrix[1][1], worldMatrix[1][2]));

				camera.SetType(AftkRender::C_Camera::E_CameraType_Perspective);
				m_pViewport->SetDirty(true);
			}
		}
	}
}


}
