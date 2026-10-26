#include "stdafx.h"
#include <system/Artery/Artery.h>

#include <EditorLib/Core.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Misc/NodeId.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Editor/GameMessaging.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>

#include <actor_studio_msg_shared/MessageTypes.h>
#include <actor_studio_msg_shared/Messages.h>

namespace Editor
{

void C_GameMessaging::NetworkInit()
{
	ArteryStartup(false, "GameMessaging");
}

void C_GameMessaging::NetworkExit()
{
 	ArteryCleanup();
}

C_GameMessaging::C_GameMessaging() : 
	m_connectionState(E_ConnectionState_Unconnected),
	m_slots(this)
{
	m_pConnection = new ArteryConnection();
	m_pLastSelection = new EditorLib::C_Selection();

	m_slots.Connect();
}

C_GameMessaging::~C_GameMessaging()
{
	Disconnect();
	m_slots.Disconnect();
	delete m_pConnection;
	delete m_pLastSelection;
}

C_GameMessaging::E_ConnectionState C_GameMessaging::GetConnectionState() const
{
	return m_connectionState;
}

S32 C_GameMessaging::Connect(U32 ip)
{
	if (m_connectionState != E_ConnectionState_Unconnected)
		return 0; // Already connecting/connected

	m_Ip = ip;

	WsSockAddr target = WsSockAddr(ip, 6317);
	m_connectionState = E_ConnectionState_Connecting;
	ArteryConnect(*m_pConnection, target, "ActorStudioMessaging", StaticCallback, this);

	return 0;
}

void C_GameMessaging::Disconnect()
{
	if (m_pConnection)
	{
		m_connectionState = E_ConnectionState_Disconnecting;
		m_pConnection->Close();
	}
}


void C_GameMessaging::StaticCallback(ArteryConnection con, void* self)
{
	((C_GameMessaging*)self)->Callback(con);
}

void C_GameMessaging::Callback(ArteryConnection con)
{
	m_connectionState = E_ConnectionState_Connected;

	ArteryWaitContext ctx;
	while(con.IsConnected())
		con.Wait(ctx);

	// If connection lost, attempt to reconnect
	if (m_connectionState != E_ConnectionState_Disconnecting)
	{
		m_connectionState = E_ConnectionState_Unconnected;
		Connect(m_Ip);
		return;
	}

	m_connectionState = E_ConnectionState_Unconnected;
}

void C_GameMessaging::OnDgNodeTransform(const EditorLib::I_DgNode& node, EditorLib::E_DgNodeTransform e)
{
	if (m_pConnection == 0 || !m_pConnection->IsConnected())
		return;

	//Get the serialisableUid
	Xt::DomComponent* pSceneNodeEl = Xt::AsComponent(const_cast<EditorLib::I_DgNode&>(node).AsXtObject()->GetXtData());
	U32 serialisableUid = 0;
	if(pSceneNodeEl)
	{
		Xt::DomField* pField = Xt::AcquireField(&pSceneNodeEl->data, "SerialisableUid");
		if(pField)
		{
			XtString suid = Xt::AsText(pField);
			serialisableUid = (U32)strtoul(suid, NULL, 0);
		}
	}

	switch(e)
	{
	case EditorLib::E_DgNodeTransform_Move:
	case EditorLib::E_DgNodeTransform_Rotate:
		{
			AftkRender::C_SceneGraphNode* pRenderSgNode = 
							C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(&node);

			
			if(pRenderSgNode)
			{
				Maths::HMat44 mat = pRenderSgNode->GetWorldMatrix();
				ActorStudioMsgShared::C_DragMsg dragMsg;

				Maths::HVec3 xVec3(mat[0]);
				xVec3.Normalise();

				Maths::HVec3 yVec3(mat[1]);
				yVec3.Normalise();

				Maths::HVec3 zVec3(mat[2]);
				zVec3.Normalise();
				
				dragMsg.m_Orientation = Maths::C_Quaternion(Maths::HMat33(xVec3,yVec3,zVec3));
						
				dragMsg.m_Position = Maths::HVec3(mat[3]);

				ArteryMessageBuilder msgBuilder;
				ArteryMessage msg;

				msgBuilder.Write(ActorStudioMsgShared::E_ActorStudioMsg_Drag);
				msgBuilder.Write(serialisableUid);
				msgBuilder.Write(dragMsg.m_Position.X());
				msgBuilder.Write(dragMsg.m_Position.Y());
				msgBuilder.Write(dragMsg.m_Position.Z());
				
				Maths::HVec4 quatComp = dragMsg.m_Orientation;
				msgBuilder.Write(quatComp.X());
				msgBuilder.Write(quatComp.Y());
				msgBuilder.Write(quatComp.Z());
				msgBuilder.Write(quatComp.W());
				msgBuilder.Build(msg);

				m_pConnection->Send(msg);
			}
		}
		break;

	case EditorLib::E_DgNodeTransform_EndMove:
	case EditorLib::E_DgNodeTransform_EndRotate: 
		{
			ArteryMessageBuilder msgBuilder;
			ArteryMessage msg;

			msgBuilder.Write(ActorStudioMsgShared::E_ActorStudioMsg_EndDrag);
			msgBuilder.Write(serialisableUid);
			msgBuilder.Build(msg);

			m_pConnection->Send(msg);
		}
		break;
	}
}

void C_GameMessaging::OnCoreSelectionChanged()
{
	if (m_pConnection == 0 || !m_pConnection->IsConnected())
		return;

	EditorLib::C_Selection currSelection;
	EditorLib::C_Core::Instance().GetSelection(currSelection);

	//For the ActorStudioMsgShared::E_ActorStudioMsg_Selected
	{
		const EditorLib::C_Selection::ItemArray& items = currSelection.GetItems();
		EditorLib::C_Selection::ItemArray::ConstIterator it = items.Begin();
		while (it != items.End())
		{
			const EditorLib::C_Selection::C_Item& item = *it;
			const EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
			EditorLib::C_SceneNode* pSceneNode = (EditorLib::C_SceneNode*)pNode;
			if(pSceneNode && pSceneNode->AsXtObject())
			{
				Xt::DomComponent* pSceneNodeEl = Xt::AsComponent(pSceneNode->AsXtObject()->GetXtData());
				U32 serialisableUid = 0;
				if(pSceneNodeEl)
				{
					Xt::DomField* pField = Xt::AcquireField(&pSceneNodeEl->data, "SerialisableUid");
					if(pField)
					{
						XtString suid = Xt::AsText(pField);
						serialisableUid = (U32)strtoul(suid, NULL, 0);
					}
				}

				const EditorLib::C_Selection::C_Item* pLastItem = m_pLastSelection->Find(item.id);

				if(!pLastItem) //not found on last selection, it's been selected
				{
					ArteryMessageBuilder msgBuilder;
					ArteryMessage msg;
					msgBuilder.Write(ActorStudioMsgShared::E_ActorStudioMsg_Selected);
					msgBuilder.Write(serialisableUid);
					msgBuilder.Build(msg);
					
					m_pConnection->Send(msg);
				}
			}
			it++;
		}
	}

	//For the ActorStudioMsgShared::E_ActorStudioMsg_Deselected
	{
		const EditorLib::C_Selection::ItemArray& items = m_pLastSelection->GetItems();
		EditorLib::C_Selection::ItemArray::ConstIterator it = items.Begin();
		while (it != items.End())
		{
			const EditorLib::C_Selection::C_Item& item = *it;
			const EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
			EditorLib::C_SceneNode* pSceneNode = (EditorLib::C_SceneNode*)pNode;
			if(pSceneNode && pSceneNode->AsXtObject())
			{
				Xt::DomComponent* pSceneNodeEl = Xt::AsComponent(pSceneNode->AsXtObject()->GetXtData());
				U32 serialisableUid = 0;
				if(pSceneNodeEl)
				{
					Xt::DomField* pField = Xt::AcquireField(&pSceneNodeEl->data, "SerialisableUid");
					if(pField)
					{
						XtString suid = Xt::AsText(pField);
						serialisableUid = (U32)strtoul(suid, NULL, 0);
					}
				}


				const EditorLib::C_Selection::C_Item* pCurrItem = currSelection.Find(item.id);

				if(!pCurrItem) //not found on current selection, it's been deselected
				{
					ArteryMessageBuilder msgBuilder;
					ArteryMessage msg;
					msgBuilder.Write(ActorStudioMsgShared::E_ActorStudioMsg_Deselected);
					msgBuilder.Write(serialisableUid);
					msgBuilder.Build(msg);
					
					m_pConnection->Send(msg);
				}
			}
			it++;
		}
	}

	*m_pLastSelection = currSelection;
}

////// SLOTS //////

C_GameMessaging::C_Slots::C_Slots(C_GameMessaging* pGameMessaging)
{
	m_pGameMessaging = pGameMessaging;
}

void C_GameMessaging::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
	
	s.dgNodeTransform.connect(this, &Editor::C_GameMessaging::C_Slots::OnDgNodeTransform);
	s.coreSelectionChanged.connect(this, &Editor::C_GameMessaging::C_Slots::OnCoreSelectionChanged);
}

void C_GameMessaging::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.dgNodeTransform.disconnect(this);
	s.coreSelectionChanged.disconnect(this);
}

void C_GameMessaging::C_Slots::OnDgNodeTransform(const EditorLib::I_DgNode& node, EditorLib::E_DgNodeTransform e)
{
	m_pGameMessaging->OnDgNodeTransform(node, e);
}

void C_GameMessaging::C_Slots::OnCoreSelectionChanged()
{
	m_pGameMessaging->OnCoreSelectionChanged();
}

}