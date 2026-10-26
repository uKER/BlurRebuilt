#include "SharedActorsPch.h"

#include <shared_actors/Components/Creation/AnimatedObjectComponentsCreate.h>
#include <shared_actors/XtActorBase.h>
#include <resource_utils/GameModel.h>
#include <resource_utils/SkeletonResourceDef.h>
#include <resource_utils/CycleResourceDef.h>
#include <animation/skeleton/WindParams.h>

XT_TYPE(Components::C_AnimationPlayerCreator::C_CycleListItem, "AnimationPlayerComponentCycleListItem")
	XT_FIELD(m_Cycle, "Cycle")
	XT_FIELD(m_Speed, "Speed")
XT_END_TYPE

XT_TYPE(Components::C_AnimationPlayerCreator, "AnimationPlayerComponent")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_DefaultCycleIndex, "DefaultCycleIndex")
XT_FIELD(m_CycleList, "CycleList")
XT_FIELD(m_Skeleton, "Skeleton")
XT_FIELD(m_WindParams, "WindParams")
//XT_FIELD(m_AnimationClip, "AnimationClip")
XT_FIELD(m_StartPlaying, "StartPlaying")
XT_FIELD(m_AutoRestart, "AutoRestart")
XT_FIELD(m_Looping, "Looping")
XT_FIELD(m_LoadCyclesLater, "LoadCyclesLater")
XT_END_TYPE

XT_TYPE(Components::C_AnimationPlayerCreator::C_XtAnimationWindParams, "AnimationWindParams")
XT_FIELD(m_Direction, "Direction")
XT_FIELD(m_DirectionVariation, "DirectionVariation")
XT_FIELD(m_Strength, "Strength")
XT_FIELD(m_StrengthVariation, "StrengthVariation")
XT_FIELD(m_VariationRate, "VariationRate")
XT_END_TYPE

XT_ABSTRACT_TYPE(Components::C_AnimatedObjectVisualCreator, "AnimatedObjectVisualComponent")
XT_BASE(Components::I_GameComponentCreator)
XT_END_TYPE

XT_ABSTRACT_TYPE(Components::C_AnimatedModelCreator::C_XtDropShadowPlacement, "DropShadowPlacement")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedModelCreator::C_XtPlaceOnBone, "PlaceDropShadowOnBone")
XT_BASE(Components::C_AnimatedModelCreator::C_XtDropShadowPlacement)
XT_FIELD(boneName, "BoneName")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedModelCreator::C_XtPlaceBetweenBones, "PlaceDropShadowBetweenBones")
XT_BASE(Components::C_AnimatedModelCreator::C_XtDropShadowPlacement)
XT_FIELD(boneName1, "BoneName1")
XT_FIELD(boneName2, "BoneName2")
XT_FIELD(blend, "Blend")
XT_FIELD(useLowestYPosition, "UseLowestYPosition")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedModelCreator::C_XtDropShadow, "DropShadow")
XT_FIELD(placement, "Placement")
XT_FIELD(offset, "Offset")
XT_FIELD(radius, "Radius")
XT_FIELD(intensity, "Intensity")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedModelCreator::C_XtDropShadowList, "DropShadowList")
XT_CALLBACK(Components::C_AnimatedModelCreator::C_XtDropShadowList::XtCallback)
XT_FIELD(dropShadows, "DropShadows")
XT_END_TYPE

XT_ABSTRACT_TYPE(Components::C_AnimatedModelCreator::LightSamplerPosition, "AnimatedModelComponent_LightSamplerPosition")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedModelCreator::ActorLsp, "AnimatedModelComponent_ActorLsp")
	XT_BASE(Components::C_AnimatedModelCreator::LightSamplerPosition)
	XT_FIELD(localOffset,	"LocalOffset")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedModelCreator::BoundingSphereCentreLsp, "AnimatedModelComponent_BoundingSphereCentreLsp")
	XT_BASE(Components::C_AnimatedModelCreator::LightSamplerPosition)
XT_END_TYPE

XT_TYPE(Components::C_AnimatedModelCreator::BoneLsp, "AnimatedModelComponent_BoneLsp")
	XT_BASE(Components::C_AnimatedModelCreator::LightSamplerPosition)
	XT_FIELD(boneName,	"BoneName")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedModelCreator::WorldPointLsp, "AnimatedModelComponent_WorldPointLsp")
	XT_BASE(Components::C_AnimatedModelCreator::LightSamplerPosition)
	XT_FIELD(worldPoint,	"WorldPoint")
XT_END_TYPE


XT_TYPE(Components::C_AnimatedModelCreator::ModelPart, "AnimatedModelComponentModelPart")
XT_FIELD(model,	"GameModel")
XT_FIELD(modelIndex, "ModelIndex")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedModelCreator::ModelInstance, "AnimatedModelComponentModelInstance")
XT_FIELD(modelParts,	"ModelParts")
XT_FIELD(skeleton,	"Skeleton")
XT_FIELD(namespacePrefix, "NamespacePrefix")
XT_FIELD(lightSamplerPosition, "LightSamplerPosition")
XT_FIELD(dropShadows, "DropShadows")
XT_FIELD(drawDistance, "DrawDistance")
XT_FIELD(disableMotionBlur, "DisableMotionBlur")
XT_FIELD(enableScale, "EnableScale")
XT_END_TYPE

void Components::C_AnimatedModelCreator::C_XtDropShadowList::XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtDropShadowList* pDropShadowList, XtCallbackContext* pContext)
{
	static U32 updateCounter = 1;
	if (eventType == kXtEtConstruct)
	{
		//pDropShadowList->checksum = Btl::HashRaw(reinterpret_cast<const U8*>(&pDropShadowList->dropShadows[0]), sizeof(C_XtDropShadow) * pDropShadowList->dropShadows.count, pDropShadowList->dropShadows.count + 1);
		pDropShadowList->checksum = updateCounter++;
		
		if (!updateCounter) updateCounter = 1;
	}
}
XT_TYPE(Components::C_AnimatedModelCreator, "AnimatedModelComponent")
XT_BASE(Components::C_AnimatedObjectVisualCreator)
XT_FIELD(m_StartHidden, "StartHidden")
XT_FIELD(m_EnableCollision, "EnableCollision")
XT_FIELD(m_ModelInstances, "ModelInstances")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedParticleEffectsCreator::Effect, "AnimatedParticleEffectsComponent_Effect")
	XT_FIELD(boneNamePattern, "BoneNamePattern")
	XT_FIELD(particleEvent, "ParticleEvent")
	XT_FIELD(specialFx, "SpecialFX")
	XT_FIELD(intensityAttributeName, "IntensityAttributeName")
	XT_FIELD(initialIntensity, "InitialIntensity")
	XT_FIELD(intensityScale, "IntensityScale")
	XT_FIELD(spawnFromCollisionMesh, "SpawnFromCollisionMesh")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedParticleEffectsCreator, "AnimatedParticleEffectsComponent")
	XT_BASE(Components::I_GameComponentCreator)
	XT_FIELD(m_StartHidden, "StartHidden")
	XT_FIELD(m_Effects, "Effects")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedEventTriggerCreator::Event, "AnimatedEventTriggerComponent_Event")	
	XT_FIELD(curveNamePattern, "CurveNamePattern")
	XT_FIELD(threshold, "Threshold")
	XT_FIELD(onGreaterThan, "OnGreaterThan")
	XT_FIELD(onLessThan, "OnLessThan")
XT_END_TYPE

XT_TYPE(Components::C_AnimatedEventTriggerCreator, "AnimatedEventTriggerComponent")
	XT_BASE(Components::I_GameComponentCreator)
	XT_FIELD(m_Events, "Events")
	XT_FIELD(m_AnimationFinishedMsgList, "OnAnimationFinished")
XT_END_TYPE

XT_TYPE(Components::C_XtAnimatedObjectAnimationControllerCreator::Object, "AnimatedObjectAnimationControllerComponent_Object")
	XT_FIELD(name, "Name")
	XT_FIELD(actor, "Actor")
XT_END_TYPE

XT_TYPE(Components::C_XtAnimatedObjectAnimationControllerCreator, "AnimatedObjectAnimationControllerComponent")
	XT_BASE(Components::I_GameComponentCreator)
	XT_FIELD(m_Objects, "Objects")
XT_END_TYPE

namespace Components
{

//
// C_AnimationPlayerCreator
//
void C_AnimationPlayerCreator::C_XtAnimationWindParams::CopyToWindParams(Animation::C_WindParams& windParams) const
{
	windParams.SetWindDirection(Load(m_Direction));
	windParams.SetWindDirectionVariation(m_DirectionVariation);
	windParams.SetWindStrength(m_Strength);
	windParams.SetWindStrengthVariation(m_StrengthVariation);
	windParams.SetWindVariationRate(m_VariationRate);
}

//
// C_XtAnimatedObjectAnimationControllerCreator
//
U32	C_XtAnimatedObjectAnimationControllerCreator::GetNumSlaves() const
{
	return m_Objects.count;
}

Actor::C_ActorHandle C_XtAnimatedObjectAnimationControllerCreator::GetSlaveActorHandle(U32 slaveIndex) const
{
	if (m_Objects[slaveIndex].actor)
	{
		return m_Objects[slaveIndex].actor->GetActorHandle();
	}
	return Actor::C_ActorHandle();
}

const CHAR8* C_XtAnimatedObjectAnimationControllerCreator::GetSlaveName(U32 slaveIndex) const
{
	return m_Objects[slaveIndex].name;
}

//
// C_AnimatedObjectAnimationControllerCreator
//

void C_AnimatedObjectAnimationControllerCreator::ClearSlaves()
{
	m_Slaves.Clear();
}

void C_AnimatedObjectAnimationControllerCreator::AddSlave(Actor::C_ActorHandle actorHandle, const Util::C_String& name)
{
	C_Slave s;
	s.actorHandle = actorHandle;
	s.name = name;
	m_Slaves.PushBack(s);
}

U32	C_AnimatedObjectAnimationControllerCreator::GetNumSlaves() const
{
	return m_Slaves.Size();
}

Actor::C_ActorHandle C_AnimatedObjectAnimationControllerCreator::GetSlaveActorHandle(U32 slaveIndex) const
{
	return m_Slaves[slaveIndex].actorHandle;
}

const CHAR8* C_AnimatedObjectAnimationControllerCreator::GetSlaveName(U32 slaveIndex) const
{
	return m_Slaves[slaveIndex].name.Chars();
}

} // end Components
