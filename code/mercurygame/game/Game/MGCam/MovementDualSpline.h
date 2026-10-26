#ifndef MOVEMENT_DUALSPLINE_H
#define MOVEMENT_DUALSPLINE_H

#pragma warning (push, 3)
#include <shared_actors/XtActorBase.h>
#include <actor_system/Actor.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>
#include <shared_actors/Components/Impl/MovementComponents.h>
#include "game/ActorComponents/GameComponents.h"
#pragma warning (pop)
#pragma warning (disable:4481) // nonstandard extension used: override specifier 'override'

namespace Actors
{
	class C_SplineActorCreate;
	class C_SplineActor;
}

namespace Cam2
{
	class C_Camera;
}
	
namespace MGCam
{
using namespace Maths;

class C_MovementDualSplineCreate : public Components::C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	static Component::C_ComponentTypeId ClassType() {return Components::E_MovementDualSpline; }
	S32 GetComponentTypeID() const {return Components::E_MovementDualSpline; }

	Actors::C_SplineActorCreate* m_Position;
	Actors::C_SplineActorCreate* m_Target;
	F32 m_AnimationTime;
};

class C_MovementDualSpline : public Components::C_MovementBase
{
public:
	C_MovementDualSpline();

public: // From C_MovementBase
	static Component::C_ComponentTypeId		ClassType() { return Components::E_MovementDualSpline; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const { return Components::E_MovementDualSpline; } // from Component::I_GameComponent
	bool OnMessage(const Messaging::C_MessageBase* pMessage) OVERRIDE;

	const C_MovementDualSplineCreate* GetCreationData() const {return static_cast<const C_MovementDualSplineCreate*>(GetCreator().GetData()); }

protected: // From C_MovementBase
	bool CreateImpl() OVERRIDE;
	void Animate(const C_TimeS& timeStep) OVERRIDE;

private:
	Actor::C_ActorHandle m_PositionHandle, m_TargetHandle;
	F32 m_Counter;
	F32 m_AnimationTime;
	bool m_Active;

	const Actors::C_SplineActor* HandleToSpline(const Actor::C_ActorHandle& handle);
};

}

#endif
