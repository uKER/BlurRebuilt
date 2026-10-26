#ifndef SHARED_COMPONENTS_H
#define SHARED_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/UID/FourCC.h>
#include <actor_system/ActorComponent.h>

namespace Components
{

class I_GameComponentCreator : public Component::I_ComponentCreator
{
};

// enum for shared component interfaces
enum E_SharedComponentIF
{
	E_IF_All = -1,
	E_IF_None = 0,
	E_IF_HiBits = FOURCC_HI20('MSCI'), // Mercury Shared Component Interface

	E_IF_UpdateManager,
	E_IF_UpdateSeed,

 	E_IF_Render,
 	E_IF_PortalBlocker,
	E_IF_AnimatedShaderParams,

	E_IF_MessageSequencer,
	E_IF_Counter,
	E_IF_Timer,
	E_IF_MessageQueue,
	E_IF_MessageForwarderList,

	E_IF_LogicGate,
	E_IF_BooleanExpressionEval,

	E_IF_ElementManager,
	E_IF_DamageableElementManager,

	E_IF_Movement,

	E_IF_AnimationController,
	
	E_IF_AnimationState,			// Animation cycle based animation state.
	E_IF_AnimatedObjectVisual,
	E_IF_AnimatedParticleEffects,
	E_IF_AnimatedEventTrigger,

	E_IF_ModelResource,
	E_IF_ModelCustomiser, 

	E_IF_MotionPredictor,			///< @see I_MotionPredictor.

	E_IF_PhysicsBody,
};


// enum for our shared components
enum E_SharedComponent
{
	E_SharedComponent_HiBits = FOURCC_HI20('MSAC'), // Mercury Shared Actor Component

	// Update Managers
	E_UpdateManager_ActiveRange,
	E_UpdateManager_Stagger,
	E_UpdateManager_Counter,
	E_UpdateSeed_Singular,

	// Message Sequencers
	E_MessageScript,
	E_MessageQueue,
	E_MessageForwarderList,
	
	E_Counter,
	E_CountdownTimer,

	// Logic
	E_LogicGate,
	E_BooleanExpressionEval,

	// Render
	E_SingleElementRenderModel,
	E_SimpleMultiElementModelRender,
	E_MultiElementRenderModel,
	E_ElementManagerObjectRender, //was E_DamageObjectRenderModel
	E_TextRender,
	E_PortalBlocker,

	// Movement:
	E_MovementAttacher,
	E_MovementTranslator,
	E_MovementRotator,
	E_MovementRotTrans,
	E_MovementSpinner,
	E_MovementWheel,
	E_MovementConstrainer,
	E_MovementAnimationSlave,
	E_MovementAimer,
	E_MovementSpline,
	E_MovementRelativeToBone,
	E_MovementAimSpline,
	E_MovementBobbingBoat,
	E_MovementKinematicSpline,
	E_MovementKinematicSplineTracker,
	E_MovementKinematicSplineSlave,

	E_DamageableElementManager,

	// Actor Studio animation components:
	E_AnimationController,

	// Cycle based animation components:
	E_AnimationPlayer,
	E_AnimatedModel,
	E_AnimatedParticleEffects,
	E_AnimatedEventTrigger,
	E_AnimatedObjectAnimationController,

	E_AnimatedShaderParams,

	// Model Resources
	E_FileModelResource,
	E_FileNameModelResource,
	E_LevelModelResource,

	E_PhysicsBody,
	E_PhysObjPhysicsBody,
};

void RegisterSharedComponents(void);

} //namespace Components

#endif
