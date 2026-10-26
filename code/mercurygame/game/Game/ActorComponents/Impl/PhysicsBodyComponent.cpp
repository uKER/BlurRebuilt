
#include "stdafx.h"

#include "Game/ActorComponents/Creation/PhysicsBodyComponentCreate.h"
//#include "GameActors/ActorCreation/PlatformActorCreate.h"
#include "Game/ActorComponents/Impl/PhysicsBodyComponent.h"

#include <gravity/sdk/BodyHelper.h>
#include <gravity/sdk/CompositeDescriptor.h>
#include <gravity/importer/Importer.h>
//#include <Game/CollideModel/CollideModel.h>
#include <actor_system/Actor.h>
// #include <GameWorld/GameWorld.h>
// #include <GameWorld/PhysicsWorld.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <Game/ActorComponents/Messages/GamePhysicsMessages.h>

#include <shared_actors/Components/Interfaces/I_ModelResource.h>
#include <game_resource/ModelData.h>

namespace Components
{
#if 0

bool C_PlatformPhysicsBody::CreateShape()
{
	m_Mass = static_cast<const Actors::C_PlatformActorCreate*>(GetActor()->GetCreationData().GetData())->m_InitialMass; // ugh

 	const Gravity::C_ImportShape *pImportShape = C_GameWorld::GetInstance()->PhysicsWorld().ImportedShape(GetCreationData()->m_PlatformId);
 	if (pImportShape)
 	{
		m_ShapeDesc = pImportShape->GetShapeDesc();
		return (m_ShapeDesc!=NULL);
	}

 	return false;
}
#endif


void C_ModelPhysicsBody::CreateShape(C_PhysicsBodyShapeDesc& shapeDesc) const
{
	const I_ModelResource *pModelResource = (const I_ModelResource *)GetActor()->QueryInterface(Components::E_IF_ModelResource);

	if(pModelResource)
	{
		const GameResource::C_Model* pModel = pModelResource->GetModelData()->GetModel(GetCreationData()->m_ModelIndex);
		const GameResource::C_ModelElement* pElement = pModel->GetElement(GetCreationData()->m_ModelElementIndex);

		const Gravity::C_ImportShape* pImportShape = pElement->GetPhysicsImportShape();
		//const Gravity::C_ImportShape* importShape = GetCreationData()->m_Model->m_pCollideModel->m_ElementList[GetCreationData()->m_ModelElementIndex].GetShape();
		if (pImportShape)
		{
			shapeDesc.Set(GetCreationData()->m_Mass, pImportShape->GetShapeDesc());
		}
	}
}


#if 0
bool C_DynamicObjectPhysicsBody::InitialiseImpl()
{
	m_ModelElemDesc = GetCreationData()->m_ElementDescs;
	return C_PhysicsBody::InitialiseImpl();
}


C_DynamicObjectPhysicsBody::~C_DynamicObjectPhysicsBody()
{
	delete m_ShapeDesc;
}


bool C_DynamicObjectPhysicsBody::CreateShape()
{
	const S32 dynObjElemCount = GetCreationData()->ElementDefs().count;
	Gravity::C_CompositeDesc* newDesc = new Gravity::C_CompositeDesc(NULL, dynObjElemCount);
	newDesc->m_DecomposeSingleComposite = false;	// Stop decomposition of single-element composite shapes.

	m_Mass = 0.0f;

	// Build a composite descriptor from active dyn obj elements:
	for (S32 e=0 ; e<dynObjElemCount ; e++)
	{
		const S32 elemDmgLevel = m_ModelElemDesc[e];
		if (elemDmgLevel >= 0)
		{
			const S32 elemIndex = GetCreationData()->ElementDefs()[e].m_DamageLevels[elemDmgLevel].m_ModelElementIndex;
			const F32 elemMass = GetCreationData()->ElementDefs()[e].m_DamageLevels[elemDmgLevel].m_Mass;
			Gravity::C_ImportShape* gs = GetCreationData()->Model()->m_pCollideModel->GetElement(elemIndex).GetShape();
			if (gs)
			{
				newDesc->AddShape(gs->GetShapeDesc(), gs->GetImportMatrix(), elemMass);
				m_Mass += elemMass;
			}
		}
	}

	if (newDesc->GetNumValidShapes() <= 0)
	{
		LOGERROR(Actors::g_Log, DBSTR"no valid shapes in descriptor");
		delete newDesc;
		return false;
	}

	newDesc->RemapShape();
	newDesc->SetRemapped();

	delete m_ShapeDesc;
	m_ShapeDesc = newDesc;

	return true;
}


bool C_DynamicObjectPhysicsBody::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		case E_DynObjPhysicsBodyMsg_UpdateShape:
		{
			const C_DynamicObjectPhysicsBodyUpdateShapeMsg* msg = Messaging::MessageCast<C_DynamicObjectPhysicsBodyUpdateShapeMsg>(pMessage);
			m_ModelElemDesc = msg->m_ElemDescList;
			SendMessageToComponent(Components::E_IF_PhysicsBody, &C_PhysicsUpdateShapeMsg(msg->m_OverrideIndex));
		}	break;

		default:
			return C_PhysicsBody::OnMessage(pMessage);
	}

	return true;
}


void C_DynamicObjectPhysicsBody::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_PhysicsBody::SerialiseForSave(stream);

	const S32 count = m_ModelElemDesc.Size();
	stream << count;
	for (S32 e=0 ; e<count ; e++)
		stream << m_ModelElemDesc[e];
}


void C_DynamicObjectPhysicsBody::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_PhysicsBody::DeserialiseForSave(stream);

	assert(m_ModelElemDesc.Empty());
	S32 count;
	stream >> count;
	for (S32 e=0 ; e<count ; e++)
	{
		S32 val;
		stream >> val;
		m_ModelElemDesc.PushBack(val);
	}
}


void C_DynamicObjectPhysicsBody::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_PhysicsBody::SerialiseForSync(stream);

	const S32 count = m_ModelElemDesc.Size();
	stream << count;
	for (S32 e=0 ; e<count ; e++)
		stream << m_ModelElemDesc[e];
}


void C_DynamicObjectPhysicsBody::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_PhysicsBody::DeserialiseForSync(stream);

	assert(m_ModelElemDesc.Empty());
	S32 count;
	stream >> count;
	for (S32 e=0 ; e<count ; e++)
	{
		S32 val;
		stream >> val;
		m_ModelElemDesc.PushBack(val);
	}
}
#endif

} // namespace Components
