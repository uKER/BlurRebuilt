#include "SharedActorsPch.h"

#include <actor_system/ActorFactory.h>

#include <shared_actors/SharedActorTypes.h>
#include <shared_actors/ActorImpl/AnimationActors.h>
#include <shared_actors/ActorImpl/AnimatedObjectActors.h>
#include <shared_actors/ActorImpl/ConstraintActor.h>
#include <shared_actors/ActorImpl/GroupActors.h>
#include <shared_actors/ActorImpl/MessageActors.h>
#include <shared_actors/ActorImpl/SplineActor.h>
#include <shared_actors/ActorImpl/SystemActors.h>



#if ACTOR_SYSTEM_MEMORY_TRACKING
namespace Actor
{
template <> inline const char* GetMemoryTrackerNameForActorType<Actors::C_AnimatedObjectActor>() {return "Animation";}
template <> inline const char* GetMemoryTrackerNameForActorType<Actors::C_AnimationPlaybackActor>() {return "Animation";}
template <> inline const char* GetMemoryTrackerNameForActorType<Actors::C_AnimationSlaveController>() {return "Animation";}
} // namespace Actor
#endif // ACTOR_SYSTEM_MEMORY_TRACKING




namespace Actors
{

void RegisterSharedActorTypes(Actor::C_ActorFactory& actorFactory)
{
	// register actors
	actorFactory.Register<Actors::C_NullActor>();
	actorFactory.Register<Actors::C_ProxyActor>();
	actorFactory.Register<Actors::C_NodeActor>();
	actorFactory.Register<Actors::C_SplineActor>();

	// Message Actors
	actorFactory.Register<Actors::C_MessageScript>();
	actorFactory.Register<Actors::C_MessageForwarderList>();
	actorFactory.Register<Actors::C_MessageQueue>();
	actorFactory.Register<Actors::C_LogicGateMessageSender>();
	actorFactory.Register<Actors::C_BoolExpressionMessageSender>();
	actorFactory.Register<Actors::C_BoolExpressionMessageForwarder>();
	actorFactory.Register<Actors::C_CountdownTimer>();
	actorFactory.Register<Actors::C_Counter>();

	// Constraints
	actorFactory.Register<Actors::C_ConstraintActor>();
	actorFactory.Register<Actors::C_DynamicFixedConstraintActor>();
	actorFactory.Register<Actors::C_DynamicDistanceConstraintActor>();
	actorFactory.Register<Actors::C_DynamicSphericalConstraintActor>();
	actorFactory.Register<Actors::C_DynamicSliderConstraintActor>();
	actorFactory.Register<Actors::C_DynamicHingeConstraintActor>();

	// Group Actors
	actorFactory.Register<Actors::C_GroupActor>();

	// Actor Studio Animation Actors
	actorFactory.Register<Actors::C_AnimationPlaybackActor>();
	actorFactory.Register<Actors::C_AnimationSlaveController>();
	
	// Animation cycle-based animation actors
	actorFactory.Register<Actors::C_AnimatedObjectActor>();

	// Constraint creation
	Actor::C_ActorCreationDataFactory::Register<Actors::C_DynamicFixedConstraintActorCreate>(E_ActorTypeId_FixedConstraint);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_DynamicDistanceConstraintActorCreate>(E_ActorTypeId_DistanceConstraint);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_DynamicSphericalConstraintActorCreate>(E_ActorTypeId_SphericalConstraint);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_DynamicSliderConstraintActorCreate>(E_ActorTypeId_SliderConstraint);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_DynamicHingeConstraintActorCreate>(E_ActorTypeId_HingeConstraint);

	// register actor creation data
	Actor::C_ActorCreationDataFactory::Register<Actors::C_GroupActorCreateRuntime>(Actors::E_ActorTypeId_Group);

	// Message Actors
	Actor::C_ActorCreationDataFactory::Register<Actors::C_MessageScriptCreate>(E_ActorTypeId_MessageScript);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_MessageQueueCreate>(E_ActorTypeId_MessageQueue);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_CountdownTimerCreate>(E_ActorTypeId_CountdownTimer);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_NodeActorCreate>(E_ActorTypeId_Node);

}

};

XT_LIBRARY(SharedActorCreate)
	XT_DEPENDS_ON(MercuryMessageActorsCreate)
	XT_REGISTER_TYPE(Actors::C_AnimationPlaybackActorCreate)
	XT_REGISTER_TYPE(Actors::C_AnimationSlaveControllerCreate)
	XT_REGISTER_TYPE(Actors::C_AnimatedObjectActorCreate)
	XT_REGISTER_TYPE(Actors::C_NullActorCreate)
	XT_REGISTER_TYPE(Actors::C_ProxyActorCreate)
	XT_REGISTER_TYPE(Actors::C_NodeActorCreate)
	XT_REGISTER_TYPE(Actors::C_SplineActorCreate)
	XT_REGISTER_TYPE(Actors::C_GroupActorCreateXt)
XT_END_LIBRARY
