#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "gravity/sharedphysics/Integrator.h"
#include "gravity/sharedphysics/Constants.h"

namespace SharedPhysics
{

class C_RigidBodyState : public C_ODEState
{
public:
	/*union
	{
		
		struct
		{
			Maths::HVec3 m_PositionDt;
			Maths::C_Quaternion m_OrientationDt;
			Maths::HVec3 m_LinearMomentumDt;
			Maths::HVec3 m_AngularMomentumDt;
		};
		//force padding to size of HVec4 in the case of non-sse implementation
		struct
		{
			Maths::HVec4 m_VectorA;
			Maths::HVec4 m_VectorB;
			Maths::HVec4 m_VectorC;
			Maths::HVec4 m_VectorD;
		};
		
		
	};
	*/

	Maths::HVec3 m_PositionDt;
	Maths::C_Quaternion m_OrientationDt;
	Maths::HVec3 m_LinearMomentumDt;
	Maths::HVec3 m_AngularMomentumDt;
	// NOTE: hack to pad data out for integrator in non SIMD builds as HVec3 has different size.
	#ifndef _SIMD_
	Maths::HVec4 m_Pad;
	#endif
	

	C_RigidBodyState() : C_ODEState(4, (Maths::HVec4*)(&m_PositionDt)), m_PositionDt(Maths::UNDEFINED), m_OrientationDt(Maths::UNDEFINED),
		m_LinearMomentumDt(Maths::UNDEFINED), m_AngularMomentumDt(Maths::UNDEFINED)
	{
	}

	/*C_RigidBodyState() : C_ODEState(4, &m_VectorA), m_PositionDt(Maths::UNDEFINED), m_OrientationDt(Maths::UNDEFINED),
		m_LinearMomentumDt(Maths::UNDEFINED), m_AngularMomentumDt(Maths::UNDEFINED)
	{
	}*/
};

class C_RigidBody
{
public:
	//Maths::HMat33 m_Pad; //Can I get rid of this pad? Hope so
	Maths::HMat33 m_OrientationMatrix;
	Maths::HMat33 m_IBody;
	Maths::HMat33 m_IBodyInverse;
	Maths::HMat33 m_InertiaTensor;
	Maths::HMat33 m_InverseInertiaTensor;

	Maths::HVec3 m_Position;
	Maths::HVec3 m_CenterOfMassOffset;
	Maths::HVec3 m_CenterOfMass;
	Maths::C_Quaternion m_Orientation;
	Maths::HVec3 m_LinearMomentum;
	Maths::HVec3 m_AngularMomentum;
	Maths::HVec3 m_LinearVelocity;
	Maths::HVec3 m_AngularVelocity;
	Maths::HVec3 m_KineticEnergy;
	Maths::HVec3 m_Force;
	Maths::HVec3 m_Torque;

	/*Maths::HF32 m_Mass;
	Maths::HF32 m_ReciprocalMass;*/
	Maths::HVec2 m_MassCoefficients;

	//Damping coefficients
	//Maths::HF32 m_LinearDamping;
	//Maths::HF32 m_AngularDamping;
	//Maths::HF32 m_LinearDrag
	//Maths::HF32 m_AngularDrag
	Maths::HVec4 m_AerialCoefficients;

	//Fluid buoyancy and drag
	/*Maths::HF32 m_BuoyancyCoefficient;
	Maths::HF32 m_LinearDragCoefficient;
	Maths::HF32 m_AngularDragCoefficient;*/
	Maths::HVec3 m_FluidCoefficients;

	//Speed limits
	/*Maths::HF32 m_MaxLinearSpeed;
	Maths::HF32 m_MaxAngularSpeed;*/
	Maths::HVec2 m_SpeedLimits;
	Maths::Vector6 m_BVector;

public:

	C_RigidBody() : m_AerialCoefficients(Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero()),
		m_FluidCoefficients(Maths::Constants<Maths::HVec3>::One()), m_SpeedLimits(SharedPhysics::Constants::DefaultMaxSpeed(), SharedPhysics::Constants::DefaultMaxAngularSpeed()),
		m_BVector(Maths::Const<Maths::Vector6>::Zero())
	{
	}

	NOALIAS __forceinline const Maths::HF32 LinearDamping() const
	{
		return m_AerialCoefficients.X();
	}

	NOALIAS __forceinline void LinearDamping(const Maths::HF32 PCREF coefficient)
	{
		m_AerialCoefficients.X(coefficient);
	}

	NOALIAS __forceinline const Maths::HF32 AngularDamping() const
	{
		return m_AerialCoefficients.Y();
	}

	NOALIAS __forceinline void AngularDamping(const Maths::HF32 PCREF coefficient)
	{
		m_AerialCoefficients.Y(coefficient);
	}

	NOALIAS __forceinline const Maths::HF32 LinearDrag() const
	{
		return m_AerialCoefficients.Z();
	}

	NOALIAS __forceinline void LinearDrag(const Maths::HF32 PCREF coefficient)
	{
		m_AerialCoefficients.Z(coefficient);
	}

	NOALIAS __forceinline const Maths::HF32 AngularDrag() const
	{
		return m_AerialCoefficients.W();
	}

	NOALIAS __forceinline void AngularDrag(const Maths::HF32 PCREF coefficient)
	{
		m_AerialCoefficients.W(coefficient);
	}

	NOALIAS __forceinline const Maths::HF32 BuoyancyCoefficient() const
	{
		return m_FluidCoefficients.X();
	}

	NOALIAS __forceinline void BuoyancyCoefficient(const Maths::HF32 PCREF coefficient)
	{
		m_FluidCoefficients.X(coefficient);
	}

	NOALIAS __forceinline const Maths::HF32 FluidLinearDragCoefficient() const
	{
		return m_FluidCoefficients.Y();
	}

	NOALIAS __forceinline void FluidLinearDragCoefficient(const Maths::HF32 PCREF coefficient)
	{
		m_FluidCoefficients.Y(coefficient);
	}

	NOALIAS __forceinline const Maths::HF32 FluidAngularDragCoefficient() const
	{
		return m_FluidCoefficients.Z();
	}

	NOALIAS __forceinline void FluidAngularDragCoefficient(const Maths::HF32 PCREF coefficient)
	{
		m_FluidCoefficients.Z(coefficient);
	}

	
	NOALIAS __forceinline const Maths::HF32 MaxLinearSpeed() const
	{
		return m_SpeedLimits.X();
	}

	NOALIAS __forceinline void MaxLinearSpeed(const Maths::HF32 PCREF coefficient)
	{
		m_SpeedLimits.X(coefficient);
	}

	NOALIAS __forceinline const Maths::HF32 MaxAngularSpeed() const
	{
		return m_SpeedLimits.Y();
	}

	NOALIAS __forceinline void MaxAngularSpeed(const Maths::HF32 PCREF coefficient)
	{
		m_SpeedLimits.Y(coefficient);
	}

	NOALIAS __forceinline const Maths::HF32 Mass() const
	{
		return m_MassCoefficients.X();
	}

	NOALIAS __forceinline void Mass(const Maths::HF32 PCREF mass)
	{
		m_MassCoefficients.X(mass);
	}

	NOALIAS __forceinline const Maths::HF32 ReciprocalMass() const
	{
		return m_MassCoefficients.Y();
	}

	NOALIAS __forceinline void ReciprocalMass(const Maths::HF32 PCREF reciprocalMass)
	{
		m_MassCoefficients.Y(reciprocalMass);
	}

	

	Maths::HVec3 CenterOfRotation() const;

	void EulerIntegrate(const SharedPhysics::C_Integrator& integrator, const Maths::C_TimeS PCREF time);

	void RK4Integrate(const SharedPhysics::C_Integrator& integrator, const Maths::C_TimeS PCREF time);

	NOALIAS void Step(const Maths::C_TimeS PCREF time, C_RigidBodyState& result) const;

	NOALIAS void Step(const Maths::C_TimeS PCREF time, const C_RigidBodyState& initialDV, C_RigidBodyState& result, const Maths::HF32 PCREF qScale) const;

	NOALIAS void Apply( const C_RigidBodyState& derivative );

	void Damp(const Maths::HF32 PCREF linearDamp, const Maths::HF32 PCREF angularDamp);

	void CalculateBVector(const Maths::HF32 PCREF time);

	void CopyVelocityToBVector();

#ifndef CFG_SPEED
	bool ValidState();
#endif

	NOALIAS __forceinline const Maths::Scalar_Bool ValidStateVector()
	{
		return m_Position.Finite() & m_Orientation.Finite()
			& m_LinearVelocity.Finite() & m_AngularVelocity.Finite();
	}


	NOALIAS inline Maths::HVec3 PointVelocity(const Maths::HVec3 PCREF worldPos) const
	{
		const Maths::HVec3 dir(worldPos - CenterOfRotation());
		return m_LinearVelocity + (dir ^ m_AngularVelocity);
	}

	NOALIAS inline void AddForce(const Maths::HVec3 PCREF force)
	{
		m_Force += force;
	}

	NOALIAS inline void AddTorque(const Maths::HVec3 PCREF torque)
	{
		m_Torque += torque;
	}

	//Apply a force in local space, centred around body's centre of mass oriented around body's 
	//local orientation
	NOALIAS inline void ApplyForceLocalSpace(const Maths::HVec3 PCREF force, const Maths::HVec3 PCREF position)
	{
		const Maths::HVec3 newForce(force * m_OrientationMatrix);
		m_Force += newForce;
		m_Torque += newForce ^ ((position - m_CenterOfMassOffset)* m_OrientationMatrix);
	}

	NOALIAS inline void ApplyForceWorldSpace(const Maths::HVec3 PCREF force, const Maths::HVec3 PCREF position)
	{
		const Maths::HVec3 dir(position - CenterOfRotation());
		m_Force += force;
		m_Torque += force ^ dir;
	}

	NOALIAS inline void ApplyImpulseLocalSpace(const Maths::HVec3 PCREF impulse, const Maths::HVec3 PCREF position)
	{
		const Maths::HVec3 newImpulse(impulse * m_OrientationMatrix);
		m_LinearMomentum += newImpulse;
		//m_AngularMomentum += newImpulse ^ (position * m_OrientationMatrix);
		m_AngularMomentum += newImpulse ^ ((position - m_CenterOfMassOffset) * m_OrientationMatrix);
		
		m_LinearVelocity = m_LinearMomentum * ReciprocalMass();
		m_AngularVelocity = m_AngularMomentum * m_InverseInertiaTensor;
	}

	NOALIAS inline void ApplyImpulseWorldSpace(const Maths::HVec3 PCREF impulse, const Maths::HVec3 PCREF worldPos)
	{
		BIZ_ASSERT(ValidState());
		const Maths::HVec3 dir(worldPos - CenterOfRotation());
		m_LinearMomentum += impulse;
		m_AngularMomentum += impulse ^ dir;
		
		m_LinearVelocity = m_LinearMomentum * ReciprocalMass();
		m_AngularVelocity = m_AngularMomentum * m_InverseInertiaTensor;
		BIZ_ASSERT(ValidState());
	}

	NOALIAS inline void AddImpulse(const Maths::HVec3 PCREF impulse)
	{
		BIZ_ASSERT(ValidState());
		m_LinearMomentum += impulse;
		m_LinearVelocity = m_LinearMomentum * ReciprocalMass();
		BIZ_ASSERT(ValidState());
	}

	NOALIAS inline void AddAngularImpulse(const Maths::HVec3 PCREF impulse)
	{
		BIZ_ASSERT(ValidState());
		m_AngularMomentum += impulse;
		m_AngularVelocity = m_AngularMomentum * m_InverseInertiaTensor;
		BIZ_ASSERT(ValidState());
	}

	NOALIAS inline void ExtractVelocities()
	{
		m_LinearVelocity = m_LinearMomentum * ReciprocalMass();
		m_AngularVelocity = m_AngularMomentum * m_InverseInertiaTensor;
	}

	NOALIAS inline Maths::HVec3 KineticEnergy()
	{
		return m_KineticEnergy;
	}

	const Maths::HMat33 GetScaledInverseInertiaTensor(const Maths::HVec3 PCREF scale) const;

	const Maths::HMat33 GetApproxScaledInverseInertiaTensor(const Maths::HF32 PCREF scale) const;
	
	void ClampLinearAndAngularVelocity(const Maths::HF32 PCREF maxSpeed, const Maths::HF32 PCREF maxRotation);

	NOALIAS void StoreVelocitiesAndPosition(const Maths::HVec3 PCREF velocity, const Maths::HVec3 PCREF angularVelocity, 
													 const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF position);

	NOALIAS void StoreVelocitiesAndPosition(const Maths::HVec3 PCREF velocity, const Maths::HVec3 PCREF angularVelocity, 
		const Maths::C_Quaternion PCREF orientation, const Maths::HVec3 PCREF position);

	NOALIAS void StoreLinearVelocity(const Maths::HVec3 PCREF velocity);

	NOALIAS void StoreAngularVelocity(const Maths::HVec3 PCREF velocity);


};

};

#include "gravity/sharedphysics/RigidBody.inl"

#endif
