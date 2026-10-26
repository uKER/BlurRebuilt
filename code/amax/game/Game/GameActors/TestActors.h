#ifndef GAME_TEST_ACTORS_H
#define GAME_TEST_ACTORS_H

#ifdef _MSC_VER
#pragma once
#endif



#include <actor_system/ActorSys.h>
#include <misc_shared/Messaging/GameMessaging.h>

#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <Resource/Resource.h>
#include <game_resource/ModelResource.h>
#include <world_render/ModelInstance.h>
#include <world_render/RenderModelData.h>
#include <SceneGraph/SceneGraph.h>
#include <game_resource/ModelData.h>

#include <Game/GameWorld/GameWorld.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>
#include <world_render/Scene.h>

namespace Game
{

namespace Actors
{

	
class C_ModelTestActorCreate : public ::Actors::C_PosActorCreate
{
public:
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_ModelTest;}
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_ModelTest;}

	Util::C_String m_ModelFilename;
	Game::C_GameWorld* m_pGameWorld;
};

// Actor Info
class C_ModelTestActor : public C_GameActor, public  Resource::C_Listener
{
public:
	C_ModelTestActor()
	  : m_pModelResource(NULL)
	  , m_pModelInstance(NULL)
	  , m_pGameWorld(NULL)
	  , m_pBody(NULL)
	  , m_pLightSampler(NULL)
	{}
	bool Create(const Actor::C_ActorCreationData *creationData)
	{
		const C_ModelTestActorCreate * pModelTestCreate = (const C_ModelTestActorCreate *)creationData;

		m_pGameWorld = pModelTestCreate->m_pGameWorld;

		m_WorldMatrix = pModelTestCreate->GetCreationMatrix();
		m_ModelHandle.Load( pModelTestCreate->m_ModelFilename, this );
		//GameResource::C_ModelResource* pModelResource = m_ModelHandle;

		return true;
	}

	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_ModelTest; }
	static const Btl::String32	TypeName()	{ return "ModelTest"; }

	virtual bool OnMessage(const Messaging::C_MessageBase *pMessage)
	{
		switch(pMessage->Type())
		{
			case GameMessaging::E_GameMsg_Update:
				if(m_pModelInstance != NULL)
				{
					//m_WorldMatrix = Maths::HMat43(m_pBody->Orientation(), m_pBody->Position());
					m_WorldMatrix = m_pBody->ElementMatrix();//m_pModelElement->GetMatrix() * m_pBody->ElementMatrix();
					m_pModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 2.0f));
					m_pModelInstance->SetElementMatrix(-1, m_WorldMatrix);

					//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix( m_WorldMatrix );
					
					if(m_pLightSampler)
					{
						m_pLightSampler->SetPosition( m_WorldMatrix.AW() );
						m_pLightSampler->Update();

						WorldRender::MaterialParameter handle = m_pModelInstance->GetRenderModel()->GetParameter("g_SHNode");
						if (handle.index != -1)
						{
							m_pModelInstance->SetParameter(handle, &m_pLightSampler->GetLightSampleData().m_SampleData[0]);	
						}

					}
				}
					
				return true;
			break;
		}

		return false;
	}

	void CreateModel()
	{
		m_pLightSampler = new WorldRender::C_LightSampler;
		m_pLightSampler->Initialise( &GetWorldRenderScene()->GetLightSamplePointScene() );

		//const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel("M_woodBox_UNDAMAGED");
		const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel(1);
		m_pModelElement = pModel->GetElement( 0 );

		if(m_pModelInstance == NULL)
		{
			m_pModelInstance = new WorldRender::C_ModelInstance( m_pModelResource->GetRenderModelData(), pModel->GetIndex(), GetSceneGraph());//pModel->GetIndex() );
			
			// this sets all elements to same matrix
			m_pModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );
			m_pModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 10.0f));
			// crashes in rendering at the moment
			SceneGraph::Insert(GetGameWorld()->GetSceneGraph(), m_pModelInstance);

			m_pModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );
			m_pModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.Translation(), 10.0f));
		}

		// create phsyics body
		{
			//m_pModelResource->GetShapeDescriptor();

			const GameResource::C_ModelElement* pElement = pModel->GetElement(0);

			const Gravity::C_ImportShape* pImportShape = pElement->GetPhysicsImportShape();
			Gravity::C_ShapeDesc* pDesc = pImportShape->GetShapeDesc();
			pDesc->RemapShape();

			SharedPhysics::C_Deactivator deactivator;
			deactivator.m_ActivateThresholdSq = 0.005f;
			deactivator.m_SleepThresholdSq = 0.001f;

			m_pBody = GetGameWorld()->GetPhysicsScene()->CreateBody();

			m_pBody->SetExternalConstants(&GetGameWorld()->GetPhysicsExternalConstants());

			Gravity::C_BodyHelper bodyHelper( pDesc );

			Maths::HMat43 bodyMat = m_WorldMatrix;

			bodyHelper.CreateBody(100.f, bodyMat.AW(), (Maths::HMat33)bodyMat, pImportShape->GetImportIneriaTensor(), m_pBody, &GetGameWorld()->m_TempBodyCollisionFilter);
			//m_pBody->SetMass(100.f);

			m_pBody->SetDeactivator(deactivator);
			GetGameWorld()->GetPhysicsScene()->AddBody(m_pBody);
			m_pBody->Slop(0.000005f);
			m_pBody->Baumgarte(0.8f);
			m_pBody->Activate();

			m_pBody->ElementMatrix( m_WorldMatrix );
		}

		AddSubscriber( GameMessaging::E_GameMsg_Update );
	}

	void DestroyModel()
	{
		SAFE_DELETE_PTR( m_pLightSampler );

		if( m_pModelInstance )
		{
			SceneGraph::Remove(GetGameWorld()->GetSceneGraph(), m_pModelInstance);
			delete m_pModelInstance;
		}
	}

	Game::C_GameWorld* GetGameWorld()
	{
		return m_pGameWorld;
	}

	void ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
	{
		switch(result)
		{
			case Resource::E_Notify_DataReady:
			{				
				GameResource::C_ModelResource* pModelResource = m_ModelHandle;
				if( pResource == pModelResource )
				{
					m_pModelResource = pModelResource;
					CreateModel();
				}
			}
			break;
			case Resource::E_Notify_DataError:
			case Resource::E_Notify_NoData:
				if( pResource == m_pModelResource )
				{
					DestroyModel();
				}
			break;
		}
		LOGDEBUG( Resource::g_Log, "Notify pResource=0x%08x, ['%s'] \"%s\" %d, 0x%08x", pResource, pResource->GetResourceTypeName().Chars(), pResource->GetResourceFilename().Chars(), result, pData );
	}

private:
	Maths::HMat43	m_WorldMatrix;

	C_GameWorld* m_pGameWorld;
	GameResource::C_ModelHandle			m_ModelHandle;
	GameResource::C_ModelResource*		m_pModelResource;
	WorldRender::C_ModelInstance*		m_pModelInstance;
	const GameResource::C_ModelElement*	m_pModelElement;

	Gravity::C_Body*					m_pBody;
	WorldRender::C_LightSampler*		m_pLightSampler;
};

} // namespace Actors


} // namespace Game


#endif //  GAME_ACTOR_TYPES_H
