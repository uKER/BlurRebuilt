#include "stdafx.h"
#include <animation/ik/IkChain.h>
#include <animation/common/AnimationMaths.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/SkeletonPose.inl>
#include <core/maths/hvec/new/IncludeGeometry.h>
#include <core/maths/hvec/new/IncludeGeometry.inl>
#include <animation/common/JointTransform.inl>

#if !defined(CFG_GOLD) && !defined(PLATFORM_SPU)
#include <system/debug/I_DebugRender.h>
#endif

namespace Animation
{

inline C_JointTransform GetNormalisedModelJointTransform(const C_Pose& pose, const C_Skeleton* skeleton, U32 jointIndex)
{
	C_JointTransform t;
	if (skeleton != NULL)
	{
		// Need to normalise entire chain!
		C_SkeletonPose skeletonPose(*skeleton, const_cast<C_Pose&>(pose));
		t = skeletonPose.GetNormalisedModelJointTransform(jointIndex);
	}
	else
	{
		 t = pose.GetJointTransform(jointIndex);
		 t.Normalise(C_JointTransform(C_JointTransform::IDENTITY));
	}
	return t;
}

inline C_JointTransform GetModelJointTransform(const C_Pose& pose, const C_Skeleton* skeleton, U32 jointIndex)
{
	if (skeleton != NULL)
	{
		C_SkeletonPose skeletonPose(*skeleton, const_cast<C_Pose&>(pose));
		return skeletonPose.GetModelJointTransform(jointIndex);
	}
	return pose.GetJointTransform(jointIndex);
}

template <typename T>
class SpuAutoPtr
{
public:
	SpuAutoPtr(T* ea)
	{
	#ifdef PLATFORM_SPU
		m_ea = ea;
		m_ptr = (T*) Spu::FetchAlloc(ea, sizeof(T));
	#else
		m_ptr = ea;
	#endif
	}
	
	~SpuAutoPtr()
	{
	#ifdef PLATFORM_SPU
		Spu::Store(m_ea, m_ptr, sizeof(T));
	#endif
	}
	
	T* operator->()
	{
		return m_ptr;
	}
		
private:
	T*					m_ptr;
#ifdef PLATFORM_SPU
	Spu::MemBaseMarker	m_memBaseMarker;
	T*					m_ea;
#endif
};

#ifndef PLATFORM_SPU

C_IkChain::C_IkChain():
	m_poleVectorOffset(Maths::Const<Maths::Point3>::Zero()),
	m_retargetRegionType(E_RetargetRegion_Global),
	m_evaluated(false),
	m_retargetingEnabled(false)
{
	for (U32 i = 0; i < E_Joint_Count; ++i)
	{
		m_jointIndices[i] = -1;
	}
}

C_IkChain::~C_IkChain()
{
}

bool C_IkChain::Create(
	const C_Skeleton& skeleton,
	const CHAR8* rootJointName,
	const CHAR8* midJointName,
	const CHAR8* effectorJointName,
	const CHAR8* poleVectorJointName,
	const Maths::Point3& poleVectorOffset)
{
	return Create(
		skeleton,
		skeleton.FindJointByName(rootJointName),
		skeleton.FindJointByName(midJointName),
		skeleton.FindJointByName(effectorJointName),
		skeleton.FindJointByName(poleVectorJointName),
		poleVectorOffset
	);
}

bool C_IkChain::Create(
	const C_Skeleton& skeleton,
	S32 rootJointIndex,
	S32 midJointIndex,
	S32 effectorJointIndex,
	S32 poleVectorJointIndex,
	const Maths::Point3& poleVectorOffset)
{
	m_evaluated = false;
	m_jointIndices[E_Joint_Root] = rootJointIndex;
	m_jointIndices[E_Joint_Mid] = midJointIndex;
	m_jointIndices[E_Joint_Effector] = effectorJointIndex;
	m_jointIndices[E_Joint_PoleVector] = poleVectorJointIndex;
	
	if (!IsValid()) return false;
	
	m_poleVectorOffset = poleVectorOffset;
	
	// Extract chain segment lengths from the base pose.
	C_JointTransform modelRootTransform = skeleton.GetModelBasePoseJointTransform(rootJointIndex);
	C_JointTransform modelMidTransform = skeleton.GetModelBasePoseJointTransform(midJointIndex);
	C_JointTransform modelEffectorTransform = skeleton.GetModelBasePoseJointTransform(effectorJointIndex);
	C_JointTransform modelPlaneJointTransform = skeleton.GetModelBasePoseJointTransform(poleVectorJointIndex);
	
	Maths::Vector3 midToEffector = modelEffectorTransform.GetPosition() - modelMidTransform.GetPosition();
	
	Maths::VF32 rootToMidDist = Maths::Distance(modelRootTransform.GetPosition(), modelMidTransform.GetPosition());
	Maths::VF32 midToEffectorDist = Maths::Length(midToEffector);
	Maths::VF32 totalDist = rootToMidDist + midToEffectorDist;
	
	m_rootToMidLength = Maths::GetF32(rootToMidDist);
	m_midToEffectorLength = Maths::GetF32(midToEffectorDist);
	m_totalLength = Maths::GetF32(totalDist);
	
	m_enableEffectorPlaneConstraint = false;
	m_enableMiddleJointPlaneConstraint = false;
	
	return true;
}

void C_IkChain::SetPoleVector(S32 poleVectorJointIndex, const Maths::Point3& poleVectorOffset)
{
	m_jointIndices[E_Joint_PoleVector] = poleVectorJointIndex;
	m_poleVectorOffset = poleVectorOffset;
}

#endif // !PLATFORM_SPU

bool C_IkChain::IsValid() const
{
	return m_jointIndices[E_Joint_Root] >= 0
		&& m_jointIndices[E_Joint_Mid] >= 0
		&& m_jointIndices[E_Joint_Effector] >= 0
		&& m_jointIndices[E_Joint_PoleVector] >= 0;
}

F32 C_IkChain::GetTotalLength() const
{
	return m_totalLength;
}


#ifndef PLATFORM_SPU
void C_IkChain::ClearTargets()
{
	m_targets.Clear();
}

U32 C_IkChain::AddTarget()
{
	BIZ_ASSERT(m_targets.Size() <= kMaxTargets);
	m_targets.PushBack(Target());
	return m_targets.Size() - 1;
}

void C_IkChain::SetNumTargets(U32 numTargets)
{
	BIZ_ASSERT(numTargets <= kMaxTargets);
	m_targets.Clear();
	m_targets.Resize(numTargets);
	
	for (U32 i = 0; i < numTargets; ++i)
	{
		m_targets[i] = Target();
	}
}

#endif

U32 C_IkChain::GetNumTargets() const
{
	return m_targets.Size();
}

void C_IkChain::ClearTargetWeights()
{
	for (U32 i = 0, count = GetNumTargets(); i < count; ++i)
	{
		m_targetWeights[i] = 0.f;
	}
}

void C_IkChain::SetTargetWeight(U32 targetIndex, F32 weight)
{
#ifndef PLATFORM_SPU
	// Disable reading of target weight from custom scalar.
	Target& t = m_targets[targetIndex];
	t.scalarIndex = -1;
#else
	// Assuming that target weight has not been set up to read from a scalar.
	#ifndef CFG_SPEED
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	BIZ_ASSERT(t->scalarIndex < 0);
	#endif
#endif
	m_targetWeights[targetIndex] = weight;
}

void C_IkChain::AccumulateTargetWeight(U32 targetIndex, F32 weight)
{
	m_targetWeights[targetIndex] += weight;
	BIZ_ASSERT(m_targetWeights[targetIndex] <= 1.f);
}

F32 C_IkChain::GetTargetWeight(U32 targetIndex) const
{
	return m_targetWeights[targetIndex];
}

#if !defined(PLATFORM_SPU)

void C_IkChain::SetTargetWeightFromCustomScalar(U32 targetIndex, S32 customScalarIndex, F32 weightScale, F32 weightOffset)
{
	Target& t = m_targets[targetIndex];
	t.scalarIndex = customScalarIndex;
	t.weightScale = weightScale;
	t.weightOffset = weightOffset;
}

#endif // !PLATFORM_SPU




void C_IkChain::SetLocalTargetRotationFromEffector(U32 targetIndex)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->rotationSource = E_Source_EffectorLocal;
	t->rotationTransformType = E_Transform_None;
}

void C_IkChain::SetLocalTargetPositionFromEffector(U32 targetIndex)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->positionSource = E_Source_EffectorLocal;
	t->positionTransformType = E_Transform_None;
}

void C_IkChain::SetTargetRotationFromEffector(U32 targetIndex)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->rotationSource = E_Source_EffectorModel;
}

void C_IkChain::SetTargetPositionFromEffector(U32 targetIndex)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->positionSource = E_Source_EffectorModel;
}

void C_IkChain::SetTargetTransform(U32 targetIndex, const C_JointTransform& transform)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->rotationSource = E_Source_Target;
	t->rotation = transform.GetRotation();
	t->positionSource = E_Source_Target;
	t->position = transform.GetPosition();
}

void C_IkChain::SetTargetPosition(U32 targetIndex, const Maths::Point3& position)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->positionSource = E_Source_Target;
	t->position = position;
}
 
void C_IkChain::SetTargetRotation(U32 targetIndex, const Maths::Quaternion& rotation)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->rotationSource = E_Source_Target;
	t->rotation = rotation;
}

void C_IkChain::SetTargetTransformFromJoint(U32 targetIndex, U32 jointIndex, const C_Pose& pose, const C_Skeleton* skeleton, const C_JointTransform& offset)
{
	C_JointTransform transform = offset * GetNormalisedModelJointTransform(pose, skeleton, jointIndex);
	SetTargetTransform(targetIndex, transform);
}

void C_IkChain::SetTargetRotationFromJoint(U32 targetIndex, U32 jointIndex, const C_Pose& pose, const C_Skeleton* skeleton, const Maths::Quaternion& offset)
{
	C_JointTransform transform = GetNormalisedModelJointTransform(pose, skeleton, jointIndex);
	SetTargetRotation(targetIndex, offset * transform.GetRotation());
}

void C_IkChain::SetTargetPositionFromJoint(U32 targetIndex, U32 jointIndex, const C_Pose& pose, const C_Skeleton* skeleton, const Maths::Point3& offset)
{
	C_JointTransform transform = GetNormalisedModelJointTransform(pose, skeleton, jointIndex);
	SetTargetPosition(targetIndex, transform.TransformPoint(offset));
}

void C_IkChain::SetTargetPositionOffset(U32 targetIndex, const Maths::Vector3& offset)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->positionTransformType = E_Transform_ModelSpace;
	t->positionOffset = offset;
}

void C_IkChain::SetTargetRotationOffset(U32 targetIndex, const Maths::Quaternion& offset)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->rotationTransformType = E_Transform_ModelSpace;
	t->rotationOffset = offset;
}

void C_IkChain::SetLocalTargetPositionOffset(U32 targetIndex, const Maths::Vector3& localOffset)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->positionTransformType = E_Transform_LocalSpace;
	t->positionOffset = localOffset;
}

void C_IkChain::SetLocalTargetRotationOffset(U32 targetIndex, const Maths::Quaternion& localOffset)
{
	SpuAutoPtr<Target> t(&m_targets[targetIndex]);
	t->rotationTransformType = E_Transform_LocalSpace;
	t->rotationOffset = localOffset;
}

void C_IkChain::SetRetargetingMatrix(const Maths::Mat43& matrix)
{
	m_retargetMatrix = matrix;	
}

void C_IkChain::SetRetargetingRotation(const Maths::Quaternion& rotation)
{
	m_retargetRotation = rotation;
}

void C_IkChain::DisableRetargeting()
{
	m_retargetingEnabled = false;
}

void C_IkChain::EnableRetargeting()
{
	m_retargetingEnabled = true;
}

void C_IkChain::SetRetargetingRegionGlobal()
{
	EnableRetargeting();
	m_retargetRegionType = E_RetargetRegion_Global;
}

void C_IkChain::SetWorldToRetargetingRegionMatrix(const Maths::Mat43& worldToRegionMatrix)
{
	m_worldToRetargetRegionMatrix = worldToRegionMatrix;
}

void C_IkChain::SetRetargetingRegionFalloff(F32 falloff)
{
	m_retargetRegionParams[3] = falloff;
}

void C_IkChain::SetRetargetingRegionCylinder(F32 front, F32 back, F32 radius)
{
	m_retargetRegionParams[0] = front;
	m_retargetRegionParams[1] = back;
	m_retargetRegionParams[2] = radius;
	m_retargetRegionType = E_RetargetRegion_Cylinder;
}

void C_IkChain::SetRetargetingRegionSphere(F32 radius)
{
	m_retargetRegionParams[2] = radius;
	m_retargetRegionType = E_RetargetRegion_Sphere;
}

void C_IkChain::SetRetargetingRegionPlane(F32 distZ)
{
	m_retargetRegionParams[0] = distZ;
	m_retargetRegionType = E_RetargetRegion_Plane;
}

void C_IkChain::SetRetargetingRegionStep(F32 distZ, F32 distY)
{
	m_retargetRegionParams[0] = distZ;
	m_retargetRegionParams[1] = distY;
	m_retargetRegionType = E_RetargetRegion_Step;
}

void C_IkChain::EnableEffectorPlaneConstraint(const Maths::Plane& plane, const Maths::Sphere& localEffectorSphere)
{
	m_enableEffectorPlaneConstraint = true;
	m_effectorConstraintPlane = plane;
	m_effectorSphere = localEffectorSphere;
}

void C_IkChain::DisableEffectorPlaneConstraint()
{
	m_enableEffectorPlaneConstraint = false;
}

void C_IkChain::EnableMiddleJointPlaneConstraint(const Maths::Plane& plane, const Maths::Sphere& localMiddleJointSphere)
{
	m_enableMiddleJointPlaneConstraint = true;
	m_middleJointConstraintPlane = plane;
	m_middleJointSphere = localMiddleJointSphere;
}

void C_IkChain::DisableMiddleJointPlaneConstraint()
{
	m_enableMiddleJointPlaneConstraint = false;
}

void C_IkChain::Evaluate(const C_Pose& pose, const C_Skeleton* skeleton, const C_JointTransform& modelWorldTransform)
{
	if (!IsValid()) return;
	
	m_evaluated = true;
	
	C_JointTransform effectorTransform	= GetModelJointTransform(pose, skeleton, m_jointIndices[E_Joint_Effector]);
	
	// At this point, could replace current effector transform with a "locked" transform.
	
	C_JointTransform targetTransform = effectorTransform;

	C_JointTransform rootTransform = GetModelJointTransform(pose, skeleton, m_jointIndices[E_Joint_Root]);	

	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();

	Maths::VF32 totalWeight		= zero;
	Maths::VF32 rotationWeight	= zero;
	Maths::VF32 positionWeight	= zero;

	Maths::Quaternion finalTargetRotation = Maths::Const<Maths::Quaternion>::Zero();
	Maths::Vector3 finalTargetPos = Maths::Const<Maths::Vector3>::Zero();

	if (m_targets.Size() > 0)
	{
		Spu::MemBaseMarker ikTargetMemBase;
	
		Target* targets = (Target*) Spu::FetchAlloc(&m_targets[0], m_targets.Size() * sizeof(Target));

		for (U32 i = 0, count = m_targets.Size(); i < count; ++i)
		{
			Target& t = targets[i];
			
			Maths::VF32 weight = Maths::MakeVF32(&m_targetWeights[i]); // Use current weight by default.
			if (t.scalarIndex >= 0)
			{
				// Use weight from custom scalar curve.
				Maths::VF32 weightOffset = Maths::MakeVF32(t.weightOffset);
				Maths::VF32 weightScale = Maths::MakeVF32(t.weightScale);
				Maths::VF32 scalarValue = pose.GetCustomScalar(t.scalarIndex);
				
				weight = Maths::MulAdd(scalarValue, weightScale, weightOffset);
				
				// Store current weight.
				m_targetWeights[i] = Maths::GetF32(weight);
			}
			
			if (weight <= zero)
			{
				continue;
			}
						
			totalWeight += weight;
			
			Maths::Point3 targetPos = effectorTransform.GetPosition();
			Maths::Quaternion targetRotation = effectorTransform.GetRotation();
			
			bool isLocalPos = false;
			bool isLocalRot = false;
			
			if (t.positionSource == E_Source_Target)
			{
				targetPos = t.position;
			}
			
			if (t.rotationSource == E_Source_Target)
			{
				targetRotation = t.rotation;
			}
				
			switch (t.positionTransformType)
			{
				case E_Transform_None:
				default:
					isLocalPos = (t.positionSource == E_Source_EffectorLocal);
					break;
				
				case E_Transform_ModelSpace:
					targetPos = targetPos  + t.positionOffset;
					break;
					
				case E_Transform_LocalSpace:
					targetPos = Maths::MakePoint3(t.positionOffset) * targetRotation + targetPos;
					break;
			}

			switch (t.rotationTransformType)
			{
				case E_Transform_None:
				default:
					isLocalRot = (t.rotationSource == E_Source_EffectorLocal);
					break;
					
				case E_Transform_ModelSpace:
					targetRotation = targetRotation * t.rotationOffset;
					break;
					
				case E_Transform_LocalSpace:
					targetRotation = t.rotationOffset * targetRotation;
					break;					
			}
			
			if (!isLocalPos)
			{
				positionWeight += weight;
				finalTargetPos = Maths::MulAdd(Maths::MakeVector3(targetPos), weight, finalTargetPos);
			}
			
			if (!isLocalRot)
			{
				rotationWeight += weight;
				finalTargetRotation = Maths::NearMulAdd(targetRotation, weight, finalTargetRotation);
			}
		}
		
	}

	// If total IK weight is less than one, then blend in the animated effector position and rotation.
	totalWeight = Maths::Max(totalWeight, Maths::Const<Maths::VF32>::One());
	
	Maths::VF32 currentEffectorPosWeight = ((totalWeight - positionWeight) / totalWeight);
	finalTargetPos = Maths::MulAdd(effectorTransform.GetTranslation(), currentEffectorPosWeight, finalTargetPos);
	
	//
	// Apply constraints, overriding target transforms.
	///
	if (m_enableEffectorPlaneConstraint)
	{
	#if 0
		// Intersect effector disc with plane.
		Maths::Point3 discCentre = (Maths::Centre(m_effectorSphere) * targetRot) + targetPos;
		Maths::Plane discPlane = Maths::MakePlane(discCentre, Maths::Const<Maths::Normal3>::AxisY() * targetRot);
		Maths::VF32 discRadius = Maths::Radius(m_effectorSphere);
		
		Maths::VF32 distToPlane = Maths::Distance(m_effectorConstraintPlane, discCentre);
		
		Maths::VF32 correctionDist = distToPlane;
		
		Maths::VF32 one = Maths::Const<Maths::VF32>::One();
		Maths::VF32 cosTheta = Maths::Normal(discPlane) | Maths::Normal(m_effectorConstraintPlane);
		Maths::VF32 sinTheta = Maths::Sqrt(one - cosTheta * cosTheta);
		
		Maths::VF32 height = discRadius * sinTheta;
		
		if (distToPlane < height)
		{
			Maths::VF32 correctionDist = distToPlane - height;
			Maths::Vector3 alignedPosition = targetPos - Maths::Normal(m_effectorConstraintPlane) * correctionDist;
			targetPos = alignedPosition;
		}
		
	#else
		// Intersect effector sphere with plane, move effector back perpendicular to plane normal until it no longer intersects.
		Maths::Sphere modelSphere = Maths::Transform(m_effectorSphere, finalTargetRotation, finalTargetPos);
		Maths::VF32 distToPlane = Maths::Distance(m_effectorConstraintPlane, Maths::Centre(modelSphere)) - Maths::Radius(modelSphere);
		Maths::Vector3 alignedPosition = finalTargetPos - Maths::Normal(m_effectorConstraintPlane) * distToPlane;
		finalTargetPos = Maths::Select(distToPlane > zero, finalTargetPos, alignedPosition);
	#endif
	}
		
	if (m_retargetingEnabled)
	{
		ApplyRetargeting(&finalTargetPos, &finalTargetRotation);
	}
	
	targetTransform.SetTranslation(finalTargetPos);
	
	// Evaluate IK.
	C_JointTransform poleVectorTransform = GetModelJointTransform(pose, skeleton, m_jointIndices[E_Joint_PoleVector]);
	C_JointTransform midTransform = GetModelJointTransform(pose, skeleton, m_jointIndices[E_Joint_Mid]);
	
	Maths::Point3 finalEffectorPos;
	Maths::Point3 finalMidPos;
	
	Maths::Vector3 poleVector = poleVectorTransform.TransformPoint(m_poleVectorOffset) - rootTransform.GetPosition();
	m_poleVector = poleVector;
	
	SolveJointPositions(
		&finalMidPos,
		&finalEffectorPos,
		rootTransform.GetPosition(),
		targetTransform.GetPosition(),
		poleVector
	);
	
	Maths::Vector3 rootToMid = midTransform.GetPosition() - rootTransform.GetPosition();
	
	// Calculate rotation to orient root joint towards the desired mid joint position.
	// FIXME: check for zero length vectors
	Maths::Quaternion rootRotDelta = CalculateShortestRotationQuaternion(
		Maths::Normalise(rootToMid),
		Maths::Normalise(finalMidPos - rootTransform.GetPosition())
	);
	
	Maths::Quaternion rootRot = rootTransform.GetRotation() * rootRotDelta;	
	
	// Taking into account the root's change in rotation (for more natural twist), calculate the rotation required
	// to orient the middle joint towards the effector.
	// FIXME: check for zero length vectors.
	Maths::Vector3 midToEffector = (effectorTransform.GetPosition() - midTransform.GetPosition()) * rootRotDelta;
	Maths::Quaternion midRotDelta = CalculateShortestRotationQuaternion(
		Maths::Normalise(midToEffector),
		Maths::Normalise(finalEffectorPos - finalMidPos)
	);
	
	midRotDelta = rootRotDelta * midRotDelta;
	
	Maths::Quaternion midRot = midTransform.GetRotation() * midRotDelta;
	
	// Blend in unaltered local space rotation into the target effector rotation.
	Maths::VF32 currentEffectorRotWeight = ((totalWeight - rotationWeight) / totalWeight);
	finalTargetRotation = Maths::NearMulAdd(effectorTransform.GetRotation() * midRotDelta, currentEffectorRotWeight, finalTargetRotation);
	targetTransform.SetRotation(Maths::Normalise(finalTargetRotation));
	
	m_targetTransform = targetTransform;

	Maths::Quaternion effectorRot = targetTransform.GetRotation();
		
	m_jointTransforms[E_Joint_Root] = C_JointTransform(rootRot, rootTransform.GetTranslation());
	m_jointTransforms[E_Joint_Mid] = C_JointTransform(midRot, Maths::MakeVector3(finalMidPos));
	m_jointTransforms[E_Joint_Effector] = C_JointTransform(effectorRot, Maths::MakeVector3(finalEffectorPos));
}
		
void C_IkChain::Apply(C_Pose& pose, const C_Skeleton* skeleton) const
{
	if (!IsValid()) return;
	
	if (skeleton != NULL)
	{
		C_SkeletonPose skeletonPose(*skeleton, pose);
		
		skeletonPose.SetModelJointTransform(m_jointIndices[E_Joint_Root], m_jointTransforms[E_Joint_Root]);
		skeletonPose.SetModelJointTransform(m_jointIndices[E_Joint_Mid], m_jointTransforms[E_Joint_Mid]);
		skeletonPose.SetModelJointTransform(m_jointIndices[E_Joint_Effector], m_jointTransforms[E_Joint_Effector]);
	}
	else
	{
		pose.GetJointTransform(m_jointIndices[E_Joint_Root]) = m_jointTransforms[E_Joint_Root];
		pose.GetJointTransform(m_jointIndices[E_Joint_Mid]) = m_jointTransforms[E_Joint_Mid];
		pose.GetJointTransform(m_jointIndices[E_Joint_Effector]) = m_jointTransforms[E_Joint_Effector];
	}
}
		
void C_IkChain::EvaluateAndApply(C_Pose& pose, const C_Skeleton* skeleton, const C_JointTransform& modelWorldTransform)
{
	Evaluate(pose, skeleton, modelWorldTransform);
	Apply(pose, skeleton);
}

void C_IkChain::SolveJointPositions(
	Maths::Point3* outMidPos,
	Maths::Point3* outEffectorPos,
	const Maths::Point3& rootPos,
	const Maths::Point3& targetEffectorPos,
	const Maths::Vector3& planeVector
) const
{
	Maths::VF32 length1 = Maths::MakeVF32(m_rootToMidLength);
	Maths::VF32 length2 = Maths::MakeVF32(m_midToEffectorLength);
	Maths::VF32 totalLength = Maths::MakeVF32(m_totalLength);
	
	Maths::Vector3 targetDir = targetEffectorPos - rootPos;
	Maths::VF32 targetDistSqr = Maths::LengthSquared(targetDir);
	Maths::VF32 invTargetDist = Maths::RcpSqrt(targetDistSqr);
	Maths::Normal3 targetNormal = Maths::MakeNormal3(targetDir * invTargetDist);
	Maths::VF32 targetDist = invTargetDist * targetDistSqr;
	
	targetDist = Maths::Min(targetDist, totalLength);

	// Calculate cosine of base joint angle relative to targetDir
	// Using cosine rule: cos(A) = (b^2 + c^2 - a^2) / 2bc
	Maths::VF32 num		   = length1 * length1 + targetDist*targetDist - length2 * length2;
	Maths::VF32 denom      = length1 * targetDist;
	Maths::VF32 cosTheta   = num * Maths::Rcp(denom + denom);
	
	// Limb "pop" control. At angles greater than the "pop angle", reduce cosTheta so that the limb
	// never quite reaches full extension which can look unrealistic.
	// cosTheta = cosPopAngle + (cosTheta - cosPopAngle)/4
	const Maths::VF32 kCosPopAngle = Maths::MakeVF32(0.9848f);		// cos(10)
	const Maths::VF32 quarter = Maths::Const<Maths::VF32>::Quarter();
	cosTheta = Maths::Select(cosTheta > kCosPopAngle, kCosPopAngle + (cosTheta - kCosPopAngle) * quarter, cosTheta);

	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	Maths::VF32 oneMinusCosTheta2 = one - cosTheta * cosTheta;

	// Floating point errors can push oneMinusCosTheta2 below zero so it's clamped here.
	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
	oneMinusCosTheta2 = Maths::Clamped(oneMinusCosTheta2, zero, one);

	const Maths::VF32 u = length1 * cosTheta;
	const Maths::VF32 v = length1 * Maths::Sqrt(oneMinusCosTheta2);

	Maths::Vector3 perpDir = planeVector - targetNormal * (planeVector | targetNormal);
	Maths::Normal3 perpNormal = Maths::Normalise(perpDir);

	*outMidPos = Maths::MakePoint3(targetNormal * u) + Maths::MulAdd(perpNormal, v, rootPos);
	*outEffectorPos = Maths::MulAdd(targetNormal, targetDist, rootPos);
}

#if !defined(CFG_GOLD) && !defined(PLATFORM_SPU)
void C_IkChain::DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& worldMatrix) const
{
	if (!IsValid() || !m_evaluated) return;
	
	debugRender->SetDepthEnable(false);
	
	const Maths::RGBA	kEffectorColour		(0xff, 0xff, 0x00);
	const Maths::RGBA	kPoleVectorColour   (0xff, 0x00, 0x00);
	const Maths::RGBA	kBoneColour 		(0xaa, 0xff, 0xaa);
	const Maths::RGBA	kRed  				(0xff, 0x00, 0x00);
	const Maths::RGBA	kGreen				(0x00, 0xff, 0x00);
	const Maths::RGBA	kBlue 				(0x00, 0x00, 0xff);
	const Maths::RGBA	kGrey 				(0x80, 0x80, 0x80);
	
	const F32 scale = 0.09f;
	
	Maths::HMat43 targetMatrix		= Maths::HMat43(m_targetTransform.AsMat43() * worldMatrix);
	Maths::HMat43 rootMatrix		= Maths::HMat43(m_jointTransforms[E_Joint_Root].AsMat43() * worldMatrix);
	Maths::HMat43 midMatrix			= Maths::HMat43(m_jointTransforms[E_Joint_Mid].AsMat43() * worldMatrix);
	Maths::HMat43 effectorMatrix    = Maths::HMat43(m_jointTransforms[E_Joint_Effector].AsMat43() * worldMatrix);

	// Draw line from root to effector.
	debugRender->Draw3dLine(rootMatrix.Translation(), effectorMatrix.Translation(), kBlue);
	
	// Draw effector target position.
	debugRender->Draw3dCross(targetMatrix.Translation(), scale * 0.5f, kEffectorColour);
	
	// Draw pole vector from root.
	debugRender->Draw3dArrow(rootMatrix.Translation(), Maths::HVec3(m_poleVector * worldMatrix), 1.f, scale, kPoleVectorColour);
	
	// Draw chain bones.
	debugRender->Draw3dLine(rootMatrix.Translation(), midMatrix.Translation(), kBoneColour);
	debugRender->Draw3dLine(midMatrix.Translation(), effectorMatrix.Translation(), kBoneColour);
	
	// Draw axes of each joint.
	for (U32 i = 0; i < E_Joint_PoleVector; ++i)
	{
		Maths::HMat43 jointMatrix(m_jointTransforms[i].AsMat43() * worldMatrix);
		
		debugRender->Draw3dLine(jointMatrix.Translation(), jointMatrix.Translation() + jointMatrix.AX() * scale, kRed);
		debugRender->Draw3dLine(jointMatrix.Translation(), jointMatrix.Translation() + jointMatrix.AY() * scale, kGreen);
		debugRender->Draw3dLine(jointMatrix.Translation(), jointMatrix.Translation() + jointMatrix.AZ() * scale, kBlue);
	}
	
	if (m_enableEffectorPlaneConstraint)
	{
		// Draw effector collision sphere.
		debugRender->Draw3dSphere(effectorMatrix.Translation(), Maths::GetF32(Maths::Radius(m_effectorSphere)), kGrey);
		
		// Draw plane.
		Maths::C_Normal planeNormal(Maths::Normal(m_effectorConstraintPlane));
		Maths::HF32 planeDist(Maths::DistanceToOrigin(m_effectorConstraintPlane));
		
		Maths::C_Plane modelPlane(Maths::HVec3(planeNormal) || planeDist);
		
		Maths::C_Plane worldPlane = modelPlane * worldMatrix;
				
		debugRender->Draw3dPlane(
			worldPlane,
			Maths::C_Vertex(effectorMatrix.Translation()),
			scale,
			Maths::RGBA(0x80, 0xdd, 0x80, 0xff)
		);
	}
	
	debugRender->SetDepthEnable(true);
}
#endif

void C_IkChain::ApplyRetargeting(Maths::Vector3* pTargetPos, Maths::Quaternion* pTargetRotation) const
{
	Maths::Vector3 finalTargetPos = *pTargetPos;
	Maths::Quaternion finalTargetRotation = *pTargetRotation;
	
	if (m_retargetRegionType == E_RetargetRegion_Global)
	{
		*pTargetPos = Maths::MakeVector3(Maths::MakePoint3(finalTargetPos) * m_retargetMatrix);
		*pTargetRotation = finalTargetRotation * m_retargetRotation;
		return;
	}
	
	// Transform point into retargeting region space.
	Maths::Vector3 regionPos = Maths::MakeVector3(Maths::MakePoint3(finalTargetPos) * m_worldToRetargetRegionMatrix);


	Maths::Vector4 params = Maths::Load<Maths::Vector4>(m_retargetRegionParams);

	Maths::VF32 falloff = Maths::W(params);		// falloff distance.
	
	// Calculate distance function, depending on region type.
	Maths::VF32 dist;
	switch (m_retargetRegionType)
	{
		case E_RetargetRegion_Cylinder:
		{
			Maths::VF32 front   = Maths::X(params);		// front of cylinder along Z axis (XY plane with +Z normal)
			Maths::VF32 back    = Maths::Y(params);		// back of cylinder along Z axis (XY plane with -Z normal)
			Maths::VF32 radius  = Maths::Z(params);		// radius of cylinder in XY plane

			Maths::Vector3 posXY = Maths::ZeroZ(regionPos);
			Maths::VF32 posZ = Maths::Z(regionPos);

			Maths::VF32 distFront = posZ - front;
			Maths::VF32 distBack  = back - posZ;
			Maths::VF32 distEdge  = Maths::Length(posXY) - radius;

			dist = Maths::Max(distFront, distBack);
			dist = Maths::Max(dist, distEdge);
			break;
		}

		default:
		case E_RetargetRegion_Sphere:
		{
			Maths::VF32 radius  = Maths::Z(params);		// radius of sphere
			dist = Maths::Length(regionPos) - radius;
			break;
		}

		case E_RetargetRegion_Plane:
		{
			Maths::VF32 distZ   = Maths::X(params);		// distance of plane along Z axis (normal is implied as +Z)
			dist = Maths::Z(regionPos) - distZ;
			break;
		}
		
		case E_RetargetRegion_Step:
		{
			Maths::VF32 distZ   = Maths::X(params);		// distance of plane along Z axis (normal is implied as +Z)
			Maths::VF32 distY   = Maths::Y(params);		// distance of plane along Y axis (normal is implied as +Y)
			dist = Maths::Z(regionPos) - distZ;
			dist = Maths::Max(dist, Maths::Y(regionPos) - distY);
			break;
		}
	}

	// Calculate position of retargeted point.
	Maths::Vector3 retargetedPos = Maths::MakeVector3(Maths::MakePoint3(finalTargetPos) * m_retargetMatrix);
	Maths::Quaternion retargetedRotation = finalTargetRotation * m_retargetRotation;

	// Blend between unaltered and retargeted position according to distance from retargeting region.
	Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
	Maths::VF32 one  = Maths::Const<Maths::VF32>::One();
	Maths::VF32 influence = Maths::Clamped(one - (dist / falloff), zero, one);
	finalTargetPos = Maths::Lerp(finalTargetPos, retargetedPos, influence);
	finalTargetRotation = Maths::NearestLerpNormalised(finalTargetRotation, retargetedRotation, influence);

	*pTargetRotation = finalTargetRotation;
	*pTargetPos = finalTargetPos;
}

//
// C_IkChain::Target
//

C_IkChain::Target::Target():
	positionSource(E_Source_EffectorLocal),
	rotationSource(E_Source_EffectorLocal),
	positionTransformType(E_Transform_None),
	rotationTransformType(E_Transform_None),
	positionProjection(E_Projection_None),
	rotationAlignment(E_Alignment_None),
	positionOffset(Maths::Const<Maths::Vector3>::Zero()),
	rotationOffset(Maths::Const<Maths::Quaternion>::Identity()),
	scalarIndex(-1),
	weightScale(0.f),
	weightOffset(0.f)
{
}
	
} // end Animation
