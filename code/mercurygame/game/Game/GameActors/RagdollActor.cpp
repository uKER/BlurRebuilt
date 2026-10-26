#include "stdafx.h"

#include <Game/GameActors/RagdollActor.h>

#include <actor_system/ActorSys.h>

#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <shared_actors/Utility/PhysicsBodyUserData.h>
#include <misc_shared/messaging/GameMessaging.h>

#include <resource/Resource.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <world_render/ModelInstance.h>
#include <world_render/RenderModelData.h>
#include <world_render/MaterialParameter.h>
#include <world_render/EnvironmentSampler.h>
#include <scenegraph/SceneGraph.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>

#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <Game/GameWorld/GameWorld.h>

#include <Game/GameDef/PhysicsConstants.h>
#include <world_render/Scene.h>
#include "gravity/sdk/ConstraintDescriptors.h"
//XT_TYPE(Game::Actors::C_ModelTestActorCreateXt, "ModelTestActor")
//	XT_BASE(::Actors::C_PosActorCreate)
//	XT_FIELD(m_GameModel, "Model")
//XT_END_TYPE


namespace Game
{

namespace Actors
{

Debug::Bool g_DisplayRagdollModels("ActorDebug.Actors.RagdollModel.ShowInfo", false);

static bool defaultRagdollStatus = false;

Debug::Bool e_rd_head("ActorDebug.Actors.RagdollModel.headStatic", defaultRagdollStatus);
Debug::Bool e_rd_neck("ActorDebug.Actors.RagdollModel.neckStatic", defaultRagdollStatus);
			
Debug::Bool e_rd_spine2("ActorDebug.Actors.RagdollModel.spine2Static", defaultRagdollStatus);
Debug::Bool e_rd_spine1("ActorDebug.Actors.RagdollModel.spine1Static", defaultRagdollStatus);
Debug::Bool e_rd_spine("ActorDebug.Actors.RagdollModel.spineStatic", defaultRagdollStatus);

Debug::Bool e_rd_hips("ActorDebug.Actors.RagdollModel.hipStatic", defaultRagdollStatus);

Debug::Bool e_rd_righthand("ActorDebug.Actors.RagdollModel.rightHandStatic", defaultRagdollStatus);
Debug::Bool e_rd_rightforearm("ActorDebug.Actors.RagdollModel.rightForeArmStatic", defaultRagdollStatus);
Debug::Bool e_rd_rightarm("ActorDebug.Actors.RagdollModel.rightArmStatic", defaultRagdollStatus);
Debug::Bool e_rd_rightshoulder("ActorDebug.Actors.RagdollModel.rightShoulderStatic", defaultRagdollStatus);

Debug::Bool e_rd_rightupleg("ActorDebug.Actors.RagdollModel.rightUpLegStatic", defaultRagdollStatus);
Debug::Bool e_rd_rightleg("ActorDebug.Actors.RagdollModel.rightLegStatic", defaultRagdollStatus);
Debug::Bool e_rd_rightfoot("ActorDebug.Actors.RagdollModel.rightFootStatic", defaultRagdollStatus);

Debug::Bool e_rd_lefthand("ActorDebug.Actors.RagdollModel.leftHandStatic", defaultRagdollStatus);
Debug::Bool e_rd_leftforearm("ActorDebug.Actors.RagdollModel.leftForeArmStatic", defaultRagdollStatus);
Debug::Bool e_rd_leftarm("ActorDebug.Actors.RagdollModel.leftArmStatic", defaultRagdollStatus);
Debug::Bool e_rd_leftshoulder("ActorDebug.Actors.RagdollModel.leftShoulderStatic", defaultRagdollStatus);

Debug::Bool e_rd_leftupleg("ActorDebug.Actors.RagdollModel.leftUpLegStatic", defaultRagdollStatus);
Debug::Bool e_rd_leftleg("ActorDebug.Actors.RagdollModel.leftLegStatic", defaultRagdollStatus);
Debug::Bool e_rd_leftfoot("ActorDebug.Actors.RagdollModel.leftFootStatic", defaultRagdollStatus);


// Actor Info

Util::C_String C_RagdollModelTestActorCreateProcedural::GetModelFilename() const
{
	return m_ModelFilename;
}

void C_RagdollModelTestActorCreateProcedural::Serialise(Sys::C_ContextualStream& stream) const
{
	C_RagdollModelTestActorCreate::Serialise(stream);
	stream << m_ModelFilename;
}

void C_RagdollModelTestActorCreateProcedural::Deserialise(Sys::C_ContextualStream& stream)
{
	C_RagdollModelTestActorCreate::Deserialise(stream);
	stream >> m_ModelFilename;
}

//Util::C_String C_RagdollModelTestActorCreateXt::GetModelFilename() const
//{
//	if (m_GameModel)
//		return Util::C_String(m_GameModel->m_Filename);
//
//	return Util::C_String();
//}

C_RagdollModelTestActor::C_RagdollModelTestActor()
  : m_pModelResource(NULL)
  , m_pModelInstance(NULL)
  , m_pEnvSampler(NULL)
{}

C_RagdollModelTestActor::~C_RagdollModelTestActor()
{
	if( m_pModelResource != NULL )
		DestroyModel();

	if ( m_pPhysicsBodyUserData != NULL )
	{
		::Actors::C_SimplePhysicsBodyUserData::Destroy( m_pPhysicsBodyUserData );
		m_pPhysicsBodyUserData = NULL;
	}
}


bool C_RagdollModelTestActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_ResourceMsg_Loaded:
		if (Messaging::MessageCast<ActorMsg::C_ResourceLoadedMsg>(pMessage)->GetValue() == 0 )
		{
			if (m_pModelResource != NULL)
				DestroyModel();

			m_pModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( 0 );
			
			if (m_pModelResource != NULL)
				CreateModel();
		}
		return true;

	case ActorMsg::E_ResourceMsg_Unloaded:
		if (Messaging::MessageCast<ActorMsg::C_ResourceUnloadedMsg>(pMessage)->GetValue() == 0 )
		{
			if (m_pModelResource != NULL)
				DestroyModel();

			m_pModelResource = NULL;
		}
		return true;

	case GameMessaging::E_GameMsg_Update:
		if(m_pModelInstance != NULL)
		{
			Maths::HVec3 origin(0.0f, 0.0f, 0.0f);
			//m_WorldMatrix = Maths::HMat43(m_pBody->Orientation(), m_pBody->Position());
			for(S32 i=0; i<m_BodyList.Size(); i++)
			{
				Maths::HMat43 mBody;
				Gravity::C_Body* pBody = m_BodyList[i].m_pBody;
				if (pBody)
					mBody = pBody->ElementMatrix();

				origin = origin + mBody.AW();

				m_pModelInstance->SetElementMatrix(m_BodyList[i].m_ElementIndex, mBody);
			}

			origin = origin * (1.0f / (F32)m_BodyList.Size());

			m_pModelInstance->SetWorldSphere( Maths::C_Sphere( origin, 3.0f ) );

			if(m_pEnvSampler)
			{
				m_pEnvSampler->SetPosition( m_WorldMatrix.AW() );
				m_pEnvSampler->Update();
				m_pEnvSampler->ApplyToModelInstance(m_pModelInstance);
			}

			for(S32 i=0; i<m_BodyList.Size(); i++)
			{
				C_BodyInfo& info = m_BodyList[i];
				Gravity::C_Body* pBody = info.m_pBody;
				
				if(info.m_DebugBool)
				{
					if(*info.m_DebugBool)
					{
						if(info.m_Active)
						{
							pBody->MakeStatic();
							info.m_Active = false;
						}
						
					}
					else
					{
						if(!info.m_Active)
						{
							pBody->Activate();
							info.m_Active = true;
						}
						
					}
				}
			}

			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix( m_WorldMatrix );					
		}
		
		return true;

		case Actor::E_ActorMsg_PhysicsBodyEvent:
			return true;
		break;

	}

	return C_GameActor::OnMessage(pMessage);
}


#ifndef CFG_GOLD
void C_RagdollModelTestActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if( g_DisplayRagdollModels )
	{
		Maths::HMat43 mElement = m_pModelElement->GetMatrix();
		Maths::HMat43 mModelDebug = mElement.InverseRotTrans() * m_WorldMatrix;
		m_pModelResource->GetModelData()->DebugRender(debugRenderPtr, mModelDebug);
	}	
}
#endif


void C_RagdollModelTestActor::CreateModel()
{
	m_pEnvSampler = new WorldRender::C_EnvironmentSampler;
	m_pEnvSampler->Initialise( GetWorldRenderScene() );

	//const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel("M_woodBox_UNDAMAGED");
	S32 modelIndex = 0;
	if(m_pModelResource->GetModelData()->GetModelCount() > 1)
		modelIndex = 1;

	const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel(modelIndex);

	m_pModelElement = pModel->GetElement(0);

	if(m_pModelInstance == NULL)
	{
		SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();
		m_pModelInstance = new WorldRender::C_ModelInstance( m_pModelResource->GetRenderModelData(), pModel->GetIndex(), pSceneGraph );
		m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_CustomSolid);
		
		// this sets all elements to same matrix
		SceneGraph::Insert(pSceneGraph, m_pModelInstance);
		m_pModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );
		
		m_LocalBoundingSphere = m_pModelElement->GetLocalBounds().BoundingSphere();
		m_pModelInstance->SetWorldSphere( m_LocalBoundingSphere * m_pModelElement->GetMatrix() * m_WorldMatrix);
		
		m_pModelInstance->SetNearFarDistance(0.0f, 100.0f);
	}

	// create phsyics body
	{
		//m_pModelResource->GetShapeDescriptor();

		U32 num = pModel->GetElementCount();

		SharedPhysics::SortedArray<Gravity::C_ImportBodyWrapper> dynamicBodies;

		Gravity::C_Scene* m_pPhysicsScene = GetPhysicsScene(); 
		Gravity::C_ExternalConstants* m_pPhysicsConstants = GetPhysicsExternalConstants();

		static U32 collisionGroup = 1;
		collisionGroup++;

		U32 allSolidMask = 0;
		assert(num <= 32); //Error, we'll run out of bit masks!!!!
		for(S32 i = 0; i < num; ++i)
		{
			allSolidMask = allSolidMask | (1 << i);
		}

		for(S32 i=0; i<num; ++i)
		{
			const GameResource::C_ModelElement* pElement = pModel->GetElement(i);
			const Gravity::C_ImportShape* pImportShape = pElement->GetPhysicsImportShape();
			if(pImportShape)
			{
				Gravity::C_ShapeDesc* pDesc = pImportShape->GetShapeDesc();
				pDesc->RemapShape();
				pDesc->SetRemapped();
				
				const Maths::HF32 mass(20.f);

				SharedPhysics::C_Deactivator deactivator;
				deactivator.m_ActivateThresholdSq = mass * 0.1f;
				deactivator.m_SleepThresholdSq = mass * 0.05f;

				Gravity::C_Body *pBody = m_pPhysicsScene->CreateBody();

				pBody->SetExternalConstants(m_pPhysicsConstants);

				Gravity::C_BodyHelper bodyHelper( pDesc );

				Maths::HMat43 bodyMat = pElement->GetMatrix() * m_WorldMatrix;

				Collision::C_CollisionFilter filter = PhysicsConstants::kDynamicObjectCollisionFilter;
				filter.m_GroupId = collisionGroup;
				filter.m_GroupFlags = allSolidMask;
				filter.m_GroupMask = 1 << i;
				bodyHelper.CreateBody( mass, bodyMat.AW(), (Maths::HMat33)bodyMat, pDesc->InertiaTensor(mass), pBody, &filter);
				//pBody->SetMass(100.f);

				pBody->SetDeactivator(deactivator);
				GetPhysicsScene()->AddBody(pBody);
				pBody->Slop( PhysicsConstants::kDefaultSlop );
				pBody->Baumgarte( PhysicsConstants::kDefaultBaumgarte );
				pBody->SetLinearDampingCoefficient(0.995f);
				pBody->SetAngularDampingCoefficient(0.995f);


				bodyHelper.MakeFloat(300.f, 30.f, 40.f, pBody, 1.f, 0.5f);

				pBody->Activate();
				/*static bool bFirst = true;
				if(bFirst)
				{
					bFirst = false;
					pBody->MakeStatic();
				}
				else
					pBody->Activate();*/

				pBody->ElementMatrix( bodyMat );

				pBody->SetUserData( m_pPhysicsBodyUserData );

				Gravity::C_ImportBodyWrapper temp(pBody, i);
				dynamicBodies.Insert(&temp);

				C_BodyInfo info(i, pBody);
				Util::C_String name = pElement->GetName();
				if(name == "e_rd_head")
				{
					info.m_DebugBool = &e_rd_head;
				}
				else if(name == "e_rd_neck")
				{
					info.m_DebugBool = &e_rd_neck;
				}
				else if(name == "e_rd_spine2")
				{
					info.m_DebugBool = &e_rd_spine2;
				}
				else if(name == "e_rd_spine1")
				{
					info.m_DebugBool = &e_rd_spine1;
				}
				else if(name == "e_rd_spine")
				{
					info.m_DebugBool = &e_rd_spine;
				}
				else if(name == "e_rd_hips")
				{
					info.m_DebugBool = &e_rd_hips;
				}
				else if(name == "e_rd_righthand")
				{
					info.m_DebugBool = &e_rd_righthand;
				}
				else if(name == "e_rd_rightforearm")
				{
					info.m_DebugBool = &e_rd_rightforearm;
				}
				else if(name == "e_rd_rightarm")
				{
					info.m_DebugBool = &e_rd_rightarm;
				}
				else if(name == "e_rd_rightshoulder")
				{
					info.m_DebugBool = &e_rd_rightshoulder;
				}
				else if(name == "e_rd_rightupleg")
				{
					info.m_DebugBool = &e_rd_rightupleg;
				}
				else if(name == "e_rd_rightleg")
				{
					info.m_DebugBool = &e_rd_rightleg;
				}
				else if(name == "e_rd_rightfoot")
				{
					info.m_DebugBool = &e_rd_rightfoot;
				}
				else if(name == "e_rd_lefthand")
				{
					info.m_DebugBool = &e_rd_lefthand;
				}
				else if(name == "e_rd_leftforearm")
				{
					info.m_DebugBool = &e_rd_leftforearm;
				}
				else if(name == "e_rd_leftarm")
				{
					info.m_DebugBool = &e_rd_leftarm;
				}
				else if(name == "e_rd_leftshoulder")
				{
					info.m_DebugBool = &e_rd_leftshoulder;
				}
				else if(name == "e_rd_leftupleg")
				{
					info.m_DebugBool = &e_rd_leftupleg;
				}
				else if(name == "e_rd_leftleg")
				{
					info.m_DebugBool = &e_rd_leftleg;
				}
				else if(name == "e_rd_leftfoot")
				{
					info.m_DebugBool = &e_rd_leftfoot;
				}
				
				
				m_BodyList.PushBack( info );
			}
		}

		{
			const GameResource::C_ModelData* pModelData = m_pModelResource->GetModelData();
			S32 constraintNum = pModelData->GetConstraintCount();

			Gravity::C_Body tempBody;
			for(S32 i = 0; i < constraintNum; ++i)
			{
				const GameResource::C_Constraint* pGameConstraint = pModelData->GetConstraint(i);
				const Gravity::C_ConstraintDescriptors* desc = pGameConstraint->GetPhysicsConstraint();
			

				//tempBody.SetOriginalExportBodyId(desc->m_PhysicsBodyAID);
				tempBody.SetConstraintBodyId(desc->m_PhysicsBodyAID);

				Gravity::C_ImportBodyWrapper temp(&tempBody, 0);
				Gravity::C_ImportBodyWrapper* pBodyAWrapper(dynamicBodies.Find(&temp));
			
				//tempBody.SetOriginalExportBodyId(desc->m_PhysicsBodyBID);
				tempBody.SetConstraintBodyId(desc->m_PhysicsBodyBID);

				Gravity::C_ImportBodyWrapper* pBodyBWrapper(dynamicBodies.Find(&temp));

				if(pBodyAWrapper && pBodyBWrapper)
				{
					Gravity::C_Body* pBodyA(pBodyAWrapper->GetBody());
					Gravity::C_Body* pBodyB(pBodyBWrapper->GetBody());

					SharedPhysics::C_Constraint* pConstraint;
					Util::C_String constraintName = pGameConstraint->GetName();


					if(desc->m_MaskProxy)
					{
						pBodyA->GetShape()->m_Filter.m_GroupFlags = pBodyA->GetShape()->m_Filter.m_GroupFlags & (~(1 << pBodyBWrapper->GetImportIndex()));
						pBodyB->GetShape()->m_Filter.m_GroupFlags = pBodyB->GetShape()->m_Filter.m_GroupFlags & (~(1 << pBodyAWrapper->GetImportIndex()));
					}
					else
					{
						pConstraint = desc->CreateConstraint(*m_pPhysicsScene->m_pGraphManager, pBodyA, pBodyB);
							
						m_pPhysicsScene->AddConstraint(pConstraint);

						if(desc->m_MaskCollisions)
						{
							pBodyA->GetShape()->m_Filter.m_GroupFlags = pBodyA->GetShape()->m_Filter.m_GroupFlags & (~(1 << pBodyBWrapper->GetImportIndex()));
							pBodyB->GetShape()->m_Filter.m_GroupFlags = pBodyB->GetShape()->m_Filter.m_GroupFlags & (~(1 << pBodyAWrapper->GetImportIndex()));
						}
					}

				}
			}
		}
	}

	AddSubscriber( GameMessaging::E_GameMsg_Update );
}



void C_RagdollModelTestActor::DestroyModel()
{
	SAFE_DELETE_PTR( m_pEnvSampler );

	if( m_pModelInstance )
	{
		SceneGraph::Remove(GetSceneGraph(), m_pModelInstance);
		delete m_pModelInstance;
		m_pModelInstance = NULL;
	}

	// destroy physics bodies
	{
		Gravity::C_Scene* pScene = GetPhysicsScene();

		for (Ut::Vector<C_BodyInfo>::Iterator it = m_BodyList.Begin(); it != m_BodyList.End(); ++it)
		{
			if (it->m_pBody != NULL)
			{
				it->m_pBody->SetUserData(NULL);
				pScene->RemoveBody(it->m_pBody);
				it->m_pBody = NULL;
			}
		}
	}
}

bool C_RagdollModelTestActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_RagdollModelTestActorCreate * pCreate = GetCreationData();

	m_WorldMatrix = pCreate->GetCreationMatrix();

	// create resource loader
	{
		// use single resource item component
		Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceItemCreate> pCreator;
		pCreator->m_Desc.m_Name = pCreate->GetModelFilename();
		pCreator->m_Desc.m_Type = GameResource::C_ModelResource::StaticGetResourceTypeId();
		pCreator->m_Desc.SetPrime( true );

		// use resource list component
		//Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceListCreate> pCreator;
		//pCreator->m_List.Reserve( 1 );
		//pCreator->AddItem( pModelTestCreate->GetModelFilename(), GameResource::C_ModelResource::StaticGetResourceTypeId(), Components::E_ResourceFlag_Prime );

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}

	return true;
}

bool C_RagdollModelTestActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	//const C_ModelTestActorCreate * pCreate = GetCreationData();

	m_pPhysicsBodyUserData = ::Actors::C_SimplePhysicsBodyUserData::Create(this);

	m_pModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( 0 );
	
	if (m_pModelResource != NULL)
		CreateModel();

	return true;
}


} // namespace Actors

} // namespace Game

