#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include "PhysicalObjectActor.h"
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <shared_actors/Components/Interfaces/I_ModelResource.h>
#include <actor_system/ActorHandleListItem.h>
#include <shared_actors/Components/Messages/RenderMessages.h>
#include "shared_actors/Components/Creation/BaseElementManagerComponentsCreate.h"
#include "shared_actors/Components/Messages/ElementManagerMessages.h"
#include "shared_Actors/Components/Creation/PhysicsBodyComponentCreate.h"
#include "shared_Actors/Components/Creation/ModelResourceComponentCreate.h"
#include <shared_actors/Utility/VisibilityDetection.h>
#include <gravity/narrowphase/Primitive.h>
#include <shared_actors/Components/Impl/RenderComponents.h>
#include <misc_shared/MathsDebug/MathsDebug.h>
#include <shared_actors\Components\Impl\BaseElementManagerComponents.h>
#include <gravity\importer\Importer.h>
#include "Game/GameActors/Components/Creation/AudioComponentsCreate.h"
#include <shared_actors/Components/Creation/AnimatedObjectComponentsCreate.h>
#include <world_render/ModelInstance.h>
#include "GameAudio/GameAudioLayer.h"

#include "system/automation/Automation.h"

#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>
#include <Dynamic/Common.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>
#include <Game\GameDef\Materials.h>
#include <Game/GameDef/PhysicsConstants.h>
#include <Game/Messaging/GameMessages.h>
#include <Game/GameWorld/ViewType.h>
#include <Game/GameWorld/RenderPass.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <GameApp.h>
//#include <GameLogic/Layers/RaceBookLayer.h>
//#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <Gravity/Triggers/TriggerSystem.h>
#include <gravity/triggers/TriggerEvent.h>

#include <shared_actors/SharedActorUtils.h>

using namespace Util;
using namespace Maths;
using namespace Game;

Debug::Float g_debugPhysObjSnagTimeBeforeDeath		("ActorDebug.PhysicalObjectActor.SnagTimeBeforeDeath", 1.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugPhysObjSnagSpeedMinBeforeDeath	("ActorDebug.PhysicalObjectActor.SnagSpeedMinBeforeDeath", 25.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(0.001f));
Debug::Float g_debugPhysObjSnagUpForceYDiff			("ActorDebug.PhysicalObjectActor.SnagUpForceYDiff", -1.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(0.001f));
Debug::Float g_debugPhysObjSnagUpForce				("ActorDebug.PhysicalObjectActor.SnagSnagUpForce", 5.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(0.001f));
Debug::Float gPhysObjChildMaxRandomSpinScale		("ActorDebug.PhysicalObjectActor.ChildMaxRandomSpinScale", 1.0f);
Debug::Float gPhysObjChildRandomVelocityScale		("ActorDebug.PhysicalObjectActor.ChildRandomVelocityScale", 0.25f);
Debug::Float g_debugPhysObjSnagUpForceThreshold		("ActorDebug.PhysicalObjectActor.SnagSnagUpForce", 200.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(0.001f));
Debug::Float g_debugPhyObjsParticleVelocityScale	("ActorDebug.PhysicalObjectActor.ParticleVelocityScale", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

// Dynamic Level Object Actor
// --------------------------

Debug::Logger &g_PhyObjLog = Debug::GetLogger("amax.actors.physicalobject");

Debug::String gPhysObjAcmeSetupBreakpointName("ActorDebug.Actors.PhysicalObjects.AcmeSetupBreakpointString", "#");


Debug::Float g_PhysObjCollisionDamageMagScale( "ActorDebug.Actors.PhysicalObjects.CollisionDamageMagScale", 0.005f);
Debug::Bool g_PhysObjShowDamageImpulses( "ActorDebug.Actors.PhysicalObjects.ShowDamageImpulses", false);
Debug::Float g_PhysObjDamageSpeed( "ActorDebug.Actors.PhysicalObjects.DamageMinSpeed", 2.5f );
Debug::Bool g_PhysObjAllStatic("ActorDebug.Actors.PhysicalObjects.AllStatic",false);
Debug::Bool g_PhysObjRender("ActorDebug.Actors.PhysicalObjects.Render",true);

// for the debug render
Debug::Bool g_PhysObjDebugRender("ActorDebug.Actors.PhysicalObject.DebugRender.Enable", false);
Debug::Float g_PhysObjDebugRenderRange("ActorDebug.Actors.PhysicalObject.DebugRender.Range", 500.0f, Debug::Range(0.0f, 100.0f) );
Debug::Bool g_PhysObjShowVelocity("ActorDebug.Actors.PhysicalObject.DebugRender.ShowVelocity", false);
Debug::Bool g_PhysObjDrawActorMat("ActorDebug.Actors.PhysicalObject.DebugRender.ActorMatrix", false);
Debug::Bool g_PhysObjDrawCollisionMat("ActorDebug.Actors.PhysicalObject.DebugRender.CollisionMatrix", false);
Debug::Bool g_PhysObjShowCollideDamageRays("ActorDebug.Actors.PhysicalObject.DebugRender.ShowCollideDamageRays", false);
Debug::Bool g_PhysObjShowHitPrimitiveInfo("ActorDebug.Actors.PhysicalObject.ShowHitPrimitiveInfo", false);

extern Debug::Float g_debugVehDynKillDot;
extern Debug::Float g_debugVehDynKillOffset;

//Debug::Float g_PhysObjBlobShadowIntensity( "ActorDebug.Actors.PhysicalObject.BlobShadow.Intensity", 1.0f, Debug::Range(0.0f, 2.0f) );

#define SAME_GROUP_FOR_ALL_STATIC

Debug::Bool g_PhysObjAddAudioTriggers("Audio.Actors.PhysicalObject.EnablePhyObjects", true);

//	Actor creation definition (Sorcery)

XT_TYPE(::Actors::C_PhysicalObjectCreate,		"PhysicalObjectActor")
	XT_BASE(::Actors::C_PosActorCreate)

	XT_FIELD(m_Definition,						"Definition")
	XT_FIELD(m_ReplacementGroup,				"ReplaceFromGroup")
	XT_FIELD(m_RaceLogicGroup,					"RaceLogicGroup")

	XT_FIELD(m_pSorceryMovementComponentCreate,	"MovementComponent")
XT_END_TYPE

namespace Actors
{


//=========================================================================
//					Component Creation Setup
//=========================================================================

//
void ::Actors::C_PhysicalObjectCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_PosActorCreate::DoSorceryCallback(pCallbackFn);

	if (m_pSorceryMovementComponentCreate != NULL)
		DoComponentSorceryCallback(pCallbackFn, *m_pSorceryMovementComponentCreate);//, &m_pMovementCreator);
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Physical object
//
C_PhysicalObject::C_PhysicalObject()
{
	m_IsEnabled				= true;
	m_BodyActive			= false;
	m_UpdateShape			= false;
	m_Damaged				= false;
	m_VisibleLastFrame		= false;
	m_TimeUntilKilled		= 0.0f;
	m_FramesInCollision		= 0;
	m_TimeInCollision		= 0.0f;
	m_LifeTimeAfterDamage	= 0.0f;
	m_ImpactVelocity		= Maths::Constants<Maths::HVec3>::Zero();

#ifndef CFG_GOLD
	m_Initialising			= true;
#endif

	m_pVisDetect			= NULL;

	m_pTriggerObject = NULL;

	m_pGameWorld = (NULL);
}


C_PhysicalObject::~C_PhysicalObject()
{

	if(g_PhysObjAddAudioTriggers)
	{
		if(m_pTriggerObject && pTriggerLayer)
		{
			pTriggerLayer->RemoveTrigger(m_pTriggerObject);
		}

			//destroy trigger for object that died in its sleep...
		DestroyTrigger(GetHandle());
	}

	if (m_pVisDetect)
	{
		C_VisibilityDetector::ReleaseVisDetect(m_pVisDetect);
		m_pVisDetect = NULL;
	}

	::Actors::C_ActorUtils::Instance().SanityCheck_Remove(this);
}

Debug::Float g_PhysObjDefaultDrawDistance("ActorDebug.Actors.PhysicalObjects.DefaultDrawDistance", 100.0f);
Debug::Float g_debugHackStaticActivateScale("PhysicsHax.StaticActiveHackScale", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

bool C_PhysicalObject::InitialiseImpl()
{
	PERF_EVENT("C_PhysicalObject.InitialisedImpl");
	const C_PhysicalObjectCreate* pCreateData = GetCreationData();

	if (pCreateData->m_ReplacementGroup && pCreateData->m_ReplacementGroup->ObjDefList.count > 0)		// Randomly replace definition from group list if there is one
	{
		Game::C_AmaxWorldLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer();
		int count = pCreateData->m_ReplacementGroup->ObjDefList.count;
		int randNum = 0;
		if (pLayer)
		{
			if (pLayer->GetWorldState() == Game::E_WorldState_LoadingRace)
			{
				randNum = pLayer->m_RandomGenerator.Rand()%pCreateData->m_ReplacementGroup->ObjDefList.count;
				pLayer->m_RandomNumberTracker.Push(randNum);
			}
			else if (pLayer->GetWorldState() == Game::E_WorldState_RestartingRace)
			{
				randNum = pLayer->m_RandomNumberTracker.Pop(); 
			}
		}
		
		m_ElementRecord = pCreateData->m_ReplacementGroup->ObjDefList[randNum];
	}
	else
		m_ElementRecord = pCreateData->m_Definition;

	if (!m_ElementRecord)
	{
		LOGERROR(::Actors::g_Log, DBSTR" '%s' has NULL def", pCreateData->m_Name.Text());
		Automation::SendProblemFoundMsg("PhysicalObject", "NullDefinition");
		return false;
	}

	// Set m_Definition:
	if (!pCreateData->IsSpawnedChild())
	{
		m_Definition = static_cast<const C_PhysicalObjectDef*>(m_ElementRecord->m_RootObject);
		m_LifeTimeAfterDamage = m_Definition->m_LifeTimeAfterDamage;
	}
	else
	{
		m_Definition = static_cast<const C_PhysicalObjectDef*>(m_ElementRecord->FindObjectDef(pCreateData->m_RootModelElementIndex));
		SetupLifeTime();
	}

	m_AudioMassType = m_Definition->m_AudioMassType;

	const Game::C_PhysicalObjectDef* pDef = Definition();
	if (!pDef)
	{
		LOGERROR(::Actors::g_Log, DBSTR" '%s' has NULL root object", pCreateData->m_Name.Text());
		Automation::SendProblemFoundMsg("PhysicalObject", "NullRootObject");
		return false;
	}

	if (!DefModel())
	{
		LOGERROR(::Actors::g_Log, DBSTR" def '%s' has NULL model handle - object killed!", pDef->m_Name.Text());
		Automation::SendProblemFoundMsg("PhysicalObject", "NullModelHandle");
		return false;
	}

	if (pDef && pDef->m_AudioEntity)
	{
		Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_SpawnedCreateInfo> pCreator;

		pCreator->m_AudioEntityDef = pDef->m_AudioEntity; 

		if (InitialiseComponent(pCreator.Get()) == NULL)
		{
			LOGERROR(::Actors::g_Log, "Failed to create audio component for PhysicalObjectActor");
			Automation::SendProblemFoundMsg("PhysicalObject", "AudioComponentFail");
			return false;
		}
	}

	m_Matrix = pCreateData->GetCreationMatrix();

	//MEMORY_TRACKER_PUSH("actors.physicalobject.damagecomponent");
	// Create element manager component:
	Component::C_ComponentCreatorFactoryHelper<Components::C_BaseDamageableElementManagerCreateInfo> pCreator;
	pCreator->m_ElementObject = m_ElementRecord;
	pCreator->m_Model = DefModel();
	//pCreator->m_RootElementName = pCreateData->m_RootElementName;
	pCreator->m_RootModelElementIndex = pCreateData->m_RootModelElementIndex;
	pCreator->m_ElementStateSetup = pCreateData->m_ElementStateSetup;
	//pCreator->m_DamagePercent = pCreateData->m_DamagePercent;
	if (InitialiseComponent(pCreator.Get()) == NULL)
	{
		LOGERROR(::Actors::g_Log, DBSTR"PhysObj '%s': damage object init failed - actor killed", GetActorName());
		Automation::SendProblemFoundMsg("PhysicalObject", "DamageObjectInitFail");
		MEMORY_TRACKER_POP("actors.physicalobject.damage");
		return false;
	}
	//MEMORY_TRACKER_POP("actors.physicalobject.damagecomponent");

	m_CurrentBodyType = pDef->m_BodyType==BasePhysicalObject::E_BodyType_Dynamic ? ActorPhysics::E_BodyType_Dynamic : (pDef->m_BodyType==BasePhysicalObject::E_BodyType_Keyframed ? ActorPhysics::E_BodyType_Animated : ActorPhysics::E_BodyType_Static);
	if (g_PhysObjAllStatic && (m_CurrentBodyType == ActorPhysics::E_BodyType_Dynamic))
		m_CurrentBodyType = ActorPhysics::E_BodyType_Static;

	// Create movement Component
	if (!pCreateData->IsSpawnedChild() && pCreateData->m_pSorceryMovementComponentCreate != NULL)
	{
		if (!InitialiseComponentFromSorcery(pCreateData->m_pSorceryMovementComponentCreate))
			return false;
	}

	//MEMORY_TRACKER_PUSH("actors.physicalobject.modelresourcecomponent");
	// Create Model Resource component:
	Component::C_ComponentCreatorFactoryHelper<Components::C_FileModelResourceCreate> pModelResourceCreationData;
	pModelResourceCreationData->m_ModelHandle = DefModel();
	if (!InitialiseComponent(pModelResourceCreationData.Get()))
	{
		LOGERROR(::Actors::g_Log, DBSTR"dynobj '%s': model resource init failed - actor killed", GetActorName());
		Automation::SendProblemFoundMsg("PhysicalObject", "ModelResourceInitFail");
		return false;
	}
	//MEMORY_TRACKER_POP("actors.physicalobject.modelresourcecomponent");

	//MEMORY_TRACKER_PUSH("actors.physicalobject.rendercomponent");
	Component::C_ComponentCreatorFactoryHelper<Components::C_ElementManagerObjectRenderCreate> mmrc;
	mmrc->m_WorldMatrix = GetMatrix();
	mmrc->m_DrawDistance = pDef->m_DrawDistance > 0.0f ? pDef->m_DrawDistance : (F32)g_PhysObjDefaultDrawDistance;
	mmrc->m_WorldMatrix = m_Matrix;
	if (!InitialiseComponent(mmrc.Get()))
	{
		LOGERROR(::Actors::g_Log, DBSTR"dynobj '%s': render component create failed", GetActorName());
		Automation::SendProblemFoundMsg("PhysicalObject", "RenderComponentCreateFail");
	}
	//MEMORY_TRACKER_POP("actors.physicalobject.rendercomponent");

	//TODO: Must override physics material to: Game::E_PhysicsMaterial_Object;

	//MEMORY_TRACKER_PUSH("actors.physicalobject.elementstate");
	// Create Gravity body component:
	Component::C_ComponentCreatorFactoryHelper<Components::C_BasePhysicalObjectPhysicsBodyCreateInfo> pCreationData;
	pCreationData->m_BodyType = m_CurrentBodyType;
	pCreationData->m_ElementObject = m_ElementRecord;
	pCreationData->m_Model = DefModel();
	if (!GetCreationData()->m_ElementStateSetup.Empty())
	{
		pCreationData->m_ElementDescs.Resize(GetCreationData()->m_ElementStateSetup.Size());
		for (S32 e=0 ; e<GetCreationData()->m_ElementStateSetup.Size() ; e++)
		{
			if (GetCreationData()->m_ElementStateSetup[e].m_IsActive)
				pCreationData->m_ElementDescs[e] = GetCreationData()->m_ElementStateSetup[e].m_DamageLevel;
			else
				pCreationData->m_ElementDescs[e] = -1;
		}
	}
	else
	{
		const S32 elemCount = pDef->ElementCount();
		pCreationData->m_ElementDescs.Resize(elemCount);
 		for (S32 e=0 ; e<elemCount ; e++)
 			pCreationData->m_ElementDescs[e] = 0;	// Assume dmg lvl zero.
	}
	//MEMORY_TRACKER_POP("actors.physicalobject.elementstate");

	const Maths::HMat43 bodyMatrix(pCreateData->GetCreationMatrix());
	pCreationData->m_Matrix = Maths::HMat43(bodyMatrix);
	pCreationData->m_CollisionFilter = Game::PhysicsConstants::kDynamicObjectCollisionFilter;//Collision::C_CollisionFilter(0xffffffff, 0xffffffff);
	pCreationData->m_CollisionFilter.SetCollisionFlags( pCreationData->m_CollisionFilter.GetCollisionFlags() & (~Game::PolyFlags::kCameraSolid));

	//MEMORY_TRACKER_PUSH("actors.physicalobject.physicscomponent");
	pCreationData->m_CollisionFilter.SetGroupId(Game::PhysicsConstants::E_PhysicsGroupID_Object);//pCreateData->m_CollideGroup;
	if (!pCreateData->IsSpawnedChild() || Definition()->m_BaseFlags & BasePhysicalObject::FlagCollideWithParent)
	{
		pCreationData->m_CollisionFilter.SetGroupFlags(0xffffffff);
		pCreationData->m_CollisionFilter.SetGroupMask( 0xffffffff);
	}
	else
	{
		pCreationData->m_CollisionFilter.SetGroupId( 0 );
		pCreationData->m_CollisionFilter.SetGroupMask( 0 );
	}
	pCreationData->m_Deactivator.m_StaticActivateThresholdSq = 0.0f;
	pCreationData->m_Deactivator.m_ActivateThresholdSq = 0.01f;
	pCreationData->m_Deactivator.m_SleepThresholdSq = 0.01f;
	pCreationData->m_Deactivator.m_sSleepCountdown = 15;
	if (Definition()->m_UseStaticActivate && Definition()->m_StaticActivationThreshold > 0.0f)
	{
		const F32 baseStaticActivate = Definition()->m_StaticActivationThreshold * (float)g_debugHackStaticActivateScale;
		const F32 staticActivateThresholdSqd = baseStaticActivate * baseStaticActivate; 
		//const F32 staticActivateThresholdSqd = Definition()->m_StaticActivationThreshold * Definition()->m_StaticActivationThreshold;
		pCreationData->m_Deactivator.m_StaticActivateThresholdSq = staticActivateThresholdSqd;
	}
	else
	{
		//Its a dynamic object, and should start off asleep!
		pCreationData->m_BodyType = ActorPhysics::E_BodyType_Dynamic;
		m_CurrentBodyType = ActorPhysics::E_BodyType_Dynamic;
	}
	pCreationData->m_ContinuousCollision = (pDef->m_BaseFlags & BasePhysicalObject::FlagContinuousCollision)!=0;
	if (!InitialiseComponent(pCreationData.Get()))
	{
		LOGERROR(::Actors::g_Log, DBSTR"Failed to init GRAVITY physics body component");
		return false;
	}
	//MEMORY_TRACKER_POP("actors.physicalobject.physicscomponent");

	SendMessageToComponent(Components::E_IF_PhysicsBody, E_PhysicsBodyMsg_Deactivate);
	SetBodyUserData();
//	m_BlobShadow.m_CollideGroup = pCreationData->m_CollisionFilter.m_GroupId;

	m_pVisDetect = C_VisibilityDetector::CreateVisDetect(GetHandle());

	//const Components::I_PhysicsBody* pPhysicsComponent = PhysicsBodyComponent();
	//if (pPhysicsComponent && pPhysicsComponent->CollideBody() && pPhysicsComponent->CollideBody()->IsActive())
	//{		
	//	Gravity::C_Body* pBody = const_cast<Gravity::C_Body*>(pPhysicsComponent->CollideBody());
	//	if (m_Definition->m_Bouncy)
	//		pBody->GetShape()->SetPhysicsMaterialIndex(Game::E_PhysicsMaterial_ObjectBouncy);
	//	else
	//		pBody->GetShape()->SetPhysicsMaterialIndex(Game::E_PhysicsMaterial_Object);
	//	m_BodyActive = true;
	//	SetUpdate(true);
	//}

	//MEMORY_TRACKER_PUSH("actors.physicalobject.animation");
	// Animation
	if (pDef->m_AnimationPlayerCreate)
	{
		if (InitialiseComponentFromSorcery(pDef->m_AnimationPlayerCreate))
		{
			// Create animation controller and slave this object to it:
			Component::C_ComponentCreatorFactoryHelper<Components::C_AnimatedObjectAnimationControllerCreator> pAcCreate;
			pAcCreate->AddSlave(GetHandle(), GetActorName());
			if (!InitialiseComponent(pAcCreate.Get()))
			{
				LOGWARN(::Actors::g_Log, DBSTR"'%s': Animation controller component init failed", GetActorName());
			}
		}
		else
		{
			LOGWARN(::Actors::g_Log, DBSTR"'%s': Animation player component init failed", GetActorName());
		}
	}
	//MEMORY_TRACKER_POP("actors.physicalobject.animation");


	AmaxRoute *pRoute  = C_GameData::Instance().mRoutes[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mRoute];
	minheight = pRoute->minYPosition;
	if (minheight == 0)
		minheight = -500.0f;

	::Actors::C_ActorUtils::Instance().SanityCheck_Register(this);	// Check this against any other objects

	return true;
}

Debug::Bool g_debugPhyObjStartEnabled("ActorDebug.PhysicalObjectActor.StartEnabled", false);

bool C_PhysicalObject::CreateImpl()
{
	if(!C_GameActor::CreateImpl())
		return false;

	const C_PhysicalObjectCreate* pCreateData = GetCreationData();
	const C_PhysicalObjectDef* pDef = Definition();

	//AddSubscriber(ActorMsg::E_GameActorMsg_Update2ndPass);
	AddSubscriber(Game::E_GameMsg_KillObjectsForCutScene);

//	SetCategory(ActorCategory_PhysicalObject);
	//SetUpdate(m_IsEnabled);
	//TODO: We might need to start some enabled, if we get issues, then we'll need a flag in Sorcery
	SetUpdate(g_debugPhyObjStartEnabled);
		
	//AddSubscriber(GameMessaging::E_GameMsg_Update);
//	SetCategory(ActorCategory_Damageable);

	//if (pCreateData->m_NoPortalCull)
	//	SendMessageToComponent(Components::E_IF_Render, E_RenderMsg_DisablePortalCulling);

	//Drop shadow
	if (pDef->m_BaseFlags & BasePhysicalObject::FlagBlobShadow)
	{
		m_DropShadowHandle = SceneGraph::AddDropShadow(GetSceneGraph());
		if (m_DropShadowHandle)
		{
			//Set parameters
			m_DropShadowHandle->SetCullDistance(60.f);
			m_DropShadowHandle->SetCullScreenSize(0.025f);
			
			m_DropShadowHandle->SetIntensity(pDef->m_BlobShadowIntensity);

			const Components::I_RenderComponent* rc = QueryInterface<Components::I_RenderComponent>();
			if (rc && rc->GetModelInstance())
			{
				Maths::C_Sphere sphere = rc->GetModelInstance()->GetWorldSphere();
				m_ShadowRadius = sphere.Radius() * pDef->m_BlobShadowRadius;
				m_ShadowPosition = sphere.Centre() * m_Matrix.InverseRotTrans();
			}
		}
		UpdateDropShadow();
	}

	// Create constraint if specified:
	if (pDef->m_Constraint)
	{
		const GameResource::C_ModelElement* elem = NULL;
		const Util::C_String elementName(pDef->m_ConstraintModelElementName.Text());
		if (elementName.Length()>0)
		{
			elem = FindModelElement(elementName);
		}
		else
		{
			// Anchor constraint at root element pos (GetMatrix() = *model* root pos = invalid anchor point):
			elem = FindModelElement(pCreateData->m_RootModelElementIndex);
		}

		Maths::HMat43 mat(GetMatrix());
		if (elem)
			mat = elem->GetMatrix() * GetMatrix();

		const Actor::C_ActorHandle constraintHandle = ActorPhysics::CreateConstraint(pDef->m_Constraint, mat, *this, pCreateData->m_ParentHandle);

		// Set constraint dependency info:
		if (pCreateData->m_ParentHandle != Actor::C_ActorHandle())
		{
			const C_ElementMgrAddDependentConstraintMsg msg(pCreateData->m_ParentElementIndex, constraintHandle);
			SendMessageToActor(pCreateData->m_ParentHandle, &msg);
		}
	}

	AddSubscriber(Game::E_GameMsg_PostCreateInit);

#ifndef CFG_GOLD
	m_Initialising = false;
#endif

	const Components::I_RenderComponent* rc = QueryInterface<Components::I_RenderComponent>();
	if (rc)
		m_ShockIntensityParameter = rc->GetMaterialParameter("g_vShockIntensity");

	// load resources for audio entities for all objects
	if (m_ElementRecord)
	{
		for (S32 i=0; i<m_ElementRecord->m_Objects.count; i++)
		{
			C_PhysicalObjectDef* pObject = static_cast<C_PhysicalObjectDef*>(m_ElementRecord->m_Objects[i]);
			if (pObject)
			{
				if (pObject->m_AudioEntity)
				{
					CacheEntityResources(pObject->m_AudioEntity, pObject->m_Name.string, true);
				}
			}
		}
	}

	if (C_GameApp::GetInstance()->GetRaceBookLayer() && C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer())
		m_pGameWorld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();
	else
		m_pGameWorld = NULL;

	if(g_PhysObjAddAudioTriggers && !pCreateData->IsSpawnedChild() && 
		this->GetAudioMassType() > Game::E_AUDIO_MASS_TYPE_LIGHT )
	{
		// create audio layer trigger
		CreateTrigger(2,GetHandle(),m_Matrix);

		if(pTriggerLayer && m_pTriggerObject)
		{
			pTriggerLayer->AddTrigger(m_pTriggerObject);	
		}

		//update the triggers
		//if(m_pTriggerObject && pTriggerLayer)
		{
			U8 data[1024 * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, 1024);
			//pTriggerLayer->SetTriggerMatrix(m_pTriggerObject, m_WorldMatrix, stack);

			m_pTriggerObject->SetMatrix(m_Matrix, stack);
		}
	}

	return true;
}


void C_PhysicalObject::KillImpl()
{
	const C_PhysicalObjectCreate* pCreationData = GetCreationData();

	RemoveSubscriber(Game::E_GameMsg_UpdateLifeTime);

	if (m_DropShadowHandle)
		SceneGraph::RemoveDropShadow(GetSceneGraph(), m_DropShadowHandle);
	
	// free all audio entities which were loaded
	if (m_ElementRecord)
	{
		for (S32 i=0; i<m_ElementRecord->m_Objects.count; i++)
		{
			C_PhysicalObjectDef* pObject = static_cast<C_PhysicalObjectDef*>(m_ElementRecord->m_Objects[i]);
			if (pObject)
			{
				if (pObject->m_AudioEntity)
				{
					bool bFreed = false;
					pObject->m_AudioEntity->ReleaseEntityType(bFreed);
					#ifndef CFG_GOLD
					//BIZ_ASSERT_MSG(!(C_GameApp::GetInstance()->GetGameAudioLayer()->AreWeInRace() && bFreed), "We are freeing audio resources for entity '%s' mid race. Physical object is '%s'. This should not happen.", pObject->m_AudioEntity->Name(), pObject->m_Name.string);
					#endif
				}
			}
		}
	}

	C_GameActor::KillImpl();
}

void C_PhysicalObject::Update(const Maths::C_TimeS& timeStep)
{
	PERF_EVENT("C_PhysicalObject.Update");

	if (GetLifetimeState() == ActorSys::E_LifetimeState_Killed) // Update being called on killed actors.
		return;

	bool visible = m_pVisDetect && m_pVisDetect->IsVisible() != 0;

	if (visible)
		UpdateDropShadow();

	const Maths::HF32 massThreshold((F32)g_debugPhysObjSnagUpForceThreshold);

	UpdateMoveableParts(timeStep);
	SendMessageToComponent(Components::E_IF_DamageableElementManager, E_ElementMgrMsg_UpdateElementMatricies);
	SendMessageToComponent(Components::E_IF_DamageableElementManager, E_ElementMgrMsg_UpdatePhysicsShapes);

	m_LocalSpace.SetPosition(m_Matrix.AW());
	Maths::C_Quaternion quat((Maths::HMat33)m_Matrix);
	m_LocalSpace.SetRotation(quat);

	/*m_FramesInCollision--;
	m_FramesInCollision = Maths::Max(m_FramesInCollision, 0);
	if (m_FramesInCollision >= 1)
	{
		m_TimeInCollision += timeStep;
		const Components::I_PhysicsBody* pPhysics = PhysicsBodyComponent();
		if (m_TimeInCollision > (float)g_debugPhysObjSnagTimeBeforeDeath &&
			pPhysics && pPhysics->CollideBody() && 
			pPhysics->CollideBody()->LinearVelocity().Length2() >= (float)g_debugPhysObjSnagSpeedMinBeforeDeath)
		{
			if (pPhysics->CollideBody()->Position().Y() > (m_ColliderPosition.Y() + (float)g_debugPhysObjSnagUpForceYDiff))
			{
				if(pPhysics->CollideBody()->Mass() < massThreshold)
				{
					Gravity::C_Body* pBody = const_cast<Gravity::C_Body*>(pPhysics->CollideBody());
					pBody->ApplyImpulse(Maths::HVec3(0.0f, (float)g_debugPhysObjSnagUpForce * pBody->Mass(), 0.0f), pPhysics->CollideBody()->Position());
					m_FramesInCollision = 0;
					m_TimeInCollision = 0.0f;
				}
			}
			else
			{
				SpawnParticleEffects();
				Kill();
			}
		}
	}
	else
	{
		m_TimeInCollision -= timeStep;
		m_TimeInCollision = Maths::Max(m_TimeInCollision, 0.0f);
	}*/

	F32	ny = m_Matrix.AW().Y();
	if (ny < minheight)
	{
		LOGWARN(g_Log, "Killing physical object %s because it's fallen off the world\n", GetActorName());
		Kill();
	}

	if(g_PhysObjAddAudioTriggers)
	{
					//update the triggers
		if(m_pTriggerObject && pTriggerLayer)
		{
			U8 data[1024 * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, 1024);
			//pTriggerLayer->SetTriggerMatrix(m_pTriggerObject, m_WorldMatrix, stack);

			m_pTriggerObject->SetMatrix(m_Matrix, stack);
		}
	}

	
//	UpdateBlobShadow();

//	SetUpdate(m_BodyActive);
}

void C_PhysicalObject::UpdateLifeTime()
{
	if (m_Damaged && m_pVisDetect)
	{
		bool visible = m_pVisDetect->IsVisible() != 0;
		
		//Reset timer
		if (visible && !m_VisibleLastFrame)
		{
			m_TimeUntilKilled		= m_LifeTimeAfterDamage;
		}
		else if (!visible)
		{
			m_TimeUntilKilled -= C_GameApp::GetInstance()->GetTimeStep();

			if (m_TimeUntilKilled <= 0.0f)
			{
				RemoveSubscriber(Game::E_GameMsg_UpdateLifeTime);
				Kill();
			}
		}

		m_VisibleLastFrame = visible;
	}
}


void C_PhysicalObject::SetBodyUserData()
{
	const Components::C_PhysicsBody* pPhysicsBody = (Components::C_PhysicsBody*)QueryInterface(Components::E_IF_PhysicsBody);
	if (pPhysicsBody && pPhysicsBody->CollideBody())
	{
		Gravity::C_Body* pGravityBody = const_cast<Gravity::C_Body*>(pPhysicsBody->CollideBody());
		m_UserData.m_hActorHandle = GetCreationData()->GetActorHandle();		
		pGravityBody->SetUserData((void*)&m_UserData);
	}
}

void C_PhysicalObject::SetVisibilityDetect()
{
	const Components::I_RenderComponent* pRenderComponent = (const Components::I_RenderComponent*)QueryInterface(Components::E_IF_Render);
	if (pRenderComponent)
	{
		if (pRenderComponent->GetModelInstance())
		{
			if (!m_pVisDetect)
				m_pVisDetect = C_VisibilityDetector::CreateVisDetect(GetHandle());

			pRenderComponent->GetModelInstance()->SetVisDetect(m_pVisDetect);
			pRenderComponent->GetModelInstance()->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, Game::E_RenderPass_DynamicObjectSolid);
			if (!Definition()->m_DisplayInEnvironment)
				pRenderComponent->GetModelInstance()->DisableVisibilityFlags(Game::E_ViewTypeFlag_CubeMap);
			if (!Definition()->m_DisplayInReflection)
				pRenderComponent->GetModelInstance()->DisableVisibilityFlags(Game::E_ViewTypeFlag_Reflection);
			if (!Definition()->m_DisplayInMirror)
				pRenderComponent->GetModelInstance()->DisableVisibilityFlags(Game::E_ViewTypeFlag_Mirror);
		}
	}
}

void C_PhysicalObject::UpdateShape()
{
	// Consolidate shape changes now:
	if (m_UpdateShape)
	{
		const Components::C_BaseDamageableElementManager* elemMgr = (const Components::C_BaseDamageableElementManager*)(QueryInterface(Components::E_IF_DamageableElementManager));
		if (elemMgr)
		{
			C_PhysicalObjectPhysicsBodyUpdateShapeMsg popbusMsg;
			popbusMsg.m_ElemDescList.Resize(elemMgr->FlatElementDefs().Size());
			for (S32 e=0 ; e<elemMgr->FlatElementDefs().Size()/*Definition()->m_FlatArray.Size()*/ ; e++)
			{
				if (ElementIsActive(e))
					popbusMsg.m_ElemDescList[e] = elemMgr->ElementDamageLevel(e);
				else
					popbusMsg.m_ElemDescList[e] = -1;
			}

			SendMessageToComponent(Components::E_IF_PhysicsBody, &popbusMsg);

			SetBodyUserData();
			
			m_UpdateShape = false;
		}
	}

	RemoveSubscriber(ActorMsg::E_GameActorMsg_Update2ndPass);
	SendMessageToComponent(Components::E_IF_DamageableElementManager, E_ElementMgrMsg_UpdateElementMatricies);

/*
	F32	ny = m_Matrix.AW().Y();
	if (ny < minheight)
	{
		LOGWARN(g_Log, "Killing physical object %s because it's fallen off the world\n", GetActorName());
		Kill();
	}
*/
}


// Enable(false) to remove from the world.
void C_PhysicalObject::Enable(bool enable)
{
	if (enable == m_IsEnabled)
		return;

	SetUpdate(enable);

	//if (enable)
	//	AddSubscriber(GameMessaging::E_GameMsg_Update);
	//else
	//	RemoveSubscriber(GameMessaging::E_GameMsg_Update);

	SendMessageToComponent(Components::E_IF_PhysicsBody, enable ? E_PhysicsBodyMsg_WorldInsert : E_PhysicsBodyMsg_WorldRemove);

	m_IsEnabled = enable;
}


// Makes this object dynamic or static.
void C_PhysicalObject::MakeDynamic(bool makeDynamic)
{
	const ActorPhysics::TBodyType newBodyType = (makeDynamic)? ActorPhysics::E_BodyType_Dynamic : ActorPhysics::E_BodyType_Static;
	if (newBodyType != m_CurrentBodyType)
	{
		SendMessageToComponent(Components::E_PhysicsBody, E_PhysicsBodyMsg_MakeDynamic);
		OnBodyTypeChanged(newBodyType);
	}
}


// Turns this object's body into an animated body.
void C_PhysicalObject::MakeAnimated()
{
	if (m_CurrentBodyType != ActorPhysics::E_BodyType_Animated)
	{
		SendMessageToComponent(Components::E_PhysicsBody, E_PhysicsBodyMsg_MakeAnimated);
		OnBodyTypeChanged(ActorPhysics::E_BodyType_Animated);
	}
}


S32 C_PhysicalObject::ElementCount() const 
{
	return ElementManagerComponent()->ElementCount();
}


S32 C_PhysicalObject::ElementIndex(const Util::C_String& elementName) const 
{
	return ElementManagerComponent()->ElementIndex(elementName);
}


bool C_PhysicalObject::IsElementIndestructible(S32 elementIndex) const 
{
	return ElementManagerComponent()->IsElementIndestructible(elementIndex);
}


S32 C_PhysicalObject::ElementIndexFromFlatPrimitiveIndex(S32 primIndex) const 
{
	return ElementManagerComponent()->ElementIndexFromFlatPrimitiveIndex(primIndex);
}


void C_PhysicalObject::OnPhysicsCollision(const C_PhysicsEventCollisionMsg& collisionMsg)
{
	//const C_PhysicalObjectCreate* pCreateData = GetCreationData();
	//const C_PhysicalObjectDef* pDef = Definition();
	//F32 impulseScale = 1.0f;

	//// OnTouched event processing:
	//const C_OnTouchedEvent& touchedEvent = pCreateData->m_OnTouchedEvent;
	//const Actor::C_Actor* otherActor = GetActorList()->GetActorPtr(collisionMsg.m_OtherActorHandle);
	//if (otherActor  &&  collisionMsg.m_ImpulseMagnitude >= touchedEvent.m_ForceThreshold)
	//{
	//	if (touchedEvent.FilterMatch(otherActor->Category()))
	//		touchedEvent.m_Msg.Send(this);
	//}

	//if (collisionMsg.m_OtherBodyType == ActorPhysics::E_BodyType_Dynamic)
	//	int break_here = 1;

	//const Maths::HVec3 impulse(collisionMsg.WorldNormal()*collisionMsg.m_ImpulseMagnitude);
	//const Maths::HF32 impulseMag = collisionMsg.m_ImpulseMagnitude;
	//const Maths::HF32 impactSpeed = impulseMag / PhysicsBodyComponent()->Mass();

	//// make ragdolls break all objects - this will need to be tweaked
	//if(!bRagdollCollide)
	//	impulseScale = 0.2f;

	//const bool applyDamage = ( ((impulseMag > PhysicsBodyComponent()->Mass()*g_PhysObjDamageSpeed) || (pDef->m_Flags & FlagDynobjDestroyOnCollide) || bRagdollBreak)
	//	&& (pDef->m_Flags & FlagPhysObjNoCollideDamage) == 0 );
	//if (applyDamage)
	//{
	//	bool bBreak = (pDef->m_Flags & FlagDynobjDestroyOnCollide) || bRagdollBreak;

	//	if (GetLifetimeState() == ActorSys::E_LifetimeState_Killed)
	//		return;

	//	// Use a ray to find out which element has been hit:
	//	Maths::HVec3 dmgDir = collisionMsg.WorldPos() - GetMatrix().AW();
	//	dmgDir.Normalise();
	//	const F32 rayLength = 2.0f;
	//	const Maths::HVec3 rayStartPos = collisionMsg.WorldPos() + dmgDir * 0.25f;
	//	const Maths::HVec3 rayEndPos = rayStartPos - dmgDir * rayLength;
	//	C_PhysicsRay* ray = new C_PhysicsRay;
	//	ray->InitialiseRay(rayStartPos, rayEndPos, &Game::PhysicsConstants::GetCollisionFilter(Game::PhysicsConstants::E_CollisionFilterType_RayAnyShootable));
	//	C_GameWorld::GetInstance()->GetPhysicsWorld()->RayTraceBody(*ray, PhysicsBodyComponent()->CollideBody());

	//	if (g_PhysObjShowCollideDamageRays)
	//		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(rayStartPos, -dmgDir , rayLength, 0.05f, Maths::RGBA(255, 0, 0));

	//	// Setup damage data:
	//	ActorDamage::C_DamageData damageData;
	//	damageData.m_eDamageType = ActorDamage::E_Damage_WorldImpact;
	//	damageData.m_DamageAmount = impulseMag * g_PhysObjCollisionDamageMagScale;
	//	damageData.SetHitImpulse(impulse * impulseScale);
	//	damageData.SetHitPosition(collisionMsg.WorldPos());
	//	damageData.m_ShapePrimitiveIndex = 0;

	//	if (ray->Hit())
	//	{
	//		damageData.m_ShapePrimitiveIndex = ray->HitShapePrimitiveIndex();
	//		if (g_PhysObjShowCollideDamageRays)
	//		{
	//			DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dCross(ray->HitWorldPosition(), 0.1f, Maths::RGBA(0, 255, 0));
	//			const Btl::String txt(Btl::String::Format("Dmg:%.02f, prim: %d", damageData.m_DamageAmount, ray->HitShapePrimitiveIndex()));
	//			DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dText(ray->HitWorldPosition(), 0.1f, txt.Chars());
	//		}
	//	}

	//	if (bBreak == false)
	//	{
	//		Damage(&damageData);
	//	}
	//	else
	//	{	
	//		Break(&damageData, false);
	//	}

	//	delete ray;
	//}
}


//=========================================================================

F32 C_PhysicalObject::RegisterCollision(Actor::C_Actor *collactor, Vehicle::CollisionInfo *collinfo)
{	
	using namespace Maths;
	//If is a collision with a vehicle
	//if (collactor->GetActorTypeId() == Game::E_ActorTypeId_Vehicle && m_Matrix.AW().Y() < collactor->GetMatrix().AW().Y())
	//{
	//	//If the vehicle is on top of the object, we must kill it

	//	const HF32 vehDynKillDot((float)g_debugVehDynKillDot);
	//	const HF32 vehDynKillOffset((float)g_debugVehDynKillOffset);
	//	const HVec3 vUp(Constants<HVec3>::UnitAxisPositiveY());
	//	const HF32 fDot(collinfo->m_ContactNormal | vUp);
	//	const HF32 fHeight(m_Matrix.AW().Y() - collactor->GetMatrix().AW().Y());
	//	m_ColliderPosition = collactor->GetMatrix().AW();
	//	if (Abs(fDot).GE(vehDynKillDot) && Abs(fHeight).GE(vehDynKillOffset))
	//	{
	//		LOGDEBUG(g_PhyObjLog, "Physical object detected below vehicle - killing off: fDot(%.3f), fHeight(%.3f)", (float)fDot, (float)fHeight);
	//		Kill();
	//		return(-1.0f);
	//	} 
	//}

	//if (collactor && collactor->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)
	//{
	//	m_ImpactVelocity = collinfo->m_ImpactVelocity;

	//	//WTF?
	//	if (!m_BodyActive)
	//	{
	//		m_BodyActive = true;
	//		SetUpdate(true);
	//	}
	//	m_FramesInCollision += 2;
	//}

	return(1.0f);
}

//F32 C_PhysicalObject::ProcessCollision(Actor::C_Actor *collactor, Vehicle::CollisionInfo *collinfo)
//{
//	if (collinfo->m_ContactPoints.Size() == 0)
//	{
//		//Expected? LOGWARN(::Actors::g_Log, DBSTR"%s: received collision with zero contact points", GetActorName());
//		return 1.0f;
//	}
//
//	Maths::HVec3	contact = collinfo->m_ContactPoints[0].m_ContactPoint;			// Just the first point contact point
//	F32 impulseval = collinfo->m_GameplayImpulse * 500.0f;				// Bodge to get sensible values from normalised impact 
//	const Maths::HF32 impulseMag = Maths::Load<Maths::HF32>(&impulseval);
//
//	const Components::I_PhysicsBody* pComponent = PhysicsBodyComponent();
//
//	const C_PhysicalObjectCreate* pCreateData = GetCreationData();
//	const C_PhysicalObjectDef* pDef = Definition();
////	F32		impulseScale = 100.0f;				// w 1.0f
//
////	const Maths::HVec3 impulse(collinfo->m_ContactNormal*collinfo->m_GameplayImpulse);
////	const Maths::HF32 impulseMag = collinfo->m_GameplayImpulse;
//	const Maths::HVec3 impulse(collinfo->m_ContactNormal*impulseval);
//	
//	const Maths::HF32 impactSpeed = impulseMag * pComponent->ReciprocalMass();
//
////	const bool applyDamage = true;//((impulseMag > PhysicsBodyComponent()->Mass()*g_PhysObjDamageSpeed));
//	const bool applyDamage = (impulseval > 0.1f) || ;
//	if (applyDamage)
//	{
//		if (GetLifetimeState() == ActorSys::E_LifetimeState_Killed)
//			return 1.0f;
//
//		Game::C_PhysicalObjectImpactMsg hitMsg(Game::E_PhysicalObjectImpactType_Hit, collinfo->m_GameplayImpulse);
//
//		if(QueryInterface(Components::E_AudioEntity_Spawned))
//			SendMessageToComponent(Components::E_AudioEntity_Spawned, &hitMsg);
//
//		// Use a ray to find out which element has been hit:
//		Maths::HVec3 dmgDir = contact - GetMatrix().AW();
//		dmgDir.Normalise();
//		//const F32 rayLength = 2.0f;
//		const Maths::HF32 rayLength(Maths::Constants<Maths::HF32>::Two());
//		const Maths::HVec3 rayStartPos = contact + dmgDir * Maths::Constants<Maths::HF32>::Quarter();
//		const Maths::HVec3 rayEndPos = rayStartPos - dmgDir * rayLength;
//		Collision::C_Ray ray;
//		Collision::C_CollisionFilter collisionFilter;
//		ray.InitialiseRay(rayStartPos, rayEndPos, &collisionFilter);
//		SharedPhysics::C_SmallSPUCache spuCache;
////:o		C_GameWorld::GetInstance()->GetPhysicsWorld()->RayTraceBody(*ray, PhysicsBodyComponent()->CollideBody());
//		const_cast<Gravity::C_Body*>(pComponent->CollideBody())->Raytrace(ray, pComponent->CollideBody()->GetScene()->m_Stack, spuCache);
//
//
//		if (g_PhysObjShowCollideDamageRays)
//			DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(rayStartPos, -dmgDir , rayLength, 0.05f, Maths::RGBA(255, 0, 0));
//
//		// Setup damage data:
//		//ActorDamage::C_DamageData damageData;
//		m_DamageData.m_eDamageType = ActorDamage::E_Damage_WorldImpact;
//		m_DamageData.m_DamageAmount = impulseMag * g_PhysObjCollisionDamageMagScale;
////		damageData.SetHitImpulse(impulse * impulseScale);
//		m_DamageData.SetHitImpulse(impulse);
//		m_DamageData.SetHitPosition(contact);
//		m_DamageData.m_ShapePrimitiveIndex = 0;
//
//		if (ray.GetResult().Hit())
//		{
//			m_DamageData.m_ShapePrimitiveIndex = ray.m_result.GetShape()->GetIndex();
//			if (g_PhysObjShowCollideDamageRays)
//			{
//				DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dCross(ray.GetResult().HitPoint(), 0.1f, Maths::RGBA(0, 255, 0));
//				const Btl::String txt(Btl::String::Format("Dmg:%.02f, prim: %d", m_DamageData.m_DamageAmount, ray.m_result.GetShape()->GetIndex()));
//				DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dText(ray.GetResult().HitPoint(), 0.1f, txt.Chars());
//			}
//		}
//
//		Damage(&m_DamageData);
//	}
//
//
//	return(1.0f);
//}

//Modified version - calls "damage all elements" if object is hit by a vehicle
F32 C_PhysicalObject::ProcessCollision(Actor::C_Actor *collactor, Vehicle::CollisionInfo *collinfo)
{
	if (collinfo->m_ContactPoints.Size() == 0 || m_Damaged)
	{
		//Expected? LOGWARN(::Actors::g_Log, DBSTR"%s: received collision with zero contact points", GetActorName());
		return 1.0f;
	}
	//m_Damaged = true;
	const Actor::C_Actor* pActorA		= NULL;
	const Actor::C_Actor* pActorB		= NULL;

	Maths::HVec3	contact = collinfo->m_ContactPoints[0].m_ContactPoint;			// Just the first point contact point
	F32 impulseval = collinfo->m_GameplayImpulse * 500.0f;				// Bodge to get sensible values from normalised impact 
	const Maths::HF32 impulseMag = Maths::Load<Maths::HF32>(&impulseval);

	const Components::I_PhysicsBody* pComponent = PhysicsBodyComponent();

	const C_PhysicalObjectCreate* pCreateData = GetCreationData();
	const C_PhysicalObjectDef* pDef = Definition();

	const Maths::HVec3 impulse(collinfo->m_ContactNormal*impulseval);
	
	const Maths::HF32 impactSpeed = impulseMag * pComponent->ReciprocalMass();

	if (GetLifetimeState() == ActorSys::E_LifetimeState_Killed)
		return 1.0f;

	Game::C_PhysicalObjectImpactMsg hitMsg(Game::E_PhysicalObjectImpactType_Hit, collinfo->m_GameplayImpulse);

	if(QueryInterface(Components::E_AudioEntity_Spawned))
		SendMessageToComponent(Components::E_AudioEntity_Spawned, &hitMsg);

	m_DamageData.m_eDamageType = ActorDamage::E_Damage_WorldImpact;
	m_DamageData.m_DamageAmount = impulseMag * g_PhysObjCollisionDamageMagScale; //1000.f
//		damageData.SetHitImpulse(impulse * impulseScale);
	m_DamageData.SetHitImpulse(impulse);
	m_DamageData.SetHitPosition(contact);
	m_DamageData.m_ShapePrimitiveIndex = 0;
	m_DamageData.m_ForceIndestructibleBase = false;

	DamageAllElements(&m_DamageData);
	//Break(&m_DamageData, false);

	return(1.0f);
}


//=========================================================================



// Gravity body events:
void C_PhysicalObject::OnPhysicsBodyEvent(const Gravity::C_BodyEvent& bodyEvent)
{
	switch (bodyEvent.GetEventType())
	{
		case Gravity::C_BodyEvent::E_WakeUp:
		{
			m_BodyActive = true;
			SetUpdate(true);

			// Note: an activate call will change the body type to dynamic and send the wake up event, so need to check body type here.
			const ActorPhysics::TBodyType bodyType = PhysicsBodyComponent()->BodyType();
			if (m_CurrentBodyType != bodyType)
			{
				OnBodyTypeChanged(bodyType);
			}
		}	break;

		case Gravity::C_BodyEvent::E_GoToSleep:
			m_BodyActive = false;
			SetUpdate(false);
			m_TimeInCollision = 0.0f;
			m_FramesInCollision = 0;
			break;

		default:
			LOGWARN(::Actors::g_Log, DBSTR"unhandled event (%d)", bodyEvent.GetEventType());
			break;
	}
}


void C_PhysicalObject::OnBodyTypeChanged(ActorPhysics::TBodyType newBodyType)
{
	assert(m_CurrentBodyType != newBodyType);

	if (newBodyType == ActorPhysics::E_BodyType_Dynamic)
	{
		//BREAK HERE
		if (Definition()->m_UseStaticActivate)
		{
			ActorDamage::C_DamageData damageData;
			damageData.m_eDamageType = ActorDamage::E_Damage_WorldImpact;
			damageData.m_DamageAmount = 1000.0f;
			damageData.SetHitImpulse(m_Matrix.AY());
			damageData.SetHitPosition(m_Matrix.AW());
			damageData.m_ShapePrimitiveIndex = 0;
			damageData.m_ForceIndestructibleBase = true;
			DamageAllElements(&damageData);
			//Break(&damageData, false);
			m_Damaged = true;
		}

// 		if (Definition()->m_FxOnMadeDynamic)
// 			::Actors::SpawnEffectActor(TFxEffectParams(Definition()->m_FxOnMadeDynamic, GetRenderMatrix()));
	}

	m_CurrentBodyType = newBodyType;
}

Debug::Float g_debugPhysicalObjectParticleQuality("Perf.PhysicalObjectParticleQuality", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

void C_PhysicalObject::SetupLifeTime()
{
	const Game::C_PhysicalObjectDef* pDef = Definition();
	if (!m_Damaged && pDef)
	{
		m_Damaged				= true;
		//m_TimeUntilKilled		= pDef->m_LifeTimeAfterDamage == 0.0f ? -1.0f : pDef->m_LifeTimeAfterDamage;
		m_LifeTimeAfterDamage	= pDef->m_LifeTimeAfterDamage;
		m_TimeUntilKilled		= m_LifeTimeAfterDamage;
		if (m_LifeTimeAfterDamage == 0.0f)
		{	
			const C_PhysicalObject* pParentActor = (const C_PhysicalObject*)GetActorList()->GetActorPtr(GetCreationData()->m_ParentHandle);
			if (pParentActor)
			{
				m_LifeTimeAfterDamage	= pParentActor->GetLifeTime();
				if (m_LifeTimeAfterDamage == -1.0f)
					m_LifeTimeAfterDamage = 0.0f;
				m_TimeUntilKilled		= m_LifeTimeAfterDamage;
				
			}
		}
		if (m_TimeUntilKilled > 0.0f)
			AddSubscriber(Game::E_GameMsg_UpdateLifeTime);
	}
}

void C_PhysicalObject::OnElementStateBroken()
{
	SpawnParticleEffects();

	//If this is a static activate object, then lets activate it...
	if(PhysicsBodyComponent())
	{
		const Gravity::C_Body* pBody = PhysicsBodyComponent()->CollideBody();
		if(pBody)
		{
			if(pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static) && 
				pBody->GetGraphNode()->m_Deactivator.m_StaticActivateThresholdSq > 0.f)
			{
				C_PhysicsBodyActivateMsg msg;
				SendMessageToComponent(Components::E_IF_PhysicsBody, &msg);
			}
		}
	}

	if(QueryInterface(Components::E_AudioEntity_Spawned))
	{
		const Game::C_PhysicalObjectImpactMsg breakMsg(Game::E_PhysicalObjectImpactType_Break, 1.0f);
		SendMessageToComponent(Components::E_AudioEntity_Spawned, &breakMsg);
	}

	SetupLifeTime();
}

void C_PhysicalObject::OnElementStateChanged()
{
	if (ElementManagerComponent()->ActiveElementCount() <= 0)
	{
//		GetCreationData()->m_OnDestroyed.Send(this);
		Kill();
		return;
	}

	m_UpdateShape = true;
	AddSubscriber(ActorMsg::E_GameActorMsg_Update2ndPass);		
}

void C_PhysicalObject::UpdateMoveableParts(const Maths::C_TimeS& timeStep)
{
	const Gravity::C_Body* body = PhysicsBodyComponent()->CollideBody();
	if (body)
	{
		
		Maths::HMat33 inverseRot(GetRenderMatrix().InverseRotTrans());
		//Maths::HVec3 localAccel = m_Accel * inverseRot;
		Maths::HVec3 localVel = body->LinearVelocity() * inverseRot;

		BasePhysicalObject::C_PivotInfo pivotInfo;
		pivotInfo.m_LocalAccel = Maths::HVec3(0.0f, 0.0f, 0.0f);
		pivotInfo.m_LocalVel = localVel;
		pivotInfo.m_TimeStep = timeStep.Get();

		C_ElementMgrUpdatePivotsMsg emupMsg(pivotInfo);
		SendMessageToComponent(Components::E_IF_DamageableElementManager, &emupMsg);
	}
}

void C_PhysicalObject::SpawnParticleEffects()
{
	if (!m_pVisDetect->IsVisible(E_ViewTypeFlag_Main) || !m_Definition)
		return;

	const Game::C_PhysicalObjectDef* pDef = m_Definition;
	m_LocalSpace.SetPosition(m_Matrix.AW());
	Maths::C_Quaternion quat((Maths::HMat33)m_Matrix);
	m_LocalSpace.SetRotation(quat);

	//Spawn particle events
	for (S32 iEvent = 0; iEvent < pDef->m_BreakParticleEvents.count; iEvent++)
	{
		const Game::C_ParticleEventSpawn* pSpawner = &pDef->m_BreakParticleEvents[iEvent];
		if (!pSpawner->m_ParticleEvent)
			continue;

		Maths::HVec3 worldPosition;
		DynamicEffect::C_Node node;
		
		if(pSpawner->m_SpawnAtHitPosition)
		{
			// if we are spawning at the hit position then get from data
			worldPosition = m_DamageData.HitPosition();

			// get the position either in local space or world space depending on whether we leave at spawn
			if (!pSpawner->m_LeaveAtSpawn)
				node.SetPosition(worldPosition * m_Matrix.InverseRotTrans());
			else
				node.SetPosition(worldPosition);
		}
		else
		{
			Maths::HVec3 spawnPosition(pSpawner->m_Position.vx, pSpawner->m_Position.vy, pSpawner->m_Position.vz);
			worldPosition = spawnPosition * m_Matrix;
			
			// either use the local space spawn position or the world position depending on whether we leave at spawn
			if(!pSpawner->m_LeaveAtSpawn)
			{
				node.SetPosition(spawnPosition);
			}
			else
			{
				node.SetPosition(worldPosition);
			}
		}
		
		node.SetRotation(DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ(), Maths::HVec3(pSpawner->m_Direction.vx, pSpawner->m_Direction.vy, pSpawner->m_Direction.vz)));
		DynamicEffect::Particle::C_EventInstanceHandle handle = pSpawner->m_ParticleEvent->pEvent->Create(node, pSpawner->m_LeaveAtSpawn ? NULL : &m_LocalSpace);
		if(DynamicEffect::Particle::C_EventInstance* pEventInstance = handle.Object())
		{
			Maths::HVec3 velocity;

			// get the physics body and work out the point velocity at the spawn point
			if(const Gravity::C_Body* pBody = PhysicsBodyComponent()->CollideBody())
			{
				const SharedPhysics::C_RigidBody* rigid = &pBody->GetBody();

				const Maths::HVec3 impulse(-m_DamageData.HitImpulse().m_Vector);
				const Maths::HVec3 dir(worldPosition - rigid->m_CenterOfMass);
				const Maths::HVec3 angularImpulse(impulse ^ dir);
				const Maths::HVec3 angularVel(dir ^ ((rigid->m_AngularMomentum + angularImpulse) * rigid->m_InverseInertiaTensor));
				velocity = rigid->m_LinearVelocity + (impulse * rigid->ReciprocalMass()) + angularVel; 
			}
			pEventInstance->SetLighting(Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 1.0f));
			pEventInstance->SetIntensity(pSpawner->m_Intensity * g_debugPhysicalObjectParticleQuality);
			pEventInstance->SetCollisionPlane(m_Matrix.Translation(), Maths::HVec3(0.f, 1.f, 0.f));
			if (!pSpawner->m_DontUseCarVelocity)
				velocity += m_ImpactVelocity * (float)g_debugPhyObjsParticleVelocityScale;
			pEventInstance->SetVelocity(velocity);
		}
	}

	m_ImpactVelocity = Maths::Constants<Maths::HVec3>::Zero();
}

bool C_PhysicalObject::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
		case ActorMsg::E_ResourceMsg_AllLoaded:
		{
			if (GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
				SetPrimed();
		}	break;

		case Actor::E_ActorMsg_ComponentCreated:
			OnComponentCreated(Messaging::MessageCast<Actor::C_ComponentCreatedMsg>(pMessage)->GetValue());
			break;

		case E_PhysicsBodyNotification_BodyCreated:
			SetBodyUserData();
			break;

		case Game::E_GameMsg_PostCreateInit:
		{
			SendMessageToComponent(Components::E_IF_DamageableElementManager, E_ElementMgrMsg_UpdateElementMatricies);
			EnableSubscriber(Game::E_GameMsg_PostCreateInit, false);		
			break;
		}

		case GameMessaging::E_GameMsg_Update:
			Update(Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue());
			break;

		case Game::E_GameMsg_UpdateLifeTime:
			UpdateLifeTime();
			break;

		case ActorMsg::E_GameActorMsg_Update2ndPass:
		{
			PERF_EVENT("C_PhysicalObject.Update2ndPass");

			UpdateShape();
			SendMessageToComponent(Components::E_IF_DamageableElementManager, E_ElementMgrMsg_UpdateElementMatricies);

			C_SetRenderMatrixMsg srmMsg(GetRenderMatrix());
			SendMessageToComponent(Components::E_IF_Render, &srmMsg);
			break;
		}

		case ActorMsg::E_GameActorMsg_Enable:
			Enable(true);
			break;

		case ActorMsg::E_GameActorMsg_Disable:
			Enable(false);
			break;

		case ::Actors::E_VisibilityMsg_OnHidden:
			break;
		case ::Actors::E_VisibilityMsg_OnVisible:
			break;

		case Actor::E_ActorMsg_WakeUp:
		case Actor::E_ActorMsg_GoToSleep:			
			// Anything to do?
			break;

		case Actor::E_ActorMsg_PhysicsBodyEvent:
		{
			const C_PhysicsBodyEventMsg* msg = Messaging::MessageCast<C_PhysicsBodyEventMsg>(pMessage);
			OnPhysicsBodyEvent(msg->m_BodyEvent);
		}	break;

		case Actor::E_ActorMsg_PhysicsEventCollision:
			OnPhysicsCollision(*Messaging::MessageCast<C_PhysicsEventCollisionMsg>(pMessage));
			break;

		case E_PhysicsBodyNotification_TransformChanged:
		{
			const C_PhysicsBodyTransformChanged* msg = Messaging::MessageCast<C_PhysicsBodyTransformChanged>(pMessage);

			C_SetRenderMatrixMsg srmMsg(msg->GetValue());
			SendMessageToComponent(Components::E_IF_Render, &srmMsg);
			m_Matrix = msg->GetValue();

			F32	ny = m_Matrix.AW().Y();
			if (ny < minheight)
			{
				LOGWARN(g_Log, "Killing physical object %s because it's fallen off the world\n", GetActorName());
				Kill();
			}

			break;
		}

		case E_ElementMgrNotification_ElementBroken:
		{
//			GetCreationData()->m_OnBroken.Send(this); // ?
			OnElementStateBroken();
			break;
		}

		case E_ElementMgrNotification_ElementDestroyed:
		{
//			GetCreationData()->m_OnBroken.Send(this);
			if (ElementManagerComponent()->ActiveElementCount() == 0)
			{
				SpawnParticleEffects();
//				GetCreationData()->m_OnDestroyed.Send(this);
				Kill();
			}
			break;
		}

		case E_ElementMgrNotification_ElementStateChanged:
		{
			SendMessageToComponent(Components::E_IF_DamageableElementManager, E_ElementMgrMsg_UpdateElementMatricies); // [MGB] send msg back to notifier?!
			OnElementStateChanged();
			break;
		}

		case E_ElementMgrNotification_MakeDynamic:
			MakeDynamic();
			break;

		case E_ElementMgrMsg_BreakElement:
		{
			const C_ElementMgrBreakElementMsg* msg = Messaging::MessageCast<C_ElementMgrBreakElementMsg>(pMessage);
			const ActorDamage::C_DamageData* dmgData = &msg->DamageData();//?msg->m_HasDamageData ? &msg->m_DamageData : NULL;
			//const Maths::C_Vector* impulseOverride = msg->m_OverrideImpulse ? &msg->ImpulseOverride() : NULL;
			//const Maths::C_Vector* angVelOverride = msg->m_OverrideAngVel ? &msg->AngularVelocityOverride() : NULL;
			
			ActorDamage::C_DamageInfo damageInfo;
			damageInfo.SetLinearMomentum(PhysicsBodyComponent()->LinearMomentum());
			damageInfo.SetLinearVelocity(PhysicsBodyComponent()->LinearVelocity());
			damageInfo.SetAngularVelocity(PhysicsBodyComponent()->AngularVelocity());
			
			
			//if (impulseOverride)
			//{
			//	damageInfo.m_OverrideLinearImpulse = true;
			//	damageInfo.SetImpulseOverride(*impulseOverride);
			//}
			//else
			//{
			//	damageInfo.m_OverrideLinearImpulse = false;
			//}

			//if (angVelOverride)
			//{
			//	damageInfo.m_OverrideAngularImpulse = true;
			//	damageInfo.SetAngularOverride(*angVelOverride);
			//}
			//else
			//{
			//	damageInfo.m_OverrideAngularImpulse = false;
			//}

			C_ElementMgrBreakElementMsg elementMgrBreakElementMsg(msg->m_ElementIndex, dmgData, damageInfo);
			SendMessageToComponent(Components::E_IF_DamageableElementManager, &elementMgrBreakElementMsg);
			break;
		}

		case E_ElementMgrMsg_DestroyElement:
		{
			const C_ElementMgrDestroyElementMsg* msg = Messaging::MessageCast<C_ElementMgrDestroyElementMsg>(pMessage);
			//const ActorDamage::C_DamageData* dmgData = msg->m_HasDamageData ? &msg->m_DamageData : NULL;
			const ActorDamage::C_DamageData* dmgData = &msg->DamageData();
			//const Maths::C_Vector* impulseOverride = msg->m_OverrideImpulse ? &msg->ImpulseOverride() : NULL;
			//const Maths::C_Vector* angVelOverride = msg->m_OverrideAngVel ? &msg->AngularVelocityOverride() : NULL;

			ActorDamage::C_DamageInfo damageInfo;
			damageInfo.SetLinearMomentum(PhysicsBodyComponent()->LinearMomentum());
			damageInfo.SetLinearVelocity(PhysicsBodyComponent()->LinearVelocity());
			damageInfo.SetAngularVelocity(PhysicsBodyComponent()->AngularVelocity());
			
			/*if (impulseOverride)
			{
				damageInfo.m_OverrideLinearImpulse = true;
				damageInfo.SetImpulseOverride(*impulseOverride);
			}
			else
			{
				damageInfo.m_OverrideLinearImpulse = false;
			}

			if (angVelOverride)
			{
				damageInfo.m_OverrideAngularImpulse = true;
				damageInfo.SetAngularOverride(*angVelOverride);
			}
			else
			{
				damageInfo.m_OverrideAngularImpulse = false;
			}*/

			C_ElementMgrDestroyElementMsg elementMgrDestroyElementMsg(msg->m_ElementIndex, dmgData, damageInfo);
			SendMessageToComponent(Components::E_IF_DamageableElementManager, &elementMgrDestroyElementMsg);
			break;
		}

		//case E_GameActorMsg_Damage:
		//{
		//	if (GetLifetimeState() == ActorSys::E_LifetimeState_Killed)
		//		return true;

		//	const C_ActorDamageMsg* damageMsg = Messaging::MessageCast<C_ActorDamageMsg>(pMessage);
		//	ActorDamage::C_DamageData damageData = damageMsg->m_DamageMsg;	// Copy
		//	if (damageData.m_eDamageType == ActorDamage::E_Damage_Bullet)
		//	{
		//		// Scale bullet impulses here:
		//		F32 frac = (damageData.HitImpulse().Get().Length()-g_PhysObjBulletImpulseGameMin)/(g_PhysObjBulletImpulseGameMax-g_PhysObjBulletImpulseGameMin);
		//		frac = Maths::Clamped(frac, 0.0f, 1.0f);
		//		const F32 mag = Maths::Lerp(g_PhysObjBulletImpulseScaleMin, g_PhysObjBulletImpulseScaleMax, frac);
		//		damageData.SetHitImpulse(damageData.HitImpulse().Normalised().Normal() * mag);
		//	}
		//	else if (damageData.m_eDamageType == ActorDamage::E_Damage_RadialExplosion)
		//	{
		//		// Meddle with explosive damage and impulse:
		//		damageData.m_DamageAmount *= g_PhysObjExplosiveDamageValueScale;
		//		damageData.SetHitImpulse(damageData.HitImpulse().Get() * g_PhysObjExplosiveDamageImpulseScale);
		//	}
		//	
		//	Damage(&damageData);
		//	break;
		//}

		//case E_DynObjMsg_Destroy:
		//{
		//	const C_DynObjDestroyMsg* damageMsg = Messaging::MessageCast<C_DynObjDestroyMsg>(pMessage);
		//	const ActorDamage::C_DamageData* dmgData = damageMsg->m_HasDamageData ? &damageMsg->m_DamageMsg : NULL;
		//	Destroy(dmgData, damageMsg->m_IgnoreIndestructibleFlag);
		//	break;
		//}

		//case E_DynObjMsg_MakeDynamic:
		//	MakeDynamic();
		//	break;

		//case E_ElementMgrMsg_SetElementIndestructible:
		//{
		//	const C_ElementMgrSetElementIndestructibleMsg* msg = Messaging::MessageCast<C_ElementMgrSetElementIndestructibleMsg>(pMessage);
		//	
		//	C_ElementMgrSetElementIndestructibleMsg emgseiMsg(msg->m_ElementIndex, msg->m_Set);
		//	SendMessageToComponent(Components::E_IF_DamageableElementManager, &emgseiMsg);
		//	break;
		//}

// 		case E_GameActorMsg_Destroy:
// 			Destroy(NULL, true);
// 			break;

		case E_ElementMgrNotification_SpawnChild:
		{
			const C_ElementMgrNotificationSpawnChildMsg* msg = Messaging::MessageCast<C_ElementMgrNotificationSpawnChildMsg>(pMessage);
			SpawnChild(*msg);
		}	break;

		case E_PhysicsBodyNotification_BodyShapeChanged:
		case E_ElementMgrNotification_InflictedDamageValIncreased:
		{
			//Do nothing...
			break;
		}

		case ActorMsg::E_DynamicObjMsg_DynamicObjectApplyImpulse:
		{
			PERF_EVENT("PhysicalObject.DynamicObjectApplyImpulse");

			if (GetLifetimeState() == ActorSys::E_LifetimeState_Killed)		// Skip if dead
				break;
				
			const ActorMsg::C_DynamicObjectApplyImpulseMsg* msg = Messaging::MessageCast<ActorMsg::C_DynamicObjectApplyImpulseMsg>(pMessage);
			Gravity::C_Body	*cbody = (Gravity::C_Body*)PhysicsBodyComponent()->CollideBody();
//			cbody->LinearVelocity(msg->m_Force);
//			cbody->Activate();
			cbody->ApplyImpulse(msg->m_Force*cbody->Mass(), m_Matrix.AW());

			if (msg->m_ElectricalEffect)
			{
				C_RenderSetSubtechniqueMsg renderSetSubtechniqueMsg(C_ActorUtils::Instance().gameworld->GetElectricSubTechniqueHandle());
				SendMessageToComponent(Components::E_IF_Render, &renderSetSubtechniqueMsg);

				const Display::C_Float4 shockIntensity(msg->m_ElectricalEffectColour.r, msg->m_ElectricalEffectColour.g, msg->m_ElectricalEffectColour.b, C_GameApp::GetInstance()->GetTimer().GetElapsedTime());
				C_RenderSetMaterialFloatParamMsg msg(m_ShockIntensityParameter);
				msg.m_Storage = shockIntensity;
				SendMessageToComponent(Components::E_IF_Render, &msg);
			}

			ActorDamage::C_DamageData dmgData;
			dmgData.m_eDamageType = ActorDamage::E_Damage_Electric;
//			dmgData.m_DamageAmount = 1.0f;
			dmgData.m_DamageAmount = msg->m_Force.Length() * msg->m_DamageScale;
			DamageAllElements(&dmgData);

			break;
		}

		
		case ActorMsg::E_DynamicObjMsg_DynamicObjectDisableElectricalEffect:
		{
			if (GetLifetimeState() == ActorSys::E_LifetimeState_Killed)		// Skip if dead
				break;

			C_RenderSetSubtechniqueMsg renderSetSubtechniqueMsg(0);
			SendMessageToComponent(Components::E_IF_Render, &renderSetSubtechniqueMsg);
			break;
		}

		case Game::E_GameMsg_KillObjectsForCutScene:
		{
			const Game::C_KillForOutroMsg* pMsg = Messaging::MessageCast<Game::C_KillForOutroMsg>(pMessage);
			float fDist2 = (m_Matrix.AW() - pMsg->m_Position).Length2();
			if (fDist2 >= pMsg->m_DistanceSquared)
				Kill();
		}
		break;
		
		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return true;
}


void C_PhysicalObject::SpawnChild(const C_ElementMgrNotificationSpawnChildMsg& msg)
{
	PERF_EVENT("C_PhysicalObject::SpawnChild");
	Components::I_ModelResource* mrc = (Components::I_ModelResource*)QueryInterface(Components::E_IF_ModelResource);
	const GameResource::C_Model* pModel = mrc->GetModelData()->GetRootModel();	//[MGB]ModelIndex
	Maths::HMat43 mat(pModel->GetMatrix() * GetMatrix());
	const Maths::HVec3 position(mat.AW());

	const Components::I_PhysicsBody* pbc = static_cast<const Components::I_PhysicsBody*>(QueryInterface(Components::E_IF_PhysicsBody));

	Maths::HVec3 vel;
	if(pbc && pbc->CollideBody())
	{
		vel = pbc->CollideBody()->PointVelocity(position);
	}
	mat.Translation(position + vel * 1.f/60.f); //Project spawned part forward by one frame (avoid embedding in car?)

	Actor::C_ActorCreationDataFactoryHelper< ::Actors::C_PhysicalObjectCreate> pPhysObjCreate;
	pPhysObjCreate->m_Name.string = "spawned_dynObj";
	pPhysObjCreate->m_Definition = m_ElementRecord;
	pPhysObjCreate->m_WorldMatrix = Store(mat);
	pPhysObjCreate->m_RootElementName = pModel->GetElement(msg.m_ModelElementIndex)->GetName();
	pPhysObjCreate->m_RootModelElementIndex = msg.m_ModelElementIndex;
	pPhysObjCreate->m_ElementStateSetup = msg.m_ElementStateSetup;
	pPhysObjCreate->m_ParentHandle = GetHandle();
	pPhysObjCreate->m_ParentElementIndex = msg.m_ParentElementIndex;
	pPhysObjCreate->m_CollideGroup = Game::PhysicsConstants::E_PhysicsGroupID_Object;
	const Actor::C_ActorHandle handle = C_ActorUtils::Instance().gameworld->GetActorList()->CreateActor(pPhysObjCreate.Get());

	// Inherit velocity:
	static Util::C_Random rnd;
	if (pbc && pbc->CollideBody())
	{
		
		//vel += vel.Length() * rnd.CubeRand(gPhysObjChildRandomVelocityScale);
		const C_PhysicsBodySetLinearVelocity linVelMsg(vel);
		SendMessageToActor(handle, &linVelMsg);
		//const C_PhysicsBodySetAngularVelocity angVelMsg(pbc->CollideBody()->LinearVelocity().Length() * rnd.CubeRand(gPhysObjChildMaxRandomSpinScale));
		const C_PhysicsBodySetAngularVelocity angVelMsg(pbc->CollideBody()->AngularVelocity());// * rnd.CubeRand(gPhysObjChildMaxRandomSpinScale));
		SendMessageToActor(handle, &angVelMsg);
		const C_PhysicsBodyActivateMsg activateMessage;
		SendMessageToActor(handle, &activateMessage);
	}

	/*if (msg.DamageData())
	{
		const Maths::HVec3 vel = msg.ForceOverrideSet() ? msg.ForceOverride() : Maths::HVec3(msg.DamageData()->HitImpulse().Get());
		const C_PhysicsBodyApplyForce message(vel, msg.DamageData()->HitPosition());
		SendMessageToActor(handle, &message);
	}*/
}


void C_PhysicalObject::OnComponentCreated(S32 componentIf)
{
	switch (componentIf)
	{
		case Components::E_IF_DamageableElementManager:
		{
			// [MGB] todo: this, better. Is there an 'owning actor created' msg?
// 			Components::C_MultiModelRenderer* rc = (Components::C_MultiModelRenderer*)QueryInterface(Components::E_IF_Render);
// 			if (rc)
// 				rc->SetupRenderData();
		} 
		break;
		case Components::E_IF_Render:
		{
			SetVisibilityDetect();
		}
		case Components::E_IF_PhysicsBody:
		{
			const Components::I_PhysicsBody* pPhysicsComponent = PhysicsBodyComponent();
			if (pPhysicsComponent && pPhysicsComponent->CollideBody())
			{		
				Gravity::C_Body* pBody = const_cast<Gravity::C_Body*>(pPhysicsComponent->CollideBody());
				if (m_Definition->m_Bouncy)
					pBody->GetShape()->SetPhysicsMaterialIndex(Game::E_PhysicsMaterial_ObjectBouncy);
				else
					pBody->GetShape()->SetPhysicsMaterialIndex(Game::E_PhysicsMaterial_Object);
				//m_BodyActive = true;
				//SetUpdate(true);
			}
		}
		break;
	}
}


const Maths::HMat43& C_PhysicalObject::GetRenderMatrix() const
{
	Components::I_RenderComponent* rc = (Components::I_RenderComponent*)QueryInterface(Components::E_IF_Render);
	if (rc)
		return rc->GetRenderMatrix();
	return C_GameActor::GetMatrix();
}


#ifndef CFG_GOLD
void C_PhysicalObject::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_PhysObjDebugRender)
		return;

	const F32 rangeSqrd = g_PhysObjDebugRenderRange * g_PhysObjDebugRenderRange;
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	const Maths::HVec3 cameraPos = viewSettings3D.GetCameraPosition();
	const Maths::HVec3 camToNode(GetRenderMatrix().AW()-cameraPos);

	const Components::I_PhysicsBody* bodyComp = PhysicsBodyComponent();
	if (!bodyComp || !bodyComp->CollideBody())
		return;

	const Gravity::C_Body* body = bodyComp->CollideBody();

	if (camToNode.LengthSquared() < rangeSqrd)
	{
		C_String bodyStr = (bodyComp->BodyType()==ActorPhysics::E_BodyType_Dynamic) ? "Dynamic" : (bodyComp->BodyType()==ActorPhysics::E_BodyType_Static ? "Static" : bodyComp->BodyType()==ActorPhysics::E_BodyType_Animated ? "Keyframed" : "UNKNOWN TYPE!");
		if (bodyComp->BodyType()==ActorPhysics::E_BodyType_Dynamic)
			bodyStr += (m_BodyActive ? " (active)" : " (inactive)");

		C_String text;
		text.Printf("%s\nDef: %s\n%s\n",(const char *)GetActorName(), (const char *)Definition()->m_Name, bodyStr.Chars());

		if (g_PhysObjShowVelocity)
		{
			Util::C_String lmom = Maths::MakeString(Maths::HVec3(PhysicsBodyComponent()->LinearMomentum()));
			Util::C_String lvel = Maths::MakeString(Maths::HVec3(PhysicsBodyComponent()->CollideBody()->LinearVelocity()));

			text += C_String().Printf("lmom:%s lvel:%s", lmom.Chars(), lvel.Chars());
		}

		if (g_PhysObjDrawCollisionMat && body)
			debugRenderPtr->Draw3dMatrix(body->ElementMatrix());

		if (g_PhysObjDrawActorMat)
			debugRenderPtr->Draw3dMatrix(GetRenderMatrix());

		debugRenderPtr->Draw3dText(GetRenderMatrix().AW(), text);
	}
}
#endif

void C_PhysicalObject::Damage(ActorDamage::C_DamageData *pDamageData)
{
	if (!pDamageData)
	{
		LOGERROR(::Actors::g_Log, DBSTR"NULL damage data received");
		return;
	}

	PERF_EVENT("PhysicalObjectActor::Damage");

//	if (pDamageData->m_eDamageType == ActorDamage::E_Damage_Bullet)
//	{
////		GetCreationData()->m_OnShot.Send(this);
//	}

	// Work out which element was hit:
	S32 elementIndex = -1;
	if (pDamageData->m_ShapePrimitiveIndex != -1)
	{
		elementIndex = ElementIndexFromFlatPrimitiveIndex(pDamageData->m_ShapePrimitiveIndex);
		if (elementIndex == -1 || !ElementIsActive(elementIndex))	// Might not be active for multiple damage per frame (e.g. shotgun)
		{
			if (g_PhysObjShowHitPrimitiveInfo)
				LOGINFO(::Actors::g_Log, "Dynobj hit: ignoring invalid element index %d (ShapePrimitiveIndex:%d)", elementIndex, pDamageData->m_ShapePrimitiveIndex);
			return;
		}
	}

	if (g_PhysObjShowHitPrimitiveInfo)
		LOGINFO(::Actors::g_Log, "Dynobj hit: ShapePrimitiveIndex:%d, elementIndex:%d", pDamageData->m_ShapePrimitiveIndex, elementIndex);

	// Note:
	// If impulse applied before damage effected then debris will get the correct momentum,
	//   but static things that get made dynamic won't.

	if (g_PhysObjShowDamageImpulses && PhysicsBodyComponent())
	{
		const Maths::HVec3 arrow(pDamageData->HitImpulse().Get()*0.1f);
		Maths::HVec3 pos(PhysicsBodyComponent()->WorldBoundingSphere().Centre());
		pos = pos * GetRenderMatrix();
		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(pos, Maths::HVec3(arrow.Normalised().Normal()), arrow.Length(), 0.05f, Maths::RGBA(0,100,0));
	}

	ActorDamage::C_DamageInfo damageInfo;
	
	if (PhysicsBodyComponent())
	{
		damageInfo.SetLinearMomentum(PhysicsBodyComponent()->LinearMomentum());
		damageInfo.SetLinearVelocity(PhysicsBodyComponent()->LinearVelocity());
		damageInfo.SetAngularVelocity(PhysicsBodyComponent()->AngularVelocity());
	}

	if (elementIndex>=0 && !pDamageData->IsDamageAreaEffect())
	{
		if (pDamageData->m_eDamageType != ActorDamage::E_Damage_WorldImpact)
		{
			C_PhysicsBodyApplyImpulse pbai(pDamageData->HitImpulse(), Maths::C_Vertex(pDamageData->HitPosition()));
			SendMessageToComponent(Components::E_IF_PhysicsBody, &pbai);
		}

		C_ElementMgrDamageElementMsg emdeMsg(elementIndex, pDamageData, damageInfo);
		SendMessageToComponent(Components::E_IF_DamageableElementManager, &emdeMsg);
	}
}


void C_PhysicalObject::DamageAllElements(ActorDamage::C_DamageData *pDamageData)
{
	PERF_EVENT("C_PhysicalObject.DamageAllElements");
	ActorDamage::C_DamageInfo damageInfo;
	damageInfo.SetLinearMomentum(PhysicsBodyComponent()->LinearMomentum());
	damageInfo.SetLinearVelocity(PhysicsBodyComponent()->LinearVelocity());
	damageInfo.SetAngularVelocity(PhysicsBodyComponent()->AngularVelocity());
	
	for (S32 e=0 ; e<ElementManagerComponent()->ElementCount() ; e++)
	{
		if (ElementIsActive(e))
		{
			C_ElementMgrDamageElementMsg emgeMsg(e, pDamageData, damageInfo);
			SendMessageToComponent(Components::E_IF_DamageableElementManager, &emgeMsg);
		}
	}
}


// Break all elements
void C_PhysicalObject::Break(const ActorDamage::C_DamageData *pDamageData, bool ignoreIndestructibleFlag)
{
	ActorDamage::C_DamageInfo damageInfo;
	damageInfo.SetLinearMomentum(PhysicsBodyComponent()->LinearMomentum());
	damageInfo.SetLinearVelocity(PhysicsBodyComponent()->LinearVelocity());
	damageInfo.SetAngularVelocity(PhysicsBodyComponent()->AngularVelocity());

	for (S32 e=0 ; e<ElementManagerComponent()->ElementCount() ; e++)
	{
		if (ElementIsActive(e))
		{
			const bool destroy = ignoreIndestructibleFlag ? true : !IsElementIndestructible(e);
			if (destroy)
			{
				C_ElementMgrBreakElementMsg embeMsg(e, pDamageData, damageInfo);
				SendMessageToComponent(Components::E_IF_DamageableElementManager, &embeMsg);			
			}
		}
	}
	m_Damaged = true;

//	GetCreationData()->m_OnBroken.Send(this);
}


// Destroys the whole object.
// Param pDamageData - may be NULL.
// Param ignoreIndestructibleFlag - set false to respect the element's current indestructible flag.  Set true (default) to always destroy all elements.
void C_PhysicalObject::Destroy(const ActorDamage::C_DamageData *pDamageData, bool ignoreIndestructibleFlag)
{
	PERF_EVENT("PhysicalObjectActor::Destroy");
	ActorDamage::C_DamageInfo damageInfo;
	damageInfo.SetLinearMomentum(PhysicsBodyComponent()->LinearMomentum());
	damageInfo.SetLinearVelocity(PhysicsBodyComponent()->LinearVelocity());
	damageInfo.SetAngularVelocity(PhysicsBodyComponent()->AngularVelocity());

	for (S32 e=0 ; e<ElementManagerComponent()->ElementCount() ; e++)
	{
		if (ElementIsActive(e))
		{
			const bool destroy = ignoreIndestructibleFlag ? true : !IsElementIndestructible(e);
			if (destroy)
			{
				C_ElementMgrDestroyElementMsg emdeMsg(e, pDamageData, damageInfo);
				SendMessageToComponent(Components::E_IF_DamageableElementManager, &emdeMsg);
			}
		}
	}

//	GetCreationData()->m_OnDestroyed.Send(this);
}


bool C_PhysicalObject::ElementIsActive(S32 elementIndex) const 
{
	return ElementManagerComponent()->ElementIsActive(elementIndex);
}

Maths::HVec3 C_PhysicalObject::ElementWorldPos(const GameResource::C_ModelElement& element) const 
{
	return element.GetMatrix().AW() * GetRenderMatrix();
}

Maths::HVec3 C_PhysicalObject::ElementOffset(const GameResource::C_ModelElement& element) const 
{
	return element.GetMatrix().AW();
}


// Apply an impulse resulting from damage.
void C_PhysicalObject::ApplyDamageImpulse(const ActorDamage::C_DamageData* damageData)
{
	assert(damageData);

	if (damageData->m_eDamageType != ActorDamage::E_Damage_WorldImpact)
	{
		C_PhysicsBodyApplyImpulse pbai(damageData->HitImpulse(), Maths::C_Vertex(damageData->HitPosition()));
		SendMessageToComponent(Components::E_IF_PhysicsBody, &pbai);
	}
}


// Apply velocity to the dynamic object centre of mass
void C_PhysicalObject::ApplyVelocity(const Maths::C_Vector &vel)
{
	const F32 KDistanceAway = 0.3f;

	Maths::C_Vector worldpos = GetRenderMatrix().AW();
	Maths::C_ImpulseKgMpS impulse = Maths::C_MassKg(PhysicsBodyComponent()->Mass()) * Maths::C_VelocityMpS(vel);

	// apply impulse at centre of mass, maybe we could do one that applies at at element pos
	C_PhysicsBodyApplyImpulse pbai(impulse, Maths::C_Vertex((worldpos+(GetRenderMatrix().AX()*KDistanceAway))));
	SendMessageToComponent(Components::E_IF_PhysicsBody, &pbai);
}


// Apply a local velocity to the dynamic object centre of mass
void C_PhysicalObject::ApplyLocalVelocity(const Maths::C_Vector &vel)
{
	Maths::C_ImpulseKgMpS impulse = Maths::C_MassKg(PhysicsBodyComponent()->Mass()) * Maths::C_VelocityMpS(vel * GetRenderMatrix().Rotation());

	// apply impulse at centre of mass, maybe we could do one that applies at at element pos
	C_PhysicsBodyApplyImpulse pbai(impulse, Maths::C_Vertex(GetRenderMatrix().AW()));
	SendMessageToComponent(Components::E_IF_PhysicsBody, &pbai);
}


// Return a model element index
S32 C_PhysicalObject::ModelElementIndex(const Util::C_String& elementName) const
{
	Components::I_ModelResource* rc = (Components::I_ModelResource*)QueryInterface(Components::E_IF_ModelResource);

	if(rc && rc->GetModelData() && rc->GetModelData()->GetRootModel())	//[MGB]ModelIndex //GetCreationData()->m_RootModelIndex))
		return rc->GetModelData()->GetRootModel()->GetElementIndex(elementName);

	return -1;
}


bool C_PhysicalObject::GetElementCollisionWorldPos(S32 elementIndex, Maths::HVec3& worldPos) const
{
	assert(elementIndex>=0);

	if (ElementManagerComponent()->CollisionShape(elementIndex))
	{
		const Maths::HVec3 pos(ElementManagerComponent()->CollisionShape(elementIndex)->GetShapeDesc()->GetBoundingBox().centre);
		worldPos = pos * GetRenderMatrix();
		return true;
	}
	else
	{
		LOGERROR(::Actors::g_Log, DBSTR"element has no collision shape");
	}

	return false;
}


// Return model element world position.
// Used for finding position of e.g. locators (elements with modelled position).
bool C_PhysicalObject::GetElementWorldPos(const Util::C_String& elementName, Maths::HVec3& worldPos) const
{
	const GameResource::C_ModelElement* element = FindModelElement(elementName);
	if (element)
	{
		worldPos = ElementWorldPos(*element);
		return true;
	}
	return false;
}


bool C_PhysicalObject::GetElementOffset(const Util::C_String& elementName, Maths::HVec3& pos) const
{
	const GameResource::C_ModelElement* element = FindModelElement(elementName);
	if (element)
	{
		pos = ElementOffset(*element);
		return true;
	}
	return false;
}


// Finds a named element in this object's model.
const GameResource::C_ModelElement* C_PhysicalObject::FindModelElement(const Util::C_String& elementName) const
{
	const Components::I_ModelResource* rc = QueryInterface<Components::I_ModelResource>();
	if (rc && rc->GetModelData() && rc->GetModelData()->GetRootModel())
		return rc->GetModelData()->GetRootModel()->GetElement(elementName);

	LOGWARN(::Actors::g_Log, DBSTR"element '%s' not found.", (const char *)elementName);
	return NULL;
}


const GameResource::C_ModelElement* C_PhysicalObject::FindModelElement(S32 modelElementIndex) const
{
	const Components::I_ModelResource* rc = QueryInterface<Components::I_ModelResource>();
	if (rc && rc->GetModelData() && rc->GetModelData()->GetRootModel())
		return rc->GetModelData()->GetRootModel()->GetElement(modelElementIndex);

	LOGWARN(::Actors::g_Log, DBSTR"element '%d' not found.", modelElementIndex);
	return NULL;
}


void C_PhysicalObject::EnableElement(S32 index, bool enable) 
{
	C_ElementMgrEnableElementMsg emeeMsg(index, enable);
	SendMessageToComponent(Components::E_IF_DamageableElementManager, &emeeMsg);
}


void C_PhysicalObject::SetCollideGroup(S32 group)
{
	const C_PhysicsBodySetCollideGroup msg(group);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &msg);
}

void C_PhysicalObject::SetCollideGroupFlags(U32 flags)
{
	const C_PhysicsBodySetCollideGroupFlags msg(flags);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &msg);
}

void C_PhysicalObject::SetCollideGroupMask(U32 mask)
{
	const C_PhysicsBodySetCollideGroupMask msg(mask);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &msg);
}

void C_PhysicalObject::SetCollideGlobalFlags(U32 flags)
{
	const C_PhysicsBodySetCollideGlobalFlags msg(flags);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &msg);
}

void C_PhysicalObject::SetCollideGlobalMask(U32 mask)
{
	const C_PhysicsBodySetCollideGlobalMask msg(mask);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &msg);
}

void C_PhysicalObject::SetCollideMaterialMask(U32 mask)
{
	const C_PhysicsBodySetCollideMaterialMask msg(mask);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &msg);
}


bool C_PhysicalObject::GetElementWorldMatrix(const Util::C_String& elementName, Maths::HMat43& worldMatrix) const
{
	const GameResource::C_ModelElement* element = FindModelElement(elementName);
	if (element)
	{
		worldMatrix = element->GetMatrix();
		worldMatrix.AW(worldMatrix.AW());
		worldMatrix = worldMatrix * GetRenderMatrix();
		return true;
	}
	return false;
}


// Note: also clears body's momentum & velocity.
bool C_PhysicalObject::SetWarpPosition(const Maths::HMat43& matrix)
{
	const Maths::HVec3 vzero;
	ActorMsg::C_ActorSetTransformMsg astMsg(matrix);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &astMsg);
	C_PhysicsBodySetLinearMomentum pbslm(vzero);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &pbslm);
	C_PhysicsBodySetAngularMomentum pbsam(vzero);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &pbsam);
	C_PhysicsBodySetLinearVelocity pbslv(vzero);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &pbslv);
	C_PhysicsBodySetAngularVelocity pbsav(vzero);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &pbsav);
	return true;
}

Debug::Bool g_debugPhysicalObjectUpdateShadow("ActorDebug.PhysicalObjectActor.UpdateShadow", false, Debug::Persist());

void C_PhysicalObject::UpdateDropShadow()
{
	if (m_DropShadowHandle)
	{
		m_DropShadowHandle->SetDimensions(m_ShadowPosition * m_Matrix, m_ShadowRadius);
#ifndef CFG_GOLD
		if (g_debugPhysicalObjectUpdateShadow)
		{
			m_DropShadowHandle->SetIntensity(Definition()->m_BlobShadowIntensity);

			const Gravity::C_Body* pBody = NULL;
			const Components::I_PhysicsBody* pPBC = QueryInterface<Components::I_PhysicsBody>();
			if (pPBC != NULL)
				pBody = pPBC->CollideBody();

			m_ShadowRadius = (pBody != NULL) ? (F32)pBody->GetLocalBoundingSphere().Radius() : 1.0f;
			m_ShadowRadius *= Definition()->m_BlobShadowRadius;
		}
#endif
	}
}


void C_PhysicalObject::SetAngularDamping(F32 factor)
{
	C_PhysicsBodySetAngularDampingFactor pbsadf(factor);
	SendMessageToComponent(Components::E_IF_PhysicsBody, &pbsadf);
}




//=========================================================================

void C_PhysicalObject::CreateTrigger(float p_Radius,const Actor::C_ActorHandle& actorHandle,Maths::HMat43	mWorldMatrix )
{
	if(!g_PhysObjAddAudioTriggers)
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
	
	//pTriggerLayer->AddTrigger(m_pTriggerObject);	

	//SharedPhysics::C_SafeStack<true> stack(1024);
	////pTriggerLayer->SetTriggerMatrix(m_pTriggerObject, mWorldMatrix, stack);
	//m_pTriggerObject->SetMatrix(mWorldMatrix, stack);
}

void C_PhysicalObject::DestroyTrigger(const Actor::C_ActorHandle& actorHandle)
{
	if(!g_PhysObjAddAudioTriggers)
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



}//namespace Actors

