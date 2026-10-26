#include "SharedActorsPch.h"
#include <shared_actors/Components/Impl/AnimatedObjectComponents.h>

#include <actor_system/ActorMessages.h>
#include <game_context/RenderContext.h>
#include <game_context/PhysicsContext.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <shared_actors/Components/Messages/AnimatedObjectMessages.h>
#include <shared_actors/Components/Messages/AnimationMessages.h>
#include <shared_actors/Components/Messages/RenderMessages.h>
#include <shared_actors/Components/Impl/MovementComponents.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/Utility/ModelResourceUsage.h>
#include <shared_actors/Utility/XtHandleSerialisation.h>
#include <actor_system/Actor.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorList.h>
#include <animated_model/AnimatedModelInstance.h>
#include <animated_model/TaskManager.h>

#include <animation/cycle/Cycle.h>
#include <animation/pose/PoseFormat.h>
#include <animation/pose/PoseMapping.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/skeleton/WindGenerator.h>
#include <animation/task/EvaluationState.h>
#include <animation/task/TaskManager.h>
#include <animation/util/DebugRender.h>
#include <animation/util/Player.inl>
#include <animation/common/JointTransform.inl>
#include <animation/common/StringUtil.h>

#include <game_resource/LevelSceneManager.h>
#include <world_render/EnvironmentSampler.h>
#include <world_render/ModelInstance.h>
#include <world_render/RenderModelData.h>
#include <resource_utils/GameModel.h>
#include <resource_utils/SkeletonResourceDef.h>
#include <resource_utils/CycleResourceDef.h>
#include <resource_utils/I_ResourceBag.h>
#include <resource_utils/ResourceBagUtils.h>
#include <core/maths/hvec/new/IncludeMinimal.h>
#include <core/maths/hvec/new/IncludeMinimal.inl>

#include <dynamic/particle/Event.h>
#include <dynamic/particle/EventInstance.h>
#include <dynamic/particle/EventInstanceHandle.h>
#include <dynamic/particle/generic/XtInterface.h>
#include <dynamic/particle/SpawnMesh.h>

#include <gravity/Importer/Importer.h>
#include <gravity/sdk/Body.h>
#include <gravity/sdk/CompositeDescriptor.h>
#include <gravity/sdk/Scene.h>
#include <gravity/sdk/ShapeDescriptors.h>
#include <gravity/sdk/KinematicController.h>
#include <gravity/narrowphase/Shapes.h>

#include <core/maths/hvec/New/AxisBox.h>
#include <core/maths/hvec/New/Sphere.h>
#include <core/maths/hvec/New/Sphere.inl>

#include <btl/containers/algorithm.h>

#include <specialfx/SpecialFX.h>

#include <actor_studio_msg_game/ActorStudioMessaging.h>

#include <system/Perf/PerfEvent.h>

namespace Gravity
{
	struct C_ExternalConstants;
}

//
// C_SimpleResourceBag
//
// Lightweight resource bag implementation with support for a single listener.
//
class C_SimpleResourceBag: public ResourceUtils::I_ResourceBag, public Resource::C_Listener
{
public:
	class C_ListenerBase
	{
	public:
		virtual void OnResourceBagLoaded()=0;
	};

	template <typename T>
	class C_ListenerDispatcher: public C_ListenerBase
	{
	public:
		C_ListenerDispatcher(T* pListener, void (T::*pFunc)(void)):
		  m_pListener(pListener),
		  m_pFunc(pFunc)
	  {
	  }

		virtual void OnResourceBagLoaded()
		{
			(m_pListener->*m_pFunc)();
		}
	private:
		T* m_pListener;
		void (T::*m_pFunc)(void);
	};

	// Constructs a simple resource bag with a single listener.
	// An arbitrary member function of the listener object can be used as the OnResourceBagLoaded callback.
	template <typename T>
	C_SimpleResourceBag(T* pListener, void (T::*pOnResourceBagLoaded)(void)):
		m_pListener(new C_ListenerDispatcher<T>(pListener, pOnResourceBagLoaded)),
		m_WaitCount(0),
		m_ListenerLockCount(0),
		m_ErrorFlag(false)
	{
	}

	virtual ~C_SimpleResourceBag()
	{
		delete m_pListener;
	}

	virtual void AddResource(const ResourceUtils::C_ResourceBagTag tag, const Sys::C_FilePath& fileName, const Resource::C_ResourceTypeId typeId)
	{
		ItemVector::Iterator it = Btl::Find(m_Items, tag);
		if (it == m_Items.End())
		{
			++m_WaitCount;
			m_Items.PushBack();
			m_Items.Back().m_Tag = tag;
			m_Items.Back().m_Handle.Load(fileName, typeId, this, *(void**)(&tag));
		}
	}

	virtual bool IsWaitingForLoad() const
	{
		return m_WaitCount != 0;
	}

#ifndef CFG_GOLD
	virtual void DebugOutputBagContents(void) const
	{
	}
#endif

	bool HadErrorsWhilstLoading() const
	{
		return m_ErrorFlag;
	}

	// Prevents premature OnResourceBagLoaded events when adding resources to the bag.
	void LockListeners()
	{
		m_ListenerLockCount++;
	}

	void UnlockListeners()
	{
		m_ListenerLockCount--;
		BIZ_ASSERT(m_ListenerLockCount >= 0);
		NotifyListenersIfComplete();
	}

	//
	// Resource::C_Listener
	//
	void ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData)
	{
		const ResourceUtils::C_ResourceBagTag tag = *reinterpret_cast<const ResourceUtils::C_ResourceBagTag*>(&pData);

		ItemVector::Iterator it = Btl::Find(m_Items, tag);
		BIZ_ASSERT(it != m_Items.End());
		C_Item& item = *it;

		if (item.m_LastNotify != result)
		{
			item.m_LastNotify = result;

			if (result != Resource::E_Notify_NoData)
			{
				m_WaitCount--;
				NotifyListenersIfComplete();
			}

			if (result == Resource::E_Notify_DataError)
			{
				LOGERROR(Actor::GetDebugLogAsset(pResource->GetResourceFilename()), DBSTR "error loading resource %s", pResource->GetResourceFilename().Chars());
				m_ErrorFlag = true;
			}

			if (result == Resource::E_Notify_NoData)
			{
				LOGERROR(Actor::GetDebugLogAsset(pResource->GetResourceFilename()), DBSTR "could not find resource %s", pResource->GetResourceFilename().Chars());
				m_ErrorFlag = true;
			}

			BIZ_ASSERT(m_WaitCount >= 0);
		}
	}

private:

	void NotifyListenersIfComplete()
	{
		if (m_ListenerLockCount > 0) return;

		if (m_WaitCount == 0 && m_pListener)
		{
			m_pListener->OnResourceBagLoaded();
		}
	}

	struct C_Item
	{
		C_Item():
			m_LastNotify(Resource::E_Notify_NoData)
		{
		}
	
		ResourceUtils::C_ResourceBagTag	m_Tag;
		Resource::C_HandleBase			m_Handle;
		Resource::E_Notify				m_LastNotify;

		bool operator ==(const ResourceUtils::C_ResourceBagTag& tag) const
		{
			return m_Tag == tag;
		}
	};

	typedef Btl::Vector<C_Item>	ItemVector;

	C_ListenerBase*	m_pListener;
	ItemVector		m_Items;
	S32				m_WaitCount;
	S32				m_ListenerLockCount;
	bool			m_ErrorFlag;
	

	virtual Resource::C_ResourceBase* GetResourceBase(const ResourceUtils::C_ResourceBagTag tag, const Resource::C_ResourceTypeId typeId) const
	{
		ItemVector::ConstIterator it = Btl::Find(m_Items, tag);
		if (it == m_Items.End())
		{
			// Resource not found.
			return NULL;
		}

		const C_Item& item = *it;

		if (item.m_LastNotify != Resource::E_Notify_DataReady)
		{
			// Data not ready either because it's not loaded or there was an error.
			return NULL;
		}

		Resource::C_ResourceBase* pBase = item.m_Handle;

		if (pBase == NULL)
		{
			// Handle not ready for some reason.
			return NULL;
		}
		
		if (CheckResourceTypeId(pBase, typeId))
		{
			return pBase;
		}
		return NULL;
	}

	bool CheckResourceTypeId(const Resource::C_ResourceBase* pBase, const Resource::C_ResourceTypeId typeId) const
	{
		if (pBase == NULL)
		{
			return false;
		}
		if (pBase->GetResourceTypeId() == typeId)
		{
			return true;
		}

		LOGERROR(Actor::GetDebugLogAsset(pBase->GetResourceFilename()), "Incorrect resource type for '%s', expected '%s' got '%s'",
			pBase->GetResourceFilename().Chars(),
			Resource::C_TypeRegistry::GetInstance()->GetResourceTypeName(typeId).Chars(), 
			pBase->GetResourceTypeName().Chars());

		return false;
	}
};


namespace Components
{

//
// C_AnimationPlayerComponent
//

C_AnimationPlayerComponent::C_AnimationPlayerComponent():
	m_EvaluationState(NULL),
	m_pResourceBag(NULL),
	m_Time(0.f),
	m_LoadingCycleIndex(-1),
	m_State(E_State_Uninitialised),
	m_FirstUpdate(true),
	m_Selected(false),
	m_CycleStreaming(false)
{
	m_pResourceBag = new C_SimpleResourceBag(this, &C_AnimationPlayerComponent::OnResourceBagLoaded);

}

C_AnimationPlayerComponent::~C_AnimationPlayerComponent()
{
	delete m_EvaluationState;
	delete m_pResourceBag;
}

void C_AnimationPlayerComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Base::SerialiseForSave(stream);
	Serialise(stream);
}

void C_AnimationPlayerComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Base::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_AnimationPlayerComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Base::SerialiseForSync(stream);
	Serialise(stream);
}

void C_AnimationPlayerComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Base::DeserialiseForSync(stream);
	Deserialise(stream);
}

bool C_AnimationPlayerComponent::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_GameActorMsg_ActionBegin:
			m_NotificationActor = Messaging::MessageCast<ActorMsg::C_ActionBeginMsg>(pMessage)->GetActorHandle();	
			Restart();
			ChangeState(E_State_Playing);
			break;
			
		case ActorMsg::E_GameActorMsg_Activate:
		{
			// Start playing if paused.
			ChangeState(E_State_Playing);
		}
		break;
	
		case ActorMsg::E_GameActorMsg_Deactivate:
		{
			// Stop playing.
			ChangeState(E_State_Stopped);
		}
		break;	

		case ActorMsg::E_GameActorMsg_Reset:
		{
			// Restart using default animation. Leaves current state alone.
			Restart();
		}
		break;	

		case ActorMsg::E_AnimatedObjectMsg_LoadCycle:
		{
			// Start loading an animation... add it to our resource bag.
			const ActorMsg::C_AnimatedObjectLoadCycleMsg* pLoadCycleMsg = Messaging::MessageCast<ActorMsg::C_AnimatedObjectLoadCycleMsg>(pMessage);

			if (!pLoadCycleMsg)
				break;

			if (pLoadCycleMsg->GetValue() < 0 || pLoadCycleMsg->GetValue() >= m_Cycles.Size())
			{
				LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), "%s: LoadCycle message has an out of range cycle index", GetActor()->GetActorName());
				break;
			}

			m_LoadingCycleIndex = pLoadCycleMsg->GetValue();

			if (m_Cycles[m_LoadingCycleIndex].resource)
			{
				if (m_Cycles[m_LoadingCycleIndex].resource->GetCycle())
				{
					if (!m_Cycles[m_LoadingCycleIndex].resource->GetCycle()->IsPartiallyLoaded())
					{
						// Cycle already loaded - no need to request again
						m_LoadingCycleIndex = -1;
						break;
					}
				}
			}

			// Request loading of full cycle.
			LoadCycle(m_LoadingCycleIndex, false);
		}
		break;
		
		case ActorMsg::E_AnimatedObjectMsg_PlayCycle:
		{
			// Start playing a new animation.
			const ActorMsg::C_AnimatedObjectPlayCycleMsg* playCycleMsg = Messaging::MessageCast<ActorMsg::C_AnimatedObjectPlayCycleMsg>(pMessage);
			
			if (!playCycleMsg)
			{
				break;
			}
			
			if (playCycleMsg->m_CycleIndex < 0 || playCycleMsg->m_CycleIndex >= m_Cycles.Size())
			{
				LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), "%s: PlayCycle message has an out of range cycle index", GetActor()->GetActorName());
				break;
			}
				
			ChangeState(E_State_Playing);				
				
			S32 cycleIndex = Maths::Clamped(playCycleMsg->m_CycleIndex, 0, m_Cycles.Size());
			
			if (m_Cycles[cycleIndex].resource != NULL)
			{
				BindCycle(cycleIndex);
				
				Animation::C_PlaybackParams params;
				params.fadeInTime = playCycleMsg->m_FadeInTime;
				params.speed = playCycleMsg->m_Speed;
				params.loopCount = playCycleMsg->m_LoopCount;
				params.layer = 1;
				
				// TODO: queuing?
				
				m_CurrentCycleState = m_AnimPlayer.Play(cycleIndex, m_Cycles[cycleIndex].instance.GetDuration(), &params);
			}
		}
		break;
	
		case GameMessaging::E_GameMsg_PreUpdate:
		{
			PERF_EVENT("C_AnimationPlayerComponent.PreUpdate");

			if (m_EvaluationState == NULL)
				break;
				
			// Update time and currently playing cycle state.
			F32 deltaTime = Messaging::MessageCast<GameMessaging::C_PreUpdateMsg>(pMessage)->GetValue();
			m_Time += deltaTime;

			Evaluate(deltaTime);
			
			// If animation isn't updating but game time is still updating, then send any action completed messages and change to
			// the stopped state to prevent any more update messages being received.
			if (m_AnimPlayer.IsFinished() && deltaTime > 0.f)
			{
				if (m_NotificationActor)
				{
					ActorMsg::C_ActionCompletedMsg actionCompletedMsg;
					actionCompletedMsg.SetActorHandle(GetActorHandle());
					SendMessageToActor(m_NotificationActor, &actionCompletedMsg);
					m_NotificationActor = Actor::C_ActorHandle();
					
					ChangeState(E_State_Finished);
				}
				else
				{
					ChangeState(E_State_Stopped);
				}
			}
			break;
		}
	
		case ActorMsg::E_AnimatedObjectMsg_SetTime:
		{
			// **** TODO: need to ensure that PreUpdate is called in order to propogate these changes to the current state
			// Set time of current cycle. Still need anim player update to be called?
			F32 absoluteTime = Messaging::MessageCast<ActorMsg::C_AnimatedObjectSetTimeMsg>(pMessage)->GetValue();
			m_Time = absoluteTime;
			if (m_CurrentCycleState.IsValid())
			{
				m_CurrentCycleState->SetTime(absoluteTime);
				
				// Prevent time being automatically updated.
				m_CurrentCycleState->SetSpeed(0.f);
			}
			break;
		}

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
		
		default:
			return Actor::I_ActorComponent::OnMessage(pMessage);
	}
	return true;
}

#ifndef CFG_GOLD
DECLARE_DEBUG_BOOL(g_DebugAnimationPlayerComponentRenderDebug, "ActorDebug.Components.AnimationPlayer.DebugRender.Enable", false);
DECLARE_DEBUG_BOOL(g_DebugAnimationPlayerComponentRenderIfSelected, "ActorDebug.Components.AnimationPlayer.DebugRender.OnlyIfSelectedInActorStudio", false);
DECLARE_DEBUG_BOOL(g_DebugAnimationPlayerComponentRenderPlayerInfo, "ActorDebug.Components.AnimationPlayer.DebugRender.ShowPlayingAnimations", false);
DECLARE_DEBUG_BOOL(g_DebugAnimationPlayerComponentRenderJointNames, "ActorDebug.Components.AnimationPlayer.DebugRender.ShowJointNames", false);
DECLARE_DEBUG_BOOL(g_DebugAnimationPlayerComponentRenderJointTransforms, "ActorDebug.Components.AnimationPlayer.DebugRender.ShowJointTransforms", false);
DECLARE_DEBUG_BOOL(g_DebugAnimationPlayerComponentRenderCustomScalars, "ActorDebug.Components.AnimationPlayer.DebugRender.ShowCustomScalars", false);
DECLARE_DEBUG_INT(g_DebugAnimationPlayerComponentFirstCustomScalarIndex, "ActorDebug.Components.AnimationPlayer.DebugRender.FirstCustomScalarIndex", 0, Debug::Range(0, 50));
DECLARE_DEBUG_INT(g_DebugAnimationPlayerComponentCustomScalarCount, "ActorDebug.Components.AnimationPlayer.DebugRender.CustomScalarCount", 6, Debug::Range(0, 32));
DECLARE_DEBUG_BOOL(g_DebugAnimationPlayerComponentRenderBones, "ActorDebug.Components.AnimationPlayer.DebugRender.ShowBones", false);
DECLARE_DEBUG_BOOL(g_DebugAnimationPlayerComponentRenderWind, "ActorDebug.Components.AnimationPlayer.DebugRender.ShowWind", false);


const CHAR8* C_AnimationPlayerComponent::C_CycleNameLookup::operator() (U32 cycleId) const
{
	Animation::C_CycleResource* resource = m_player.m_Cycles[cycleId].resource;
	if (resource != NULL)
	{
		const Animation::C_Cycle* cycle = resource->GetCycle();
		return cycle != NULL ? cycle->GetName().GetString() : "<NULL>";
	}
	return "<NOT_LOADED>";
}


void C_AnimationPlayerComponent::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if(!g_DebugAnimationPlayerComponentRenderDebug)
		return;
	if (!m_EvaluationState)
		return;

	if (g_DebugAnimationPlayerComponentRenderIfSelected && !m_Selected)
		return;
		
	Maths::HMat43 worldMatrix;
	if (GetActor())
	{
		worldMatrix = GetActor()->GetMatrix();
	}
	
	debugRenderPtr->SetDepthEnable(false);
	
	debugRenderPtr->Draw3dMatrix(worldMatrix, 0.2f);
	
	Animation::DebugRenderPoseTransforms(
		debugRenderPtr,
		m_EvaluationState->GetPoseReadOnly(),
		m_EvaluationState->GetSkeleton(),
		worldMatrix,
		g_DebugAnimationPlayerComponentRenderJointTransforms,
		g_DebugAnimationPlayerComponentRenderJointNames,
		g_DebugAnimationPlayerComponentRenderBones
	);
	
	if (g_DebugAnimationPlayerComponentRenderPlayerInfo)
	{
		C_CycleNameLookup cycleNameLookup(*this);
		m_AnimPlayer.DebugRender(debugRenderPtr, worldMatrix, cycleNameLookup);
	}
	
	const Animation::C_Pose& pose = m_EvaluationState->GetPoseReadOnly();
	if (g_DebugAnimationPlayerComponentRenderCustomScalars && pose.GetNumCustomScalars() > 0)
	{
		
		Util::C_String str;
		Util::C_String str2;
		
		{
			S32 beginIndex = Maths::Clamped((S32)g_DebugAnimationPlayerComponentFirstCustomScalarIndex, 0, Maths::Max((S32)pose.GetNumCustomScalars() - (S32)g_DebugAnimationPlayerComponentCustomScalarCount, 0));
			S32 endIndex = Maths::Min(beginIndex + (S32)g_DebugAnimationPlayerComponentCustomScalarCount, (S32) pose.GetNumCustomScalars());
			
			for (S32 i = beginIndex; i < endIndex; ++i)
			{
				str2.Printf("(%d) %s %.2f\n", i, pose.GetFieldName(Animation::E_PoseFieldType_CustomScalar, i).GetString(), (F32)pose.GetCustomScalar(i));
				str += str2;
			}
			debugRenderPtr->Draw3dText(worldMatrix.AW() + Maths::HVecY(4.f), str);
		}
	}

	if (g_DebugAnimationPlayerComponentRenderWind && GetWindParams())
	{
		Animation::C_WindGenerator windGen;

		// If wind is not being evaluated, then run evaluation anyway just to give some visual feedback about how the wind will behave.
		if (!(m_EvaluationState->GetSkeletonInstance().HasConstraints() && m_EvaluationState->GetSkeletonInstance().HasDynamicJoints()))
		{
			m_EvaluationState->GetSkeletonInstance().ApplyWindForces(*GetWindParams(), 1.f / 30.f);
		}
		windGen.SetTime(m_EvaluationState->GetSkeletonInstance().GetWindTime());
		windGen.Update(*GetWindParams(), 0.f);
		Maths::Vector3 f = windGen.GenerateWindForce(0);
		debugRenderPtr->Draw3dArrow(worldMatrix.AW(), Maths::HVec3(f), 0.5f / GetWindParams()->GetWindStrength(), 0.1f, Maths::RGBA(0xff, 0xff, 0x00, 0xff));
	}
	
	debugRenderPtr->SetDepthEnable(true);
}
#endif

void C_AnimationPlayerComponent::OnResourceBagLoaded()
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetActor()->GetMemoryTracker());
#endif

	if (LoadingCycle())
	{
		// Loading a cycle post-priming (rudimentary streaming).
		m_Cycles[m_LoadingCycleIndex].resource = m_pResourceBag->GetResource<Animation::C_CycleResource>(m_Cycles[m_LoadingCycleIndex].resourceTag);
		BindCycle(m_LoadingCycleIndex);
		m_LoadingCycleIndex = -1;
	}
	else
	{
		// Completing the priming phase.
		if (m_pResourceBag->HadErrorsWhilstLoading())
		{
			SendMessageToActor(Actor::E_ActorMsg_Kill);
		}
		else if (GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
		{
			SetPrimed();
		}
	}
}

const Animation::C_Pose& C_AnimationPlayerComponent::GetPose() const
{
	return m_EvaluationState->GetPoseReadOnly();
}
	
const Animation::C_PoseMatrixCache& C_AnimationPlayerComponent::GetPoseMatrixCache() const
{
	return m_EvaluationState->GetPoseMatrixCacheReadOnly();
}
	
const Animation::C_Skeleton* C_AnimationPlayerComponent::GetSkeleton() const
{
	return m_EvaluationState->GetSkeleton();
}

const Animation::C_WindParams* C_AnimationPlayerComponent::GetWindParams() const
{
	if (GetCreationData()->m_WindParams)
	{
		return &m_WindParams;
	}
	return NULL;
}

bool C_AnimationPlayerComponent::IsAnimating() const
{
	return m_State == E_State_Playing;
}

	
const Animation::C_Pose& C_AnimationPlayerComponent::GetPoseNoSync() const
{
	return m_EvaluationState->GetPoseNonBlocking();
}
	
const Animation::C_PoseMatrixCache& C_AnimationPlayerComponent::GetPoseMatrixCacheNoSync() const
{
	return m_EvaluationState->GetPoseMatrixCacheNonBlocking();
}


bool C_AnimationPlayerComponent::InitialiseImpl()
{
	if (!Base::InitialiseImpl())
		return false;
		
	const C_AnimationPlayerCreator* creationData = GetCreationData();

	m_CycleStreaming = creationData->m_LoadCyclesLater;

	if (creationData->m_StartPlaying)
	{
		ChangeState(E_State_Playing);

		// TODO: force load of default cycle?
	}
	else
	{
		ChangeState(E_State_Stopped);
	}
	
	// Setup default cycle in cycle player.
	m_DefaultCycleIndex = -1;
	if (creationData->m_CycleList.count > 0)
	{
		m_DefaultCycleIndex = Maths::Clamped(creationData->m_DefaultCycleIndex, 0, creationData->m_CycleList.count - 1);
	}
		
	for (S32 i = 0; i < creationData->m_CycleList.count; ++i)
	{
		if (creationData->m_CycleList[i].m_Cycle)
		{
			AddCycle(creationData->m_CycleList[i].m_Cycle);
		}
	}
	
	if (m_Cycles.Size() > 0)
	{
		Animation::C_PlaybackParams params;
		params.autoRestart = GetCreationData()->m_AutoRestart;
		params.loopMode = GetCreationData()->m_Looping ? Animation::E_LoopMode_Continuous : Animation::E_LoopMode_Once;
		m_CurrentCycleState = m_AnimPlayer.Play(m_DefaultCycleIndex, 1.f, &params);
	}
	else
	{
		if (!creationData->m_Skeleton)
		{
			// No cycles, no skeleton - cannot continue.
			LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), "%s: No animation cycles or skeleton specified - actor will not be created", GetActor()->GetActorName());
			return false;
		}
	}
		
	return true;
}

bool C_AnimationPlayerComponent::PrimeImpl()
{
	bool primed = Base::PrimeImpl();

	m_pResourceBag->LockListeners();
	
	// Start loading of skeleton.
	m_SkeletonResource = NULL;
	
	if (GetCreationData()->m_Skeleton)
	{
		AddToResourceBag(*m_pResourceBag, MakeResourceBagTag(GetCreationData()->m_Skeleton), GetCreationData()->m_Skeleton);
	}

	// Load cycles.
	LoadAllCycles();

	m_pResourceBag->UnlockListeners();
	
	primed = primed && !m_pResourceBag->IsWaitingForLoad();

	if (primed)
	{
		BIZ_ASSERT(GetLifetimeState() == ActorSys::E_LifetimeState_Primed);
	}

	return primed;
}

bool C_AnimationPlayerComponent::CreateImpl()
{
	if (!Base::CreateImpl())
	{
		return false;
	}

	
	const C_AnimationPlayerCreator* pCreationData = GetCreationData();

	if (pCreationData->m_WindParams)
	{
		pCreationData->m_WindParams->CopyToWindParams(m_WindParams);
	}

	// Set up resource pointers.
	for (U32 i = 0, count = m_Cycles.Size(); i < count; ++i)
	{
		m_Cycles[i].resource = m_pResourceBag->GetResource<Animation::C_CycleResource>(m_Cycles[i].resourceTag);
	}
	m_SkeletonResource = m_pResourceBag->GetResource<Animation::C_SkeletonResource>(MakeResourceBagTag(pCreationData->m_Skeleton));

	// Build pose format from skeleton and cycles. 
	m_PoseFormat.Clear();
	
	const Animation::C_Skeleton* skeleton = NULL;
	
	if (m_SkeletonResource != NULL)
	{
		skeleton = m_SkeletonResource->GetSkeleton();
		m_PoseFormat.SetSkeleton(skeleton);
	}
	
	Animation::C_PoseFieldMask fieldMask;
	
	if (skeleton != NULL)
	{
		// If we have a skeleton it fully describes all joint transform fields and we can ignore those
		// in the cycles.
		fieldMask.Disable(Animation::E_PoseFieldType_JointTransform);
	}
	
	for (U32 i = 0, count = m_Cycles.Size(); i < count; ++i)
	{
		const CycleInfo& c = m_Cycles[i];
		if (c.resource != NULL)
		{
			m_PoseFormat.AddCycleFields(c.resource->GetCycle(), fieldMask);
			
			if (skeleton == NULL)
			{
				// First world space cycle will contain all the joint transform fields required.
				fieldMask.Disable(Animation::E_PoseFieldType_JointTransform);
			}
		}
	}
	
	// Initialise evaluation state using this pose format.
	m_EvaluationState = Animation::C_EvaluationState::New(16);
	m_EvaluationState->Create(m_PoseFormat, skeleton);
	
	// Ensure cycle instances have been constructed for all cycles.
	BindAllCycles();

	if (m_EvaluationState == NULL)
	{
		LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR " Failed to create Animation Player Component, resources didn't load");
		return false;
	}

	// Set durations for active cycles.
	{
		// Update animation player as we need its active cycle list.
		m_AnimPlayer.Update(0.f);
		
		for (U32 i = 0, count = m_AnimPlayer.GetNumActiveCycles(); i < count; ++i)
		{
			Animation::C_CycleStateHandle cs = m_AnimPlayer.GetActiveCycleState(i);
			U32 cycleIndex = cs->GetCycleId();
			F32 duration = m_Cycles[cycleIndex].instance.GetDuration();
			cs->SetCycleDuration(duration);
		}
	}
	
	// Evaluate first frame of animation, even if paused to ensure the current evaluation
	// state contains sensible data.
	Evaluate(0.f);
	
	m_FirstUpdate = true;
	
	return true;
}

U32 C_AnimationPlayerComponent::AddCycle(const XtHandle<ResourceUtils::C_XtCycleResourceDef>& resourceDef)
{
	for (U32 i = 0, count = m_Cycles.Size(); i < count; ++i)
	{
		if (m_Cycles[i].resourceDef == resourceDef)
		{
			return i;
		}
	}
	U32 index = m_Cycles.Size();
	m_Cycles.PushBack();
	m_Cycles.Back().resourceDef = resourceDef;
	m_Cycles.Back().resource = NULL;
	m_Cycles.Back().instance.Clear();
	return index;
}

void C_AnimationPlayerComponent::LoadAllCycles()
{
	bool headerOnly = CycleStreaming();
	for (U32 i = 0, count = m_Cycles.Size(); i < count; ++i)
	{
		LoadCycle(i, headerOnly);
	}
}

void C_AnimationPlayerComponent::LoadCycle(U32 cycleIndex, bool headerOnly)
{
	CycleInfo& c = m_Cycles[cycleIndex];
	
	c.resource = NULL;
	
	if (c.resourceDef == NULL)
	{
		return;
	}

	Sys::C_FilePath filename(c.resourceDef->GetFileName());

	if (headerOnly)
	{
		// Just load cycle headers to save memory. Full cycles will be loaded when "LoadCycle" message is received.
		filename += Animation::C_CycleResource::kHeaderOnlySuffix;
		c.resourceTag = MakeResourceBagTag(filename.Chars());
	}
	else
	{
		// Load full cycles.
		c.resourceTag = MakeResourceBagTag(c.resourceDef);
	}

	m_pResourceBag->AddResource(c.resourceTag, filename, Animation::C_CycleResource::StaticGetResourceTypeId());
}

void C_AnimationPlayerComponent::BindAllCycles()
{
	for (U32 i = 0, count = m_Cycles.Size(); i < count; ++i)
	{
		BindCycle(i);
	}
}

void C_AnimationPlayerComponent::BindCycle(U32 cycleIndex)
{
	CycleInfo& c = m_Cycles[cycleIndex];
	
	if (c.resource != NULL)
	{
		if (c.resource->IsDataReady())
		{
			const Animation::C_Cycle* cycle = c.resource->GetCycle();
			if (cycle != NULL)
			{
				c.instance.Create(cycle, &m_EvaluationState->GetPose());
			}
			else
			{
				c.instance.Clear();
			}
		}
	}
}

void C_AnimationPlayerComponent::Restart()
{
	m_Time = 0.f;
	m_AnimPlayer.Stop();
	
	if (m_Cycles.Size() > 0)
	{
		Animation::C_PlaybackParams params;
		params.autoRestart = GetCreationData()->m_AutoRestart;
		params.loopMode = GetCreationData()->m_Looping ? Animation::E_LoopMode_Continuous : Animation::E_LoopMode_Once;
		params.autoRestart = GetCreationData()->m_AutoRestart;
		m_CurrentCycleState = m_AnimPlayer.Play(m_DefaultCycleIndex, m_Cycles[m_DefaultCycleIndex].instance.GetDuration(), &params);
	}
}

void C_AnimationPlayerComponent::ChangeState(E_State newState)
{
	if (newState == m_State) return;
	
	switch(newState)
	{
		case E_State_Stopped:
			RemoveSubscriber(GameMessaging::E_GameMsg_PreUpdate);
			break;
		case E_State_Playing:
			AddSubscriber(GameMessaging::E_GameMsg_PreUpdate);
			break;
		case E_State_Finished:
			RemoveSubscriber(GameMessaging::E_GameMsg_PreUpdate);
			break;
	}
	m_State = newState;
}

void C_AnimationPlayerComponent::Evaluate(F32 deltaTime)
{
	m_AnimPlayer.Update(deltaTime);

	if (m_AnimPlayer.IsPaused() && !m_FirstUpdate)
	{
		if (!m_EvaluationState->GetSkeletonInstance().HasDynamicJoints())
		{
			// No dynamic joints, so no change in animation state.
			return;
		}

		if (!GetCreationData()->m_WindParams)
		{
			// Dynamic joints, but no wind - therefore animation state will be unchanged.
			return;
		}

		// at this point, the animation will be paused but wind will be influencing the dynamic joints.
	}

	Animation::C_CommandList& cmdList = m_EvaluationState->GetCommandList();

	cmdList.Clear();
	for (U32 i = 0, count = m_AnimPlayer.GetNumActiveCycles(); i < count; ++i)
	{
		Animation::C_CycleStateHandle cs = m_AnimPlayer.GetActiveCycleState(i);
		U32 cycleIndex = cs->GetCycleId();
		
		// Make sure cycle duration is up to date.
		F32 duration = m_Cycles[cycleIndex].instance.GetDuration();
		cs->SetCycleDuration(duration);
		
		cmdList.BlendCycle(
			cs->GetBlendMode(),
			&m_Cycles[cycleIndex].instance,
			cs->GetTime(),
			cs->GetWeight(),
			cs->GetLoopMode()
		);
	}

	const Animation::C_Skeleton* skeleton = NULL;
	if (m_SkeletonResource != NULL)
	{
		skeleton = m_SkeletonResource->GetSkeleton();
	}

	Maths::HMat43 worldMatrix;
	if (GetActor())
	{
		worldMatrix = GetActor()->GetMatrix();
	}

	if (skeleton != NULL)
	{
		Animation::C_SkeletonInstance& skeletonInst = m_EvaluationState->GetSkeletonInstance();

		if (skeletonInst.HasConstraints())
		{
			if (skeletonInst.HasDynamicJoints())
			{
				skeletonInst.UpdateDynamicJoints(deltaTime);

				const C_AnimationPlayerCreator* pCreationData = GetCreationData();

				if (pCreationData->m_WindParams)
				{
					#ifndef CFG_SPEED
					pCreationData->m_WindParams->CopyToWindParams(m_WindParams);
					#endif
					cmdList.ApplyWindForces(m_WindParams, deltaTime);
				}
			}
			cmdList.ApplyConstraints(m_EvaluationState->GetSkeletonInstance(), worldMatrix);
		}
	}			

	m_EvaluationState->ScheduleEvaluationTask();

	m_FirstUpdate = false;
}

void C_AnimationPlayerComponent::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << U32(m_Cycles.Size());
	for (U32 i = 0, count = m_Cycles.Size(); i < count; ++i)
	{
		stream << m_Cycles[i].resourceDef;
	}

	stream << m_DefaultCycleIndex;
	stream << m_AnimPlayer;
	stream << m_AnimPlayer.GetCycleStateId(m_CurrentCycleState);
	stream << m_Time;
	stream << U32(m_State);
}

void C_AnimationPlayerComponent::Deserialise(Sys::C_ContextualStream& stream)
{
	m_Cycles.Clear();
	U32 numCycles = 0;
	stream >> numCycles;
	m_Cycles.Resize(numCycles);
	for (U32 i = 0; i < numCycles; ++i)
	{
		stream >> m_Cycles[i].resourceDef;
		m_Cycles[i].resource = NULL;
	}

	stream >> m_DefaultCycleIndex;
	stream >> m_AnimPlayer;
	S32 cycleStateId = -1;
	stream >> cycleStateId;
	m_CurrentCycleState = m_AnimPlayer.GetCycleStateFromId(cycleStateId);
	stream >> m_Time;
	U32 state = 0;
	stream >> state;
	BIZ_ASSERT(m_State == E_State_Uninitialised);	// making the assumption that this component has been recreated, not deserialised in place.
	ChangeState(static_cast<E_State>(state));
}

//
// C_AnimatedCollisionShapes
//

class C_AnimatedCollisionShapes
{
public:
	enum E_State
	{
		E_State_Inactive,
		E_State_Dynamic,
		E_State_Static
	};

	C_AnimatedCollisionShapes(Gravity::C_Scene* physicsScene, Gravity::C_ExternalConstants* externalConstants);
	
	~C_AnimatedCollisionShapes();

	void ImportShapesFromModel(const GameResource::C_Model& model, const Animation::C_PoseFormat& poseFormat);

	void Update(const Animation::C_PoseMatrixCache& poseMatrixCache, const Maths::Mat43& worldMatrix, F32 deltaTime);
	
	void ChangeState(E_State state);
	E_State GetState() const;
	
	void SetActive(bool active);
	
	bool IsActive() const;
	
	U32 GetNumShapes() const;

private:

	void AddShape(const Gravity::C_ImportShape* importShape, S32 transformIndex);
	

	struct Shape
	{
		Shape(): body(NULL), transformIndex(-1), kinematicController(NULL) {}
		
		Gravity::C_Body*	body;
		S32					transformIndex;
		Gravity::C_KinematicController*	kinematicController;
	};
	
	Gravity::C_Scene*				m_Scene;
	Gravity::C_ExternalConstants*	m_ExternalConstants;
	Btl::Vector<Shape>				m_Shapes;
	E_State							m_State;
	bool							m_FirstUpdate;
};

C_AnimatedCollisionShapes::C_AnimatedCollisionShapes(Gravity::C_Scene* physicsScene, Gravity::C_ExternalConstants* externalConstants):
	m_Scene(physicsScene),
	m_ExternalConstants(externalConstants),
	m_State(E_State_Inactive),
	m_FirstUpdate(true)
{
}

C_AnimatedCollisionShapes::~C_AnimatedCollisionShapes()
{
	for (U32 i = 0, numShapes = m_Shapes.Size(); i < numShapes; ++i)
	{
		Shape& shape = m_Shapes[i];
		if (shape.body->GetScene() != NULL)
		{
			m_Scene->RemoveBody(shape.body);
		}
		m_Scene->RemoveKinematicController(shape.kinematicController);
		delete shape.kinematicController;
	}
}

void C_AnimatedCollisionShapes::ImportShapesFromModel(const GameResource::C_Model& model, const Animation::C_PoseFormat& poseFormat)
{
	for (U32 i = 0, numElements = model.GetElementCount(); i < numElements; ++i)
	{
		const GameResource::C_ModelElement* element = model.GetElement(i);
		
		if (element != NULL)
		{
			S32 transformIndex = poseFormat.FindFieldByName(Animation::E_PoseFieldType_JointTransform, element->GetName().Chars());
			AddShape(element->GetPhysicsImportShape(), transformIndex);
		}
	}
}

void C_AnimatedCollisionShapes::AddShape(const Gravity::C_ImportShape* importShape, S32 transformIndex)
{
	if (importShape == NULL || transformIndex < 0) return;
	
	Gravity::C_ShapeDesc* desc = importShape->GetShapeDesc();
	if (desc == NULL) return;
	
	m_Shapes.PushBack();
	Shape& shape = m_Shapes.Back();
	
	shape.body = m_Scene->CreateBody();
	
	shape.transformIndex = transformIndex;
	
	// Remap around centre of mass
	desc->RemapShape();
	desc->SetRemapped();
	
	Gravity::C_BodyHelper bodyHelper(desc);
	Maths::HMat43 matrix;
	Collision::C_CollisionFilter filter;
	bodyHelper.CreateBody(1.f, matrix.AW(), matrix.Rotation(), shape.body, &filter);
	
	shape.body->SetExternalConstants(m_ExternalConstants);
	shape.body->Deactivate();
	shape.body->ElementMatrix(matrix);

	shape.kinematicController = new Gravity::C_KinematicController(shape.body);
	m_Scene->AddKinematicController(shape.kinematicController);
	
	shape.body->MakeKinematic();
	
	if (m_State != E_State_Inactive)
	{
		m_Scene->AddBody(shape.body);
	}
	
	m_FirstUpdate = true;
}

void C_AnimatedCollisionShapes::ChangeState(E_State state)
{
	if (state != m_State)
	{
		switch (state)
		{
			case E_State_Inactive:
			{
				for (U32 i = 0, numShapes = m_Shapes.Size(); i < numShapes; ++i)
				{
					// Ensure shapes are not in the scene.
					if (m_Shapes[i].body->GetScene() != NULL)
					{
						m_Scene->TemporarilyRemoveBody(m_Shapes[i].body);
					}
				}
				break;
			}
				
			case E_State_Dynamic:
			{
				for (U32 i = 0, numShapes = m_Shapes.Size(); i < numShapes; ++i)
				{
					// Ensure shape is in the scene.
					if (m_Shapes[i].body->GetScene() == NULL)
					{
						m_Scene->AddBody(m_Shapes[i].body);
					}
					m_Shapes[i].body->MakeKinematic();
				}

				break;
			}
				
			case E_State_Static:
			{
				for (U32 i = 0, numShapes = m_Shapes.Size(); i < numShapes; ++i)
				{
					// Ensure shape is in the scene.
					if (m_Shapes[i].body->GetScene() == NULL)
					{
						m_Scene->AddBody(m_Shapes[i].body);
					}
					m_Shapes[i].kinematicController->Stop();
				}
				break;
			}
				
			default:
				BIZ_ASSERT_FAIL("Invalid AnimatedCollisionShape state");
				break;
		}
		m_State = state;
	}
}

C_AnimatedCollisionShapes::E_State C_AnimatedCollisionShapes::GetState() const
{
	return m_State;
}

void C_AnimatedCollisionShapes::Update(const Animation::C_PoseMatrixCache& poseMatrixCache, const Maths::Mat43& worldMatrix, F32 deltaTime)
{
	if (m_State == E_State_Inactive)
	{
		return;
	}
	
	const Maths::Mat43* matrices = poseMatrixCache.GetSkinningMatrices();

	if (deltaTime <= 0.f)
	{
		m_FirstUpdate = true;
	}
	
	const Maths::HF32 dt(deltaTime);
	for (U32 i = 0, numShapes = m_Shapes.Size(); i < numShapes; ++i)
	{
		Shape& shape = m_Shapes[i];
		
		Maths::HMat43 elementMatrix(matrices[shape.transformIndex] * worldMatrix);
		
		if (m_FirstUpdate)
		{
			shape.kinematicController->Teleport(elementMatrix);
		}
		else
		{
			shape.kinematicController->SetTargetTransformation(elementMatrix, dt);
		}
	}
	m_FirstUpdate = false;
}

void C_AnimatedCollisionShapes::SetActive(bool active)
{
	ChangeState(active ? E_State_Dynamic : E_State_Static);
}

bool C_AnimatedCollisionShapes::IsActive() const
{
	return m_State == E_State_Dynamic || (m_State == E_State_Static && m_FirstUpdate);
}

U32 C_AnimatedCollisionShapes::GetNumShapes() const
{
	return m_Shapes.Size();
}


//
// C_AnimatedModelComponent
//

S32	C_AnimatedModelComponent::s_NoMotionBlurOpaqueRenderPass = -1;
S32	C_AnimatedModelComponent::s_NoMotionBlurAlphaTestRenderPass = -1;
S32	C_AnimatedModelComponent::s_SolidRenderPass = -1;
S32	C_AnimatedModelComponent::s_PunchRenderPass = -1;
S32	C_AnimatedModelComponent::s_AlphaRenderPass = -1;

void C_AnimatedModelComponent::SetNoMotionBlurRenderPasses(S32 opaqueRenderPass, S32 alphaTestRenderPass)
{
	s_NoMotionBlurOpaqueRenderPass = opaqueRenderPass;
	s_NoMotionBlurAlphaTestRenderPass = alphaTestRenderPass;
}

void C_AnimatedModelComponent::SetSolidRenderPass(S32 solidRenderPass)
{
	s_SolidRenderPass = solidRenderPass;
}

void C_AnimatedModelComponent::SetPunchRenderPass(S32 punchRenderPass)
{
	s_PunchRenderPass = punchRenderPass;
}

void C_AnimatedModelComponent::SetAlphaRenderPass(S32 alphaRenderPass)
{
	s_AlphaRenderPass = alphaRenderPass;
}

C_AnimatedModelComponent::C_AnimatedModelComponent():
	m_AnimatedCollisionShapes(NULL),
	m_pResourceBag(NULL),
	m_Visible(false),
	m_Selected(false)
{
	m_pResourceBag = new C_SimpleResourceBag(this, &C_AnimatedModelComponent::OnResourceBagLoaded);
}

C_AnimatedModelComponent::~C_AnimatedModelComponent()
{
	delete m_AnimatedCollisionShapes;
	m_AnimatedCollisionShapes = NULL;
	
	DestroyModels();

	delete m_pResourceBag;
	m_pResourceBag = NULL;
}

bool C_AnimatedModelComponent::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	PERF_EVENT("C_AnimatedModelComponent::MessageHandler");
	switch(pMessage->Type())
	{
		// TODO: snooping messages here to figure out when to activate
		// maybe anim player should send out messages to say that its beginning to animate?
		case ActorMsg::E_AnimatedObjectMsg_PlayCycle:
		case ActorMsg::E_GameActorMsg_ActionBegin:
		case ActorMsg::E_GameActorMsg_Activate:
			ChangeState(E_State_Active, true);
			SetVisible(true);
			break;	

		// Leave deactivation to the result of polling I_AnimationStateComponent
		//case ActorMsg::E_GameActorMsg_Deactivate:
		//	ChangeState(E_State_Inactive);
		//	break;
		
		case ActorMsg::E_AnimatedObjectMsg_SetController:
		{
			m_ControllerHandle = Messaging::MessageCast<ActorMsg::C_AnimatedObjectSetControllerMsg>(pMessage)->GetActorHandle();
			break;
		}

		case ActorMsg::E_GameActorMsg_SetTransform:
		{
			const ActorMsg::C_ActorSetTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage);

			// Update with new world matrix.
			UpdateFromAnimationPlayer(msg->Transform().Matrix(), 0.f);
			break;
		}
		
		case ActorMsg::E_GameActorMsg_AnimateTransform:
		{
			const ActorMsg::C_ActorAnimateTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage);
			
			// Update with new world matrix.
			UpdateFromAnimationPlayer(msg->Transform().Matrix(), msg->m_TimeStep);
			break;
		}
	
		case GameMessaging::E_GameMsg_Update:
		{
			F32 deltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();

			if (ShouldUpdateDuringUpdateMessage())
			{
				UpdateFromAnimationPlayer(GetActor()->GetMatrix(), deltaTime);
			}

			break;
		}
		
#ifndef CFG_GOLD
		case E_RenderMsg_RequestResourceUsage:
		{
			if (m_Visible && (m_State == E_State_Active))
			{
				const C_RenderNotifyResourceUsage msg(CalcWorldSphere(), true, GetActor()->GetHandle(), GetInterfaceID());
				GetActorList()->BroadcastMessage(&msg);
			}
			break;
		}

		case E_RenderMsg_RequestResourceTotal:
		{
			C_ModelResourceUsage resourceUsage;
			AccumulateResourceUsage(resourceUsage);
			
			const C_RenderNotifyResourceTotalMsg msg(CalcWorldSphere().Centre(), resourceUsage.CalcTotalGraphicsBytes(), 0);
			GetActorList()->BroadcastMessage(&msg);

			break;
		}

		case E_RenderMsg_DrawResourceUsage:
		{
			C_ModelResourceUsage resourceUsage;
			AccumulateResourceUsage(resourceUsage);

			const C_RenderDrawResourceUsageMsg* pMsg = Messaging::MessageCast<C_RenderDrawResourceUsageMsg>(pMessage);

			if (pMsg->GetDebugRender() != NULL)
			{
				resourceUsage.DebugRender(*pMsg->GetDebugRender());
			}

			break;
		}

		case E_RenderMsg_DumpResourceUsage:
		{
			C_ModelResourceUsage resourceUsage;
			AccumulateResourceUsage(resourceUsage);

			const C_RenderDumpResourceUsageMsg* pMsg = Messaging::MessageCast<C_RenderDumpResourceUsageMsg>(pMessage);

			if (pMsg->GetDebugLogger() != NULL)
			{
				resourceUsage.DebugDump(*pMsg->GetDebugLogger());
			}

			if (pMsg->GetFile() != NULL)
			{
				resourceUsage.DebugDump(*pMsg->GetFile());
			}

			break;
		}
#endif // CFG_GOLD

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

		default:
			return Actor::I_ActorComponent::OnMessage(pMessage);
	}
	return true;
}

void C_AnimatedModelComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Base::SerialiseForSave(stream);
	Serialise(stream);
}

void C_AnimatedModelComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Base::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_AnimatedModelComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Base::SerialiseForSync(stream);
	Serialise(stream);
}

void C_AnimatedModelComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Base::DeserialiseForSync(stream);
	Deserialise(stream);
}

const Components::I_Movement* C_AnimatedModelComponent::GetActorMovementComponent() const
{
	BIZ_ASSERT(GetActor() != NULL);
	return GetActor()->QueryInterface<Components::I_Movement>();
}

bool C_AnimatedModelComponent::ShouldUpdateDuringUpdateMessage() const
{
	const Components::I_Movement* pMovementComponent = GetActorMovementComponent();
	if (!pMovementComponent)
	{
		return true;
	}

	if (pMovementComponent->IsAnimating() && pMovementComponent->IsEnabled())
	{
		// Defer model update until the movement component has set our world matrix
		// via AnimateTransform or SetTransform messages.
		return false;
	}
	return true;
}

#ifndef CFG_GOLD
DECLARE_DEBUG_BOOL(g_DebugAnimatedModelComponentRenderDebug, "ActorDebug.Components.AnimatedModel.DebugRender.Enable", false);
DECLARE_DEBUG_BOOL(g_DebugAnimatedModelComponentRenderIfSelected, "ActorDebug.Components.AnimatedModel.DebugRender.OnlyIfSelectedInActorStudio", false);
DECLARE_DEBUG_BOOL(g_DebugAnimatedModelComponentRenderJointNames, "ActorDebug.Components.AnimatedModel.DebugRender.ShowLocalJointNames", false);
DECLARE_DEBUG_BOOL(g_DebugAnimatedModelComponentRenderJointTransforms, "ActorDebug.Components.AnimatedModel.DebugRender.ShowLocalJointTransforms", false);
DECLARE_DEBUG_BOOL(g_DebugAnimatedModelComponentRenderBones, "ActorDebug.Components.AnimatedModel.DebugRender.ShowLocalBones", false);
DECLARE_DEBUG_BOOL(g_DebugAnimatedModelComponentRenderLightSamplerPosition, "ActorDebug.Components.AnimatedModel.DebugRender.ShowLightSamplerPosition", false);

void C_AnimatedModelComponent::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if(!g_DebugAnimatedModelComponentRenderDebug)
		return;

	if (g_DebugAnimatedModelComponentRenderIfSelected && !m_Selected)
		return;
		
	Maths::HMat43 worldMatrix;
	if (GetActor())
	{
		worldMatrix = GetActor()->GetMatrix();
	}
			
	debugRenderPtr->Draw3dMatrix(worldMatrix, 0.2f);
	
	debugRenderPtr->SetDepthEnable(false);
	
	for (U32 i = 0, count = m_ModelInstances.Size(); i < count; ++i)
	{
		const ModelInstance& modelInstance = m_ModelInstances[i];
	
		if (g_DebugAnimatedModelComponentRenderLightSamplerPosition)
		{
			if (modelInstance.envSampler != NULL)
			{
				debugRenderPtr->Draw3dCross(modelInstance.envSampler->GetPosition(), 0.5f, Maths::RGBA(0x20, 0xff, 0x20));
			}
		}
		
		if (modelInstance.evaluationState != NULL)
		{
			Animation::DebugRenderPoseTransforms(
				debugRenderPtr,
				modelInstance.evaluationState->GetPoseReadOnly(),
				modelInstance.evaluationState->GetSkeleton(),
				worldMatrix,
				g_DebugAnimatedModelComponentRenderJointTransforms,
				g_DebugAnimatedModelComponentRenderJointNames,
				g_DebugAnimatedModelComponentRenderBones
			);
		}
	}
	
	debugRenderPtr->SetDepthEnable(true);
}

Maths::C_Sphere C_AnimatedModelComponent::CalcWorldSphere() const
{
	Maths::C_Sphere worldSphere(Maths::Constants<Maths::HVec3>::Zero(), Maths::Constants<Maths::HF32>::Zero());

	if (!m_ModelInstances.Empty())
	{
		Btl::Vector<ModelInstance>::ConstIterator it = m_ModelInstances.Begin();
		worldSphere = it->animatedModelInstance.GetModelInstance()->GetWorldSphere();

		for (++it; it != m_ModelInstances.End(); ++it)
		{
			const Maths::C_Sphere otherSphere = it->animatedModelInstance.GetModelInstance()->GetWorldSphere();

			const Maths::HVec3 D(otherSphere.Centre() - worldSphere.Centre()); // vector from centre of first sphere to centre of second sphere
			const Maths::HF32 d(D.Length()); // distance between centres of spheres

			if ((d + otherSphere.Radius()) <= worldSphere.Radius())
			{
				// first sphere contains second sphere
			}
			else if ((d + worldSphere.Radius()) <= otherSphere.Radius())
			{
				// second sphere contains first sphere
				worldSphere = otherSphere;
			}
			else
			{
				// calculate the centre and radius of the smallest sphere that contains both
				const Maths::C_Vertex Cn(worldSphere.Centre() + (D * ((d - worldSphere.Radius() + otherSphere.Radius()) / (d * Maths::Constants<Maths::HF32>::Two()))));
				const Maths::HF32 rn((d + worldSphere.Radius() + otherSphere.Radius()) * Maths::Constants<Maths::HF32>::Half());
				worldSphere = Maths::C_Sphere(Cn, rn);
			}
		}
	}

	return worldSphere;
}

void C_AnimatedModelComponent::AccumulateResourceUsage(C_ModelResourceUsage& ioResourceUsage) const
{
	for (Btl::Vector<ModelInstance>::ConstIterator i = m_ModelInstances.Begin(); i != m_ModelInstances.End(); ++i)
	{
		for (Btl::Vector<ModelPart>::ConstIterator j = i->modelParts.Begin(); j != i->modelParts.End(); ++j)
		{
			if ((j->modelResource != NULL) && (j->modelResource->GetRenderModelData() != NULL))
			{
				ioResourceUsage.AddModelResource(j->modelResource->GetRenderModelData(), j->modelIndex);
			}
		}
	}
}
#endif


bool C_AnimatedModelComponent::InitialiseImpl()
{
	if (!Base::InitialiseImpl())
		return false;
		
	m_Visible = !GetCreationData()->m_StartHidden;
	m_FirstUpdate = true;
	m_ControllerHandle = GetActorHandle();
	m_State = E_State_Uninitialised;
	ChangeState(E_State_Inactive, false);
	
	m_CollisionState = C_AnimatedCollisionShapes::E_State_Inactive;

	return true;
}

void C_AnimatedModelComponent::OnResourceBagLoaded()
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetActor()->GetMemoryTracker());
#endif

	if (m_pResourceBag->HadErrorsWhilstLoading())
	{
		SendMessageToActor(Actor::E_ActorMsg_Kill);
	}
	else if (GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
	{
		SetPrimed();
	}
}

#if 0

void C_AnimatedModelComponent::ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData)
{
	switch (result)
	{
		case Resource::E_Notify_NoData:
			LOGERROR(Actor::GetDebugLogAsset(pResource->GetResourceFilename()), DBSTR "Missing resource '%s'", pResource->GetResourceFilename().Chars());
			
			if (GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
			{
				SendMessageToActor(Actor::E_ActorMsg_Kill);
			}
			break;
		
		case Resource::E_Notify_DataReady:
		{
			break;
		}
		
		case Resource::E_Notify_DataError:	
			LOGERROR(Actor::GetDebugLogAsset(pResource->GetResourceFilename()), DBSTR "Data error loading resource '%s'", pResource->GetResourceFilename().Chars());
			
			if(GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
			{
				// Abort!
				SendMessageToActor(Actor::E_ActorMsg_Kill);
			}
			break;
			
		default:
			break;
	}
	
	if (result == Resource::E_Notify_DataReady && GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
	{
		m_NumPendingResources--;
		BIZ_ASSERT(m_NumPendingResources >= 0);
		if (m_NumPendingResources == 0)
		{
			for (U32 i = 0, count = m_ModelInstances.Size(); i < count; ++i)
			{
				ModelInstance& modelInstance = m_ModelInstances[i];
				
				modelInstance.skeletonResource = modelInstance.skeletonResourceHandle;
				if (modelInstance.skeletonResource != NULL)
				{
					if (modelInstance.skeletonResource->IsDataReady())
					{
						modelInstance.skeleton = modelInstance.skeletonResource->GetSkeleton();
					}
					else
					{
						modelInstance.skeletonResource = NULL;
					}
				}
				
				for (U32 j = 0, partCount = modelInstance.modelParts.Size(); j < partCount; ++j)
				{
					ModelPart& modelPart = modelInstance.modelParts[j];
					
					modelPart.modelResource = modelPart.modelResourceHandle;
					if (modelPart.modelResource != NULL)
					{
						if (!modelPart.modelResource->IsDataReady())
						{
							modelPart.modelResource = NULL;
						}
					}
				}
			}

			SetPrimed();
			
			//LOGINFO(Actor::GetDebugLogGeneral(), DBSTR "Actor '%s' models are primed", GetActor()->GetActorName());
		}
	}
}
#endif

const GameResource::C_ModelElement* C_AnimatedModelComponent::FindModelElement(const CHAR8* elementName) const
{
	Util::C_String name(elementName);
	for (U32 i = 0, count = m_ModelInstances.Size(); i < count; ++i)
	{
		const ModelInstance& m = m_ModelInstances[i];
		
		for (U32 j = 0, numParts = m.modelParts.Size(); j < numParts; ++j)
		{
			if (m.modelParts[j].modelResource != NULL)
			{
				const GameResource::C_ModelData* modelData = m.modelParts[j].modelResource->GetModelData();
				BIZ_ASSERT(modelData);
				
				for (S32 k = 0; k < modelData->GetModelCount(); ++k)
				{
					const GameResource::C_Model* model = modelData->GetModel(k);
					BIZ_ASSERT(model);
					
					const GameResource::C_ModelElement* element = model->GetElement(name);
					if (element != NULL)
					{
						return element;
					}
				}
			}
		}
	}
	return NULL;
}

bool C_AnimatedModelComponent::PrimeImpl()
{
	bool primed = Base::PrimeImpl();

	m_pResourceBag->LockListeners();

	const C_AnimatedModelCreator* creation = GetCreationData();
	m_ModelInstances.Clear();

	// Add valid model instances to local models array.
	for (U32 i = 0, count = creation->m_ModelInstances.count; i < count; ++i)
	{
		const C_AnimatedModelCreator::ModelInstance& modelInstanceDef = creation->m_ModelInstances[i];
	
		if (modelInstanceDef.modelParts.count == 0)
		{
			continue;
		}

		// Check model parts all reference valid models.
		U32 numValidParts = 0;
		for (U32 partIndex = 0, numParts = modelInstanceDef.modelParts.count; partIndex < numParts; ++partIndex)
		{
			if (modelInstanceDef.modelParts[partIndex].model)
			{
				++numValidParts;
			}
		}
		
		if (0 == numValidParts)
		{
			LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), "%s: model instance number %d has no valid model parts", GetActor()->GetActorName(), i);
			continue;
		}

		// Create model instance.
		m_ModelInstances.PushBack();
		ModelInstance& modelInst = m_ModelInstances.Back();
		modelInst.creationIndex = i;
		modelInst.namespacePrefix = NULL;
		modelInst.motionBlurDisabled = modelInstanceDef.disableMotionBlur;
		modelInst.enableScale = modelInstanceDef.enableScale;
		
		if (*(modelInstanceDef.namespacePrefix.Text()) != '\0')
		{
			modelInst.namespacePrefix = modelInstanceDef.namespacePrefix.Text();
		}
		
		F32 nearDrawDistance = 0.f;
		F32 farDrawDistance = modelInstanceDef.drawDistance;
		
		if (farDrawDistance <= 0.f)
		{
			farDrawDistance = 250.f;
		}
		
		modelInst.animatedModelInstance.SetNearFarDistance(nearDrawDistance, farDrawDistance);
		
		// Set up light sampler position method.
		if (modelInstanceDef.lightSamplerPosition != NULL)
		{
			modelInst.lightSamplerPositionType = modelInstanceDef.lightSamplerPosition->GetType();
			
			switch (modelInst.lightSamplerPositionType)
			{
				case C_AnimatedModelCreator::E_LightSamplerPositionType_Actor:
				{
					const C_AnimatedModelCreator::ActorLsp* actorLsp = static_cast<const C_AnimatedModelCreator::ActorLsp*>(modelInstanceDef.lightSamplerPosition);
					modelInst.lightSamplerOffset = Maths::MakePoint3(actorLsp->localOffset.vx, actorLsp->localOffset.vy, actorLsp->localOffset.vz);
					break;
				}
				
				case C_AnimatedModelCreator::E_LightSamplerPositionType_BoundingSphereCentre:
					break;
				
				case C_AnimatedModelCreator::E_LightSamplerPositionType_Bone:
				{
					const C_AnimatedModelCreator::BoneLsp* boneLsp = static_cast<const C_AnimatedModelCreator::BoneLsp*>(modelInstanceDef.lightSamplerPosition);
					modelInst.lightSamplerBoneName = boneLsp->boneName;
					modelInst.lightSamplerBoneIndex = -1;
					break;
				}

				case C_AnimatedModelCreator::E_LightSamplerPositionType_WorldPoint:
				{
					const C_AnimatedModelCreator::WorldPointLsp* lsp = static_cast<const C_AnimatedModelCreator::WorldPointLsp*>(modelInstanceDef.lightSamplerPosition);
					modelInst.lightSamplerOffset = Maths::MakePoint3(lsp->worldPoint.vx, lsp->worldPoint.vy, lsp->worldPoint.vz);
					break;
				}
				
				default:
				break;
			}
		}
		else
		{
			modelInst.lightSamplerPositionType = C_AnimatedModelCreator::E_LightSamplerPositionType_Actor;
			modelInst.lightSamplerOffset = Maths::Const<Maths::Point3>::Zero();
		}

		modelInst.m_xtDropShadowList = modelInstanceDef.dropShadows;

		// Add model parts (1:1 mapping with parts in Sorcery definition)
		modelInst.modelParts.Resize(modelInstanceDef.modelParts.count);
		for (U32 j = 0, partCount = modelInstanceDef.modelParts.count; j < partCount; ++j)
		{
			modelInst.modelParts[j].modelIndex = modelInstanceDef.modelParts[j].modelIndex;
		}
	}	
		
	for (U32 i = 0, count = m_ModelInstances.Size(); i < count; ++i)
	{
		ModelInstance& modelInst = m_ModelInstances[i];
		
		modelInst.skeletonResource = NULL;
		modelInst.skeleton = NULL;
		
		const C_AnimatedModelCreator::ModelInstance& modelInstanceDef = creation->m_ModelInstances[modelInst.creationIndex];
			
		// Load skeleton, if there is one.
		if (modelInstanceDef.skeleton)
		{
			AddToResourceBag(*m_pResourceBag, MakeResourceBagTag(modelInstanceDef.skeleton), modelInstanceDef.skeleton);
		}
		
		// Load models.
		for (U32 j = 0, partCount = modelInst.modelParts.Size(); j < partCount; ++j)
		{

			modelInst.modelParts[j].modelResource = NULL;
			if (modelInstanceDef.modelParts[j].model)
			{
				AddToResourceBag(*m_pResourceBag, MakeResourceBagTag(modelInstanceDef.modelParts[j].model), modelInstanceDef.modelParts[j].model);
			}
		}
	}

	m_pResourceBag->UnlockListeners();
	
	primed = primed && !m_pResourceBag->IsWaitingForLoad();

	if (primed)
	{
		BIZ_ASSERT(GetLifetimeState() == ActorSys::E_LifetimeState_Primed);
	}
	
	return primed; 
}

bool C_AnimatedModelComponent::CreateImpl()
{
	if (!Base::CreateImpl())
	{
		return false;
	}

	for (U32 i = 0, count = m_ModelInstances.Size(); i < count; ++i)
	{
		ModelInstance& modelInstance = m_ModelInstances[i];

		XtHandle<ResourceUtils::C_XtSkeletonResourceDef> skeletonDef = GetCreationData()->m_ModelInstances[modelInstance.creationIndex].skeleton;
		
		if (skeletonDef)
		{
			modelInstance.skeletonResource = m_pResourceBag->GetResource<Animation::C_SkeletonResource>(MakeResourceBagTag(skeletonDef));
			modelInstance.skeleton = modelInstance.skeletonResource->GetSkeleton();
		}
		else
		{
			modelInstance.skeletonResource = NULL;
			modelInstance.skeleton = NULL;
		}
		
		for (U32 j = 0, partCount = modelInstance.modelParts.Size(); j < partCount; ++j)
		{
			ModelPart& modelPart = modelInstance.modelParts[j];

			XtHandle<ResourceUtils::C_GameModel> modelDef = GetCreationData()->m_ModelInstances[modelInstance.creationIndex].modelParts[j].model;

			if (modelDef)
			{
				modelPart.modelResource = m_pResourceBag->GetResource<GameResource::C_ModelResource>(MakeResourceBagTag(modelDef));
			}
			else
			{
				modelPart.modelResource = NULL;
			}
		}
	}

	return true;
}

void C_AnimatedModelComponent::ExternalDependenciesCreatedImpl()
{
	// At this point we can query the evaluation state of the animation player component
	// in order to identify visibility channels etc.
	const Animation::C_Pose* sourcePose = NULL;
	const Animation::C_Skeleton* sourceSkeleton = NULL;
	bool isActive = false;
	const Components::I_AnimationStateComponent* animState = NULL;
	if (GetActorList()->ActorExists(m_ControllerHandle))
	{
		const Actor::C_Actor* actor = GetActorList()->GetActorPtr(m_ControllerHandle);
		if (actor->QueryInterface(&animState))
		{
			sourcePose = &animState->GetPose();
			sourceSkeleton = animState->GetSkeleton();
		}
	}
	
	delete m_AnimatedCollisionShapes;
	m_AnimatedCollisionShapes = NULL;
	
	if (GetCreationData()->m_EnableCollision)
	{
		m_AnimatedCollisionShapes = new C_AnimatedCollisionShapes(GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene(), GetContext<GameContext::I_PhysicsContext>()->GetDefaultExternalConstants());
		m_AnimatedCollisionShapes->ChangeState(static_cast<C_AnimatedCollisionShapes::E_State>(m_CollisionState));
	}
	
	CreateModels(sourcePose, sourceSkeleton);

	// Start active to run a single update which will set model's world matrix, material parameters etc.
	m_State = E_State_Uninitialised;
	ChangeState(E_State_Active, false);
	
	if (m_AnimatedCollisionShapes != NULL)
	{
		if (m_AnimatedCollisionShapes->GetNumShapes() == 0)
		{
			// Didn't actually create any shapes - no need to keep the shape list around.
			delete m_AnimatedCollisionShapes;
			m_AnimatedCollisionShapes = NULL;
		}
	}

#ifndef CFG_GOLD
	AddSubscriber(E_RenderMsg_RequestResourceUsage);
#endif
}

void C_AnimatedModelComponent::KillImpl()
{
	// Destroy collision shapes.
	delete m_AnimatedCollisionShapes;
	m_AnimatedCollisionShapes = NULL;

	// Remove models so that they don't get rendered the same frame that the parent actor is deleted.
	DestroyModels();
}

bool C_AnimatedModelComponent::CreateModels(const Animation::C_Pose* sourcePose, const Animation::C_Skeleton* sourceSkeleton)
{
	SceneGraph::C_SceneGraph* pSceneGraph = GetActor()->GetContext<GameContext::I_RenderContext>()->GetSceneGraph();
	WorldRender::C_Scene* pScene = GetActor()->GetContext<GameContext::I_RenderContext>()->GetLevelSceneManager()->GetWorldRenderScene();
	
	for (U32 i = 0, count = m_ModelInstances.Size(); i < count; ++i)
	{
		ModelInstance& modelInstance = m_ModelInstances[i];
		
		Animation::C_PoseFormat poseFormat;
		Animation::C_PoseFieldMask poseFieldMask;
		
		bool needEvaluationState = false;
		
		if (sourceSkeleton != NULL)	// i.e. if source animation is in local space
		{
			// Local space animation remapping.
			if (sourceSkeleton == modelInstance.skeleton || modelInstance.skeleton == NULL)
			{
				// Same skeleton, copy source pose exactly.
				poseFieldMask.EnableAll();
			}
			else
			{
				// Model has its own unique skeleton.
				poseFormat.SetSkeleton(modelInstance.skeleton);
				
				// Gather all other non-skeleton related fields from source pose.
				poseFieldMask.EnableAll();
				poseFieldMask.Disable(Animation::E_PoseFieldType_JointTransform);
				poseFieldMask.Disable(Animation::E_PoseFieldType_TrajectoryTransform);
				poseFieldMask.Disable(Animation::E_PoseFieldType_TrajectoryDeltaTransform);

				needEvaluationState = true;
			}
		}
		else
		{
			// TODO: remapping of global space animations? need to have a list of transform names in this model
			// Possible if we can construct a pose from elements/bone matrices in this model (as per gun animations).
			poseFieldMask.EnableAll();
		}
		
		// Add required fields from source pose.
		poseFormat.AddPoseFieldsWithNamespace(sourcePose, poseFieldMask, modelInstance.namespacePrefix);
		
		if (needEvaluationState)
		{
			BIZ_ASSERT(modelInstance.evaluationState == NULL);
			
			// Create local evaluation state for this model.
			modelInstance.evaluationState = Animation::C_EvaluationState::New(1);
			modelInstance.evaluationState->Create(poseFormat, modelInstance.skeleton);
			
			// Create a mapping from the source animation state to local pose.
			BIZ_ASSERT(modelInstance.poseMapping == NULL);
			modelInstance.poseMapping = new Animation::C_PoseMapping(*sourcePose, modelInstance.evaluationState->GetPoseReadOnly(), modelInstance.namespacePrefix);
			BIZ_ASSERT(modelInstance.poseMapping);
			if (modelInstance.poseMapping->IsEmpty())
			{
				LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), "%s: Failed to map model using namespace '%s'", GetActor()->GetActorName(), modelInstance.namespacePrefix);
			}

		}
			
		modelInstance.animatedModelInstance.Create(pSceneGraph, poseFormat);
			
		// Add individual model parts.
		for (U32 j = 0, numParts = modelInstance.modelParts.Size(); j < numParts; ++j)
		{
			if (!modelInstance.modelParts[j].modelResource)
			{
				continue;
			}

			modelInstance.animatedModelInstance.AddPart(modelInstance.modelParts[j].modelResource, modelInstance.modelParts[j].modelIndex);
			
			
			if (m_AnimatedCollisionShapes != NULL)
			{
				const GameResource::C_Model* model = modelInstance.modelParts[j].modelResource->GetModelData()->GetModel(modelInstance.modelParts[j].modelIndex);
				if (model != NULL)
				{
					m_AnimatedCollisionShapes->ImportShapesFromModel(*model, poseFormat);
				}
			}
		}
			
		// Create a light sampler.
		modelInstance.envSampler = new WorldRender::C_EnvironmentSampler;
		modelInstance.envSampler->Initialise(pScene);
		
		// Setup light sampler position method.
		if (modelInstance.lightSamplerPositionType == C_AnimatedModelCreator::E_LightSamplerPositionType_Bone)
		{
			modelInstance.lightSamplerBoneIndex = sourcePose->FindJointTransformByName(modelInstance.lightSamplerBoneName);
		}

		if (modelInstance.motionBlurDisabled)
		{
			if (s_NoMotionBlurOpaqueRenderPass >= 0)
			{
				modelInstance.animatedModelInstance.SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, s_NoMotionBlurOpaqueRenderPass);
			}
			
			if (s_NoMotionBlurAlphaTestRenderPass >= 0)
			{
				modelInstance.animatedModelInstance.SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_ALPHA_TEST, s_NoMotionBlurAlphaTestRenderPass);
			}
		}
		else
		{
			if (s_SolidRenderPass >= 0)
			{
				modelInstance.animatedModelInstance.SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, s_SolidRenderPass);
			}
			
			if (s_PunchRenderPass >= 0)
			{
				modelInstance.animatedModelInstance.SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_ALPHA_TEST, s_PunchRenderPass);
			}
		}

		if (s_AlphaRenderPass >= 0)
		{
			modelInstance.animatedModelInstance.SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, s_AlphaRenderPass);
		}
	
		if (m_Visible)
		{
			modelInstance.animatedModelInstance.InsertIntoSceneGraph();
		}
	}
	return true;
}

void C_AnimatedModelComponent::DestroyModels()
{
	for (U32 i = 0, count = m_ModelInstances.Size(); i < count; ++i)
	{
		m_ModelInstances[i].Destroy();
	}
	m_ModelInstances.Clear();
}

void C_AnimatedModelComponent::UpdateFromAnimationPlayer(const Maths::Mat43& worldMatrix, F32 deltaTime)
{
	if (!m_Visible)
		return;
		
	if (!GetActorList()->ActorExists(m_ControllerHandle))
	{
		return;
	}
		
	const Actor::C_Actor* actor = GetActorList()->GetActorPtr(m_ControllerHandle);
	const Components::I_AnimationStateComponent* animState = NULL;
	if (!actor->QueryInterface(&animState))
	{
		return;
	}
	
	for (U32 modelIndex = 0, count = m_ModelInstances.Size(); modelIndex < count; ++modelIndex)
	{
		ModelInstance& modelInstance = m_ModelInstances[modelIndex];
		
		//
		// Update lighting.
		//
		if (modelInstance.envSampler != NULL)
		{	
			Maths::HVec3 samplePos = Maths::HVec3(GetLightSamplerPosition(modelInstance, worldMatrix, animState->GetPose(), animState->GetSkeleton()));
			modelInstance.envSampler->SetPosition(samplePos);
			modelInstance.envSampler->Update();
			
			// Apply light sampler to all parts of the model instance.
			for (U32 partIndex = 0, numParts = modelInstance.animatedModelInstance.GetNumParts(); partIndex < numParts; ++partIndex)
			{
				modelInstance.envSampler->ApplyToModelInstance(modelInstance.animatedModelInstance.GetModelInstance(partIndex));
			}
		}

		//
		// Update drop shadow.
		//
		if (modelInstance.HasDropShadows())
		{
			modelInstance.UpdateDropShadows(animState->GetPose(), animState->GetSkeleton(), worldMatrix);
		}
				
		//
		// Update collision.
		//
		if (m_AnimatedCollisionShapes != NULL)
		{
			m_AnimatedCollisionShapes->Update(animState->GetPoseMatrixCache(), worldMatrix, deltaTime);
		}
		
		//
		// Update model and local animation pose, if necessary.
		//
		const Animation::C_Skeleton* evalStateSkeleton = animState->GetSkeleton();
		if (modelInstance.evaluationState == NULL)
		{
			// Update model using the animation state's pose.
			modelInstance.animatedModelInstance.ScheduleUpdateTask(worldMatrix, animState->GetPoseNoSync(), animState->GetPoseMatrixCacheNoSync());
		}
		else
		{
			// Update model's own evaluation state using source pose.
			BIZ_ASSERT(modelInstance.evaluationState);
			BIZ_ASSERT(modelInstance.poseMapping);

			Animation::C_CommandList& cmdList = modelInstance.evaluationState->GetCommandList();
			Animation::C_SkeletonInstance& skeletonInstance = modelInstance.evaluationState->GetSkeletonInstance();
			
			cmdList.Clear();
			cmdList.CopyInPose(animState->GetPose(), animState->GetSkeleton(), *modelInstance.poseMapping, !modelInstance.enableScale);

			if (skeletonInstance.HasConstraints())
			{
				if (skeletonInstance.HasDynamicJoints())
				{
					skeletonInstance.UpdateDynamicJoints(deltaTime);

					const Animation::C_WindParams* pWindParams = animState->GetWindParams();

					if (pWindParams)
					{
						cmdList.ApplyWindForces(*pWindParams, deltaTime);
					}
				}
				cmdList.ApplyConstraints(skeletonInstance, worldMatrix);
			}
			
			modelInstance.evaluationState->ScheduleEvaluationTask();
			
			modelInstance.animatedModelInstance.ScheduleUpdateTask(
				worldMatrix,
				modelInstance.evaluationState->GetPoseNonBlocking(),
				modelInstance.evaluationState->GetPoseMatrixCacheNonBlocking()
			);
		}
	}

	// Update state.
	if (animState->IsAnimating())
	{
		ChangeState(E_State_Active, true);
	}
	else
	{
		ChangeState(E_State_Inactive, true);
	}
	
	m_FirstUpdate = false;
}

void C_AnimatedModelComponent::ChangeState(E_State state, bool setCollisionState)
{
	if (m_State != state)
	{
		switch (state)
		{
			default:
			case E_State_Inactive:
				RemoveSubscriber(GameMessaging::E_GameMsg_Update);
				break;
			case E_State_Active:
				AddSubscriber(GameMessaging::E_GameMsg_Update);
				break;
		}
		m_State = state;
	}
	
	if (setCollisionState && m_AnimatedCollisionShapes != NULL)
	{
		m_AnimatedCollisionShapes->SetActive(state == E_State_Active);
	}
}

void C_AnimatedModelComponent::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_ControllerHandle;
	stream << m_Visible;
	U8 collisionState = 0;
	if (m_AnimatedCollisionShapes != NULL)
	{
		collisionState = m_AnimatedCollisionShapes->GetState();
	}
	stream << collisionState;
}

void C_AnimatedModelComponent::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_ControllerHandle;
	stream >> m_Visible;
	stream >> m_CollisionState;
	
	// Ensure that previous frame's values aren't used for anything
	m_FirstUpdate = true;
}

Maths::Point3 C_AnimatedModelComponent::GetLightSamplerPosition(const ModelInstance& modelInstance, const Maths::Mat43& worldMatrix, const Animation::C_Pose& pose, const Animation::C_Skeleton* skeleton) const
{
	Maths::Point3 pos;
	
	switch (modelInstance.lightSamplerPositionType)
	{
		case C_AnimatedModelCreator::E_LightSamplerPositionType_Actor:
			pos = modelInstance.lightSamplerOffset * worldMatrix;
			break;
			
		case C_AnimatedModelCreator::E_LightSamplerPositionType_BoundingSphereCentre:
			if (m_FirstUpdate)
			{
				pos = Maths::MakePoint3(Maths::RowW(worldMatrix));
			}
			else
			{
				pos = modelInstance.animatedModelInstance.GetWorldSphere().Centre();
			}
			break;
			
		case C_AnimatedModelCreator::E_LightSamplerPositionType_Bone:
			if (modelInstance.lightSamplerBoneIndex >= 0)
			{
				pos = pose.GetModelJointTransform(modelInstance.lightSamplerBoneIndex, skeleton).GetPosition() * worldMatrix;
			}
			else
			{
				pos = Maths::MakePoint3(Maths::RowW(worldMatrix));
			}
			break;

		case C_AnimatedModelCreator::E_LightSamplerPositionType_WorldPoint:
			pos = modelInstance.lightSamplerOffset;
			break;
			
		default:
			break;
			
	}
	return pos;
}

void C_AnimatedModelComponent::SetVisible(bool visible)
{
	if (visible != m_Visible)
	{
		for (U32 i = 0, count = m_ModelInstances.Size(); i < count; ++i)
		{
			if (visible)
			{
				m_ModelInstances[i].animatedModelInstance.InsertIntoSceneGraph();
		
			}
			else
			{
				m_ModelInstances[i].animatedModelInstance.RemoveFromSceneGraph();
			}
		}
		m_Visible = visible;
	}
}

C_AnimatedModelComponent::ModelPart::ModelPart():
	modelResource(NULL)
{
}

//
// C_AnimatedModelComponent::C_DropShadow
//

C_AnimatedModelComponent::C_DropShadow::C_DropShadow():
	m_jointIndex1(-1),
	m_jointIndex2(-1),
	m_firstUpdate(true)
{
}

C_AnimatedModelComponent::C_DropShadow::~C_DropShadow()
{
}

void C_AnimatedModelComponent::C_DropShadow::Create(SceneGraph::C_SceneGraph* pSceneGraph)
{
	if (!m_handle)
	{
		m_handle = SceneGraph::AddDropShadow(pSceneGraph);
	}

	if (m_handle)
	{
		m_handle->SetCullDistance(60.f);
		m_handle->SetCullScreenSize(0.0125f);
		m_handle->SetIntensity(1.f);
	}

	AttachToBoundingSphere();
	m_firstUpdate = true;
	SetOffset(Maths::Const<Maths::Vector3>::Zero());
}

void C_AnimatedModelComponent::C_DropShadow::Destroy(SceneGraph::C_SceneGraph* pSceneGraph)
{
	if (m_handle)
	{
		SceneGraph::RemoveDropShadow(pSceneGraph, m_handle);
	}
}

void C_AnimatedModelComponent::C_DropShadow::SetIntensity(F32 intensity)
{
	if (m_handle)
	{
		m_handle->SetIntensity(intensity);
	}
}

void C_AnimatedModelComponent::C_DropShadow::SetRadius(F32 radius)
{
	m_sphere = Maths::MakeSphere(Maths::Centre(m_sphere), Maths::MakeVF32(radius));
}

void C_AnimatedModelComponent::C_DropShadow::AttachToBoundingSphere()
{
	m_jointIndex1 = -1;
	m_jointIndex2 = -1;
}

void C_AnimatedModelComponent::C_DropShadow::AttachToBone(S32 jointIndex)
{
	m_jointIndex1 = jointIndex;
	m_jointIndex2 = -1;
}

void C_AnimatedModelComponent::C_DropShadow::SetOffset(const Maths::Vector3& offset)
{
	m_offset = offset;
}

void C_AnimatedModelComponent::C_DropShadow::AttachBetweenBones(S32 jointIndex1, S32 jointIndex2, F32 blend, U32 attachFlags)
{
	m_jointIndex1 = jointIndex1;
	m_jointIndex2 = jointIndex2;
	m_blend = blend;

	if (m_jointIndex2 >= 0 && m_jointIndex1 < 0)
	{
		m_jointIndex1 = m_jointIndex2;
		m_jointIndex2 = -1;
	}

	if (attachFlags & E_AttachFlag_KeepLowestYPosition)
	{
		m_keepLowestYPosition = true;
	}
	else
	{
		m_keepLowestYPosition = false;
	}
}

void C_AnimatedModelComponent::C_DropShadow::Update(const Animation::C_Pose& pose, const Animation::C_Skeleton* pSkeleton, const AnimatedModel::C_AnimatedModelInstance& animatedModelInstance, const Maths::Mat43& worldMatrix)
{
	Maths::Point3 pos = Maths::Centre(m_sphere);

	if (m_jointIndex1 < 0 && m_jointIndex2 < 0)
	{
		if (m_firstUpdate)
		{
			// Won't necessarily match up if model has moved quite far in local/model space.
			pos = Maths::MakePoint3(Maths::RowW(worldMatrix));
		}
		else
		{
			pos = animatedModelInstance.GetWorldSphere().Centre();
		}
		pos = pos * worldMatrix;
	}
	else
	{
		BIZ_ASSERT(m_jointIndex1 >= 0);

		Animation::C_JointTransform t1 = pose.GetModelJointTransform(m_jointIndex1, pSkeleton);

		if (m_jointIndex2 >= 0)
		{
			Animation::C_JointTransform t2 =  pose.GetModelJointTransform(m_jointIndex2, pSkeleton);

			pos = Maths::MakePoint3(Maths::Lerp(t1.GetTranslation(), t2.GetTranslation(), Maths::MakeVF32(m_blend)));
			pos += m_offset;
			pos = pos * worldMatrix;

			if (m_keepLowestYPosition)
			{
				Maths::Vector4 axis = Maths::ColY(worldMatrix);
				Maths::VF32 one = Maths::Const<Maths::VF32>::One();
				Maths::VF32 y1 = Maths::MakeVector4(t1.GetTranslation(), one) | axis;
				Maths::VF32 y2 = Maths::MakeVector4(t2.GetTranslation(), one) | axis;
				Maths::SetY(&pos, Maths::Min(y1, y2));
			}
		}
		else
		{
			pos = t1.GetPosition() + t1.TransformVector(m_offset);
			pos = pos * worldMatrix;
		}
	}
	m_firstUpdate = false;

	Maths::VF32 radius = Maths::Radius(m_sphere);
	m_sphere = Maths::MakeSphere(pos, radius);
	
	if (m_handle)
	{
		m_handle->SetDimensions(Maths::HVec3(pos), Maths::GetF32(radius));
	}
}

//
// C_AnimatedModelComponent::ModelInstance
// 

C_AnimatedModelComponent::ModelInstance::ModelInstance():
	skeletonResource(NULL), skeleton(NULL), evaluationState(NULL), poseMapping(NULL), envSampler(NULL),
	lightSamplerPositionType(C_AnimatedModelCreator::E_LightSamplerPositionType_Actor),
	lightSamplerOffset(Maths::Const<Maths::Point3>::Zero()),
	m_numDropShadows(0),
	m_xtDropShadowListChecksum(0),
	namespacePrefix(NULL),
	lightSamplerBoneName(NULL),
	lightSamplerBoneIndex(-1),
	motionBlurDisabled(false),
	enableScale(false)
{
}

void C_AnimatedModelComponent::ModelInstance::Destroy()
{
	delete envSampler;
	envSampler = NULL;
	delete evaluationState;
	evaluationState = NULL;
	delete poseMapping;
	poseMapping = NULL;

	for (U32 i = 0, count = m_numDropShadows; i < count; ++i)
	{
		m_dropShadows[i].Destroy(animatedModelInstance.GetSceneGraph());
	}
}

bool C_AnimatedModelComponent::ModelInstance::HasDropShadows() const
{
	if (m_numDropShadows > 0)
	{
		return true;
	}
	else
	{
		if (m_xtDropShadowList)
		{
			return m_xtDropShadowList->dropShadows.count > 0;
		}
	}
	return false;
}

void C_AnimatedModelComponent::ModelInstance::UpdateDropShadows(const Animation::C_Pose& pose, const Animation::C_Skeleton* pSkeleton, const Maths::Mat43& worldMatrix)
{
	SceneGraph::C_SceneGraph* pSceneGraph = animatedModelInstance.GetSceneGraph();

	// TODO: respect visibility? e.g. start hidden, don't show
	if (m_xtDropShadowList)
	{
		if (m_xtDropShadowListChecksum != m_xtDropShadowList->checksum)
		{
			m_numDropShadows = 0;
			Btl::String128 boneStr;
			for (U32 i = 0, count = m_xtDropShadowList->dropShadows.count; i < count && m_numDropShadows < kMaxDropShadows; ++i)
			{
				const C_AnimatedModelCreator::C_XtDropShadow& xtDropShadow = m_xtDropShadowList->dropShadows[i];
				C_DropShadow& shadow = m_dropShadows[m_numDropShadows];

				if (!xtDropShadow.placement)
				{
					continue;
				}

				bool createdShadow = false;

				switch (xtDropShadow.placement->Type())
				{
					case C_AnimatedModelCreator::E_DropShadowPlacementType_OnBone:
					{
						const C_AnimatedModelCreator::C_XtPlaceOnBone* place = static_cast<const C_AnimatedModelCreator::C_XtPlaceOnBone*>(xtDropShadow.placement);
						
						S32 jointIndex = GetShadowBoneJointIndex(place->boneName, pose);
						if (jointIndex >= 0)
						{
							shadow.Create(pSceneGraph);
							shadow.AttachToBone(jointIndex);
							createdShadow = true;
						}
						break;
					}
					
					case C_AnimatedModelCreator::E_DropShadowPlacementType_BetweenBones:
					{
						const C_AnimatedModelCreator::C_XtPlaceBetweenBones* place = static_cast<const C_AnimatedModelCreator::C_XtPlaceBetweenBones*>(xtDropShadow.placement);

						S32 jointIndex1 = GetShadowBoneJointIndex(place->boneName1, pose);
						S32 jointIndex2 = GetShadowBoneJointIndex(place->boneName2, pose);

						if (jointIndex1 >= 0 || jointIndex2 >= 0)
						{
							shadow.Create(pSceneGraph);
							U32 flags = 0;
							if (place->useLowestYPosition)
							{
								flags |= C_DropShadow::E_AttachFlag_KeepLowestYPosition;
							}
							shadow.AttachBetweenBones(jointIndex1, jointIndex2, Maths::Clamped(place->blend, 0.f, 1.f), flags);
							createdShadow = true;
						}
						break;
					}
					
					default:
						break;
				}

				if (!createdShadow)
				{
					continue;
				}

				shadow.SetRadius(xtDropShadow.radius);
				shadow.SetIntensity(xtDropShadow.intensity);
				shadow.SetOffset(Maths::MakeVector3(xtDropShadow.offset.vx, xtDropShadow.offset.vy, xtDropShadow.offset.vz));
				++m_numDropShadows;
			}

			// Store checksum so it's possible to tell if the record has changed.
			m_xtDropShadowListChecksum = m_xtDropShadowList->checksum;
		}
	}
	else
	{
		for (U32 i = 0, count = m_numDropShadows; i < count; ++i)
		{
			m_dropShadows[i].Destroy(pSceneGraph);
		}
	}

	// Update drop shadows.
	for (U32 i = 0, count = m_numDropShadows; i < count; ++i)
	{
		m_dropShadows[i].Update(pose, pSkeleton, animatedModelInstance, worldMatrix);
	}
}

S32 C_AnimatedModelComponent::ModelInstance::GetShadowBoneJointIndex(const char* boneName, const Animation::C_Pose& pose) const
{
	Btl::String128 boneStr;
	if (namespacePrefix)
	{
		boneStr = namespacePrefix;
	}
	boneStr += boneName;
	S32 jointIndex = pose.FindJointTransformByName(boneStr.Chars());
	if (jointIndex < 0)
	{
		// Try bone name without namespace.
		jointIndex = pose.FindJointTransformByName(boneName);
	}
	return jointIndex;
}



//
// C_AnimatedParticleEffectsComponent
//
C_AnimatedParticleEffectsComponent::C_AnimatedParticleEffectsComponent():
	m_pResourceBag(NULL),
	m_State(E_State_Uninitialised),
	m_Visible(false),
	m_FirstUpdate(true)
{
}

C_AnimatedParticleEffectsComponent::~C_AnimatedParticleEffectsComponent()
{
	DestroyEmitters();

	delete m_pResourceBag;
	m_pResourceBag = NULL;
}

const C_AnimatedParticleEffectsCreator*	 C_AnimatedParticleEffectsComponent::GetCreationData() const
{
	return static_cast<const C_AnimatedParticleEffectsCreator*>(GetCreator().GetData());
}

bool C_AnimatedParticleEffectsComponent::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
		// TODO: snooping messages here to figure out when to activate
		// maybe anim player should send out messages to say that its beginning to animate?
		case ActorMsg::E_AnimatedObjectMsg_PlayCycle:
		case ActorMsg::E_GameActorMsg_ActionBegin:
		case ActorMsg::E_GameActorMsg_Activate:
			ChangeState(E_State_Active);
			SetVisible(true);
			break;	

		// Leave deactivation to the result of polling I_AnimationStateComponent
		//case ActorMsg::E_GameActorMsg_Deactivate:
		//	ChangeState(E_State_Inactive);
		//	break;
		
		case ActorMsg::E_AnimatedObjectMsg_SetController:
		{
			m_ControllerHandle = Messaging::MessageCast<ActorMsg::C_AnimatedObjectSetControllerMsg>(pMessage)->GetActorHandle();
			CreateEmitters();
			break;
		}

		case ActorMsg::E_GameActorMsg_SetTransform:
		{
			const ActorMsg::C_ActorSetTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage);
			UpdateFromAnimationPlayer(msg->Transform().Matrix(), 0.f);
			break;
		}
		
		case ActorMsg::E_GameActorMsg_AnimateTransform:
		{
			const ActorMsg::C_ActorAnimateTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage);
			UpdateFromAnimationPlayer(msg->Transform().Matrix(), msg->m_TimeStep);
			break;
		}
	
		case GameMessaging::E_GameMsg_Update:
		{
			if (ShouldUpdateDuringUpdateMessage())
			{
				F32 deltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
				UpdateFromAnimationPlayer(GetActor()->GetMatrix(), deltaTime);
			}
			break;
		}
		
		default:
			return Actor::I_ActorComponent::OnMessage(pMessage);
	}
	return true;
}

void C_AnimatedParticleEffectsComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Base::SerialiseForSave(stream);
	Serialise(stream);
}

void C_AnimatedParticleEffectsComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Base::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_AnimatedParticleEffectsComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Base::SerialiseForSync(stream);
	Serialise(stream);
}

void C_AnimatedParticleEffectsComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Base::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_AnimatedParticleEffectsComponent::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_ControllerHandle;
	stream << m_Visible;
}

void C_AnimatedParticleEffectsComponent::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_ControllerHandle;
	stream >> m_Visible;
}

#ifndef CFG_GOLD
DECLARE_DEBUG_BOOL(g_DebugAnimatedParticleEffectsComponentRenderDebug, "ActorDebug.Components.AnimatedParticleEffects.DebugRender.Enable", false);
DECLARE_DEBUG_BOOL(g_DebugAnimatedParticleEffectsComponentRenderEmitters, "ActorDebug.Components.AnimatedParticleEffects.DebugRender.ShowEmitters", false);
DECLARE_DEBUG_BOOL(g_DebugAnimatedParticleEffectsComponentRenderIntensities, "ActorDebug.Components.AnimatedParticleEffects.DebugRender.ShowEmitterIntensities", false);
DECLARE_DEBUG_BOOL(g_DebugAnimatedParticleEffectsComponentRenderSpawnMeshes, "ActorDebug.Components.AnimatedParticleEffects.DebugRender.ShowSpawnMeshes", false);

void C_AnimatedParticleEffectsComponent::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_DebugAnimatedParticleEffectsComponentRenderDebug) return;
	
	debugRenderPtr->SetDepthEnable(false);
	
	Util::C_String str;
	for (U32 i = 0, count = m_Emitters.Size(); i < count; ++i)
	{
		if (g_DebugAnimatedParticleEffectsComponentRenderEmitters)
		{
			Maths::HMat43 emitterMatrix = Maths::C_QuatTrans(m_Emitters[i].node).Matrix();
			debugRenderPtr->Draw3dMatrix(emitterMatrix, 0.2f);
			debugRenderPtr->Draw3dArrow(emitterMatrix.Translation(), m_Emitters[i].velocity, 1.f / 30.f, 0.1f, Maths::RGBA(0x20, 0xa0, 0x20));
		}
		if (g_DebugAnimatedParticleEffectsComponentRenderIntensities)
		{
			str.Printf("%.2f%s", m_Emitters[i].intensity, m_Emitters[i].IsActive() ? "*" : "");
			debugRenderPtr->Draw3dText(Maths::HVec3(&m_Emitters[i].node.GetPosition().x), str);
		}
		if (g_DebugAnimatedParticleEffectsComponentRenderSpawnMeshes && m_Emitters[i].spawnMesh != NULL)
		{
			m_Emitters[i].spawnMesh->Display(Maths::C_QuatTrans(m_Emitters[i].node).Matrix(), debugRenderPtr);	
		}
	}
}
#endif

bool C_AnimatedParticleEffectsComponent::InitialiseImpl()
{
	if (!Base::InitialiseImpl())
		return false;
	
	m_ControllerHandle = GetActorHandle();
	m_State = E_State_Uninitialised;
	m_Visible = !GetCreationData()->m_StartHidden;
	ChangeState(E_State_Inactive);
	
	return true;
}

bool C_AnimatedParticleEffectsComponent::PrimeImpl()
{
	bool primed = Base::PrimeImpl();

	// Prime resources used by Special FX system.
	const C_AnimatedParticleEffectsCreator* creator = GetCreationData();
	for (U32 effectIndex = 0, count = creator->m_Effects.count; effectIndex < count; ++effectIndex)
	{
		const C_AnimatedParticleEffectsCreator::Effect& effectDef = creator->m_Effects[effectIndex];

		if (effectDef.specialFx)
		{
			if (!m_pResourceBag)
			{
				m_pResourceBag = new C_SimpleResourceBag(this, &C_AnimatedParticleEffectsComponent::OnResourceBagLoaded);
				m_pResourceBag->LockListeners();
			}
			
			effectDef.specialFx->AddToResourceBag(*m_pResourceBag);
		}
	}

	if (m_pResourceBag)
	{
		// A resource bag was created, so we're not primed until it's finished loading.
		m_pResourceBag->UnlockListeners();
		primed = (GetLifetimeState() == ActorSys::E_LifetimeState_Primed);
	}

	return primed;
}

void C_AnimatedParticleEffectsComponent::ExternalDependenciesCreatedImpl()
{
	// Animation player creates its pose during priming, so during the creation
	// phase it will be ready for any other object to access.
	CreateEmitters();

	// Always start active to run a single update which will set model's world matrix, material parameters etc.
	m_State = E_State_Uninitialised;
	ChangeState(E_State_Active);
}

void C_AnimatedParticleEffectsComponent::OnResourceBagLoaded()
{
	if (GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
	{
		SetPrimed();
	}
}

void C_AnimatedParticleEffectsComponent::ChangeState(E_State state)
{
	if (m_State != state)
	{
		if (m_State == E_State_Uninitialised)
		{
			m_FirstUpdate = true;
		}

		switch (state)
		{
			default:
			case E_State_Inactive:
				RemoveSubscriber(GameMessaging::E_GameMsg_Update);
				break;
			case E_State_Active:
				AddSubscriber(GameMessaging::E_GameMsg_Update);
				break;
		}
		m_State = state;
	}
}

void C_AnimatedParticleEffectsComponent::UpdateFromAnimationPlayer(const Maths::Mat43& worldMatrix, F32 deltaTime)
{
	if (!m_Visible)
		return;

	if (!GetActorList()->ActorExists(m_ControllerHandle))
	{
		return;
	}
	
	const Actor::C_Actor* actor = GetActorList()->GetActorPtr(m_ControllerHandle);
	const Components::I_AnimationStateComponent* animState = NULL;
	if (!actor->QueryInterface(&animState))
	{
		return;
	}
	
	U32 numEmitters = m_Emitters.Size();
	
	MathsStorage::Float4A16 collisionPlaneOrigin;
	MathsStorage::Float4A16 collisionPlaneNormal;
	
	Maths::Store(Maths::RowW(worldMatrix), collisionPlaneOrigin.v);
	Maths::Store(Maths::MakeVector3(Maths::Normalise(Maths::RowY(worldMatrix))), collisionPlaneNormal.v);
	
	const Animation::C_Pose& pose = animState->GetPose();
	const Animation::C_PoseMatrixCache& poseMatrixCache = animState->GetPoseMatrixCache();

	const Maths::Mat43* modelMatrices = poseMatrixCache.GetSkinningMatrices();

	const Animation::C_Skeleton* skeleton = animState->GetSkeleton();
	Animation::C_JointTransform worldTransform(worldMatrix);

	Maths::HF32 invDeltaTime(1.f / deltaTime);

	const Sys::I_ContextProvider* pContextProvider = GetActorList()->GetDefaultContextProvider();
	
	//
	// Update emitter intensities and transforms, creating new events if intensities goes above zero and
	// destroying existing event instances when intensities go to zero.
	//
	for (U32 i = 0; i < numEmitters; ++i)
	{
		Emitter& emitter = m_Emitters[i];
		
		F32 newIntensity = 1.f;
		
		if (emitter.intensityScalarIndex >= 0)
		{
			newIntensity = pose.GetCustomScalar(emitter.intensityScalarIndex);
		}
		
		newIntensity *= emitter.intensityScale;
		
		bool spawnParticleEvent = false;
		bool spawnSpecialFX = false;
		
		// Activate emitter when intensity rises above zero.
		if (newIntensity > 0.f && emitter.intensity == 0.f)
		{
			// Only spawn effects if they are non-repeating.
			spawnParticleEvent = true;
			spawnSpecialFX = true;

			if (emitter.eventInstance.Object() != NULL)
			{
				BIZ_ASSERT(emitter.eventInstance->GetEvent() != NULL);
				
				if (emitter.eventInstance->GetEvent()->GetDefinition().bRepeating)
				{
					// Repeating effects don't need to be re-created.
					spawnParticleEvent = false;
				}
			}
		}
		emitter.intensity = newIntensity;

		// Update transforms for all emitters.
		{
			Animation::C_JointTransform jointTransform = pose.GetModelJointTransform(emitter.jointIndex, skeleton) * worldTransform;
			Maths::Point3 jointPos = jointTransform.GetPosition();
			Maths::Quaternion jointRotation = jointTransform.GetRotation();

			if (!m_FirstUpdate)
			{
				Maths::HVec3 prevPosition = Maths::Load<Maths::HVec3>(&emitter.node.GetPosition());
				Maths::HVec3 newPosition(jointPos);
				emitter.velocity = (newPosition - prevPosition) * invDeltaTime;
			}
			else
			{
				emitter.velocity = Maths::HVec3();
			}


			emitter.node.SetPosition(Maths::HVec3(jointPos));
			emitter.node.SetRotation(Maths::C_Quaternion(jointRotation));
		}
			
		if (spawnParticleEvent && emitter.xtEvent.Object() != NULL)
		{
			// Spawn particle system effect.
			DynamicEffect::C_Node node;
			emitter.eventInstance = emitter.xtEvent->pEvent->Create(node, &emitter.node);

			if (emitter.eventInstance.Object() != NULL)
			{
				if (emitter.spawnMesh != NULL)
				{
					emitter.eventInstance->SetSpawnMesh(emitter.spawnMesh);
				}
				emitter.eventInstance->SetCollisionPlane(collisionPlaneOrigin, collisionPlaneNormal);
				emitter.eventInstance->SetVelocity(emitter.velocity);
			}
		}

		if (spawnSpecialFX && emitter.xtSpecialFx)
		{
			// Spawn SpecialFX effect.
			SpecialFXEmitter* pSpecialFXEmitter = m_SpecialFXEmitterPool.Allocate();
			pSpecialFXEmitter->Spawn(*emitter.xtSpecialFx, pContextProvider, m_pResourceBag);
			pSpecialFXEmitter->SetEmitterIndex(i);
			pSpecialFXEmitter->SetContinuous(emitter.IsContinuous());
			m_SpecialFXEmitters.PushBack(pSpecialFXEmitter);
		}
		
		// Update particle event intensity, destroying the event if intensity drops to zero.
		if (emitter.eventInstance.Object() != NULL)
		{
			if (emitter.intensity > 0.f)
			{
				emitter.eventInstance->SetIntensity(emitter.intensity);
			}
			else
			{
				emitter.eventInstance->Free();
			}
		}
	}

	// Update all SpecialFX emitters.
	for (S32 i = 0, count = m_SpecialFXEmitters.Size(); i < count; ++i)
	{
		m_SpecialFXEmitters[i]->Update(deltaTime, m_Emitters[m_SpecialFXEmitters[i]->GetEmitterIndex()]);

		// Erase inactive emitters.
		if (!m_SpecialFXEmitters[i]->IsActive())
		{
			m_SpecialFXEmitterPool.Free(m_SpecialFXEmitters[i]);
			m_SpecialFXEmitters.EraseByIndexNonOrdered(i);
			--count;
			--i;
		}
	}

	if (animState->IsAnimating())
	{
		ChangeState(E_State_Active);
	}
	else
	{
		ChangeState(E_State_Inactive);
	}
	m_FirstUpdate = false;
}

void C_AnimatedParticleEffectsComponent::CreateEmitters()
{
	DestroyEmitters();

	const Animation::C_Pose* sourcePose = NULL;
	const Components::I_AnimationStateComponent* animState = NULL;
	if (GetActorList()->ActorExists(m_ControllerHandle))
	{
		const Actor::C_Actor* actor = GetActorList()->GetActorPtr(m_ControllerHandle);
		if (actor->QueryInterface(&animState))
		{
			sourcePose = &animState->GetPose();
		}
	}
	
	const I_AnimatedObjectVisualComponent* modelComponent = NULL;
	GetActor()->QueryInterface(&modelComponent);
			
	const C_AnimatedParticleEffectsCreator* creator = GetCreationData();
	
	//
	// For each effect, create emitters for each bone the effect is attached to.
	//
	
	m_Emitters.Clear();
	
	if (sourcePose != NULL)
	{
		U32 numJoints = sourcePose->GetNumJointTransforms();
		
		Btl::String64 attributeName;
	
		for (U32 effectIndex = 0, count = creator->m_Effects.count; effectIndex < count; ++effectIndex)
		{
			const C_AnimatedParticleEffectsCreator::Effect& effectDef = creator->m_Effects[effectIndex];

			if (!effectDef.particleEvent && !effectDef.specialFx)
			{
				LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), "%s: Effect %d has no particle event or Special FX", GetActor()->GetActorName(), effectIndex);
				continue;
			}

			//
			// Decide whether or not to create a spawn mesh for particle events.
			//
			bool createSpawnMesh = false;
			if (effectDef.particleEvent != NULL && effectDef.spawnFromCollisionMesh && modelComponent != NULL)
			{
				// Only create a spawn mesh if particle event has emitters with mesh spawn distribution type.
				for (U32 j = 0, numEmitters = effectDef.particleEvent->emitters.count; j < numEmitters; ++j)
				{
					if (effectDef.particleEvent->emitters[j].spawnParams.distributionType == DynamicEffect::Particle::E_EmitterDistribution_Mesh)
					{
						createSpawnMesh = true;
						break;
					}
				}
				
				if (!createSpawnMesh)
				{
					LOGDEBUG(Actor::GetDebugLogLevelDesign(GetActor()), "%s: Effect %d has spawn from collision mesh enabled, but particle event has no emitters that spawn from mesh", GetActor()->GetActorName(), effectIndex);
				}
			}

			//
			// Create an emitter for each joint with a name that matches the effect's joint name pattern.
			//
						
			bool matched = false;
			for (U32 j = 0; j < numJoints; ++j)
			{
				if (sourcePose->GetJointTransformName(j).Match(effectDef.boneNamePattern))
				{
					S32 scalarIndex = -1;
					
					bool createEmitter = false;
					
					if (*effectDef.intensityAttributeName.Text() != 0)
					{
						attributeName = sourcePose->GetJointTransformName(j).GetString();
						attributeName += ".";
						attributeName += effectDef.intensityAttributeName.Text();
						Animation::C_StringId customScalarName(attributeName.Chars(), false);
						scalarIndex = sourcePose->FindCustomScalarByName(customScalarName);
						
						// Only create an emitter if the candidate transform has an intensity attribute.
						if (scalarIndex >= 0)
						{
							createEmitter = true;
						}
					}
					else
					{
						// Always create "continuous" emitters when no intensity attribute name has been entered.
						createEmitter = true;
					}
					
					if (createEmitter)
					{
						m_Emitters.PushBack();
						Emitter& emitter = m_Emitters.Back();
					
						emitter.jointIndex = j;
						emitter.intensityScalarIndex = scalarIndex;
					
						// These fields are shared by emitter instances - could separate out into another structure?
						emitter.intensityScale = Maths::Clamped(effectDef.intensityScale, 0.f, 1.f);
						emitter.xtEvent = effectDef.particleEvent;
						emitter.xtSpecialFx = effectDef.specialFx;
						
						emitter.intensity = 0.f;
										
						if (createSpawnMesh)
						{
							BIZ_ASSERT(effectDef.particleEvent != NULL);

							const GameResource::C_ModelElement* pModelElement = modelComponent->FindModelElement(sourcePose->GetJointTransformName(j).GetString());
							
							if (pModelElement != NULL)
							{
								const Gravity::C_ImportShape* importShape = pModelElement->GetPhysicsImportShape();
								
								if (importShape != NULL)
								{
									Gravity::C_ShapeDesc* shapeDesc = importShape->GetShapeDesc();
									
									if (shapeDesc != NULL)
									{
										BIZ_ASSERT(emitter.spawnMesh == NULL);
										delete emitter.spawnMesh;
										emitter.spawnMesh = new DynamicEffect::Particle::C_SpawnMesh;
										BuildSpawnMesh(*emitter.spawnMesh, shapeDesc, Maths::HMat43());
										emitter.spawnMesh->CalculateArea();
									}
								}
							}
						}
					}
					matched = true;
				}
			}
			if (!matched)
			{
				LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), "%s: No bones matched pattern '%s', effect %d will not be used", GetActor()->GetActorName(), effectDef.boneNamePattern.Text(), effectIndex);
			}
		}
	}	
}

void C_AnimatedParticleEffectsComponent::DestroyEmitters()
{
	for (U32 i = 0, count = m_Emitters.Size(); i < count; ++i)
	{
		if (m_Emitters[i].eventInstance.Object() != NULL)
		{
			m_Emitters[i].eventInstance->Free();
		}
		delete m_Emitters[i].spawnMesh;
		m_Emitters[i].spawnMesh = NULL;
		m_Emitters[i].intensity = 0.f;
	}
	m_Emitters.Clear();

	// Destroy Special FX emitters.
	for (U32 i = 0, count = m_SpecialFXEmitters.Size(); i < count; ++i)
	{
		m_SpecialFXEmitterPool.Free(m_SpecialFXEmitters[i]);
	}
	m_SpecialFXEmitters.Clear();
}

void C_AnimatedParticleEffectsComponent::ResetEmitters()
{
	for (U32 i = 0, count = m_Emitters.Size(); i < count; ++i)
	{
		if (m_Emitters[i].eventInstance.Object() != NULL)
		{
			m_Emitters[i].eventInstance->Free();
		}
		m_Emitters[i].intensity = 0.f;
	}

	// Destroy all SpecialFX emitters.
	for (U32 i = 0, count = m_SpecialFXEmitters.Size(); i < count; ++i)
	{
		m_SpecialFXEmitterPool.Free(m_SpecialFXEmitters[i]);
	}
	m_SpecialFXEmitters.Clear();
}

void C_AnimatedParticleEffectsComponent::SetVisible(bool visible)
{
	if (visible != m_Visible)
	{
		if (!visible)
		{
			ResetEmitters();
		}
		m_Visible = visible;
	}
}

void C_AnimatedParticleEffectsComponent::BuildSpawnMesh(DynamicEffect::Particle::C_SpawnMesh& spawnMesh, Gravity::C_ShapeDesc* shapeDesc, const Maths::HMat43& transform) const
{
	if (shapeDesc != NULL)
	{
		switch (shapeDesc->m_Type)
		{
			case Collision::E_Composite_Shape:
				BuildSpawnMesh(spawnMesh, static_cast<Gravity::C_CompositeDesc*>(shapeDesc), transform);
				break;
			
			case Collision::E_InstancedTriangleMesh_Shape:
				BuildSpawnMesh(spawnMesh, static_cast<Gravity::C_InstancedTriangleMeshDesc*>(shapeDesc), transform);
				break;
			
			case Collision::E_ConvexHull_Shape:
				BuildSpawnMesh(spawnMesh, static_cast<Gravity::C_ConvexHullDesc*>(shapeDesc), transform);
				break;
			
			default:
			break;
		}
	}
}

void C_AnimatedParticleEffectsComponent::BuildSpawnMesh(DynamicEffect::Particle::C_SpawnMesh& spawnMesh, Gravity::C_InstancedTriangleMeshDesc* instancedTriMeshDesc, const Maths::HMat43& transform) const
{
	Maths::HMat43 mInstance = transform;// * m;
	Maths::HMat33 mInstanceRot = Maths::HMat33(mInstance);
	
	Collision::OptimisedAABBTree* pTree = &instancedTriMeshDesc->m_pShape->m_AABBTree;
	
	const S32 vertexCount = pTree->m_NumVertices;
	U32 offset = 0;
	
	const U32 dataSize = pTree->m_NumTriangles;


	while(offset < dataSize)
	{
		Collision::C_TriangleHeader* header = pTree->m_Triangles.GetHeader(offset);
		offset += sizeof(Collision::C_TriangleHeader);
		Collision::AABBTreeTriangle* tri = pTree->m_Triangles.GetTriangles(offset);
		offset += sizeof(Collision::AABBTreeTriangle) * header->m_NumTriangles;
	
		for(U32 a =0; a < header->m_NumTriangles; ++a)
		{
			/*Maths::HVec3 vPosA = Maths::Load<Maths::HVec3>(&pTree->m_pVertices[tri->m_IndexA]) * mInstance;
			Maths::HVec3 vPosB = Maths::Load<Maths::HVec3>(&pTree->m_pVertices[tri->m_IndexB]) * mInstance;
			Maths::HVec3 vPosC = Maths::Load<Maths::HVec3>(&pTree->m_pVertices[tri->m_IndexC]) * mInstance;*/

			Maths::HVec3 vPosA = Maths::HVec3(&pTree->m_pVertices[tri->m_IndexA]) * mInstance;
			Maths::HVec3 vPosB = Maths::HVec3(&pTree->m_pVertices[tri->m_IndexB]) * mInstance;
			Maths::HVec3 vPosC = Maths::HVec3(&pTree->m_pVertices[tri->m_IndexC]) * mInstance;
			Maths::HVec3 vNormal = Maths::Load<Maths::HVec3>(&tri->m_SurfaceNormal) * mInstanceRot;

			S32 indexA = spawnMesh.AddVertex(vPosA, vNormal);
			S32 indexB = spawnMesh.AddVertex(vPosB, vNormal);
			S32 indexC = spawnMesh.AddVertex(vPosC, vNormal);
			spawnMesh.AddTriangle(indexA, indexB, indexC);			

			tri++;
		}
	}	
}

void C_AnimatedParticleEffectsComponent::BuildSpawnMesh(DynamicEffect::Particle::C_SpawnMesh& spawnMesh, Gravity::C_CompositeDesc* compositeDesc, const Maths::HMat43& transform) const
{
	for (U32 i = 0, count = compositeDesc->shapes.Size(); i < count; ++i)
	{
		BuildSpawnMesh(spawnMesh, compositeDesc->shapes[i].m_pShapeDesc, compositeDesc->shapes[i].m_RelativeTransformation * transform);
	}
}

void C_AnimatedParticleEffectsComponent::BuildSpawnMesh(DynamicEffect::Particle::C_SpawnMesh& spawnMesh, Gravity::C_ConvexHullDesc* convexHullDesc, const Maths::HMat43& transform) const
{
	// Add vertices first.
	const S32 firstVertexIndex = spawnMesh.GetNumVertices();
	const U32 numVerts = convexHullDesc->m_Points.Size();
	
	Maths::HVec3 centre = convexHullDesc->CalculateCentreOfMass();
	Maths::HVec3 offset = convexHullDesc->m_RenderOffset;
	
	Maths::HMat33 normalTransform = transform.Rotation();
	
	for (U32 i = 0; i < numVerts; ++i)
	{
		// Convex hulls are roughly spherical so create rough normals using vertex positions relative to centre of the hull.
		// It might be better to calculate normals on a per-triangle basis and duplicate vertices, but for the time
		// being having shared normals should be acceptable.
		spawnMesh.AddVertex((convexHullDesc->m_Points[i] + offset) * transform, (convexHullDesc->m_Points[i] - centre) * normalTransform);
	}
	
	BIZ_ASSERT(numVerts == spawnMesh.GetNumVertices() - firstVertexIndex);
	
	// Add triangles, referencing previously added vertices.
	for (U32 i = 0, numTris = convexHullDesc->m_Triangles.Size(); i < numTris; ++i)
	{
		const Gravity::C_TriangleIndices& tri = convexHullDesc->m_Triangles[i];
		spawnMesh.AddTriangle(tri.m_IndexA + firstVertexIndex, tri.m_IndexB + firstVertexIndex, tri.m_IndexC + firstVertexIndex);
	}
}

const Components::I_Movement* C_AnimatedParticleEffectsComponent::GetActorMovementComponent() const
{
	BIZ_ASSERT(GetActor() != NULL);


	return GetActor()->QueryInterface<Components::I_Movement>();
}

bool C_AnimatedParticleEffectsComponent::ShouldUpdateDuringUpdateMessage() const
{
	const Components::I_Movement* pMovementComponent = GetActorMovementComponent();
	if (!pMovementComponent)
	{
		return true;
	}

	if (pMovementComponent->IsAnimating() && pMovementComponent->IsEnabled())
	{
		// Defer model update until the movement component has set our world matrix
		// via AnimateTransform or SetTransform messages.
		return false;
	}
	return true;
}

//
// C_AnimatedParticleEffectsComponent::Emitter
//

C_AnimatedParticleEffectsComponent::Emitter::~Emitter()
{
}

bool C_AnimatedParticleEffectsComponent::Emitter::IsActive() const
{
	bool active = false;
	if (eventInstance.Object() != NULL)
	{
		active = eventInstance->Active();
	}
	return active;
}

bool C_AnimatedParticleEffectsComponent::Emitter::IsContinuous() const
{
	return intensityScalarIndex < 0;
}

//
// C_AnimatedParticleEffectsComponent::SpecialFXEmitter
//
C_AnimatedParticleEffectsComponent::SpecialFXEmitter::SpecialFXEmitter():
	m_instance(NULL),
	m_lifetime(0.f),
	m_duration(0.f),
	m_isContinuous(false)
{
}

C_AnimatedParticleEffectsComponent::SpecialFXEmitter::~SpecialFXEmitter()
{
	Destroy();
}

void C_AnimatedParticleEffectsComponent::SpecialFXEmitter::Spawn(
	const SpecialFX::C_SpecialFX& specialFXRecord,
	const Sys::I_ContextProvider* pContextProvider,
	const ResourceUtils::I_ResourceBag* pResourceBag)
{
	m_instance = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(specialFXRecord, pContextProvider, pResourceBag);

	m_lifetime = 0.f;
	
	if (m_instance)
	{
		m_duration = m_instance->GetDuration();
		m_instance->Start();
		m_isContinuous = false;
	}
}

void C_AnimatedParticleEffectsComponent::SpecialFXEmitter::Destroy()
{
	if (m_instance)
	{
		m_instance->Stop();
		delete m_instance;
		m_instance = NULL;
	}
}

void C_AnimatedParticleEffectsComponent::SpecialFXEmitter::Update(F32 deltaTime, const Emitter& parentEmitter)
{
	if (!IsActive()) return;

	m_lifetime += deltaTime;

	bool isActive = true;
	if (m_lifetime >= m_duration)
	{
		if (m_isContinuous && m_duration > 0.f)
		{
			while (m_lifetime >= m_duration)
			{
				m_lifetime -= m_duration;
			}
		}
		else
		{
			isActive = false;
		}
	}
	
	if (isActive)
	{
		m_instance->UpdateIntensity(parentEmitter.intensity);
		m_instance->UpdateWorldMatrix(Maths::C_QuatTrans(parentEmitter.node).Matrix());
		m_instance->Update(m_lifetime);
	}
	else
	{
		Destroy();
	}
}

void C_AnimatedParticleEffectsComponent::SpecialFXEmitter::SetContinuous(bool isContinuous)
{
	m_isContinuous = isContinuous;
}

bool C_AnimatedParticleEffectsComponent::SpecialFXEmitter::IsActive() const
{
	return m_instance != NULL;
}


//
// C_AnimatedEventTriggerComponent
//

C_AnimatedEventTriggerComponent::C_AnimatedEventTriggerComponent()
{
}

C_AnimatedEventTriggerComponent::~C_AnimatedEventTriggerComponent()
{
}

const C_AnimatedEventTriggerCreator* C_AnimatedEventTriggerComponent::GetCreationData() const
{
	return static_cast<const C_AnimatedEventTriggerCreator*>(GetCreator().GetData());
}

bool C_AnimatedEventTriggerComponent::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		// TODO: snooping messages here to figure out when to activate
		// maybe anim player should send out messages to say that its beginning to animate?
		case ActorMsg::E_AnimatedObjectMsg_PlayCycle:
		case ActorMsg::E_GameActorMsg_ActionBegin:
		case ActorMsg::E_GameActorMsg_Activate:
			ChangeState(E_State_Active);
			break;	

		// Leave deactivation to the result of polling I_AnimationStateComponent
		//case ActorMsg::E_GameActorMsg_Deactivate:
		//	ChangeState(E_State_Inactive);
		//	break;
		
		case ActorMsg::E_AnimatedObjectMsg_SetController:
		{
			m_ControllerHandle = Messaging::MessageCast<ActorMsg::C_AnimatedObjectSetControllerMsg>(pMessage)->GetActorHandle();
			CreateEvents();
			if (m_Events.Size() == 0)
			{
				ChangeState(E_State_Inactive);
			}
			break;
		}
		
		case GameMessaging::E_GameMsg_Update:
		{
			F32 deltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
			Update(deltaTime);
			break;
		}
		
		default:
			return Actor::I_ActorComponent::OnMessage(pMessage);
	}
	return true;
	
}

void C_AnimatedEventTriggerComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Base::SerialiseForSave(stream);
	Serialise(stream);
}

void C_AnimatedEventTriggerComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Base::DeserialiseForSave(stream);
	Deserialise(stream);
}


void C_AnimatedEventTriggerComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Base::SerialiseForSync(stream);
	Serialise(stream);
}

void C_AnimatedEventTriggerComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Base::DeserialiseForSync(stream);
	Deserialise(stream);
}


void C_AnimatedEventTriggerComponent::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << U8(m_State);
	stream << U32(m_Events.Size());
	for (U32 i = 0, count = m_Events.Size(); i < count; ++i)
	{
		const Event& e = m_Events[i];
		stream << e.scalarCurveIndex;
		stream << e.eventIndex;
		stream << e.currentValue;
	}
	stream << m_ControllerHandle;
}

void C_AnimatedEventTriggerComponent::Deserialise(Sys::C_ContextualStream& stream)
{
	U8 stateValue = 0;
	stream >> stateValue;
	m_State = E_State_Uninitialised;
	ChangeState(static_cast<E_State>(stateValue));
	U32 numEvents =0;
	stream >> numEvents;
	m_Events.Resize(numEvents);
	for (U32 i = 0; i < numEvents; ++i)
	{
		Event& e = m_Events[i];
		stream >> e.scalarCurveIndex;
		stream >> e.eventIndex;
		stream >> e.currentValue;
	}
	stream >> m_ControllerHandle;
}


#ifndef CFG_GOLD
void C_AnimatedEventTriggerComponent::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
}
#endif

bool C_AnimatedEventTriggerComponent::InitialiseImpl()
{
	if (!Base::InitialiseImpl())
		return false;
		
	m_State = E_State_Uninitialised;
	m_ControllerHandle = GetActorHandle();
	return true;
}

void C_AnimatedEventTriggerComponent::ExternalDependenciesCreatedImpl()
{
	const C_AnimatedEventTriggerCreator* creationData = GetCreationData();
	
	CreateEvents();
	
	if (m_Events.Size() > 0)
	{
		ChangeState(E_State_Active);
	}
	else
	{
		ChangeState(E_State_Inactive);		
	}
}

void C_AnimatedEventTriggerComponent::ChangeState(E_State state)
{
	if (m_State != state)
	{
		switch (state)
		{
			default:
			case E_State_Inactive:
				RemoveSubscriber(GameMessaging::E_GameMsg_Update);
				break;
			case E_State_Active:
				AddSubscriber(GameMessaging::E_GameMsg_Update);
				break;
		}
		m_State = state;
	}
}

void C_AnimatedEventTriggerComponent::CreateEvents()
{
	m_Events.Clear();

	if (!GetActorList()->ActorExists(m_ControllerHandle))
	{
		return;
	}
		
	const Actor::C_Actor* actor = GetActorList()->GetActorPtr(m_ControllerHandle);
	const Components::I_AnimationStateComponent* animState = NULL;
	if (!actor->QueryInterface(&animState))
	{
		return;
	}

	const C_AnimatedEventTriggerCreator* creationData = GetCreationData();
	
	const Animation::C_Pose& pose = animState->GetPose();
	
	const U32 scalarCurveCount = pose.GetNumCustomScalars();
	
	// Find custom curves matching pattern.
	for (U32 eventDefIndex = 0, eventDefCount = creationData->m_Events.count; eventDefIndex < eventDefCount; ++eventDefIndex)
	{
		const C_AnimatedEventTriggerCreator::Event& eventDef = creationData->m_Events[eventDefIndex];
		
		if (*eventDef.curveNamePattern == 0)
		{
			continue;
		}

		S32 prevEventCount = m_Events.Size();

		for (U32 scalarCurveIndex = 0; scalarCurveIndex < scalarCurveCount; ++scalarCurveIndex)
		{
			const Animation::C_StringId& scalarName = pose.GetCustomScalarName(scalarCurveIndex);
			
			if (scalarName.Match(eventDef.curveNamePattern))
			{
				Event e;
				e.scalarCurveIndex = scalarCurveIndex;
				e.eventIndex = eventDefIndex;
				// Event triggers always have an assumed default initial value of zero so that a value of 1 on the first frame
				// can be used to trigger an event. Otherwise, if the initial value is copied from the pose there will be no
				// change in value and it would impossible to trigger an event on the first frame.
				e.currentValue = 0.f;
				m_Events.PushBack(e);
			}
		}

		if (m_Events.Size() == prevEventCount)
		{
			LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), "Actor %s: could not find any event trigger curves matching pattern '%s'", GetActor()->GetActorName(), eventDef.curveNamePattern.Text());
		}
	}
}

void C_AnimatedEventTriggerComponent::Update(F32 deltaTime)
{
	if (!GetActorList()->ActorExists(m_ControllerHandle))
	{
		return;
	}
		
	const Actor::C_Actor* actor = GetActorList()->GetActorPtr(m_ControllerHandle);
	const Components::I_AnimationStateComponent* animState = NULL;
	if (!actor->QueryInterface(&animState))
	{
		return;
	}
	
	const C_AnimatedEventTriggerCreator* creationData = GetCreationData();
	const Animation::C_Pose& pose = animState->GetPose();
	const Actor::C_Actor* parentActor = GetActor();
		
	for (U32 i = 0, count = m_Events.Size(); i < count; ++i)
	{
		Event& e = m_Events[i];
		
		BIZ_ASSERT(e.scalarCurveIndex >= 0);
		BIZ_ASSERT(e.eventIndex >= 0);
		
		const C_AnimatedEventTriggerCreator::Event& eventDef = creationData->m_Events[static_cast<S32>(e.eventIndex)];
		
		const F32 oldValue = e.currentValue;
		const F32 newValue = pose.GetCustomScalar(e.scalarCurveIndex);
		
		if (newValue > eventDef.threshold && oldValue <= eventDef.threshold)
		{
			eventDef.onGreaterThan.Send(parentActor);
		}
		else if (newValue < eventDef.threshold && oldValue >= eventDef.threshold)
		{
			eventDef.onLessThan.Send(parentActor);
		}
		
		e.currentValue = newValue;
	}
	
	if (animState->IsAnimating())
	{
		ChangeState(E_State_Active);
	}
	else
	{
		creationData->m_AnimationFinishedMsgList.Send(parentActor);
		ChangeState(E_State_Inactive);
	}
}

//
// C_AnimatedObjectAnimationController
//

C_AnimatedObjectAnimationController::C_AnimatedObjectAnimationController()
{
}

C_AnimatedObjectAnimationController::~C_AnimatedObjectAnimationController()
{
}


const I_AnimatedObjectAnimationControllerCreator* C_AnimatedObjectAnimationController::GetCreationData() const
{
	return static_cast<const I_AnimatedObjectAnimationControllerCreator*>(GetCreator().GetData());
}

F32 C_AnimatedObjectAnimationController::GetAnimationLength() const
{
	return 0.f;
}

const Level::C_AnimTValue	C_AnimatedObjectAnimationController::GetCurrentTValue(void) const
{
	return 0.f;
}

const bool	C_AnimatedObjectAnimationController::GetObjectTranslation(const S32 objectID, Maths::C_Vertex &translation) const
{
	Animation::C_JointTransform t;
	if (GetObjectTransform(objectID, t))
	{
		translation = Maths::C_Vertex(t.GetPosition()) * GetActor()->GetMatrix();
		return true;
	}
	return false;
}

const bool	C_AnimatedObjectAnimationController::GetObjectLocalTranslation(const S32 objectID, Maths::C_Vertex &translation) const
{
	Animation::C_JointTransform t;
	if (GetObjectTransform(objectID, t))
	{
		translation = Maths::C_Vertex(t.GetPosition());
		return true;
	}
	return false;
}

const bool	C_AnimatedObjectAnimationController::GetObjectRotation(const S32 objectID, Maths::C_Quaternion &rotation) const
{
	Animation::C_JointTransform t;
	if (GetObjectTransform(objectID, t))
	{
		rotation = Maths::C_Quaternion(t.GetRotation()) * Maths::C_Quaternion(GetActor()->GetMatrix().Rotation());
		return true;
	}
	return false;
}

const bool	C_AnimatedObjectAnimationController::GetObjectLocalRotation(const S32 objectID, Maths::C_Quaternion &rotation) const
{
	Animation::C_JointTransform t;
	if (GetObjectTransform(objectID, t))
	{
		rotation = Maths::C_Quaternion(t.GetRotation());
		return true;
	}
	return false;
}

const bool	C_AnimatedObjectAnimationController::GetCameraFOV(const S32 objectID, F32 &fValue) const
{
	if (m_AnimationState)
	{
		const Animation::C_Pose& pose = m_AnimationState->GetPose();
		
		const ObjectInfo& o = m_ObjectInfo[objectID];
		
		if (o.cameraFovIndex < 0)
			return false;
		
		fValue = pose.GetCustomScalar(o.cameraFovIndex);
		
		// This conversion to degrees could be moved into the animation converter.
		fValue = Maths::RadToDeg(fValue);
			
		return true;
	}
	return false;
}

const S32	C_AnimatedObjectAnimationController::GetNamedFloatCurveChannelID(const S32 objectID, const Util::C_String &name) const
{
	S32 index = -1;
	if (m_AnimationState)
	{
		// Use object name as a prefix, e.g. if name is "fov" and object is called "camera1" then search for "camera1.fov"
		const Animation::C_Pose& pose = m_AnimationState->GetPose();
		const ObjectInfo& o = m_ObjectInfo[objectID];
		Btl::String128 fullName = GetCreationData()->GetSlaveName(o.index);
		fullName += ".";
		fullName += name;
		index = pose.FindCustomScalarByName(Animation::C_StringId(fullName.Chars(), false));
	}
	return index;
}

const bool	C_AnimatedObjectAnimationController::GetNamedFloatCurveValue(const S32 objectID, const S32 channelID, F32 &fValue) const
{
	S32 index = -1;
	if (m_AnimationState && channelID >= 0)
	{
		const Animation::C_Pose& pose = m_AnimationState->GetPose();
		fValue = pose.GetCustomScalar(channelID);
		return true;
	}
	return false;
}

const U32	C_AnimatedObjectAnimationController::GetNumberOfNamedCurves(const S32 objectID) const
{
	if (!m_AnimationState)
	{
		return 0;
	}
	if (objectID < 0 || objectID >= m_ObjectInfo.Size())
	{
		return 0;
	}
	return m_ObjectInfo[objectID].GetNumScalarCurves();
}

const char*	C_AnimatedObjectAnimationController::GetNamedCurveName(const S32 objectID, U32 index) const
{
	if (objectID < 0 || objectID >= m_ObjectInfo.Size())
	{
		return NULL;
	}

	const ObjectInfo& o = m_ObjectInfo[objectID];

	const Animation::C_Pose& pose = m_AnimationState->GetPose();

	S32 scalarIndex = o.scalarCurveIndices[index];
	Animation::C_StringId scalarName = pose.GetCustomScalarName(scalarIndex);
	BIZ_ASSERT(scalarName.IsValid());

	const CHAR8* objectName = GetCreationData()->GetSlaveName(o.index);
	const CHAR8* curveName = scalarName.GetString();

	// Skip past object name in the scalar name string.
	// For example, with the name "Object1.a_curve", skip along until we're left with ".a_curve"
	while (*objectName == *curveName && *objectName && *curveName)
	{
		++objectName;
		++curveName;
	}
	BIZ_ASSERT(!*objectName);				// Should have reached end of object name string.
	BIZ_ASSERT(*curveName == '.'); // Separator should be between object name and curve name.

	// Skip past separator to leave the final curve name.
	++curveName;

	return curveName;
}


#ifndef CFG_GOLD
void C_AnimatedObjectAnimationController::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
}
#endif

bool C_AnimatedObjectAnimationController::CreateImpl()
{
	if (!I_AnimationController::CreateImpl())
		return false;

	m_AnimationState = 0;
	const Actor::C_Actor* parent = GetActor();
	if (!parent->QueryInterface(&m_AnimationState))
	{
		return false;
	}
	BIZ_ASSERT(m_AnimationState);

	return true;
}

void C_AnimatedObjectAnimationController::InternalDependenciesCreatedImpl()
{
	// inform children that we are their controller.
	ActorMsg::C_AnimationSetControllerMsg msg;
	msg.SetActorHandle(GetActorHandle());
	
	const I_AnimatedObjectAnimationControllerCreator* create = GetCreationData();
	
	const Animation::C_Pose& pose = m_AnimationState->GetPose();
	
	m_ObjectInfo.Clear();
	
	for (U32 i = 0, count = create->GetNumSlaves(); i < count; ++i)
	{
		const Actor::C_ActorHandle childHandle = create->GetSlaveActorHandle(i);
		
		if (GetActorList()->GetActorPtr(childHandle))
		{
			S32 objectId = m_ObjectInfo.Size();
			msg.SetValue(objectId);
			
			m_ObjectInfo.PushBack();
			m_ObjectInfo.Back().actorHandle = childHandle;
			m_ObjectInfo.Back().index = i;
			
			// Lookup transform.
			m_ObjectInfo.Back().transformIndex = pose.FindJointTransformByName(Animation::C_StringId(create->GetSlaveName(i), false));
			
			// Lookup camera FOV.
			Btl::String128 slaveNameWithDot = create->GetSlaveName(i);
			slaveNameWithDot += '.';
			Btl::String128 str = slaveNameWithDot;
			str += "fov";
			m_ObjectInfo.Back().cameraFovIndex = pose.FindCustomScalarByName(Animation::C_StringId(str.Chars(), false));

			// Count number of curves.
			m_ObjectInfo.Back().scalarCurveIndices.Clear();
			for (U32 curveIndex = 0, numCurves = pose.GetNumCustomScalars(); curveIndex < numCurves; ++curveIndex)
			{
				if (Animation::StringUtil::HasPrefix(pose.GetCustomScalarName(curveIndex).GetString(), slaveNameWithDot.Chars()))
				{
					m_ObjectInfo.Back().scalarCurveIndices.PushBack(curveIndex);
				}
			}
						
			// Inform child actor of its controller and object ID.
			SendMessageToActor(childHandle, &msg);
		}
	}
}

bool C_AnimatedObjectAnimationController::GetObjectTransform(S32 objectId, Animation::C_JointTransform& outTransform) const
{
	BIZ_ASSERT(m_AnimationState);
	
	const ObjectInfo& o = m_ObjectInfo[objectId];
	if (o.transformIndex < 0)
		return false;
	
	const Animation::C_Pose& pose = m_AnimationState->GetPose();
	outTransform = pose.GetModelJointTransform(o.transformIndex, m_AnimationState->GetSkeleton());
	return true;
}

}//namespace Components
