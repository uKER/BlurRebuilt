#ifndef ANIMATION_STEERINGCONTROLLER_H
#define ANIMATION_STEERINGCONTROLLER_H

namespace DbUtil
{
	class I_DebugRender;
}

namespace Animation
{
	class C_IkChain;
	
	/**
	Retargets steering wheel animations onto differently sized and positioned steering wheels.
	
	Example usage:
	
	@code
		C_SteeringController controller;
		C_IkChain left;
		C_IkChain right;
		
		// Setup
		controller.Clear();
		controller.AddArm(left, left.AddTarget());
		controller.AddArm(left, right.AddTarget());
		
		// Setup mapping from reference wheel used for animations onto the target wheel.
		controller.SetReferenceWheel(wheelMatrix, wheelRadius);
		controller.SetTargetWheel(targetWheelMatrix, targetWheelRadius);
		
		// Update IK chains...
		controller.Update();
	@endcode
	
	*/
	class C_SteeringController
	{
	public:
		static const U32 kMaxArms = 2;
		
		enum E_Axis
		{
			E_Axis_X	= 0,
			E_Axis_Y,
			E_Axis_Z
		};
		
		C_SteeringController();
		
		/// Removes all arms from the controller.
		void Clear();
		
		/// Adds an arm, returning true if successful.
		bool AddArm(C_IkChain& ikChain);
		
		/// Returns number of arms being controlled.
		U32 GetNumArms();
		
		/// Sets the local wheel normal axis (Z by default).
		void SetWheelNormalAxis(E_Axis normalAxis);
		
		/// Sets the transform and radius for the reference wheel used to create the underlying animations.
		void SetReferenceWheel(const Maths::Mat43& referenceWheelMatrix, F32 radius);
		
		/// Sets the target transform and radius for the wheel that this controller will adapt the animation to use.
		void SetTargetWheel(const Maths::Mat43& targetWheelMatrix, F32 radius);

		/// Sets the typical distance from the wheel to the character's hands when the wheel is being held.
		void SetDistanceFromWheelToHands(F32 dist);

		/// Sets the falloff distance that reduces the effect of the steering IK the further away the hands are from the wheel.
		void SetFalloffDistance(F32 falloffDist);
		
		/// Updates the arm IK chains.
		/// NB. does not set target weights at the moment.
		void Update(bool reflectTargetX = false);
		
		#ifndef CFG_GOLD
		void DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& modelWorldMatrix, F32 rotationAngle) const;
		#endif
		
	private:
		Maths::Mat43	m_invRefMatrix;
		Maths::Mat43	m_targetMatrix;
		Maths::Mat43	m_refToTargetMatrix;
		Maths::Quaternion	m_refToTargetRotation;
		C_IkChain*		m_armIkChains[kMaxArms];
		U32				m_numArms;
		F32				m_oneOverRefRadius;
		F32				m_targetRadius;
		F32				m_retargetingFalloff;
		F32				m_retargetingExpansionDist;
		E_Axis			m_normalAxis;
		bool			m_reflectX;
		bool			m_dirty;
	};
}

#endif
