#ifndef ANIMATION_IKCHAIN_H
#define ANIMATION_IKCHAIN_H

#include <animation/common/JointTransform.h>
//#include <maths/hvec/new/IncludeGeneral.h>
//#include <maths/hvec/new/IncludeGeneral.inl>

namespace DbUtil
{
	class I_DebugRender;
}

namespace Animation
{
	class C_Skeleton;
	class C_IkTarget;
	class C_Pose;

	/**
	A 3-joint IK chain, solved analytically.

	The three joints are as follows:	
		- Root: the start of the chain, e.g. the upper arm/shoulder joint
		- Mid : the middle of the chain and the point at which it will bend, e.g. the elbow
		- Effector: the end of the chain, e.g. the hand
	
	A fourth joint is used to define the end of the "pole vector". The pole vector is a vector
	from the root joint and defines the direction of the chain's bend and the plane in which the
	chain's joints should lie.
	
	The IK chain is posed by creating one or more IK targets via AddTarget() and then setting
	target positions and/or rotations using the various SetTargetXXX() methods. The active
	targets and blended together to create a final target position and rotation for the 
	IK chain's effector. Note that if the total weight of all IK targets is less than one,
	then the current effector position will be blended in to bring the total weight up to one.
	*/

	class ALIGN16 C_IkChain
	{
	private:
		struct Target;
	public:
		C_IkChain();
		~C_IkChain();
		
		//
		// Initialisation.
		// 
	
		/// Creates the IK chain from joint name strings.
		bool Create(
			const C_Skeleton& skeleton,
			const CHAR8* rootJointName,
			const CHAR8* midJointName,
			const CHAR8* effectorJointName,
			const CHAR8* poleVectorJointName,
			const Maths::Point3& poleVectorOffset = Maths::Const<Maths::Point3>::Zero()
		);
	
		/// Creates the IK chain from joint indices.
		bool Create(
			const C_Skeleton& skeleton,
			S32 rootJointIndex,
			S32 midJointIndex,
			S32 effectorJointIndex,
			S32 poleVectorJointIndex,
			const Maths::Point3& poleVectorOffset = Maths::Const<Maths::Point3>::Zero()
		);
		
		/// Sets the IK chain's pole vector, which defines the direction of the chain's bend and the plane in which it will lie.
		void SetPoleVector(S32 poleVectorJointIndex, const Maths::Point3& poleVectorOffset = Maths::Const<Maths::Point3>::Zero());
	
		/// Returns the index of the root joint. (e.g. upper leg or upper arm/shoulder)
		S32 GetRootJointIndex() const			{ return m_jointIndices[E_Joint_Root]; }
		
		/// Returns the index of the middle joint. (e.g. elbow or knee).
		S32 GetMidJointIndex() const			{ return m_jointIndices[E_Joint_Mid]; }
		
		/// Returns the index of the effector joint. (e.g. hand).
		S32 GetEffectorJointIndex() const		{ return m_jointIndices[E_Joint_Effector]; }
		
		/// Returns true if IK chain is valid and can be evaluated.
		bool IsValid() const;
		
		/// Returns the total length of the IK chain.
		F32 GetTotalLength() const;
		
		
		//
		// List of IK targets for the chain.
		//
		
		/// Removes all IK targets.
		void ClearTargets();
		
		/// Clears the target list and resizes it to contain @a numTargets default targets with indices
		/// of 0 to numTargets - 1 inclusive.
		void SetNumTargets(U32 numTargets);
		
		/// Adds a new IK target, returning its index. By default the target will keep the effector at its current position and rotation.
		U32 AddTarget();
		
		/// Returns the number of IK targets.
		U32 GetNumTargets() const;
		
		//
		// Target weights.
		//

		/// Sets weights of all targets to zero.
		void ClearTargetWeights();
				
		/// Sets the weight of a target.
		void SetTargetWeight(U32 targetIndex, F32 weight);

		void AccumulateTargetWeight(U32 targetIndex, F32 weight);
		
		/// Gets the current weight of a target.
		F32 GetTargetWeight(U32 targetIndex) const;
		
		/// Sets the weight of a target to be that of a custom scalar at the time the IK chain is applied.
		/// The @a weightScale and @a weightOffset parameter are applied to the custom scalar value to allow
		/// modification of weights.
		void SetTargetWeightFromCustomScalar(U32 targetIndex, S32 customScalarIndex, F32 weightScale = 1.f, F32 weightOffset = 0.f);
		
		//
		// Configuring position and rotation of IK targets.
		//
		
		/// Sets the IK target rotation to match the effector's current local space rotation.
		void SetLocalTargetRotationFromEffector(U32 targetIndex);
		
		/// Sets the IK target position to match the effector's current local space position.
		void SetLocalTargetPositionFromEffector(U32 targetIndex);

		/// Sets the IK target rotation to match the effector's current model space rotation.
		void SetTargetRotationFromEffector(U32 targetIndex);
		
		/// Sets the IK target position to match the effector's current model space position.
		void SetTargetPositionFromEffector(U32 targetIndex);
		
					
		/// Sets the model space transform for an IK target.
		void SetTargetTransform(U32 targetIndex, const C_JointTransform& transform);
		 
		/// Sets the model space position for an IK target.
		void SetTargetPosition(U32 targetIndex, const Maths::Point3& position);
		 
		/// Sets the model space rotation for an IK target.
		void SetTargetRotation(U32 targetIndex, const Maths::Quaternion& rotation);
		
		
		/// Sets a target's transform to that of a joint in a pose.
		void SetTargetTransformFromJoint(
			U32 targetIndex,
			U32 jointIndex,
			const C_Pose& pose,
			const C_Skeleton* skeleton,
			const C_JointTransform& offset = C_JointTransform(C_JointTransform::IDENTITY));
		
		/// Sets a target's rotation to that of a joint in a pose.
		void SetTargetRotationFromJoint(
			U32 targetIndex,
			U32 jointIndex,
			const C_Pose& pose,
			const C_Skeleton* skeleton,
			const Maths::Quaternion& offset = Maths::Const<Maths::Quaternion>::Identity());
		
		/// Sets a target's position to that of a joint in a pose.
		void SetTargetPositionFromJoint(
			U32 targetIndex,
			U32 jointIndex,
			const C_Pose& pose,
			const C_Skeleton* skeleton,
			const Maths::Point3& offset = Maths::Const<Maths::Point3>::Zero());		
				
		//
		// Setup of IK target offsets.
		//
		
		/// Applies a model space offset to the IK target position.
		void SetTargetPositionOffset(U32 targetIndex, const Maths::Vector3& offset);

		/// Applies a model space rotation to the IK target rotation.
		void SetTargetRotationOffset(U32 targetIndex, const Maths::Quaternion& offset);
			
		/// Applies a local space (i.e. local to target position and rotation) offset to the IK target position.
		void SetLocalTargetPositionOffset(U32 targetIndex, const Maths::Vector3& localOffset);

		/// Sets the IK target rotation to be a local space rotation relative to the effector's current rotation.
		void SetLocalTargetRotationOffset(U32 targetIndex, const Maths::Quaternion& localOffset);

		//
		// Retargeting.
		//
		// After the target point for the IkChain has been calculated, a retargeting transform
		// can be applied to move the target point into a new position. Retargeting is only
		// applied if the target point lies within a certain distance (falloff) of the retargeting
		// region, defined as a volume in space. This provides a means of adapting an animation that
		// has a character interacting with an object in the world to work with objects located at
		// different places or different sizes, e.g. steering wheels.
		//

		void SetRetargetingMatrix(const Maths::Mat43& matrix);// just for points, could be rotation if scale can be accounted for.
		void SetRetargetingRotation(const Maths::Quaternion& rotation);	// implicit? just decompose in SetRetargetingMatrix?

		void EnableRetargeting();
		void DisableRetargeting();

		void SetWorldToRetargetingRegionMatrix(const Maths::Mat43& worldToRegionMatrix);
		
		void SetRetargetingRegionFalloff(F32 falloff);
		
		void SetRetargetingRegionGlobal();
		void SetRetargetingRegionCylinder(F32 frontZ, F32 backZ, F32 radius);
		void SetRetargetingRegionSphere(F32 radius);
		void SetRetargetingRegionPlane(F32 distZ);
		void SetRetargetingRegionStep(F32 distZ, F32 distY);
		
		//
		// Plane collision constraints applied to final combined target.
		// 
						
		/// Ensures that the effector will not be any closer to the specified plane than @a effectorRadius.
		void EnableEffectorPlaneConstraint(const Maths::Plane& plane, const Maths::Sphere& localEffectorSphere);
		
		void DisableEffectorPlaneConstraint();
		
		void EnableMiddleJointPlaneConstraint(const Maths::Plane& plane, const Maths::Sphere& localMiddleJointSphere);
		
		void DisableMiddleJointPlaneConstraint();
		
		//
		// Evaluation.
		//
		
		/// Calculates the final effector target and builds transforms for each joint in the chain.
		void Evaluate(const C_Pose& pose, const C_Skeleton* skeleton, const C_JointTransform& modelWorldTransform);
		
		// Applies the evaluated IK chain to a pose.
		void Apply(C_Pose& pose, const C_Skeleton* skeleton) const;
		
		/// Evalutes and applies the IK chain to a pose. Same as calling Evaluate() followed by Apply().
		void EvaluateAndApply(C_Pose& pose, const C_Skeleton* skeleton, const C_JointTransform& modelWorldTransform);

		/// Calculates the position of the mid and effector joints in order to reach the specified
		/// target effector position.
		void SolveJointPositions(
			Maths::Point3* outMidPos,
			Maths::Point3* outEffectorPos,
			const Maths::Point3& rootPos,
			const Maths::Point3& targetEffectorPos,
			const Maths::Vector3& poleVector
		) const;
		
		#ifndef CFG_GOLD
		void DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& worldMatrix) const;
		#endif

	private:

		void ApplyRetargeting(Maths::Vector3* pTargetPos, Maths::Quaternion* pTargetRotation) const;

		enum E_Joint
		{
			E_Joint_Root	= 0,
			E_Joint_Mid,
			E_Joint_Effector,
			E_Joint_PoleVector,
			E_Joint_Count
		};
		
		enum E_Transform
		{
			E_Transform_None	= 0,
			E_Transform_ModelSpace,
			E_Transform_LocalSpace
		};
		
		// Where the position or rotation of a target comes from.
		enum E_Source
		{
			E_Source_EffectorLocal	= 0,
			E_Source_EffectorModel,
			E_Source_Target
		};
		
		enum E_Projection
		{
			E_Projection_None	 = 0,
			E_Projection_NearestPointOnPlane,
			E_Projection_DropOntoPlane,
			E_Projection_NearestPointOnInfiniteCylinder,
			E_Project_Count
		};
		
		enum E_Alignment
		{
			E_Alignment_None	= 0
		};

		// Identifies the type of retargeting region
		// When effector is inside a region, a retargeting transform is applied.
		// Influence is defined by distance to region and a falloff value.
		// Useful to preserve the motion of an animation o
		enum E_RetargetRegion
		{
			E_RetargetRegion_Global = 0,	// Retargeting always on.
			E_RetargetRegion_Cylinder,		// Region is cylinder lying along Z axis.
			E_RetargetRegion_Sphere,		// Region is sphere at origin.
			E_RetargetRegion_Plane,			// Region is XY plane with normal +Z.
			E_RetargetRegion_Step			// Region is XY plane (+Z normal) and XZ plane (+Y normal).
		};
		
		struct ALIGN16 Target
		{
			Target();
			
			Maths::Point3		position;
			Maths::Quaternion	rotation;
			Maths::Vector3		positionOffset;
			Maths::Quaternion	rotationOffset;
			
			F32					weightScale;			// Scale and offset for custom scalar weights.
			F32					weightOffset;
			
			S16					scalarIndex;			// Scalar. If -1, then weight is +1.
			
			U16					positionSource			: 2;
			U16					rotationSource			: 2;
			U16					positionTransformType	: 2;
			U16					rotationTransformType	: 2;
			U16					positionProjection		: 3;
			U16					rotationAlignment		: 3;
		};

		static const U32 kMaxTargets = 4;
	
		Maths::Point3		m_poleVectorOffset;
		Btl::Vector<Target>	m_targets;
		F32					m_targetWeights[kMaxTargets];

		C_JointTransform	m_jointTransforms[E_Joint_Count];
		
		C_JointTransform	m_targetTransform;
		Maths::Vector3		m_poleVector;
		
		Maths::Plane		m_effectorConstraintPlane;
		Maths::Sphere		m_effectorSphere;
		Maths::Plane		m_middleJointConstraintPlane;
		Maths::Sphere		m_middleJointSphere;

		// Retargeting transformation applied to model space effector position.
		Maths::Mat43		m_retargetMatrix;
		Maths::Quaternion	m_retargetRotation;

		// Retargeting region matrix used to transform effector position into retargeting region space.
		Maths::Mat43		m_worldToRetargetRegionMatrix;	// for position

		// Cylinder radius, start and end.
		F32					m_retargetRegionParams[4];
		E_RetargetRegion	m_retargetRegionType;
		
		S16					m_jointIndices[E_Joint_Count];
		F32					m_rootToMidLength;
		F32					m_midToEffectorLength;
		F32					m_totalLength;
		bool				m_enableEffectorPlaneConstraint;
		bool				m_enableMiddleJointPlaneConstraint;
		bool				m_evaluated;
		bool				m_retargetingEnabled;
	};
}

#endif

