#include "SharedActorsPch.h"
#include <shared_actors/SharedActorMessages.h>

#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/messaging/Messaging.h>
#include <shared_actors/ActorMessages/GroupActorMessages.h>
#include <shared_actors/ActorMessages/ModelCacheMessages.h>
#include <shared_actors/ActorMessages/ConstraintActorMessages.h>
#include <shared_actors/Components/Messages/AnimationMessages.h>
#include <shared_actors/Components/Messages/ElementManagerMessages.h>
#include <shared_actors/Components/Messages/LogicMessages.h>
#include <shared_actors/Components/Messages/MessageComponentMessages.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <shared_actors/Components/Messages/RenderMessages.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <shared_actors/Components/Messages/TrackingMessages.h>
#include <shared_actors/Components/Messages/TriggerMessages.h>
#include <shared_actors/utility/VisibilityDetection.h>

namespace ActorMsg
{
#ifndef CFG_GOLD
void RegisterSharedMessageDebugInfo()
{
	using Messaging::RegisterMessageDebugInfo;

	// E_GameActorMsg
	{
		const char* pMessageNames [] = 
		{
			"E_GameActorMsg_Enable",
			"E_GameActorMsg_Disable",
			"E_GameActorMsg_Reset",
			"E_GameActorMsg_SetUsable",
			"E_GameActorMsg_SetNotUsable",
			"E_GameActorMsg_RequestUse",	
			"E_GameActorMsg_Use",			
			"E_GameActorMsg_Activate",	
			"E_GameActorMsg_Deactivate",
			"E_GameActorMsg_Open",
			"E_GameActorMsg_Close",
			"E_GameActorMsg_Increment",
			"E_GameActorMsg_Decrement",
			"E_GameActorMsg_SetIntegerValue",
			"E_GameActorMsg_FlipFlop",
			"E_GameActorMsg_ActionBegin",
			"E_GameActorMsg_ActionCompleted",
			"E_GameActorMsg_ActionAborted",
			"E_GameActorMsg_ControlStarted",
			"E_GameActorMsg_ControlEnded",
			"E_GameActorMsg_Update2ndPass",
			"E_GameActorMsg_SetTransform",
			"E_GameActorMsg_AnimateTransform",
			"E_GameActorMsg_PhysicsConstraintDestroyed",
			"E_GameActorMsg_SetProxyActor",
			"E_GameActorMsg_UpdateSeedPosition",
			"E_GameActorMsg_SeedRequestPosition",
			"E_GameActorMsg_CountedActorPosition",
			"E_GameActorMsg_CountedActorOrder",
			"E_GameActorMsg_DisableSlowMotion",
		};

		StaticAssert(lengthof(pMessageNames) == E_GameActorMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_GameActorMsg_Count, E_GameActorMsg_Begin);
	}

	//enum E_LogicMsg
	{
		const char * pMessageNames [] =
		{
			"E_LogicMsg_SetInputTrue",
			"E_LogicMsg_SetInputFalse",
			"E_LogicMsg_SetInputActor",
			"E_LogicMsg_SetGateType",
			"E_LogicMsg_CalculateExpression",
			"E_LogicMsg_OutputTrue",
			"E_LogicMsg_OutputFalse",
		};

		StaticAssert(lengthof(pMessageNames) == E_LogicMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_LogicMsg_Count, E_LogicMsg_Begin);
	}

	//enum E_MessageForwarderMsg
	{
		const char * pMessageNames [] =
		{
			"E_MessageForwarderMsg_SetSlot",
			"E_MessageForwarderMsg_IncrementSlot",
			"E_MessageForwarderMsg_DecrementSlot",
			"E_MessageForwarderMsg_SetSelectionType",
		};

		StaticAssert(lengthof(pMessageNames) == E_MessageForwarderMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_MessageForwarderMsg_Count, E_MessageForwarderMsg_Begin);
	}

	// E_MovementMsg
	{
		const char* pMessageNames [] = 
		{
			"E_MovementMsg_SetParent",
			"E_MovementMsg_ClearParent",
			"E_MovementMsg_IncMovementCount",
			"E_MovementMsg_DecMovementCount",
			"E_MovementMsg_AddChild",
			"E_MovementMsg_RemoveChild",
			"E_MovementMsg_Update",
			"E_MovementMsg_Enable",
			"E_MovementMsg_Disable",
			"E_MovementMsg_Translate",
			"E_MovementMsg_Rotate",
			"E_MovementMsg_SpinAxisY",
			"E_MovementMsg_SpinStop",
			"E_MovementMsg_MoveAndAlignTo",
			"E_MovementMsg_MoveAndAlignToActor",
			"E_MovementMsg_Spline",
			"E_MovementMsg_KinematicSpline",
			"E_MovementMsg_KinematicSplineTracker",
			"E_MovementMsg_KinematicSplineCommand",
			"E_MovementMsg_AddSplineSlave",
			"E_MovementMsg_RemoveSplineSlave",
			"E_MovementMsg_SetSlaveMasterOffset",
			"E_MovementMsg_SetLastSlaveOffset",
			"E_MovementMsg_UpdateSlave",
			"E_MovementMsg_AimAt",
			"E_MovementMsg_AimBetween",
			"E_MovementMsg_Track",
			"E_MovementMsg_TrackLag",
			"E_MovementMsg_TrackPrediction",
			"E_MovementMsg_Constrain",
		};

		StaticAssert(lengthof(pMessageNames) == ActorMsg::E_MovementMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, ActorMsg::E_MovementMsg_Count, ActorMsg::E_MovementMsg_Begin);
	}

	// E_MovementNotificationMsg
	{
		const char* pMessageNames [] = 
		{
			"E_MovementNotificationMsg_MovementComplete",
			"E_MovementNotificationMsg_AimerAligned",
			"E_MovementNotificationMsg_AimerLostAlignment",
		};

		StaticAssert(lengthof(pMessageNames) == ActorMsg::E_MovementNotificationMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, ActorMsg::E_MovementNotificationMsg_Count, ActorMsg::E_MovementNotificationMsg_Begin);
	}

	// E_PhysicsBodyMsg
	{
		const char* pMessageNames [] = 
		{
			"E_PhysicsBodyMsg_WorldInsert",
			"E_PhysicsBodyMsg_WorldRemove",
			"E_PhysicsBodyMsg_Deactivate",
			"E_PhysicsBodyMsg_Activate",
			"E_PhysicsBodyMsg_Position",
			"E_PhysicsBodyMsg_ApplyForce",
			"E_PhysicsBodyMsg_ApplyImpulse",
			"E_PhysicsBodyMsg_ApplyLocalImpulse",
			"E_PhysicsBodyMsg_AddTorque",
			"E_PhysicsBodyMsg_SetLinearMomentum",
			"E_PhysicsBodyMsg_SetAngularMomentum",
			"E_PhysicsBodyMsg_SetLinearVelocity",
			"E_PhysicsBodyMsg_SetAngularVelocity",
			"E_PhysicsBodyMsg_SetMaxLinearVelocity",
			"E_PhysicsBodyMsg_SetMaxAngularVelocity",
			"E_PhysicsBodyMsg_SetLinearDampingFactor",
			"E_PhysicsBodyMsg_SetAngularDampingFactor",
			"E_PhysicsBodyMsg_SetMinResolveIterations",
			"E_PhysicsBodyMsg_UpdateShape",
			"E_PhysicsBodyMsg_MakeStatic",
			"E_PhysicsBodyMsg_MakeDynamic",
			"E_PhysicsBodyMsg_MakeAnimated",
			"E_PhysicsBodyMsg_SetCollideGroup",
			"E_PhysicsBodyMsg_SetCollideGroupMask",
			"E_PhysicsBodyMsg_SetCollideGroupFlags",
			"E_PhysicsBodyMsg_SetCollideGlobalFlags",
			"E_PhysicsBodyMsg_SetCollideGlobalMask",
			"E_PhysicsBodyMsg_SetCollideMaterialMask",
			"E_PhysicsBodyMsg_SetBodyFlag",
			"E_PhysicsBodyMsg_ClearBodyFlag",
			"E_PhysicsBodyMsg_SetDynamicsFlag",
			"E_PhysicsBodyMsg_SetRaytraceOnlyGroup",
			"E_PhysicsBodyMsg_ForceActive",
			"E_PhysicsBodyMsg_AllowDeactivate",
			"E_PhysicsBodyMsg_SetActivationThreshold",
			"E_PhysicsBodyMsg_SetMaxEvents",
			"E_PhysicsBodyMsg_AcceptEventType",
			"E_PhysicsBodyMsg_RejectEventType",
			"E_PhysicsBodyMsg_SetMinCollisionEventImpulse",
			"E_PhysicsBodyMsg_CollisionEvent",
			"E_PhysicsBodyMsg_MakeFrameOfReference",
			"E_PhysicsBodyMsg_DisableBuoyancy",
			"E_PhysicsBodyMsg_SetCentreOfMassOffset",

			"E_PhysicsMsg_MakeStaticOnSettled",

			"E_PhysicsBodyNotification_TransformChanged",
			"E_PhysicsBodyNotification_BodyCreated",
			"E_PhysicsBodyNotification_BodyShapeChanged",

			"E_PhysicsTriggerNotification_Enter",
			"E_PhysicsTriggerNotification_Exit",

			"E_PhysObjPhysicsBodyMsg_UpdateShape",
			"E_PhysObjPhysicsBodyMsg_OverrideCollision",
			"E_PhysObjPhysicsBodyMsg_UpdatePivotShapes"
		};

		StaticAssert(lengthof(pMessageNames) == E_PhysicsBodyMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_PhysicsBodyMsg_Count, E_PhysicsBodyMsg_Begin);
	}

	// E_PhysicsGeneralMsg
	{
		const char* pMessageNames [] = 
		{
			"E_ActorMsg_RemoveCollision",
			"E_ActorMsg_PhysicsEventCollision",
			"E_ActorMsg_PhysicsBodyEvent",
		};

		StaticAssert(lengthof(pMessageNames) == Actor::E_PhysicsGeneralMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, Actor::E_PhysicsGeneralMsg_Count, Actor::E_PhysicsGeneralMsg_Begin);
	}

	// E_ElementManagerMsg
	{
		const char* pMessageNames [] = 
		{
			"E_ElementMgrMsg_EnableElement",
//			"E_ElementMgrMsg_ReduceElementStrength",
			"E_ElementMgrMsg_ChangeElementDamageLevel",
			"E_ElementMgrMsg_DamageElement",
			"E_ElementMgrMsg_BreakElement",
			"E_ElementMgrMsg_BreakNamedElement",
			"E_ElementMgrMsg_DestroyElement",
			"E_ElementMgrMsg_DestroyNamedElement",
			"E_ElementMgrMsg_SetElementIndestructible",
			"E_ElementMgrMsg_SetAllElementsIndestructible",
			"E_ElementMgrMsg_UpdateElementMatricies",
			"E_ElementMgrMsg_UpdatePivots",
			"E_ElementMgrMsg_UpdatePivot",
			"E_ElementMgrMsg_UpdateScriptedPivot",
			"E_ElementMgrMsg_UpdatePhysicsShapes",
			"E_ElementMgrMsg_IncInflictedDamage",
			"E_ElementMgrMsg_AddDependentConstraint",
			"E_ElementMgrMsg_EnableInflictedDamageNotifications",
			"E_ElementMgrMsg_DetachElement",
			// Notifications:
			"E_ElementMgrNotification_ElementBroken",
			"E_ElementMgrNotification_ElementDestroyed",
			"E_ElementMgrNotification_ElementStateChanged",
			"E_ElementMgrNotification_MakeDynamic",
			"E_ElementMgrNotification_SpawnChild",
			"E_ElementMgrNotification_InflictedDamageValIncreased",
		};

		StaticAssert(lengthof(pMessageNames) == E_ElementMgrMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_ElementMgrMsg_Count, E_ElementMgrMsg_Begin);
	}

	// E_GroupMsg
	{
		const char* pMessageNames [] = 
		{
			"E_GroupMsg_AddActor",
			"E_GroupMsg_RemoveActor",
			"E_GroupMsg_Inherit",
			"E_GroupMsg_EnableMessages",
			"E_GroupMsg_DisableMessages",
			"E_GroupMsg_DeleteActors",
			"E_GroupMsg_SetForwardingVolume",
			"E_GroupMsg_CreateActorList",
			"E_GroupMsg_NotifyActorListCreated",
		};

		StaticAssert(lengthof(pMessageNames) == E_GroupMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_GroupMsg_Count, E_GroupMsg_Begin);
	}

	// E_ConstraintMsg
	{
		const char* pMessageNames [] = 
		{
			"E_ConstraintMsg_Destroy",
			"E_ConstraintMsg_ChangeBody",
		};

		StaticAssert(lengthof(pMessageNames) == E_ConstraintMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_ConstraintMsg_Count, E_ConstraintMsg_Begin);
	}

	// E_ResourceMsg
	{
		const char* pMessageNames [] = 
		{
			"E_ResourceMsg_Load",
			"E_ResourceMsg_Unload",
			"E_ResourceMsg_LoadAll",
			"E_ResourceMsg_UnloadAll",
			"E_ResourceMsg_Loaded",
			"E_ResourceMsg_Unloaded",
			"E_ResourceMsg_Error",
			"E_ResourceMsg_AllLoaded",
			"E_ResourceMsg_AllUnloaded",
			"E_ResourceMsg_AllCritical",
		};

		StaticAssert(lengthof(pMessageNames) == E_ResourceMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_ResourceMsg_Count, E_ResourceMsg_Begin);
	}

	// E_TrackingMsg
	{
		const char* pMessageNames [] = 
		{
			"E_TrackingMsg_Position",
			"E_TrackingMsg_Lost",
			"E_TrackingMsg_Actor",
		};

		StaticAssert(lengthof(pMessageNames) == E_TrackingMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_TrackingMsg_Count, E_TrackingMsg_Begin);
	}

	// E_TriggerMsg
	{
		const char* pMessageNames [] = 
		{
			"E_TriggerMsg_Enter",
			"E_TriggerMsg_Exit",
			"E_TriggerMsg_Crossed",
			"E_TriggerMsg_AddVolume",
			"E_TriggerMsg_VolumeParent",
			"E_TriggerMsg_SetMask",
		};

		StaticAssert(lengthof(pMessageNames) == E_TriggerMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_TriggerMsg_Count, E_TriggerMsg_Begin);
	}

	// E_AnimationMsg
	{
		const char* pMessageNames [] = 
		{
			"E_AnimationMsg_SetTValue",				
			"E_AnimationMsg_MoveToTValue",				
			"E_AnimationMsg_SetAnimationController",
		};

		StaticAssert(lengthof(pMessageNames) == E_AnimationMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_AnimationMsg_Count, E_AnimationMsg_Begin);
	}

	// E_VisibilityMsg
	{
		const char* pMessageNames [] = 
		{
			"E_VisibilityMsg_OnVisible", 
			"E_VisibilityMsg_OnHidden",
		};

		StaticAssert(lengthof(pMessageNames) == Actors::E_VisibilityMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, Actors::E_VisibilityMsg_Count, Actors::E_VisibilityMsg_Begin);
	}

	// E_ModelCacheMsg
	{
		const char* pMessageNames [] = 
		{
			"E_ModelCacheMsg_Invalidated", 
		};

		StaticAssert(lengthof(pMessageNames) == BasePhysicalObject::E_ModelCacheMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, BasePhysicalObject::E_ModelCacheMsg_Count, BasePhysicalObject::E_ModelCacheMsg_Begin);
	}

	// E_RenderMsg
	{
		const char* pMessageNames [] = 
		{
			"E_RenderMsg_SetMatrix",
			"E_RenderMsg_SetRenderOffset",
			"E_RenderMsg_Show",
			"E_RenderMsg_Hide",
			"E_RenderMsg_DisablePortalCulling",
			"E_RenderMsg_ShowElement",
			"E_RenderMsg_HideElement",
			"E_RenderMsg_SetElementMatrix",
			"E_RenderMsg_EnableMotionBlur",
			"E_RenderMsg_SetSubtechnique",
			"E_RenderMsg_UpdateAnimatedShaderParams",
			"E_RenderMsg_SetMaterialFloatParam",
			"E_RenderMsg_EnableVisibilityFlags",
			"E_RenderMsg_DisableVisibilityFlags",
			"E_RenderMsg_RequestResourceUsage",
			"E_RenderMsg_RequestResourceTotal",
			"E_RenderMsg_NotifyResourceUsage",
			"E_RenderMsg_NotifyResourceTotal",
			"E_RenderMsg_DrawResourceUsage",
			"E_RenderMsg_DumpResourceUsage",
		};

		StaticAssert(lengthof(pMessageNames) == E_RenderMsg_Count, array_size_must_match_enum_count);
		RegisterMessageDebugInfo(pMessageNames, E_RenderMsg_Count, E_RenderMsg_Begin);
	}
}
#endif

} // namespace ActorMsg

