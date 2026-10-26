#ifndef _ACTORPHYSICS_H
#define _ACTORPHYSICS_H


#include <actor_system/ActorDeclare.h>


namespace Actors
{
	struct C_ConstraintTypeBase;
}

namespace Components
{
	class I_PhysicsBody;
}

namespace Gravity
{
	class C_Body;
}

namespace SharedPhysics
{
	class C_Constraint;
}

namespace Sys
{
	class C_ContextualStream;
}


//
// Auxhiliary actor physics declarations.
//
namespace ActorPhysics
{
	extern Debug::Bool gLogForces;

	typedef SharedPhysics::C_Constraint* TConstraintHandle;
	const TConstraintHandle KInvalidConstraintHandle = NULL;

	enum TBodyType
	{
		E_BodyType_Dynamic=0,
		E_BodyType_Static,
		E_BodyType_Animated,

		KBodyTypeCount,
		E_BodyType_Invalid
	};


	struct TBuoyancyCoeffs
	{
		TBuoyancyCoeffs() : m_Coeff(1.0f), m_MaxCoeff(1.0f), m_LinearDragCoeff(1.0f), m_AngularDragCoeff(1.0f) {}
		TBuoyancyCoeffs(F32 coeff, F32 maxCoeff, F32 linearDrag, F32 angularDrag)
			: m_Coeff(coeff==0.0f ? 1.0f : coeff),
			  m_MaxCoeff(maxCoeff==0.0f ? 1.0f : maxCoeff),
			  m_LinearDragCoeff(linearDrag==0.0f ? 1.0f : linearDrag),
			  m_AngularDragCoeff(angularDrag==0.0f ? 1.0f : angularDrag)
			  {}

		void Serialise(Sys::C_ContextualStream& stream) const;
		void Deserialise(Sys::C_ContextualStream& stream);

		F32 m_Coeff;
		F32 m_MaxCoeff;
		F32 m_LinearDragCoeff;
		F32 m_AngularDragCoeff;
	};


	TBodyType BodyType(const Gravity::C_Body* body);

	const Components::I_PhysicsBody* GetPBC(const Actor::C_Actor* pActor);

	const Gravity::C_Body* GetGravityBody(const Actor::C_Actor* pActor);

	Actor::C_ActorHandle CreateConstraint(const Actors::C_ConstraintTypeBase* constraintBase, const Maths::HMat43& matrix, const Actor::C_Actor& actorA, const Actor::C_ActorHandle& actorHandleB=Actor::C_ActorHandle());


	Btl::String Stringify(const Maths::HVec3& vec);

} // namespace ActorPhysics

#endif
