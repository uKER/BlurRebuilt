#include "SharedActorsPch.h"

#include <actor_system/GameComponentFactory.h>
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/TriggerComponents.h>
#include <shared_actors/Components/Impl/AnimationController.h>
#include <shared_actors/Components/Impl/AnimatedObjectComponents.h>
#include <shared_actors/Components/Impl/AnimatedShaderParams.h>
#include <shared_actors/Components/Impl/MessageComponents.h>
#include <shared_actors/Components/Impl/LogicComponents.h>
#include <shared_actors/Components/Impl/ModelResourceComponent.h>
#include <shared_actors/Components/Impl/MovementComponents.h>
#include <shared_actors/Components/Impl/MovementSpline.h>
#include <shared_actors/Components/Impl/MovementKinematicSpline.h>
#include <shared_actors/Components/Impl/PhysicsBodyComponent.h>
#include <shared_actors/Components/Impl/RenderComponents.h>
#include <shared_actors/Components/Impl/ResourceLoaderImpl.h>
#include <shared_actors/Components/Impl/TrackingImpl.h>
#include <shared_actors/Components/Impl/TriggerImpl.h>
#include <shared_actors/Components/Impl/UpdateManager.h>

XT_ABSTRACT_TYPE(Components::I_GameComponentCreator, "ComponentCreateBase")
XT_BASE(Component::I_ComponentCreator)
XT_END_TYPE

XT_LIBRARY(SharedComponentCreation)
XT_REGISTER_TYPE(Components::I_GameComponentCreator)
XT_REGISTER_TYPE(Components::C_AnimationControllerCreator)
XT_REGISTER_TYPE(Components::C_AnimationPlayerCreator)
XT_REGISTER_TYPE(Components::C_AnimationPlayerCreator::C_XtAnimationWindParams)
XT_REGISTER_TYPE(Components::C_AnimationPlayerCreator::C_CycleListItem)
XT_REGISTER_TYPE(Components::C_AnimatedObjectVisualCreator)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::C_XtDropShadowPlacement)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::C_XtPlaceOnBone)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::C_XtPlaceBetweenBones)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::C_XtDropShadow)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::C_XtDropShadowList)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::LightSamplerPosition)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::ActorLsp)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::BoundingSphereCentreLsp)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::BoneLsp)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::WorldPointLsp)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::ModelPart)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator::ModelInstance)
XT_REGISTER_TYPE(Components::C_AnimatedModelCreator)
XT_REGISTER_TYPE(Components::C_AnimatedParticleEffectsCreator::Effect)
XT_REGISTER_TYPE(Components::C_AnimatedParticleEffectsCreator)
XT_REGISTER_TYPE(Components::C_AnimatedEventTriggerCreator::Event)
XT_REGISTER_TYPE(Components::C_AnimatedEventTriggerCreator)
XT_REGISTER_TYPE(Components::C_XtAnimatedObjectAnimationControllerCreator::Object)
XT_REGISTER_TYPE(Components::C_XtAnimatedObjectAnimationControllerCreator)

XT_DEPENDS_ON(LogicComponentsCreation)
XT_DEPENDS_ON(MessageComponentCreation)
XT_DEPENDS_ON(SharedMovementComponentCreation)
XT_END_LIBRARY

namespace Components
{

void RegisterSharedComponents(void)
{
	Component::C_GameComponentFactory *factory = Component::C_GameComponentFactory::GetInstance();
	assert(factory);

	// Resource components
	factory->Register<C_ResourceItem>();
	factory->Register<C_ResourceList>();

	// Tracking components
	factory->Register<C_SpecificActorTracking>();

	// Trigger components
	factory->Register<C_BoxTrigger>();
	factory->Register<C_SphereTrigger>();
	factory->Register<C_TriggerSourceSphere>();
	factory->Register<C_TriggerSourceBox>();

	// Update managers
	factory->Register<C_ActiveRangeUpdateComponent>();
	factory->Register<C_StaggerUpdateComponent>();
	factory->Register<C_CounterUpdateComponent>();
	factory->Register<C_UpdateSeedSingularComponent>(C_UpdateSeedSingularComponent::Construct, C_UpdateSeedSingularComponent::Destroy);

	// Message Components
	factory->Register<C_MessageScript>();
	factory->Register<C_MessageQueue>();
	factory->Register<C_MessageForwarderList>();
	factory->Register<C_Counter>();
	factory->Register<C_CountdownTimer>();

	// Logic Components
	factory->Register<C_LogicGate>();
	factory->Register<C_BoolExpressionEvaluator>();

	// Render components
	factory->Register<C_ModelSimpleMultiElementRender>();
	factory->Register<C_ElementManagerObjectRender>();
	factory->Register<C_ModelSingleElementRender>();
	factory->Register<C_PortalBlocker>();

	// Movement Components
	factory->Register<C_MovementAttacher>();
	factory->Register<C_MovementTranslator>();
	factory->Register<C_MovementRotator>();
	factory->Register<C_MovementRotTrans>();
	factory->Register<C_MovementSpinner>();
	factory->Register<C_MovementWheel>();
	factory->Register<C_MovementConstrainer>();
	factory->Register<C_MovementAnimationSlave>();
	factory->Register<C_MovementAimer>();
	factory->Register<C_MovementSpline>();
	factory->Register<C_MovementKinematicSpline>();
	factory->Register<C_MovementKinematicSplineTracker>();
	factory->Register<C_MovementKinematicSplineSlave>();
	factory->Register<C_MovementRelativeToBone>();
	factory->Register<C_MovementAimSpline>();
	factory->Register<C_MovementBobbingBoat>();

	factory->Register<C_AnimationController>();
	factory->Register<C_PhysicsBody>();
	factory->Register<C_PhysicalObjectPhysicsBody>();
	
	factory->Register<C_AnimationPlayerComponent>();
	factory->Register<C_AnimatedModelComponent>();
	factory->Register<C_AnimatedParticleEffectsComponent>();
	factory->Register<C_AnimatedEventTriggerComponent>();
	factory->Register<C_AnimatedObjectAnimationController>();

	factory->Register<C_AnimatedShaderParams>();

	factory->Register<C_LevelModelResource>();
	factory->Register<C_FileModelResource>();
	factory->Register<C_FileNameModelResource>();

	// dynamic creation data
	Component::C_ComponentCreatorFactory::Register<C_ActiveRangeUpdateComponentCreator>();
	Component::C_ComponentCreatorFactory::Register<C_AnimatedShaderParamsCreator>();
	Component::C_ComponentCreatorFactory::Register<C_StaggerUpdateComponentCreator>();
	Component::C_ComponentCreatorFactory::Register<C_CounterUpdateComponentCreator>();
	Component::C_ComponentCreatorFactory::Register<C_ResourceItemCreate>();
	Component::C_ComponentCreatorFactory::Register<C_ResourceListCreate>();
	Component::C_ComponentCreatorFactory::Register<C_SpecificActorTrackingCreate>();
	Component::C_ComponentCreatorFactory::Register<C_BoxTriggerCreate>();
	Component::C_ComponentCreatorFactory::Register<C_SphereTriggerCreate>();
	Component::C_ComponentCreatorFactory::Register<C_TriggerSourceBoxCreate>();
	Component::C_ComponentCreatorFactory::Register<C_TriggerSourceSphereCreate>();
	Component::C_ComponentCreatorFactory::Register<C_UpdateSeedSingularCreator>();
	Component::C_ComponentCreatorFactory::Register<C_MovementAttacherDynamicCreateInfo>();
	Component::C_ComponentCreatorFactory::Register<C_FileModelResourceCreate>();
	Component::C_ComponentCreatorFactory::Register<C_FileNameModelResourceCreate>();
	Component::C_ComponentCreatorFactory::Register<C_LevelModelResourceCreate>();
	Component::C_ComponentCreatorFactory::Register<C_ModelSingleElementRenderCreate>();
	Component::C_ComponentCreatorFactory::Register<C_ModelSimpleMultiElementRenderCreate>();
	Component::C_ComponentCreatorFactory::Register<C_ElementManagerObjectRenderCreate>();

	// Register Components Interface Debug
	factory->RegisterComponentIFName(E_IF_AnimationController, "E_IF_AnimationController");
	factory->RegisterComponentIFName(E_IF_AnimationState, "E_IF_AnimationState");
	factory->RegisterComponentIFName(E_IF_AnimatedObjectVisual, "E_IF_AnimatedObjectVisual");
	factory->RegisterComponentIFName(E_IF_AnimatedParticleEffects, "E_IF_AnimatedParticleEffects");
	factory->RegisterComponentIFName(E_IF_AnimatedEventTrigger, "E_IF_AnimatedEventTrigger");
	factory->RegisterComponentIFName(E_IF_AnimatedShaderParams, "E_IF_AnimatedShaderParams");
	factory->RegisterComponentIFName(E_IF_UpdateManager, "E_IF_UpdateManager");
	factory->RegisterComponentIFName(E_IF_UpdateSeed, "E_IF_UpdateSeed");
	factory->RegisterComponentIFName(E_IF_Render, "E_IF_Render");
	factory->RegisterComponentIFName(E_IF_MessageSequencer, "E_IF_MessageSequencer");
	factory->RegisterComponentIFName(E_IF_MessageForwarderList, "E_IF_MessageForwarderList");
	factory->RegisterComponentIFName(E_IF_MessageQueue, "E_IF_MessageQueue");
	factory->RegisterComponentIFName(E_IF_Counter, "E_IF_Counter");
	factory->RegisterComponentIFName(E_IF_Timer, "E_IF_Timer");
	factory->RegisterComponentIFName(E_IF_Movement, "E_IF_Movement");
	factory->RegisterComponentIFName(E_IF_PhysicsBody, "E_IF_PhysicsBody");
	factory->RegisterComponentIFName(E_IF_ElementManager, "E_IF_ElementManager");
	factory->RegisterComponentIFName(E_IF_DamageableElementManager, "E_IF_DamageableElementManager");
	factory->RegisterComponentIFName(E_IF_ModelResource, "E_IF_ModelResource");
	factory->RegisterComponentIFName(E_IF_ModelCustomiser, "E_IF_ModelCustomiser");
	factory->RegisterComponentIFName(E_IF_PortalBlocker, "E_IF_PortalBlocker");

	// Trigger Components
	factory->RegisterComponentIFName(E_TriggerIF_Volume, "E_TriggerIF_Volume");	
	factory->RegisterComponentIFName(E_TriggerIF_SourceVolume, "E_TriggerIF_SourceVolume");	

	// Tracking Components
	factory->RegisterComponentIFName(E_TrackingIF_Actor, "E_TrackingIF_Actor");

}

}	// namespace Components
