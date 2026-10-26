#include "GameDebugPch.h"

#include <game_debug/DebugModelRender.h>
#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>
#include <gravity/sdk/PhysicsSDK.h>
#include <world_render/ModelInstance.h>
#include <game_resource/ModelData.h>
#include <game_resource/LevelSceneManager.h>
#include <SceneGraph/SceneGraph.h>
#include <world_render/MaterialParameter.h>
#include <world_render/RenderModelData.h>
#include <world_render/EnvironmentSampler.h>

#include <gravity/narrowphase/Ray.h>

namespace GameDebug
{

Debug::Int g_TestModelRenderModelId("GameSys.TestModelRender.ModelId", -1, Debug::Range( -1, 100 ) );
Debug::Int g_TestModelRenderElementId("GameSys.TestModelRender.ElementId", -1, Debug::Range( -1, 100 ) );
Debug::Bool g_TestModelRenderModelNames("GameSys.TestModelRender.ModelNames", false);
Debug::Bool g_TestModelRenderElementNames("GameSys.TestModelRender.ElementNames", false);
Debug::Bool g_TestModelRenderUseIdentity("GameSys.TestModelRender.UseIdentity", false);
Debug::Bool g_TestModelRenderElementBounds("GameSys.TestModelRender.ElementBounds", false);
Debug::Bool g_TestModelRenderModelBounds("GameSys.TestModelRender.ModelBounds", false);
Debug::Bool g_TestModelRenderModelDataBounds("GameSys.TestModelRender.ModelDataBounds", false);

Debug::Bool g_TestModelPlaceAtOrigin("GameSys.TestModelRender.PlaceAtOrigin", false);
Debug::Bool g_TestModelPlaceOnGround("GameSys.TestModelRender.PlaceOnGround", false);
Debug::Bool g_TestModelPlaceInfrontOfCamera("GameSys.TestModelRender.PlaceInfrontOfCamera", false);

Debug::Float g_TestModelRotX("GameSys.TestModelRender.RotX", 0.0f, Debug::Range(0.0f, 360.0f));
Debug::Float g_TestModelRotY("GameSys.TestModelRender.RotY", 0.0f, Debug::Range(0.0f, 360.0f));
Debug::Float g_TestModelRotZ("GameSys.TestModelRender.RotZ", 0.0f, Debug::Range(0.0f, 360.0f));

Debug::Float g_TestModelPosX("GameSys.TestModelRender.PosX", 0.0f, Debug::Range(-5.0f, 5.0f));
Debug::Float g_TestModelPosY("GameSys.TestModelRender.PosY", 0.0f, Debug::Range(-5.0f, 5.0f));
Debug::Float g_TestModelPosZ("GameSys.TestModelRender.PosZ", 0.0f, Debug::Range(-5.0f, 5.0f));

Debug::Bool g_TestModelRender("GameSys.TestModelRender.RenderTestModel", false);
Debug::Bool g_TestPlatformRender("GameSys.TestModelRender.RenderTestPlatforms", false);

Debug::String g_SimpleUiModelString("GameSys.TestModelRender.Filename", "dontship/models/system/SpecSphere.model");

// C_DebugModelDataRender

C_DebugModelDataRender::C_DebugModelDataRender()
	: m_pSceneGraph( NULL )
	, m_pEnvSampler( NULL )
	, m_pEnvMapGPUTextureOverride( NULL )
{
	m_Origin.SetIdentity();
}

C_DebugModelDataRender::~C_DebugModelDataRender()
{
}
	
void C_DebugModelDataRender::SetCollisionFilter( const Collision::C_CollisionFilter& filter )
{
	m_CollisionFilter = filter;
}

void C_DebugModelDataRender::CreateModelInstances()
{
	// create a model instance for each model...
	S32 modelCount = 0;
	
	if(m_pModelData)
	{
		modelCount = m_pModelData->GetModelCount();
	}

	m_ModelInstances.Resize( modelCount );

	S32 totalElementCount = 0;
	for( S32 modelNo=0; modelNo < modelCount; modelNo++ )
	{
		const GameResource::C_Model* pModel = m_pModelData->GetModel(modelNo);
		totalElementCount += pModel->GetElementCount();
	}

	m_BodyList.Resize( totalElementCount );
	S32 bodyIndex = 0;

	for( S32 modelNo=0; modelNo < modelCount; modelNo++ )
	{
		// Create render model instance
		WorldRender::C_ModelInstance* pNewModelInstance = new WorldRender::C_ModelInstance( m_pRenderModelData, modelNo, m_pSceneGraph );
		pNewModelInstance->SetWorldSphere(Maths::C_Sphere( Maths::HVec3( 0.0f, 0.0f, 0.0f) * m_Origin , 2.0f));
		m_ModelInstances[modelNo] = pNewModelInstance;

		// create physics bodies
		const GameResource::C_Model* pModel = m_pModelData->GetModel(modelNo);
		const S32 elementCount = pModel->GetElementCount();
		for(S32 elementNo=0; elementNo< elementCount; elementNo++)
		{
			const GameResource::C_ModelElement* pElement = pModel->GetElement(elementNo);
			const Gravity::C_ImportShape* pImportShape = pElement->GetPhysicsImportShape();

			if (pImportShape)
			{
				Gravity::C_ShapeDesc* pDesc = pImportShape->GetShapeDesc();
				pDesc->RemapShape();
				pDesc->SetRemapped();
				
				const Maths::HF32 mass(100.f);

				SharedPhysics::C_Deactivator deactivator;
				deactivator.m_ActivateThresholdSq = mass * 0.05f;
				deactivator.m_SleepThresholdSq = mass * 0.01f;

				Gravity::C_Body* pBody = m_pPhysicsScene->CreateBody();
				m_BodyList[bodyIndex] = pBody;

				pBody->SetExternalConstants(m_pExternalConstants);

				Gravity::C_BodyHelper bodyHelper( pDesc );

				Maths::HMat43 bodyMat = pElement->GetMatrix() * m_Origin;

				//Collision::C_CollisionFilter collisionFilter = PhysicsConstants::kDynamicObjectCollisionFilter;
				//collisionFilter.m_GroupId = 0xffff;

				bodyHelper.CreateBody( mass, bodyMat.AW(), (Maths::HMat33)bodyMat, pDesc->InertiaTensor(mass), pBody, &m_CollisionFilter);
				//m_pBody->SetMass(100.f);

				F32 kDefaultSlop = 0.000005f;
				F32 kDefaultBaumgarte = 0.8f;

				pBody->SetDeactivator(deactivator);
				pBody->Slop( kDefaultSlop );
				pBody->Baumgarte( kDefaultBaumgarte );
				pBody->MakeStatic();
				//pBody->Activate();

				pBody->ElementMatrix( bodyMat );

				//m_pPhysicsScene->TemporarilyRemoveBody( pBody );
			}
			else
			{
				m_BodyList[bodyIndex] = NULL;
			}

			bodyIndex++;
		}
	}		
}

void C_DebugModelDataRender::DestroyModelInstances()
{			
	for(S32 modelNo = 0; modelNo < m_ModelInstances.Size(); ++modelNo)
	{
		WorldRender::C_ModelInstance* pModelInstance = m_ModelInstances[modelNo];
		if( pModelInstance->GetCullNode() )
		{
			SceneGraph::Remove( m_pSceneGraph, pModelInstance );
		}
		SAFE_DELETE_PTR( m_ModelInstances[modelNo] );
	}
	m_ModelInstances.Clear();

	const S32 bodyCount = m_BodyList.Size();
	for( S32 bodyNo=0; bodyNo<bodyCount; bodyNo++ )
	{
		if( m_BodyList[bodyNo] )
		{
			if( m_BodyList[bodyNo] )
			{
				m_pPhysicsScene->RemoveBody( m_BodyList[bodyNo] );
				m_BodyList[bodyNo] = NULL;
			}
		}		
	} // On with the bodyCount.

	m_BodyList.Clear();

}

void C_DebugModelDataRender::SetEnvMapOverride(const void* pEnvMapGPUTextureOverride)
{
	m_pEnvMapGPUTextureOverride = pEnvMapGPUTextureOverride;
}

void C_DebugModelDataRender::UpdateModelInstances(const bool gamePaused)
{
	if(!m_pModelData)
	{
		return;
	}

	Maths::HMat43 mModelData = m_Origin;
	if(g_TestModelRenderUseIdentity)
	{
		mModelData.SetIdentity();
	}

	if( g_TestModelRenderModelDataBounds )
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox( m_pModelData->GetBounds(), mModelData, Maths::RGBA( 0xff, 0x00, 0x00, 0xff) );
	}

	// update light sampler
	if(m_pEnvSampler)
	{
		m_pEnvSampler->SetPosition( m_pModelData->GetBounds().Centre() * mModelData );
		m_pEnvSampler->Update();
	}

	S32 modelBodyIndex = 0;
	const S32 modelCount = m_pModelData->GetModelCount();
	for( S32 modelNo=0; modelNo < modelCount; modelNo++ )
	{
		GameResource::C_Model* pModel = m_pModelData->GetModel( modelNo );
		WorldRender::C_ModelInstance* pModelInstance = m_ModelInstances[modelNo];

		bool renderModel = true;
		if( g_TestModelRenderModelId != -1 )
		{
			if(modelNo != g_TestModelRenderModelId)
			{
				renderModel = false;
			}
		}

		if( renderModel )
		{
			if( !pModelInstance->GetCullNode() )
			{
				SceneGraph::Insert(	m_pSceneGraph, pModelInstance );
			}

			if( !gamePaused )
			{
				for(S32 elementNo=0; elementNo<pModel->GetElementCount(); elementNo++)
				{
					S32 bodyIndex = modelBodyIndex + elementNo;
					const GameResource::C_ModelElement* pElement = pModel->GetElement(elementNo);

					Gravity::C_Body* pBody = m_BodyList[bodyIndex];
					if(pBody)
					{
						if( !pBody->GetScene() )
						{
							m_pPhysicsScene->AddBody( pBody );
						}

						Maths::HMat43 bodyMat = pElement->GetMatrix() * m_Origin;
						pBody->ElementMatrix( bodyMat );

					}
				}
			}
		}
		else
		{
			if( pModelInstance->GetCullNode() )
			{
				SceneGraph::Remove(m_pSceneGraph, pModelInstance );
			}

			if( !gamePaused )
			{
				for(S32 elementNo=0; elementNo<pModel->GetElementCount(); elementNo++)
				{
					S32 bodyIndex = modelBodyIndex + elementNo;
					const GameResource::C_ModelElement* pElement = pModel->GetElement(elementNo);

					Gravity::C_Body* pBody = m_BodyList[bodyIndex];
					if(pBody)
					{
						if( pBody->GetScene() )
						{
							m_pPhysicsScene->TemporarilyRemoveBody( pBody );
						}
					}
				}
			}
		}

		Maths::HMat43 mModel = pModel->GetMatrix();
		if(g_TestModelRenderUseIdentity)
		{
			mModel.SetIdentity();
		}

		mModel = mModel * m_Origin;

		if( renderModel )
		{
			if( g_TestModelRenderModelNames )
			{
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText( mModel.AW(), 0.25f, pModel->GetName() );
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix( mModel );
			}

			if( g_TestModelRenderModelBounds )
			{
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox( pModel->GetLocalBounds(), mModel, Maths::RGBA( 0x00, 0xff, 0x00, 0xff) );
			}
		}

		/*pModelInstance->SetWorldSphere( pModel->GetLocalBounds().BoundingSphere() * mModel  );  */
		pModelInstance->SetWorldSphere( pModel->GetBoundingSphere() * mModel  );  

		const S32 elementCount = pModel->GetElementCount();
		for(S32 elementNo=0; elementNo<elementCount; elementNo++)
		{
			GameResource::C_ModelElement* pModelElement = pModel->GetElement( elementNo );				
			Maths::HMat43 m = pModelElement->GetMatrix();

			if(g_TestModelRenderUseIdentity)
			{
				m.SetIdentity();
			}

			m = m * m_Origin;

			pModelInstance->SetElementMatrix( elementNo, m );

			if (m_pEnvSampler)
			{
				m_pEnvSampler->ApplyToModelInstance(pModelInstance);

				if(m_pEnvMapGPUTextureOverride != NULL)
				{
					if (pModelInstance != NULL)
					{
						const WorldRender::C_Model *pModel = pModelInstance->GetRenderModel();
						WorldRender::MaterialParameter hEnvMap = pModel->GetParameter("g_tEnvironment");

						pModelInstance->SetParameter(hEnvMap, &m_pEnvMapGPUTextureOverride);	
					}
				}
			}		

			bool renderElement = renderModel;
			if( g_TestModelRenderElementId != -1 )
			{
				if(elementNo != g_TestModelRenderElementId)
				{
					renderElement = false;
				}
			}

			pModelInstance->SetElementVisibility( elementNo, renderElement );

			if( renderElement )
			{
				if( g_TestModelRenderElementNames )
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix( m );
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText( m.AW(), 0.25f, pModelElement->GetName().Chars() );
				}

				if( g_TestModelRenderElementBounds )
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox( pModelElement->GetLocalBounds(), m, Maths::RGBA( 0x00, 0xff, 0xff, 0xff) );
				}
			}
		}

		modelBodyIndex += pModel->GetElementCount();
	}
}
	
void C_DebugModelDataRender::Update( const bool enable, const bool gamePaused )
{
	if(enable)
	{
		if( m_ModelInstances.Size() == 0 )
		{
			CreateModel();
			CreateModelInstances();
		}

		UpdateModelInstances( gamePaused );
	}
	else
	{
		if( m_ModelInstances.Size() > 0 )
		{
			DestroyModel();
			DestroyModelInstances();
		}
	}
}

// C_DebugModelRender
C_DebugModelRender::C_DebugModelRender()	
	: m_pModelResource( NULL )
{
	m_WorldOrigin.SetIdentity();
}

C_DebugModelRender::~C_DebugModelRender()
{
}

void C_DebugModelRender::Init( GameResource::C_LevelSceneManager* pLevelSceneManager, Gravity::C_ExternalConstants* pExternalConstants )
{
	m_pLevelSceneManager = pLevelSceneManager;
	m_pSceneGraph = pLevelSceneManager->GetWorldRenderScene()->GetSceneGraph();
	m_pPhysicsScene = pLevelSceneManager->GetPhysicsScene();
	m_pExternalConstants = pExternalConstants;

	m_pEnvSampler = new WorldRender::C_EnvironmentSampler();
	m_pEnvSampler->Initialise( pLevelSceneManager->GetWorldRenderScene() );
}

void C_DebugModelRender::Close()
{
	SAFE_DELETE_PTR(m_pEnvSampler);
	DestroyModel();
}


void C_DebugModelRender::CreateModel()
{
	Util::C_String fileName = g_SimpleUiModelString.Value();
	if( fileName.Length() == 0 )
	{
		fileName = "models/testmodels/mymodel.model";
	}
	m_ModelHandle.Load( fileName );
	m_pModelResource = m_ModelHandle;
	Resource::WaitForResourceData(m_pModelResource); // debug code, resource wait allowed
	m_pModelData =	m_pModelResource->GetModelData();

	m_pRenderModelData = m_pModelResource->GetRenderModelData();

}


void C_DebugModelRender::DestroyModel()
{
	m_pModelResource = NULL;
}

void C_DebugModelRender::Update( const Maths::HMat43& mCameraToWorld, const bool gamePaused )
{
	if( g_TestModelPlaceAtOrigin )
	{
		m_WorldOrigin.SetIdentity();
	}

	if( g_TestModelPlaceInfrontOfCamera )
	{
		//Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		//Maths::HMat43 cameraMatrix = view.m_View.GetViewSettings3D().GetCameraToWorld();
	
		m_WorldOrigin = mCameraToWorld;
		m_WorldOrigin.AW( Maths::HVec3( 0.0, 0.0f, 5.0f ) * mCameraToWorld );
	}

	if( g_TestModelPlaceOnGround )
	{
		Collision::C_Ray ray;

		Collision::C_CollisionFilter rayCollisionFilter( 0xffffffff, 0xffffffff, 0xffff, 0, 0 );

		//Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		//Maths::HMat43 cameraMatrix = view.m_View.GetViewSettings3D().GetCameraToWorld();

		Maths::HVec3 start = Maths::HVec3( 0.0f, 0.0f, 0.0f ) * mCameraToWorld;
		Maths::HVec3 end = Maths::HVec3( 0.0f, 0.0f, 30.0f ) * mCameraToWorld;

		ray.InitialiseRay( start, end, &rayCollisionFilter );
								
		m_pPhysicsScene->DoRayTrace( ray, m_pPhysicsScene->m_Stack, false );

		if(ray.m_result.m_pShape)
		{
			Maths::HMat33 orient( Maths::AxisY(ray.m_result.m_normal), Maths::AxisZ(ray.m_Direction) );
			Maths::HMat43 finalTransform( orient, ray.m_end );
			m_WorldOrigin = finalTransform;
		}
	}

	Maths::HMat33 rotM = Maths::HMat33( Maths::RotX( Maths::DegToRad((F32)g_TestModelRotX)) ) * Maths::HMat33( Maths::RotZ( Maths::DegToRad((F32)g_TestModelRotZ) ) ) * Maths::HMat33( Maths::RotY( Maths::DegToRad((F32)g_TestModelRotY) ) );
	Maths::HVec3 pos = Maths::HVec3( (F32)g_TestModelPosX, (F32)g_TestModelPosY, (F32)g_TestModelPosZ );
	Maths::HMat43 offsetM = Maths::HMat43( rotM, pos );
	m_Origin = offsetM * m_WorldOrigin;

	C_DebugModelDataRender::Update( g_TestModelRender, gamePaused );
}

// C_DebugPlatformRender
C_DebugPlatformRender::C_DebugPlatformRender()	
{
}

C_DebugPlatformRender::~C_DebugPlatformRender()
{
}

void C_DebugPlatformRender::Init( GameResource::C_LevelSceneManager* pLevelSceneManager, Gravity::C_ExternalConstants* pExternalConstants, GameResource::C_ModelData* pModelData, WorldRender::C_RenderModelData* pRenderModelData)
{
	m_pLevelSceneManager = pLevelSceneManager;
	m_pSceneGraph = pLevelSceneManager->GetWorldRenderScene()->GetSceneGraph();
	m_pPhysicsScene = pLevelSceneManager->GetPhysicsScene();
	m_pExternalConstants = pExternalConstants;
	m_pModelData = pModelData;
	m_pRenderModelData = pRenderModelData;

	m_pEnvSampler = new WorldRender::C_EnvironmentSampler();
	m_pEnvSampler->Initialise( pLevelSceneManager->GetWorldRenderScene() );
}

void C_DebugPlatformRender::Close()
{
	SAFE_DELETE_PTR(m_pEnvSampler);
	DestroyModelInstances();
}
	
void C_DebugPlatformRender::Update(const bool gamePaused)
{
	C_DebugModelDataRender::Update( g_TestPlatformRender, gamePaused );
}

} // namespace GameDebug
