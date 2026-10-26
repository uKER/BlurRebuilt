#ifndef I_ANIMATION_CONTROLLER_H
#define I_ANIMATION_CONTROLLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/XtLevelAnimation.h>

namespace Components
{

class I_AnimationController : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_AnimationController;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_AnimationController;} // from Component::I_GameComponent

// TODO: Implement interface functions to get curve values from T values
// 	virtual const U32								GetNumAnimationChannels(void) const = 0;
// 	virtual const Level::C_XtAnimationChannel *		GetAnimationChannel(const U32 channelNo) const = 0; 

	virtual F32	GetAnimationLength() const = 0;

	virtual const Level::C_AnimTValue	GetCurrentTValue(void) const = 0;
	//virtual const Level::C_XtAnimationChannel*	GetObjectChannel(S32 objectID, U32 channelID) const = 0;

	virtual const bool	GetObjectTranslation(const S32 objectID, Maths::C_Vertex &translation) const = 0;
	virtual const bool	GetObjectLocalTranslation(const S32 objectID, Maths::C_Vertex &translation) const = 0;
	virtual const bool	GetObjectRotation(const S32 objectID, Maths::C_Quaternion &rotation) const = 0;
	virtual const bool	GetObjectLocalRotation(const S32 objectID, Maths::C_Quaternion &rotation) const = 0;
	virtual const bool	GetCameraFOV(const S32 objectID, F32 &fValue) const = 0;

	virtual const S32	GetNamedFloatCurveChannelID(const S32 objectID, const Util::C_String &name) const = 0;
	virtual const bool	GetNamedFloatCurveValue(const S32 objectID, const S32 channelID, F32 &fValue) const = 0;

	virtual const U32	GetNumberOfNamedCurves(const S32 objectID) const = 0;
	virtual const char*	GetNamedCurveName(const S32 objectID,U32 index) const = 0;

#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const = 0;
#endif
};


}//namespace Components

#endif