#include "stdafx.h"


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <shared_actors/SharedActorUtils.h>
#include <shared_actors/Utility/VisibilityDetection.h>
#include <resource_utils/GameModel.h>

#include <Resource/Resource.h>
#include <world_render/ModelInstance.h>
#include <world_render/RenderModelData.h>
#include <world_render/Scene.h>
#include <world_render/EnvironmentSampler.h>
#include <SceneGraph/SceneGraph.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>

#include <GameApp.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameLogic/ActorStudioActors/DynamicObjectActor.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/GameData/GameData.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Data/RLDataStructures.h>

#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/ViewType.h>
#include <Game/GameWorld/RenderPass.h>
#include <Game/GameDef/PhysicsConstants.h>
#include <Game/Messaging/GameflowMessages.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <system/Memory.h>

#include <shared_actors/Components/Messages/PhysicsMessages.h>

#include "Game/GameActors/Components/Creation/AudioComponentsCreate.h"

#include <Render/Deferred/DropShadows.h>

#include <animated_model/AnimatedModelInstance.h>
#include <animation/common/StringId.h>
#include <animation/deferred/CommandList.h>
#include <animation/pose/PoseFormat.h>


#include <Gravity/SDK/ShapeDescriptors.h>
#include <Gravity/Triggers/TriggerSystem.h>
#include <gravity/triggers/TriggerEvent.h>
#include <gravity/sdk/KinematicController.h>

#include <actor_studio_msg_game/ActorStudioMessaging.h>



Debug::Logger &g_DynObjLog = Debug::GetLogger("amax.actors.dynamicobject");
Debug::Float g_DrawDistanceFactor("ActorDebug.Actors.DynamicObjectActor.DrawDistanceFactor", 275.0f);
//Debug::Float g_DrawDistanceFactor("DynamicObjects.Objects.DrawDistanceFactor", 275.0f);
Debug::Bool g_debugEnableDynamicObjectCollision("DynamicObjects.EnablePhysics", true);
Debug::Float g_debugVehDynKillDot("DynamicObjects.KillDot", 0.96f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugVehDynKillOffset("DynamicObjects.KillHeight", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Bool g_DynObjAddAudioTriggers("Audio.Actors.DynamicObject.EnableDynObjects", true);

extern Debug::Float g_debugPhysObjSnagTimeBeforeDeath;
extern Debug::Float g_debugPhysObjSnagSpeedMinBeforeDeath;
extern Debug::Float g_debugPhysObjSnagUpForceYDiff;
extern Debug::Float g_debugPhysObjSnagUpForce;

//#pragma optimize("", off)

//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::DynamicObjectCreationData,		"DynamicObjActor")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_DynamicObjectDef,				"DynamicObjType")
	XT_FIELD(m_ReplacementGroup,				"ReplaceFromGroup")

	XT_FIELD(m_AnimStartTimer,					"AnimStartTimer")
	XT_FIELD(m_RaceLogicGroup,					"RaceLogicGroup")

	XT_FIELD(m_IgnoreAlignment,					"IgnoreAlignment")

	XT_FIELD(m_OnPlayerCollision,				"OnPlayerCollision")
	XT_FIELD(m_OnCollision,						"OnCollision")
	XT_FIELD(m_OnDestroyed,						"OnDestroyed")

	XT_FIELD(m_VehicleCollisionMessage,			"VehicleCollisionMessage")

	XT_FIELD(m_pSorceryMovementComponentCreate,	"MovementComponent")

	XT_FIELD(m_EnableRespawn,					"EnableRespawn")
//	XT_FIELD(m_AudioComponent,					"AudioComponent")
XT_END_TYPE


//=========================================================================
//					Component Creation Setup
//=========================================================================


void Actors::DynamicObjectCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH("actors.dynamicobject.creation.sorcery");
	C_PosActorCreate::DoSorceryCallback(pCallbackFn);

	if (m_pSorceryMovementComponentCreate != NULL)
		DoComponentSorceryCallback(pCallbackFn, *m_pSorceryMovementComponentCreate, &m_pMovementCreator);
	//[NEEDS DEFINE]MEMORY_TRACKER_POP("actors.dynamicobject.creation.sorcery");
}



namespace Actors
{


//=========================================================================
//					Dynamic Actor Core Routines
//=========================================================================



DynamicObjectActor::DynamicObjectActor() : m_pModelResource(NULL), m_pModelInstance(NULL), m_pBody(NULL), m_pEnvSampler(NULL), m_KinematicController(NULL), m_pGameWorld(NULL)
{
	S32		a;


	objdef = NULL;

	starttime = 0.0f;
	lifespan = 0.0f;
//	simpledamage = 100.0f;
//	num_hits = 0;
	possible_error = false;

	for(a=0;a<4;a++)
		m_pSprings[a] = NULL;

	m_ShadowIntensity = -1.f;
	m_ShadowRadius = 0.f;

	collision = true;
	initial_state = false;
	
	m_NumberOfAnimations = 0;
	anim_active = false;

	m_bVisible = false;
	m_fLastAnimTime = 0;
	m_Freeze = false;
	m_HasBeenHit = false;

	for(a=0;a<Game::NUM_ANIMATION_EVENT_TYPES;a++)
	{
		m_ValidAnimation[a] = false;
		m_pCycleResource[a] = NULL;
		m_pCycleInstance[a] = NULL;
	}

	m_pVisDetect = NULL;
	m_pTriggerObject = NULL;
	m_kinematic = false;

	m_ModelCreated = false;

	m_FramesInCollision		= 0;
	m_TimeInCollision		= 0.0f;
	m_RespawnTime			= 0.0f;

	m_PhysType = Game::E_PHYSICS_BEHAVIOUR_NONE;

	m_Selected = false;

	m_DontMakeKinematic = false;
	m_bVisibleLastFrame = false;

//	m_last_collision_time	= 0.0f;
//	m_total_collision_time	= 0.0f;
}


//=========================================================================

DynamicObjectActor::~DynamicObjectActor()
{
	//destroy trigger for object that died in its sleep...
	DestroyTrigger(GetHandle());

	if (m_pModelResource != NULL)				// Also removes physics model
		DestroyModel();

	Actors::C_VisibilityDetector::ReleaseVisDetect(m_pVisDetect);

	//// release the audio resources
	//if (GetCreationData()->m_DynamicObjectDef && GetCreationData()->m_DynamicObjectDef->m_DynamicObjectAudioDef)
	//{
	//	GetCreationData()->m_DynamicObjectDef->m_DynamicObjectAudioDef->ReleaseEntityType();
	//	LOGINFO(Game::g_Log, "Freeing DynamiObjectActor '%s'", objdef->m_DynamicObjectAudioDef->Name());
	//}
}


//=========================================================================

bool DynamicObjectActor::InitialiseImpl()					// Called first
{
	//S32		random_entry;
	m_pVisDetect = Actors::C_VisibilityDetector::CreateVisDetect(GetHandle());

	if (!C_GameActor::InitialiseImpl())
		return(false);

	const DynamicObjectCreationData *pCreate = GetCreationData();
	m_WorldMatrix = pCreate->GetCreationMatrix();

	if (pCreate->m_ReplacementGroup && pCreate->m_ReplacementGroup->ObjDefList.count > 0)
		objdef = pCreate->m_ReplacementGroup->ObjDefList[(rand()%pCreate->m_ReplacementGroup->ObjDefList.count)];
	else
		objdef = pCreate->m_DynamicObjectDef;

	if (!objdef)
	{
		LOGERROR(Game::g_Log, "Dynamic object has a null object definition");
		return(false);
	}

	m_PhysType = objdef->m_PhysType;						// Local version of physics type in case it changes
	
	if (pCreate->GetMovementCreator() != NULL)				// Create movement component
	{
		if (!InitialiseComponent(pCreate->m_pMovementCreator))
			return(false);
	}
	

	//disabling dynamic objects intil crash is fixed
	////create audio components

	if (objdef && objdef->m_DynamicObjectAudioDef)
	{
		Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_SpawnedCreateInfo> pCreator;

		pCreator->m_AudioEntityDef = objdef->m_DynamicObjectAudioDef; 

		if (InitialiseComponent(pCreator.Get()) == NULL)
		{
			LOGERROR(Game::g_Log, "Failed to create audio component for dynamic Object actor");
			return false;
		}
	}


	AmaxRoute *pRoute  = C_GameData::Instance().mRoutes[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mRoute];
	minheight = pRoute->minYPosition;
	if (minheight == 0)
		minheight = -500.0f;				// w -20 The default cull plane is at -20 metres unless specified and not zero (Only temp until all routes have this defined)

	::Actors::C_ActorUtils::Instance().SanityCheck_Register(this);	// Check this against any other objects

	return(true);
}


//=========================================================================

bool DynamicObjectActor::CreateImpl()						// Called second
{
	U32		a;

	if (!C_GameActor::CreateImpl())
		return(false);

	const DynamicObjectCreationData * pCreate = GetCreationData();

	if (objdef)
	{
		if (!objdef->m_ModelType)
		{
			LOGERROR(Game::g_Log, DBSTR "DynamicObjectActor %s has null Model", GetActorName());
			return false;
		}

		m_ModelHandle.Load( objdef->m_ModelType->m_Filename.Text() );	// don't do resource notify here as it succeeds all the time until m_ValidAnimation has been initialised

		/*if (objdef->m_DynamicObjectAudioDef)
		{
			objdef->m_DynamicObjectAudioDef->SetupEntityType();
			LOGINFO(Game::g_Log, "Loading DynamiObjectActor '%s'", objdef->m_DynamicObjectAudioDef->Name());
		}*/

		if (objdef->m_AnimEvents.count > Game::NUM_ANIMATION_EVENT_TYPES)
			LOGERROR(Game::g_Log, DBSTR "Duplicate animation events in %s", GetActorName());

		m_NumberOfAnimations = objdef->m_AnimEvents.count;
		for(a=0;a<m_NumberOfAnimations;a++)						// For all events
		{
			if (objdef->m_AnimEvents[a].m_Animation)			// See if we're startig or stopping an animation
			{
				m_ValidAnimation[a] = true;
				m_CycleHandle[a].Load( objdef->m_AnimEvents[a].m_Animation->m_Filename.Text() ); // don't do resource notify here as it succeeds all the time until m_ValidAnimation has been initialised
			}
			else
			{
				m_ValidAnimation[a] = false;
			}
		}


		((Resource::C_ResourceBase*)m_ModelHandle)->AddListener(this);
		for(a=0;a<m_NumberOfAnimations;a++)						// For all events
		{
			if (objdef->m_AnimEvents[a].m_Animation)			// See if we're startig or stopping an animation
			{
				((Resource::C_ResourceBase*)m_CycleHandle[a])->AddListener(this);
			}
		}

	}
	else
	{
		LOGERROR(Game::g_Log, DBSTR "DynamicObjectActor %s has no Dynamic Object Definition", GetActorName());
		return false;
	}

	
	AddSubscriber(ActorMsg::E_DynamicObjMsg_RenderDebug);
	AddSubscriber(ActorMsg::E_DynamicObjMsg_SorceryUpdate);
	AddSubscriber(Game::E_GameflowMessage_RaceLoaded);
	AddSubscriber(Game::E_GameMsg_KillObjectsForCutScene);

		
	return(true);
}

//=========================================================================

void DynamicObjectActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	U32		a;
	bool	create = false;

	switch(result)
	{
		case Resource::E_Notify_DataReady:
		{				
//			GameResource::C_ModelResource* pModelResource = m_ModelHandle;
//			if (pModelResource == pResource)
//				m_pModelResource = pModelResource;

			if (pResource == m_ModelHandle)
				m_pModelResource = m_ModelHandle;


			for(a=0;a<m_NumberOfAnimations;a++)
			{
				Animation::C_CycleResource* pCycleResource = m_CycleHandle[a];
				if (pCycleResource == pResource)
					m_pCycleResource[a] = pCycleResource;
			}

			create = (m_pModelResource != NULL);							// See if we've got everything loaded
			for(a=0;a<m_NumberOfAnimations;a++)
				if (m_ValidAnimation[a])
					if (m_pCycleResource[a] == NULL)
						create = false;
					
			if (create)
			{
				if (m_ModelCreated)
				{
					LOGERROR( Resource::g_Log, "Dynamic Actor '%s' (%s) has attempted to create it's model twice (Possibly using a physical model with dynamic model animations)", GetCreationData()->GetName(), GetCreationData()->m_DynamicObjectDef->m_ModelType.Object()->m_Filename.Text());
				}
				else
				{
					CreateModel();
					m_ModelCreated = true;
				}
			}
		}
		break;
		
		case Resource::E_Notify_DataError:
		{
			const DynamicObjectCreationData * pCreate = GetCreationData();
			LOGERROR(Game::g_Log, DBSTR "DynamicObjectActor '%s' failed to load model '%s'", GetActorName(), pCreate->m_DynamicObjectDef->m_ModelType->m_Filename.string);
		}
		break;
		
		case Resource::E_Notify_NoData:
			if( pResource == m_pModelResource )
			{
				DestroyModel();
			}
		break;
	}
}

//=========================================================================

extern Debug::Float g_debugHackStaticActivateScale;

void DynamicObjectActor::CreateModel()
{
	U32		a;

	MEMORY_TRACKER_PUSH("actors.dynamicobject");

	if (!GetCreationData()->m_IgnoreAlignment)
		C_ActorUtils::Instance().AlignToScene(&m_WorldMatrix, objdef->m_Alignment, 3.0f, GetPhysicsScene());

	S32 modelIndex = 0;													// Can only deal with one model element for now
	if (m_pModelResource->GetModelData()->GetModelCount() > 1)
		modelIndex = 1;
	else
		LOGWARN( Resource::g_Log, "Dynamic Actor '%s' (%s) should be a physical object", GetCreationData()->GetName(), GetCreationData()->m_DynamicObjectDef->m_ModelType.Object()->m_Filename.Text());


	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH("actors.dynamicobject.scenegraph");
	const GameResource::C_Model* pModel = m_pModelResource->GetModelData()->GetModel(modelIndex);

	num_model_elements = pModel->GetElementCount();
	m_pModelElement = pModel->GetElement(0);
	if (m_pModelInstance == NULL)							// Create render model
	{
		// combine all of the bounding boxes of all of the elements
		Maths::C_AxisBox bbox;
		for(a=0; a<num_model_elements; a++)
			bbox.UpdateExtents(pModel->GetElement(a)->GetLocalBounds());
		// convert bounding box to bounding sphere
		m_LocalBoundingSphere = bbox.BoundingSphere();

		SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();
		//m_pModelInstance = new WorldRender::C_ModelInstance( m_pModelResource->GetRenderModelData(), pModel->GetIndex(), pSceneGraph);
				

		Animation::C_PoseFormat		poseFormat;
		Animation::C_PoseFieldMask poseFieldMask;

		for(a=0;a<m_NumberOfAnimations;a++)
		{
			if (m_pCycleResource[a] != NULL)
			{
				poseFormat.AddCycleFields(m_pCycleResource[a]->GetCycle(), poseFieldMask);
				poseFieldMask.Disable(Animation::E_PoseFieldType_JointTransform);
			}
		}
		
			
		m_AnimatedModel.Create( m_pModelResource, pModel->GetIndex(), pSceneGraph, poseFormat );
			
		for(a=0;a<m_NumberOfAnimations;a++)
			if (m_pCycleResource[a] != NULL)
				m_pCycleInstance[a] = m_AnimatedModel.CreateCycleInstance(m_pCycleResource[a]->GetCycle());
		

		m_pModelInstance = m_AnimatedModel.GetAnimatedModelInstance().GetModelInstance();
		m_pModelInstance->SetElementMatrix(-1, m_pModelElement->GetMatrix() * m_WorldMatrix );		// this sets all elements to same matrix

//		m_pModelInstance->SetElementVisibility(1, true);

		m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, Game::E_RenderPass_DynamicObjectSolid);
		m_AnimatedModel.GetAnimatedModelInstance().InsertIntoSceneGraph();

		m_pModelInstance->SetWorldSphere(m_LocalBoundingSphere * m_WorldMatrix);
		
		
		F32 farDist;
			
		// either calculate a draw distance or read it from definition
		if(objdef->m_DrawDistance == 0.0f)
			farDist	= Maths::Min( 500.0f, g_DrawDistanceFactor * m_LocalBoundingSphere.Radius() ) + m_LocalBoundingSphere.Radius();		// USING A DEBUG VAR ????????
		else
			farDist	= objdef->m_DrawDistance;
		
		m_pModelInstance->SetNearFarDistance(0.0f, farDist);
		m_pModelInstance->SetVisDetect(m_pVisDetect);

//		StartAnimation(&objdef->AnimEvent_OnCreation);
		StartAnimation(Game::E_ANIM_EVENT_TYPE_ONCREATION);
		UpdateRenderModel();
	}
	//[NEEDS DEFINE]MEMORY_TRACKER_POP("actors.dynamicobject.scenegraph");


	if (objdef->m_SwapElementsOnCollision)						// Hide the damaged element if we're going to swap the elements
	{
		m_pModelInstance->SetElementVisibility(0, true);
		m_pModelInstance->SetElementVisibility(1, false);
	}


	const GameResource::C_ModelElement* pElement = pModel->GetElement(0);
	const Gravity::C_ImportShape* pImportShape = pElement->GetPhysicsImportShape();
	const Maths::HF32 mass(objdef->m_Weight);							// Get weight from dynamic obj definition
	m_AudioMassType = objdef->m_AudioMassType;

	if (pImportShape && g_debugEnableDynamicObjectCollision)														// Create physics body ?
	{
		MEMORY_TRACKER_PUSH("actors.dynamicobject.gravity");
		// Create a physics body
		m_pBody = GetPhysicsScene()->CreateBody();

		// Setup ref to this actor in the physics body so we know whats hit what during collisions
		m_UserData.m_hActorHandle = GetCreationData()->GetActorHandle();
		m_pBody->SetUserData((void*)&m_UserData);

		// Get it's required shape
		Gravity::C_ShapeDesc* pDesc = pImportShape->GetShapeDesc();
		
		if (objdef->m_Bouncy)
			pDesc->m_PhysicsMaterialIndex = Game::E_PhysicsMaterial_ObjectBouncy;
		else
			pDesc->m_PhysicsMaterialIndex = Game::E_PhysicsMaterial_Object;

		pDesc->RemapShape();
		pDesc->SetRemapped();
		Gravity::C_BodyHelper bodyHelper( pDesc );
//		bodyHelper.CreateBody( mass, m_WorldMatrix.AW(), (Maths::HMat33)m_WorldMatrix, pDesc->InertiaTensor(mass), m_pBody, &Game::PhysicsConstants::kDynamicObjectCollisionFilter);
		Collision::C_CollisionFilter filter(Game::PhysicsConstants::kDynamicObjectCollisionFilter);
		filter.SetGroupId(Game::PhysicsConstants::E_PhysicsGroupID_None);
		filter.SetCollisionFlags(filter.GetCollisionFlags() & ~(Game::PolyFlags::kCameraSolid));
		
		bodyHelper.CreateBody( mass, m_WorldMatrix.AW(), (Maths::HMat33)(pElement->GetMatrix() * m_WorldMatrix), pDesc->InertiaTensor(mass), m_pBody, &filter);
		m_pBody->GetShape()->SetPhysicsMaterialIndex(pDesc->m_PhysicsMaterialIndex);

		// Set deactivator for putting physics to sleep
		SharedPhysics::C_Deactivator deactivator;


//		if (objdef->m_Breakaway > 0)							// This allows static objects to automatically change into dynamic objects
		//if ((objdef->m_Breakaway > 0) && (m_PhysType != Game::E_PHYSICS_BEHAVIOUR_POST))		// Ignore breakaway if this is a post (done manually in collision)
		//	deactivator.m_StaticActivateThresholdSq = objdef->m_Breakaway * 1000000.0f;			// Silly numbers because it's a sqr of the force!
		//else
		//	deactivator.m_StaticActivateThresholdSq = -1.0f;	// w m_ActivateThresholdSq

		if(m_PhysType == Game::E_PHYSICS_BEHAVIOUR_FIXED)
		{
			deactivator.m_StaticActivateThresholdSq = -1.f;
		}
		else
		{
			deactivator.m_StaticActivateThresholdSq = 0.0f;
			//deactivator.m_StaticActivateThresholdSq = ((mass * 0.01f) * (mass * 0.01f)) * (float)g_debugHackStaticActivateScale;
			deactivator.m_ActivateThresholdSq = (mass * 0.01f);//deactivator.m_StaticActivateThresholdSq;
		}
//		deactivator.m_StaticActivateThresholdSq = 9900000.0f;	// TEST TEST TEST  - NO EFFECT!
//		deactivator.m_ActivateThresholdSq = deactivator.m_StaticActivateThresholdSq;


		if (objdef->m_SleepThreshold > 0)						// For stopping jigging
			deactivator.m_SleepThresholdSq = objdef->m_SleepThreshold * objdef->m_SleepThreshold;
		else
			deactivator.m_SleepThresholdSq = (mass * 0.01f);		// This is the default value

		m_pBody->SetDeactivator(deactivator);


		//if (objdef->m_LinearDamping > 0)						// Like air resistance
		//	m_pBody->SetLinearDampingCoefficient(objdef->m_LinearDamping);
		//else
		//	m_pBody->SetLinearDampingCoefficient(0.99999f);

		//if (objdef->m_AngularDamping > 0)
		//	m_pBody->SetAngularDampingCoefficient(objdef->m_AngularDamping);
		//else
		//	m_pBody->SetAngularDampingCoefficient(0.99f);

		m_pBody->SetLinearDampingCoefficient(0.999f);

		m_pBody->SetAngularDampingCoefficient(0.99f);

//		m_pBody->SetMaxLinearVelocity(100.0f);				// If we needed to
		m_pBody->SetMaxAngularVelocity(30.0f);				// Stop problems with long thin objects


//		if (objdef->m_RangeOfMovement != 0.0f)				// Option to limit movement
//				TODO TODO TODO  Once physics constraint is in


		// Additional constants
		m_pBody->Slop( Game::PhysicsConstants::kDefaultSlop );
		m_pBody->Baumgarte( Game::PhysicsConstants::kDefaultBaumgarte );
		m_pBody->SetExternalConstants( GetPhysicsExternalConstants() );
		
									// Put object to sleep otherwise game start very slow as everything settles

		// Align it to the render model
		//m_pBody->ElementMatrix( m_WorldMatrix );
		m_pBody->ElementMatrix( pElement->GetMatrix() * m_WorldMatrix );

		// Some models need to be initialised as static to begin with
		if ((m_PhysType == Game::E_PHYSICS_BEHAVIOUR_FIXED) || (m_PhysType == Game::E_PHYSICS_BEHAVIOUR_POST))
			m_pBody->MakeStatic();
		else
			m_pBody->Deactivate();	


		// Insert into physics world
		if (m_PhysType != Game::E_PHYSICS_BEHAVIOUR_NONE)			// Might create it anyway for now in case we need to change it's properties later
		{
			GetPhysicsScene()->AddBody(m_pBody);
			
//			if(GetCreationData()->GetMovementCreator() != NULL)
			if (!m_DontMakeKinematic)
			{
				if ((GetCreationData()->GetMovementCreator() != NULL) || (m_PhysType == Game::E_PHYSICS_BEHAVIOUR_KINEMATIC))
				{
					m_kinematic = true;
					m_pBody->MakeKinematic();
					m_KinematicController = new Gravity::C_KinematicController(m_pBody);
					GetPhysicsScene()->AddKinematicController(m_KinematicController);
					m_PhysType = Game::E_PHYSICS_BEHAVIOUR_KINEMATIC;
				}
			}
		}
//		if (m_PhysType == Game::E_PHYSICS_BEHAVIOUR_POST)			// Attach them to the ground with a spring
//			AttachWithSpring(0.01f, 30000.0f,5000.0f, 500.0f,100.0f, 0.0f,0.0f);

		MEMORY_TRACKER_POP("actors.dynamicobject.gravity");

		//[NEEDS DEFINE]MEMORY_TRACKER_PUSH("actors.dynamicobject.gravity.spring");
		if (m_PhysType == Game::E_PHYSICS_BEHAVIOUR_VEHICLE)		// Attach them to the ground with 4 springs
		{
			Collision::C_CRAxisBox	bbox = m_pBody->GetShape()->m_LocalBoundingBox;

			AttachWithSpring(0.1f, mass*10,3000.0f, 0.1f,1.0f, bbox.extents.Z(),bbox.extents.X());
			AttachWithSpring(0.1f, mass*10,3000.0f, 0.1f,1.0f, bbox.extents.Z(),-bbox.extents.X());
			AttachWithSpring(0.1f, mass*10,3000.0f, 0.1f,1.0f, -bbox.extents.Z(),bbox.extents.X());
			AttachWithSpring(0.1f, mass*10,3000.0f, 0.1f,1.0f, -bbox.extents.Z(),-bbox.extents.X());
		}
		//[NEEDS DEFINE]MEMORY_TRACKER_POP("actors.dynamicobject.gravity.spring");

		CheckInitialState();
	}

	m_ShockIntensityParameter = m_pModelInstance->GetRenderModel()->GetParameter("g_vShockIntensity");

	MEMORY_TRACKER_PUSH("actors.dynamicobject.lightsampler");
	// Create light sampler
	m_pEnvSampler = new WorldRender::C_EnvironmentSampler;
	m_pEnvSampler->Initialise( GetWorldRenderScene() );
	MEMORY_TRACKER_POP("actors.dynamicobject.lightsampler");
	starttime = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();		// Remember when we were born

	if (C_GameApp::GetInstance()->GetRaceBookLayer() && C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer())
		m_pGameWorld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();
	else
		m_pGameWorld = NULL;

	UpdateLightSampler();

	//Drop shadow
	if (objdef->m_DisplayShadow)
	{
		m_ShadowIntensity = objdef->m_ShadowIntensity;
		m_DropShadowHandle = SceneGraph::AddDropShadow(GetSceneGraph());
		if (m_DropShadowHandle)
		{
			//Set parameters
			m_DropShadowHandle->SetCullDistance(60.f);
			m_DropShadowHandle->SetCullScreenSize(0.025f);
			
			m_DropShadowHandle->SetIntensity(m_ShadowIntensity);
		}
		UpdateDropShadow();
	}

	//Setup views
	if (!objdef->m_DisplayInEnvironment)
	{
		//Remove environment mask
		m_pModelInstance->DisableVisibilityFlags(Game::E_ViewTypeFlag_CubeMap);
	}
	if (!objdef->m_DisplayInReflection)
	{
		//Remove reflection mask
		m_pModelInstance->DisableVisibilityFlags(Game::E_ViewTypeFlag_Reflection);
	}
	if (!objdef->m_DisplayInMirror)
	{
		//Remove environment mask
		m_pModelInstance->DisableVisibilityFlags(Game::E_ViewTypeFlag_Mirror);
	}

//	if (num_model_elements == 2)						// Bit of a damage hack for first playable so only works for models with 2 elements
//		m_pModelInstance->SetElementVisibility(1, false);

	if(g_DynObjAddAudioTriggers && this->GetAudioMassType() > Game::E_AUDIO_MASS_TYPE_LIGHT)
	{
		// create audio layer trigger
		CreateTrigger(2,GetHandle(),m_WorldMatrix);

	}

	
	MEMORY_TRACKER_POP("actors.dynamicobject");
}

//=========================================================================


void DynamicObjectActor::DestroyModel()
{
	U32		a;
	
	if (m_pModelInstance)
	{
		m_AnimatedModel.GetAnimatedModelInstance().RemoveFromSceneGraph();
		
		//delete m_pModelInstance;
		m_pModelInstance->SetVisDetect(NULL);
		m_pModelInstance = NULL;
		m_AnimatedModel.Destroy();


		for(a=0;a<4;a++)											// Remove any springs
			if (m_pSprings[a] != NULL)
				GetPhysicsScene()->RemoveSpring(m_pSprings[a]);


		if (m_pBody != NULL)										
		{
			GetPhysicsScene()->RemoveBody(m_pBody);					// Should be deleted with scene (can't do it immediately)
			if (m_kinematic)
			{
				GetPhysicsScene()->RemoveKinematicController(m_KinematicController);
				delete m_KinematicController;
				m_KinematicController = NULL;
			}
			m_pBody = NULL;
		}
	}

	SAFE_DELETE_PTR(m_pEnvSampler);

	//Destroy Shadow
	if (m_DropShadowHandle)
	{
		SceneGraph::RemoveDropShadow(GetSceneGraph(), m_DropShadowHandle);
	}
}

//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================



bool DynamicObjectActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	//PERF_EVENT("DynamicObjectActor.OnMessage");
	switch(pMessage->Type())
	{
		case Game::E_GameflowMessage_RaceLoaded:							// Animations started here so everything is syncronized
			m_DelayedStartTime = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->m_RaceLoadTime + GetCreationData()->m_AnimStartTimer;		// So it's exact
			AddSubscriber( GameMessaging::E_GameMsg_Update );

/*
			if (GetCreationData()->m_AnimStartTimer != 0.0f)						// Start timer for delayed animation event
			{
//				m_DelayedStartTime = C_GameApp::GetInstance()->GetTimer().GetElapsedTime() + GetCreationData()->m_AnimStartTimer;
				m_DelayedStartTime = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->m_RaceLoadTime + GetCreationData()->m_AnimStartTimer;		// So it's exact
				AddSubscriber( GameMessaging::E_GameMsg_Update );
			}
			else
			{
				m_DelayedStartTime = 0.0f;
			}
*/
			break;

		case GameMessaging::E_GameMsg_Update:
			Update();
			break;

		case ActorMsg::E_GameActorMsg_Disable:
			KillThis();
			break;

		case Game::E_GameMsg_UpdateLifeTime:
			UpdateLifeTime();
			break;


		case ActorMsg::E_GameActorMsg_SetTransform:
			{
				const ActorMsg::C_ActorSetTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage);
				m_WorldMatrix = msg->Transform().Matrix();
				UpdateWorldPosition(true);
/*
				if (m_pModelInstance)
					UpdateRenderModel();

				if (m_pBody)
				{
					if (m_kinematic)
						m_KinematicController->Teleport(m_WorldMatrix);
					else
						m_pBody->Position(m_WorldMatrix.Translation());
				}
*/
			}
			break;

		case ActorMsg::E_GameActorMsg_AnimateTransform:
			{

				const ActorMsg::C_ActorAnimateTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage);
				m_WorldMatrix = msg->Transform().Matrix();
				UpdateWorldPosition(true);
/*
				if (m_pModelInstance)
					UpdateRenderModel();

				if (m_pBody)
				{
					if (m_kinematic)
						m_KinematicController->SetTargetTransformation(m_WorldMatrix, msg->m_TimeStep);
					else
						m_pBody->Position(m_WorldMatrix.Translation());
				}
*/
			}
			break;

		case Actor::E_ActorMsg_PhysicsBodyEvent:
			{
				const C_PhysicsBodyEventMsg* msg = Messaging::MessageCast<C_PhysicsBodyEventMsg>(pMessage);
				switch( msg->m_BodyEvent.GetEventType() )
				{
					case Gravity::C_BodyEvent::E_WakeUp:
						{
							AddSubscriber( Game::E_GameMsg_ActivePhysicsBodyUpdate );	
							//if(pTriggerLayer && m_pTriggerObject)
							//{
							//	pTriggerLayer->AddTrigger(m_pTriggerObject);	
							//}
							/*U8 data[1024 * sizeof(void*)];
							SharedPhysics::C_SafeStack<true> stack(data, 1024);
							m_pTriggerObject->SetMatrix(mWorldMatrix, stack);*/

	//						if (lifespan == 0.0f)
	//							SetLifespan(objdef->m_LifespanAfterDamage);			// In case a lampost has been activated because something nearby has moved
							break;
						}

					case Gravity::C_BodyEvent::E_GoToSleep:
						RemoveSubscriber( Game::E_GameMsg_ActivePhysicsBodyUpdate );						
//						m_pModelInstance->SetSubtechniqueHandle(0);				// Will eventually put this on a timer although that might cause things to switch of at the same time which might look odd
						//if(m_pTriggerObject && pTriggerLayer)
						//{
						//	pTriggerLayer->RemoveTrigger(m_pTriggerObject);
						//}
						m_TimeInCollision = 0.0f;
						m_FramesInCollision = 0;
						break;
				}
			}
			break;

		case Game::E_GameMsg_ActivePhysicsBodyUpdate:
			ActivePhysicsBodyUpdate();
			break;

		case ActorMsg::E_DynamicObjMsg_AnimationUpdate:
			UpdateRenderModel();
			break;

		case ActorMsg::E_DynamicObjMsg_RenderDebug:
			RenderDebugInfo();
			break;

		case ActorMsg::E_DynamicObjMsg_SorceryUpdate:
			ReloadSorceryData();
			break;

		case ActorMsg::E_DynamicObjMsg_StartDynamicObjectAnimation:
		{
			const ActorMsg::C_StartDynamicObjectAnimationMsg* msg = Messaging::MessageCast<ActorMsg::C_StartDynamicObjectAnimationMsg>(pMessage);
			StartAnimation(msg->m_AnimationEvent);
			break;
		}

		case ActorMsg::E_DynamicObjMsg_DynamicObjectApplyImpulse:
		{
			const ActorMsg::C_DynamicObjectApplyImpulseMsg* msg = Messaging::MessageCast<ActorMsg::C_DynamicObjectApplyImpulseMsg>(pMessage);
			if(m_pBody)
			{
				m_pBody->ApplyImpulse(Maths::Min(msg->m_Force, msg->m_Force*m_pBody->Mass()), m_WorldMatrix.Translation());
				if (msg->m_ElectricalEffect)
				{
					m_pModelInstance->SetSubtechniqueHandle(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetElectricSubTechniqueHandle());
					const Display::C_Float4 shockIntensity(msg->m_ElectricalEffectColour.r, msg->m_ElectricalEffectColour.g, msg->m_ElectricalEffectColour.b, C_GameApp::GetInstance()->GetElapsedRaceTime());
					m_pModelInstance->SetParameter(m_ShockIntensityParameter, (F32*)&shockIntensity);
				}
			}
			break;
		}

		case ActorMsg::E_DynamicObjMsg_DynamicObjectDisableElectricalEffect:
		{
			m_pModelInstance->SetSubtechniqueHandle(0);				// Will eventually put this on a timer although that might cause things to switch of at the same time which might look odd
			break;
		}

		case ::Actors::E_VisibilityMsg_OnHidden:
			if (anim_active)
				RemoveSubscriber(ActorMsg::E_DynamicObjMsg_AnimationUpdate);
			m_bVisible = false;
			break;
		case ::Actors::E_VisibilityMsg_OnVisible:
			if (anim_active)
				AddSubscriber(ActorMsg::E_DynamicObjMsg_AnimationUpdate);
			m_bVisible = true;
			break;

		case ActorMsg::E_DynamicObjMsg_DynamicObjectSetPhysicsType:
		{
			const ActorMsg::C_DynamicObjectSetPhysicsTypeMsg* msg = Messaging::MessageCast<ActorMsg::C_DynamicObjectSetPhysicsTypeMsg>(pMessage);
			if (m_pBody)
			{
				if ((m_PhysType == Game::E_PHYSICS_BEHAVIOUR_NONE) && (msg->m_PhysType != Game::E_PHYSICS_BEHAVIOUR_NONE))
					GetPhysicsScene()->AddBody(m_pBody);
				if ((m_PhysType != Game::E_PHYSICS_BEHAVIOUR_NONE) && (msg->m_PhysType == Game::E_PHYSICS_BEHAVIOUR_NONE))
					GetPhysicsScene()->RemoveBody(m_pBody);

				switch(msg->m_PhysType)
				{
					case Game::E_PHYSICS_BEHAVIOUR_NONE:
						break;

					case Game::E_PHYSICS_BEHAVIOUR_FIXED:
						m_pBody->MakeStatic();
						break;

					case Game::E_PHYSICS_BEHAVIOUR_DYNAMIC:
						m_pBody->Activate();
						break;

					case Game::E_PHYSICS_BEHAVIOUR_KINEMATIC:
						m_pBody->MakeKinematic();
						break;
	/*																// NOT USED AT THE MOMENT
					case Game::E_PHYSICS_BEHAVIOUR_POST:
						break;

					case Game::E_PHYSICS_BEHAVIOUR_VEHICLE:
						break;
	*/
				}

				m_PhysType = msg->m_PhysType;
			}
			else
			{
//				if (msg->m_PhysType == Game::E_PHYSICS_BEHAVIOUR_DYNAMIC)			// For trailers etc that use movement components but aren't kniematic
//					m_DontMakeKinematic = true;
//					TODO -CHECK THIS IS WORKING
			}
			break;
		}

// --------- Actor Studio - Drag and Drop ----------

		case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		{
			const ActorStudioMsgGame::C_DragMsg* msg = Messaging::MessageCast<ActorStudioMsgGame::C_DragMsg>(pMessage);
			m_WorldMatrix = msg->GetValue();
			UpdateWorldPosition(false);
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_EndDrag:
		{
			UpdateWorldPosition(true);
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
		{
			m_Selected = true;
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
		{
			m_Selected = false;
		}
		break;

		case Game::E_GameMsg_KillObjectsForCutScene:
		{
			const Game::C_KillForOutroMsg* pMsg = Messaging::MessageCast<Game::C_KillForOutroMsg>(pMessage);
			float fDist2 = (m_WorldMatrix.AW() - pMsg->m_Position).Length2();
			if (fDist2 >= pMsg->m_DistanceSquared)
				KillThis();
		}
		break;


		default:
			return C_GameActor::OnMessage(pMessage);
//			return(true);
	}

	return(true);
}

//=========================================================================


void DynamicObjectActor::ActivePhysicsBodyUpdate()
{
	PERF_EVENT("DynamicObjectActor.ActivePhysicsBodyUpdate");

	if (m_PhysType != Game::E_PHYSICS_BEHAVIOUR_NONE)
	{
		if (m_pBody)
		{
			if (m_pBody->IsActive())
			{
				m_WorldMatrix = m_pBody->ElementMatrix();
				// Update render model world matrix, but don't update animation - let the animation update message do that.
				UpdateRenderModel(false, false);

				F32	ny = m_WorldMatrix.Translation().Y();
				if (ny < minheight)
				{
					LOGWARN(g_Log, "Killing dynamic object %s because it's fallen off the world\n", GetActorName());
					KillThis();
				}

				if(g_DynObjAddAudioTriggers)
				{
					//update the triggers
					if(m_pTriggerObject && pTriggerLayer)
					{
						U8 data[1024 * sizeof(void*)];
						SharedPhysics::C_SafeStack<true> stack(data, 1024);
						//pTriggerLayer->SetTriggerMatrix(m_pTriggerObject, m_WorldMatrix, stack);

						m_pTriggerObject->SetMatrix(m_WorldMatrix, stack);
					}
 				}
			}
		}
	}

	//m_FramesInCollision--;
	//m_FramesInCollision = Maths::Max(m_FramesInCollision, 0);
	//if (m_FramesInCollision >= 1)
	//{
	//	m_TimeInCollision += C_GameApp::GetInstance()->GetTimeStep();
	//	if (m_TimeInCollision > (float)g_debugPhysObjSnagTimeBeforeDeath &&
	//		m_pBody && m_pBody->LinearVelocity().Length2() >= (float)g_debugPhysObjSnagSpeedMinBeforeDeath)
	//	{
	//		if (m_pBody->Position().Y() > (m_ColliderPosition.Y() + (float)g_debugPhysObjSnagUpForceYDiff))
	//		{
	//			m_pBody->ApplyImpulse(Maths::HVec3(0.0f, (float)g_debugPhysObjSnagUpForce * m_pBody->Mass(), 0.0f), m_pBody->Position());
	//			m_FramesInCollision = 0;
	//			m_TimeInCollision = 0.0f;
	//		}
	//		else
	//		{
	//			//if (GetCreationData()->m_DynamicObjectDef->m_DebrisType != 0)
	//			//	SpawnDebris();
	//			KillThis();
	//		}
	//	}
	//}
	//else
	//{
	//	m_TimeInCollision -= C_GameApp::GetInstance()->GetTimeStep();
	//	m_TimeInCollision = Maths::Max(m_TimeInCollision, 0.0f);
	//}
}


//=========================================================================

void DynamicObjectActor::UpdateLightSampler()
{
	if (m_pEnvSampler)
	{
		m_pEnvSampler->SetPosition( m_LocalBoundingSphere.Centre() * m_WorldMatrix );
		m_pEnvSampler->Update();
		m_pEnvSampler->ApplyToModelInstance(m_pModelInstance);
	}
}

//=========================================================================

void DynamicObjectActor::UpdateDropShadow()
{
	if (m_DropShadowHandle)
	{

		m_ShadowPosition = m_LocalBoundingSphere.Centre() * m_WorldMatrix;
		m_ShadowRadius = m_LocalBoundingSphere.Radius();

		m_DropShadowHandle->SetDimensions(m_ShadowPosition, m_ShadowRadius);
	}
}

//=========================================================================

void DynamicObjectActor::Update()					// Update logic called each frame
{
	bool	unsubscribe = true;


	if (!objdef)								// Don't update if we haven't fully initialised
		return;


	PERF_EVENT("DynamicObjectActor.Update");

	if (lifespan != 0.0f)
	{
		if (C_GameApp::GetInstance()->GetElapsedRaceTime() > lifespan)
			KillThis();
		else
			unsubscribe = false;
	}

	if (m_DelayedStartTime != 0.0f)
	{
		if (C_GameApp::GetInstance()->GetTimer().GetElapsedTime() > m_DelayedStartTime)
		{
			StartAnimation(Game::E_ANIM_EVENT_TYPE_ONDELAYEDSTART);
			m_AnimationTime = 0.0f;
			m_DelayedStartTime = 0.0f;
		}
		else
		{
			unsubscribe = false;
		}
	}	

if(g_DynObjAddAudioTriggers)
{
	//update the audio trigger 
	if(m_pTriggerObject && pTriggerLayer)
	{
		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);
		//pTriggerLayer->SetTriggerMatrix(m_pTriggerObject, m_WorldMatrix, stack);

		m_pTriggerObject->SetMatrix(m_WorldMatrix, stack);
	}
}

	if (unsubscribe)
		RemoveSubscriber( GameMessaging::E_GameMsg_Update );
}

void DynamicObjectActor::UpdateLifeTime()
{
	if (!m_bVisible)
	{
		if (m_bVisibleLastFrame)
			m_RespawnTime = objdef->m_RespawnTimer;
		else
		{
			m_RespawnTime -= C_GameApp::GetInstance()->GetTimeStep();
			if (m_RespawnTime <= 0.0f)
				Respawn();
		}
	}	

	m_bVisibleLastFrame = m_bVisible;
}

void DynamicObjectActor::Respawn()
{	
	m_HasBeenHit = false;
	m_WorldMatrix = GetCreationData()->GetCreationMatrix();
	if (!GetCreationData()->m_IgnoreAlignment)
		C_ActorUtils::Instance().AlignToScene(&m_WorldMatrix, objdef->m_Alignment, 3.0f, GetPhysicsScene());
	UpdateWorldPosition(true, true);
	m_pBody->Deactivate();	
	m_pBody->MakeStatic();
	RemoveSubscriber(Game::E_GameMsg_UpdateLifeTime);
	if (objdef->m_SwapElementsOnCollision)						// Hide the damaged element if we're going to swap the elements
	{
		m_pModelInstance->SetElementVisibility(0, true);
		m_pModelInstance->SetElementVisibility(1, false);
	}
}

//=========================================================================

void DynamicObjectActor::UpdateRenderModel(bool visibleOverride, bool updateAnimation)
{
	PERF_EVENT("DynamicObjectActor.UpdateRenderModel");

	bool isVisible = m_pVisDetect->IsVisible() != 0 || visibleOverride;

	if (isVisible)
	{
		UpdateLightSampler();
		UpdateDropShadow();
	}
	
	bool isPlayingAnimation = (anim_active && (m_pCycleResource[anim_number] != NULL));
	
	if (isPlayingAnimation)
	{
//		m_AnimationTime += C_GameApp::GetInstance()->GetTimeStep();

		if (!m_Freeze)
		{
			F32 fCur = C_GameApp::GetInstance()->GetElapsedRaceTime();
			m_AnimationTime += fCur - m_fLastAnimTime;
			m_fLastAnimTime = fCur;
//			m_AnimationTime += C_GameApp::GetInstance()->GetTimeStep();
		}
	}
	
	if (m_AnimatedModel.IsCreated())
	{
		if (isVisible && isPlayingAnimation && updateAnimation)
		{
			Animation::C_CommandList& cmdList = m_AnimatedModel.GetCommandList();
			cmdList.Clear();
			cmdList.BlendCycle(Animation::E_BlendMode_Accumulate, m_pCycleInstance[anim_number], m_AnimationTime, 1.f);
			m_AnimatedModel.UpdateAsync(AnimatedModel::E_Sync_PreRender, m_WorldMatrix);
		}
		else
		{
			m_AnimatedModel.SetWorldMatrix(m_WorldMatrix);
		}
	}
}

//=========================================================================

void DynamicObjectActor::UpdateWorldPosition(bool movephysics, bool visibleOverride)
{
	if (m_pModelInstance)
		UpdateRenderModel(visibleOverride, false);

	if (movephysics)
	{
		if (m_pBody)
		{
			if (m_kinematic)
			{
				//m_KinematicController->Teleport(m_WorldMatrix);
				m_KinematicController->SetTargetTransformation(m_WorldMatrix, C_GameApp::GetInstance()->GetTimeStep());
			}
			else
			{
				//m_pBody->Position(m_WorldMatrix.Translation());
				m_pBody->ElementMatrix(m_WorldMatrix);
			}
		}
	}
}

//=========================================================================

void DynamicObjectActor::StartAnimation(Game::E_ANIM_EVENT_TYPE eventnum)
{
	S32		a;

	if (objdef->m_SwapElementsOnCollision && (eventnum == Game::E_ANIM_EVENT_TYPE_ONCOLLISION))
	{
		m_pModelInstance->SetElementVisibility(0, false);
		m_pModelInstance->SetElementVisibility(1, true);
		return;
	}

	for(a=0;a<objdef->m_AnimEvents.count;a++)			// Would just drop out if no animation events
	{
		if (objdef->m_AnimEvents[a].m_EventType == eventnum)		// Find this event
		{
			if (objdef->m_AnimEvents[a].m_StopAnimation)
			{
				m_Freeze = true;
			}
			else if (m_ValidAnimation[a])
			{
				if ((!anim_active) || (anim_number != a))
					m_AnimationTime = 0.0f;
				anim_active = true;
				anim_number = a;
		
				m_fLastAnimTime = C_GameApp::GetInstance()->GetElapsedRaceTime();
				if (m_bVisible)
					AddSubscriber(ActorMsg::E_DynamicObjMsg_AnimationUpdate);
//				AddSubscriber(ActorMsg::E_DynamicObjMsg_AnimationUpdate);
				m_Freeze = false;
			}
			else
			{
				anim_active = false;
				RemoveSubscriber(ActorMsg::E_DynamicObjMsg_AnimationUpdate);
				m_Freeze = true;
			}
			return;
		}
	}
}


//=========================================================================
//						Miscellaneous
//=========================================================================


//DynamicObjectActor *DynamicObjectActor::SpawnDebris()
//{
//	DynamicObjectActor		*spawnactor;
//
//	Maths::HMat43 mat = m_WorldMatrix;
//
////	mat.Translation(m_WorldMatrix.Translation() + (m_WorldMatrix.AY() * 2.0f) + (m_WorldMatrix.AX() * ((F32)(rand()&7)-3.0f)) + (m_WorldMatrix.AZ() * ((F32)(rand()&7)-3.0f)));			// Just offset it for now
//	Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
//	//mat.Translation(m_WorldMatrix.Translation() + (m_WorldMatrix.AY() * 0.5f));
//	mat.Translation(m_WorldMatrix.Translation() + (m_WorldMatrix.AY() * half));
//
////	spawnactor = SpawnDynamicObject(&mat, GetCreationData()->m_DynamicObjectDef->m_DebrisType);
//	spawnactor = C_ActorUtils::Instance().SpawnDynamicObject(&mat, GetCreationData()->m_DynamicObjectDef->m_DebrisType);
//	spawnactor->SetLifespan(5.0f + (((F32)(rand()&31))/3.0f) );
//	Maths::HVec3	linspeed((F32)(rand()&7)-3.0f, (F32)(rand()&7) + 3.0f, (F32)(rand()&7)-3.0f);
//	Maths::HVec3	angspeed((F32)(rand()&3)-2.0f, (F32)(rand()&3)-2.0f, (F32)(rand()&3)-2.0f);
//	spawnactor->SetInitialSpeeds(&linspeed, &angspeed, false);
//	return(spawnactor);
//}


//=========================================================================

void DynamicObjectActor::SetLifespan(F32 duration)				// Sets how long before this object will commit suicide
{
	if (duration != 0.0f)
	{ 
		lifespan = C_GameApp::GetInstance()->GetElapsedRaceTime() + duration;
//		if (lifespan != 0.0f)
//		{
			AddSubscriber( GameMessaging::E_GameMsg_Update );
//		}
	}
}

//=========================================================================

void DynamicObjectActor::KillThis()					// Cleans up any children etc before killing itself
{
	::Actors::C_ActorUtils::Instance().SanityCheck_Remove(this);

	// Causes problems if actor already killed off
//	GetCreationData()->m_OnDestroyed.Send(this);	// Notify other actors that this object has been destroyed
	Kill();

	//Destroy Shadow
	if (m_DropShadowHandle)
	{
		SceneGraph::RemoveDropShadow(GetSceneGraph(), m_DropShadowHandle);
	}
}


//=========================================================================


F32 DynamicObjectActor::RegisterCollision(Actor::C_Actor *collactor, Vehicle::CollisionInfo *collinfo)		// Gets called if crates are stacked !!!
{
//	PERF_EVENT("DynamicObjectActor::RegisterCollision");
	
	using namespace Maths;
	//if (!collision)
	//	return(-1.0f);								// Ignore this collision


	//If is a collision with a vehicle
	//if ((collactor->GetActorTypeId() == Game::E_ActorTypeId_Vehicle) && (m_WorldMatrix.AW().Y() < collactor->GetMatrix().AW().Y()) && (	m_PhysType != Game::E_PHYSICS_BEHAVIOUR_KINEMATIC))
	//{
	//	//If the vehicle is on top of the object, we must kill it
	//	/*Maths::HVec3 vUp(0.0f, 1.0f, 0.0f);
	//	float fDot = collinfo->m_ContactNormal | vUp;
	//	float fHeight = m_WorldMatrix.AW().Y() - collactor->GetMatrix().AW().Y();
	//	m_ColliderPosition = collactor->GetMatrix().AW();
	//	if (Maths::Abs(fDot) >= (float)g_debugVehDynKillDot && Maths::Abs(fHeight) >= (float)g_debugVehDynKillOffset)
	//	{
	//		LOGDEBUG(g_DynObjLog, "Dynamic object detected below vehicle - killing off: fDot(%.3f), fHeight(%.3f)", fDot, fHeight);
	//		if (GetCreationData()->m_DynamicObjectDef->m_DebrisType != 0)
	//			SpawnDebris();

	//		KillThis();
	//		return(-1.0f);
	//	}*/

	//	const HF32 vehDynKillDot((float)g_debugVehDynKillDot);
	//	const HF32 vehDynKillOffset((float)g_debugVehDynKillOffset);
	//	const HVec3 vUp(Constants<HVec3>::UnitAxisPositiveY());
	//	const HF32 fDot(collinfo->m_ContactNormal | vUp);
	//	const HF32 fHeight( m_WorldMatrix.AW().Y() - collactor->GetMatrix().AW().Y());
	//	m_ColliderPosition = collactor->GetMatrix().AW();
	//	if (Abs(fDot).GE(vehDynKillDot) && Abs(fHeight).GE(vehDynKillOffset))
	//	{
	//		LOGDEBUG(g_DynObjLog, "Dynamic object detected below vehicle - killing off: fDot(%.3f), fHeight(%.3f)", (float)fDot, (float)fHeight);
	//		//if (GetCreationData()->m_DynamicObjectDef->m_DebrisType != 0)
	//		//	SpawnDebris();

	//		KillThis();
	//		return(-1.0f);
	//	} 		
	//}

	//KierS - got rid of to avoid objects shooting upwards!
//	if (collactor && collactor->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)
//	{		
//		m_FramesInCollision += 2;
//
///*		// NO LONGER NEEDED - CHADS FIXED THINGS :)
//		// Check for objects getting stuck inside vehicles
//		F32		elapsedtime = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
//		F32		difftime = elapsedtime - m_last_collision_time;
//		if (difftime > 0.1f)
//		{
//			m_total_collision_time	= 0.0f;
//		}
//		else
//		{
//			m_last_collision_time = elapsedtime;
//			m_total_collision_time += difftime;
//			if (m_total_collision_time > 2.0f)
//			{
//				KillThis();
//				return(-1.0f);
//			}
//		}
//*/
//	}

	//[cwright - bug: 22765 - collision call-back optimisation broke re-spawning and flashing cones. Moved this from ProcessCollision()]
	{
		if (!m_HasBeenHit && objdef->m_RespawnTimer > 0.0f)//GetCreationData()->m_EnableRespawn)
		{
			m_HasBeenHit = true;
			m_RespawnTime = objdef->m_RespawnTimer;
			AddSubscriber(Game::E_GameMsg_UpdateLifeTime);
			SetLifespan(objdef->m_LifespanAfterDamage);						// Option to kill off after a given number of seconds - Will probably remove when damage system is working correctly
		}

		StartAnimation(Game::E_ANIM_EVENT_TYPE_ONCOLLISION);			// This object may need to start a new animation
	}

	return(objdef->m_VehicleDamage);
}

//=========================================================================

F32 DynamicObjectActor::ProcessCollision(Actor::C_Actor *collactor, Vehicle::CollisionInfo *collinfo)		// Gets called if crates are stacked !!!
{
//	F32		old_damage = simpledamage;
	
	if (!collision)
		return(-1.0f);								// Ignore this collision

	//[cwright - bug: 22765 - collision call-back optimisation broke re-spawning and flashing cones. Moved this to RegisterCollision()]
	//{
		//if (!m_HasBeenHit && objdef->m_RespawnTimer > 0.0f)//GetCreationData()->m_EnableRespawn)
		//{
		//	m_HasBeenHit = true;
		//	m_RespawnTime = objdef->m_RespawnTimer;
		//	AddSubscriber(Game::E_GameMsg_UpdateLifeTime);
		//}

		//SetLifespan(objdef->m_LifespanAfterDamage);						// Option to kill off after a given number of seconds - Will probably remove when damage system is working correctly
		//StartAnimation(Game::E_ANIM_EVENT_TYPE_ONCOLLISION);			// This object may need to start a new animation
	//}
	
	if (collinfo->m_GameplayImpulse	> 0.1f)
	{
		const Actors::DynamicObjectCreationData* creationData = GetCreationData();
		if (collactor->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)			// If hit by a car
		{
			creationData->m_OnCollision.Send(this);					// Notify other actors that this object has been hit

			Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor *)collactor;							
			class RacingPhysics::Car *pCar = pActor->GetVehiclePhysics();
			class RacingPhysics::Vehicle *pVeh =  static_cast<RacingPhysics::Vehicle*>(pCar);
			if (pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER) 
				creationData->m_OnPlayerCollision.Send(this);					// Notify other actors that this object has been hit by the player

			if(creationData->m_VehicleCollisionMessage)
				SendMessageToActor(pActor->GetHandle(), creationData->m_VehicleCollisionMessage->GetMessage(this));		// Send any specified messages to the vehicle thats triggered the trigger :)
		}

//		num_hits++;

		//if (creationData->m_DynamicObjectDef->m_DebrisType != 0)
		//	SpawnDebris();

//		simpledamage -= collinfo->m_GameplayImpulse * 50.0f;			// w 50.0f	Temp damage stuff - Also stops cars getting stuck
//		if ((simpledamage < 50.0f) && (old_damage >= 50.0f))			// Temp swap of models for now
//			CheckSwapRenderModel();

/*
		if (m_PhysType == Game::E_PHYSICS_BEHAVIOUR_POST)
		{
			if ((collinfo->m_GameplayImpulse > 0.3f) || (num_hits >= 3))			// Is this collision big enough to break the pole?
			{
				m_pBody->Activate();
				return(-1.0f);								// Ignore this collision (will still need to damage the car)
			}
			else
			{												// Bend the pole over
//				if (collactor->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)			// If hit by a car
//				{
					Maths::HMat43	bentmat(m_WorldMatrix);
					Maths::HVec3	dir(m_WorldMatrix.Translation() - collactor->GetMatrix().Translation());
					dir.Normalise();
					dir = (dir * collinfo->m_GameplayImpulse) + bentmat.AY();
					dir.Normalise();
					bentmat.AY(dir);

					C_ActorUtils::Instance().NormaliseMat(&bentmat);
					m_pBody->ElementMatrix(bentmat);

					m_WorldMatrix = bentmat;
					UpdateRenderModel();

//				}
			}
		}
*/
	}
	return(objdef->m_VehicleDamage);
}


//=========================================================================


void DynamicObjectActor::AttachWithSpring(F32 rest_len, F32 lin_coeff, F32 lin_damp, F32 ang_coeff, F32 ang_damp, F32 zoff, F32 xoff)
{
	S32		a;

	Gravity::C_Body* scenebody = GetPhysicsScene()->GetStaticWorldBody();

	SharedPhysics::C_Spring* spring = new SharedPhysics::C_Spring;

	spring->m_RestLength = rest_len;
	spring->SetLinearCoefficient(lin_coeff, lin_damp);
	spring->SetAngularCoefficient(ang_coeff, ang_damp);
	//spring->SetAngleAndLimit(0.001f, 0.001f);

	spring->m_pRigidA = &m_pBody->GetBody();
	spring->m_pNodeA = m_pBody->GetGraphNode();
	
	spring->m_LocalContactA = -m_pBody->ElementOffset() + Maths::HVec3( xoff, 0.0f, zoff );


	spring->m_pRigidB = &scenebody->GetBody();
	spring->m_pNodeB = scenebody->GetGraphNode();
//	spring->m_LocalContactB = m_WorldMatrix.Translation() + Maths::HVec3( xoff, 0.0f, zoff );		// World position to connect to
	spring->m_LocalContactB = m_WorldMatrix.Translation() + (m_WorldMatrix.AX() * xoff) + (m_WorldMatrix.AZ() * zoff);		// World position to connect to

	GetPhysicsScene()->AddSpring(spring);

	for(a=0;a<4;a++)							// Store it for later so it can be cleaned up
		if (m_pSprings[a] == NULL)
		{
			m_pSprings[a] = spring;
			return;
		}
	LOGERROR(Game::g_Log, DBSTR "Dynamic Actor uses more than 4 springs");
}


//=========================================================================

void DynamicObjectActor::SetInitialSpeeds(Maths::HVec3 *linspeed, Maths::HVec3 *angspeed, bool collisiondetection)
{
	initial_state = true;
	initial_lin_velocity = *linspeed;
	initial_ang_velocity = *angspeed;
	collision = collisiondetection;				// Don't collide this object (e.g. debris)
	CheckInitialState();
}

//=========================================================================


void DynamicObjectActor::CheckInitialState()
{
	if (initial_state)
	{
		if (m_pBody && (m_PhysType != Game::E_PHYSICS_BEHAVIOUR_FIXED))
		{
//			m_pBody->LinearVelocity(initial_lin_velocity);
//			m_pBody->AngularVelocity(initial_ang_velocity);
			m_pBody->LinearVelocity(Maths::HVec3((F32)(rand()&7)-3.0f, (F32)(rand()&7), (F32)(rand()&7)-3.0f));
			m_pBody->AngularVelocity(Maths::HVec3(2.0f, 1.0f, 0.5f));
			m_pBody->Activate();
			AddSubscriber( Game::E_GameMsg_ActivePhysicsBodyUpdate );	// In case it's not in the scene yet
			initial_state = false;
		}
	}
}

//=========================================================================

void DynamicObjectActor::RenderDebugInfo()
{
	F32		height = 0.25f;

	if (possible_error)
		height = 0.75f;

	

	const DynamicObjectCreationData* creationData = GetCreationData();
	if(creationData)
	{
		Util::C_String text;

		if(creationData->m_DynamicObjectDef) //ensure DynamicObjectDef defined in sorcery
		{
			text.Printf("%s : %s", creationData->GetName(), creationData->m_DynamicObjectDef->m_ModelType.Object()->m_Filename.Text());
		}
		else
		{
			text.Printf("%s", creationData->GetName());
		}

		if (m_Selected)
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(Maths::HVec3(m_WorldMatrix.Translation()), "Selected");
		else
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(m_WorldMatrix.Translation(), height, text);
	}
}

//=========================================================================

void DynamicObjectActor::ReloadSorceryData()			// Check for weight etc changing "on-the-fly"
{
}


//=========================================================================

void DynamicObjectActor::CreateTrigger(float p_Radius,const Actor::C_ActorHandle& actorHandle,Maths::HMat43	mWorldMatrix )
{
	if(!g_DynObjAddAudioTriggers)
		return;

	if (m_pTriggerObject)
		DestroyTrigger(actorHandle);

	if (/*!m_pRLC || */!m_pGameWorld)
		return;

	//->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects);

	//m_pGameWorld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetGameWorld();

	/*Gravity::C_TriggerLayer**/ pTriggerLayer = m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects);
	if (!pTriggerLayer)
		return;

	m_pTriggerObject	= new Gravity::C_Trigger;

	if (!m_pTriggerObject)
		return;

	Actor::C_ActorHandle  handle = GetHandle();

	m_pTriggerObject->SetUserData( SharedActorUtils::HandleToUserData( handle ) );

	Collision::C_CollisionFilter filter;

	Gravity::C_SphereDesc desc;
	desc.m_Radius = p_Radius;
	m_pTriggerObject->m_pTriggerShape = desc.CreateShape(&filter);
	
	pTriggerLayer->AddTrigger(m_pTriggerObject);	

	//SharedPhysics::C_SafeStack<true> stack(1024);
	////pTriggerLayer->SetTriggerMatrix(m_pTriggerObject, mWorldMatrix, stack);
	//m_pTriggerObject->SetMatrix(mWorldMatrix, stack);
}

void DynamicObjectActor::DestroyTrigger(const Actor::C_ActorHandle& actorHandle)
{

	if(!g_DynObjAddAudioTriggers)
	 return;

	if (!m_pTriggerObject || !m_pGameWorld)
		return;
							
	//Gravity::C_TriggerLayer* pTriggerLayer = gameworld->GetTriggerLayer(Game::E_TriggerLayer_Audio_DynamicObjects);
	if (!pTriggerLayer)
		return;

	//Actor::C_ActorHandle defaultHandle;
	m_pTriggerObject->SetUserData( SharedActorUtils::HandleToUserData( GetHandle() ) );
	
	pTriggerLayer->RemoveTrigger(m_pTriggerObject);

	SAFE_DELETE_PTR(m_pTriggerObject);
}

}// End namespace Actors





//=========================================================================
//						Might be useful one day
//=========================================================================


//		LOGINFO( Resource::g_Log, "THIS IS A DEBUG MESSAGE\n");
//		LOGINFO( Resource::g_Log, "Time = %f\n", C_GameApp::GetInstance()->GetTimer().GetElapsedTime());




//#pragma optimize("", on)
