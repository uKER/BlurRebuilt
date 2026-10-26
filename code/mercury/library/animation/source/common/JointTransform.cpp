#include "stdafx.h"
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/common/ObjectTransform.h>
#include <animation/common/ObjectTransform.inl>

namespace Animation
{

Maths::Vector4 C_JointTransform::s_normalisationConstants = Maths::MakeVector4(
	1024.f,				// 1/threshold
	1024.f * 1024.f,	// 1/threshold^2
	0.f,
	0.f);

/**
	@note Any scale or shear present in matrix @a m will be removed.
*/
C_JointTransform::C_JointTransform( const Maths::Mat43& m )
{
	const Maths::Mat33 scaleShearRotateMatrix = MakeMat33( m );
	const Maths::Vector3 translation = Maths::RowW( m );
	
	Maths::ScaleMat3 scaleMat;
	Maths::ShearMat3 shear;
	Maths::Mat33 rotate;
	
	bool ok = Maths::DecomposeScaleShearRotate( scaleShearRotateMatrix, &scaleMat, &shear, &rotate );
	
	
#if 0
	// Disabled this check, since on PS3 shear for an identity matrix sometimes comes out as (1,1,1)?
	{
		BIZ_ASSERT( ok );
		// check that shear is identity.
		const Maths::Vector3 s = Maths::GetShear( shear );
		const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
		const Maths::VF32 eps = Maths::Const<Maths::VF32>::Epsilon2();
		BIZ_ASSERT( Maths::Abs(Maths::X(s) ) < eps );
		BIZ_ASSERT( Maths::Abs(Maths::Y(s) ) < eps );
		BIZ_ASSERT( Maths::Abs(Maths::Z(s) ) < eps );
	}
#endif
	
	Maths::Vector3 scale = Maths::GetScale(scaleMat);

	const Maths::Vector3 one = Maths::Const<Maths::Vector3>::One();
	const Maths::Vector3 eps = Maths::Const<Maths::Vector3>::Epsilon3();

	// Snap scale to (1,1,1) if it's close enough.
	scale = Maths::Select((scale < (one - eps)) | (scale > (one + eps)), scale, one);
	//scale = Maths::Select(scale > (one + eps), scale, one);

	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();

	m_rotation = Maths::MakeUnitQuaternion(	rotate );
	m_translation = translation;
	m_scale = scale;
}

C_JointTransform::C_JointTransform( const C_ObjectTransform& objectTransform )
{
	m_rotation = objectTransform.GetRotationField();
	m_translation = objectTransform.GetTranslationField();
	m_scale = objectTransform.GetScaleField();
}

bool C_JointTransform::IsIdentity() const
{
	const Maths::VF32 eps = Maths::Const<Maths::VF32>::Epsilon3();
	Maths::Vector4 dq = Maths::Abs(Maths::GetVector4(Maths::Const<Maths::Quaternion>::Identity() - GetRotation()));
	Maths::Vector3 dt = Maths::Abs(Maths::Const<Maths::Vector3>::Zero() - GetTranslation());
	Maths::Vector3 ds = Maths::Abs(Maths::Const<Maths::Vector3>::One() - GetScale());

	dq = Maths::FlushElementsToZero(dq, eps);
	dt = Maths::FlushElementsToZero(dt, eps);
	ds = Maths::FlushElementsToZero(ds, eps);

	Maths::Vector4 zero4 = Maths::Const<Maths::Vector4>::Zero();
	Maths::Vector3 zero3 = Maths::Const<Maths::Vector3>::Zero();

	return (dq == zero4) && (dt == zero3) && (ds == zero3);
}

void C_JointTransform::Normalise(const C_JointTransform& basePoseTransform)
{
	Normalise(this, 1, &basePoseTransform);
}

void C_JointTransform::Normalise(
	C_JointTransform* __restrict transforms,
	U32 count,
	const C_JointTransform* __restrict basePoseTransforms
)
{
	const Maths::Vector4 constants = s_normalisationConstants;
	const Maths::VF32 oneOverDefaultThreshold = Maths::X(constants);
	const Maths::VF32 oneOverDefaultThresholdSquared = Maths::Y(constants);

	const Maths::Quaternion zeroR = Maths::Const<Maths::Quaternion>::Zero();
	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
		
	for ( U32 i = 0; i < count; ++i )
	{
		const C_JointTransform localTransform = transforms[i];
		
		const Maths::Quaternion rotation = localTransform.GetRotation();
		const Maths::Quaternion basePoseR = basePoseTransforms[i].GetRotation();
		const Maths::VF32 weightRSquared = Maths::LengthSquared( rotation );
		const Maths::Quaternion normR = rotation * Maths::RcpSqrt( weightRSquared );
		const Maths::VF32 blendR = Maths::Max( one - ( weightRSquared * oneOverDefaultThresholdSquared ), zero );
		const Maths::Quaternion finalRotation = Maths::NearestLerpNormalised( normR, basePoseR, blendR );
		
		const Maths::Vec4F32 transWithWeight = localTransform.GetTranslationField().GetValueAndWeight();
		const Maths::Vec4F32 basePoseT = Maths::MakeVec4F32( basePoseTransforms[i].GetTranslation(), one );
		const Maths::VF32 weightT = Maths::W( transWithWeight );
		const Maths::Vec4F32 normT = transWithWeight * Maths::Rcp( weightT );
		const Maths::VF32 blendT = Maths::Max( one - ( weightT * oneOverDefaultThreshold ), zero );
		const Maths::Vec4F32 finalTranslation = Maths::Lerp( normT, basePoseT, blendT );

		const Maths::Vec4F32 scaleWithWeight = localTransform.GetScaleField().GetValueAndWeight();
		const Maths::Vec4F32 basePoseS = Maths::MakeVec4F32(basePoseTransforms[i].GetScale(), one);
		const Maths::VF32 weightS = Maths::W( scaleWithWeight );
		const Maths::Vec4F32 normS = scaleWithWeight * Maths::Rcp( weightS );
		const Maths::VF32 blendS = Maths::Max( one - ( weightS * oneOverDefaultThreshold ), zero );
		const Maths::Vec4F32 finalScale = Maths::Lerp( normS, basePoseS, blendS );

		transforms[i].SetRotation( finalRotation );
		transforms[i].GetTranslationField() = finalTranslation;
		transforms[i].GetScaleField() = finalScale;
	}
}

void C_JointTransform::Normalise(
	C_JointTransform* transforms,
	U32 count,
	bool unitiseZeroWeights
)
{
	const Maths::Vector4 constants = s_normalisationConstants;
	const Maths::VF32 oneOverDefaultThreshold = Maths::X(constants);
	const Maths::VF32 oneOverDefaultThresholdSquared = Maths::Y(constants);

	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	
	Maths::Quaternion defaultR;
	Maths::Vec4F32 defaultT;
	Maths::Vec4F32 defaultS;
	
	if (unitiseZeroWeights)
	{
		defaultR = Maths::Const<Maths::Quaternion>::Identity();
		defaultT = Maths::MakeVec4F32( Maths::Const<Maths::Vector3>::Zero(), one );
		defaultS = Maths::Const<Maths::Vec4F32>::One();
	}
	else
	{
		defaultR = Maths::Const<Maths::Quaternion>::Zero();
		defaultT = Maths::Const<Maths::Vec4F32>::Zero();
		defaultS = Maths::MakeVec4F32(Maths::Const<Maths::Vector3>::One(), zero);
	}
	
	for ( U32 i = 0; i < count; ++i )
	{
		const C_JointTransform localTransform = transforms[ i ];
		
		const Maths::Quaternion rotation = localTransform.GetRotation();
		const Maths::VF32 weightRSquared = Maths::LengthSquared( rotation );
		const Maths::Quaternion normR = rotation * Maths::RcpSqrt( weightRSquared );
		const Maths::VF32 blendR = Maths::Max( one - ( weightRSquared * oneOverDefaultThresholdSquared ), zero );
		const Maths::Quaternion finalRotation = Maths::NearestLerpNormalised( normR, defaultR, blendR );
		
		const Maths::Vec4F32 transWithWeight = localTransform.GetTranslationField().GetValueAndWeight();
		const Maths::VF32 weightT = Maths::W( transWithWeight );
		const Maths::Vec4F32 normT = transWithWeight * Maths::Rcp( weightT );
		const Maths::VF32 blendT = Maths::Max( one - ( weightT * oneOverDefaultThreshold ), zero );
		const Maths::Vec4F32 finalTranslation = Maths::Lerp( normT, defaultT, blendT );

		const Maths::Vec4F32 scaleWithWeight = localTransform.GetScaleField().GetValueAndWeight();
		const Maths::VF32 weightS = Maths::W( scaleWithWeight );
		const Maths::Vec4F32 normS = scaleWithWeight * Maths::Rcp( weightS );
		const Maths::VF32 blendS = Maths::Max( one - ( weightS * oneOverDefaultThreshold ), zero );
		const Maths::Vec4F32 finalScale = Maths::Lerp( normS, defaultS, blendS );
				
		transforms[i].SetRotation( finalRotation );
		transforms[i].GetTranslationField() = finalTranslation;
		transforms[i].GetScaleField() = finalScale;
	}
}


} // end Animation
