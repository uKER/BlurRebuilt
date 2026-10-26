#ifndef ANIMATION_DYNAMICJOINTCONTEXT_H
#define ANIMATION_DYNAMICJOINTCONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/JointTransform.h>

namespace Animation
{

	/**
		Common variables used for updating dynamic joints.
	*/
	class C_DynamicJointContext
	{
	public:
		C_DynamicJointContext( const C_JointTransform& inModelWorldTransform );
		
		/// Returns true if dynamic joint should simulate during this update.
		bool ShouldSimulate() const;
		
		/// Returns an interpolation value for blending between previous and current
		/// dynamic joint states.
		F32 GetInterpolationAmount() const;
		
		/// Set fixed time step and its inverse.
		void SetFixedTimeStep( F32 timeStep );
		
		void SetGravity(const Maths::Vector3& worldGravity);
		
		Maths::Vector3	gravity;				///< Gravitational acceleration in world space.
		Maths::Vector3	modelGravity;			///< Gravitational acceleration in model space.
		Maths::Vector3	force;					///< World space force vector.
		F32				timeLeft;				///< Time elapsed since last simulation step.
		F32				fixedTimeStep;			///< Simulation time step size.
		F32				invFixedTimeStep;		///< Inverse of simulation time step.
		const C_JointTransform modelWorldTransform;	///< Model to world space transform.
		const C_JointTransform worldModelTransform;	///< World to model space transform (inverse of modelWorldTransform).
		Maths::Plane	worldWaterPlane;
		bool			enabled;				///< True if dynamic joints are enabled.
		bool			waterPlaneEnabled;		///< True if water plane enabled and buoyancy to applied to joints below it.
		bool			reset;					///< True if dynamic joints should be reset to sensible transforms e.g. for the first update.
	};

}

#endif
