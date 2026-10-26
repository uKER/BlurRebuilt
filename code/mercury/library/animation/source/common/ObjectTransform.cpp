#include "stdafx.h"
#include <animation/common/ObjectTransform.h>
#include <animation/common/ObjectTransform.inl>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>

namespace Animation
{


C_ObjectTransform::C_ObjectTransform( const Maths::Mat43& m )
{
	const Maths::Mat33 scaleShearRotateMatrix = MakeMat33( m );
	const Maths::Vector3 translation = Maths::RowW( m );
	
	Maths::ScaleMat3 scale;
	Maths::ShearMat3 shear;
	Maths::Mat33 rotate;
	
	bool ok = Maths::DecomposeScaleShearRotate( scaleShearRotateMatrix, &scale, &shear, &rotate );
	
#ifndef CFG_SPEED
	{
		assert( ok );
		// check that shear is identity.
		const Maths::Vector3 s = Maths::GetShear( shear );
		const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
		const Maths::VF32 eps = Maths::Const<Maths::VF32>::Epsilon2();
		assert( Maths::Abs(Maths::X(s) ) < eps );
		assert( Maths::Abs(Maths::Y(s) ) < eps );
		assert( Maths::Abs(Maths::Z(s) ) < eps );
	}
#endif

	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();

	m_rotation = Maths::MakeUnitQuaternion(	rotate );
	m_translation = translation;
	m_scale = Maths::GetScale( scale );
}

C_ObjectTransform::C_ObjectTransform( const C_JointTransform& jointTransform ):
	m_rotation(jointTransform.GetRotationField()),
	m_translation(jointTransform.GetTranslationField()),
	m_scale(jointTransform.GetScaleField())
{
}

Maths::Vector4 C_ObjectTransform::s_normalisationConstants = Maths::MakeVector4(
	1024.f,				// 1/threshold
	1024.f * 1024.f,	// 1/threshold^2
	0.f,
	0.f
);
	
void C_ObjectTransform::Normalise(
	C_ObjectTransform* __restrict transforms,
	U32 count,
	const C_ObjectTransform* __restrict basePoseTransforms
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
		const C_ObjectTransform localTransform = transforms[i];
		
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
		const Maths::Vec4F32 basePoseS = Maths::MakeVec4F32(localTransform.GetScale(), one);
		const Maths::VF32 weightS = Maths::W( scaleWithWeight );
		const Maths::Vec4F32 normS = scaleWithWeight * Maths::Rcp( weightS );
		const Maths::VF32 blendS = Maths::Max( one - ( weightS * oneOverDefaultThreshold ), zero );
		const Maths::Vec4F32 finalScale = Maths::Lerp( normS, basePoseS, blendS );

		transforms[i].SetRotation( finalRotation );
		transforms[i].GetTranslationField() = finalTranslation;
		transforms[i].GetScaleField() = finalScale;
	}
}	

void C_ObjectTransform::Normalise(
	C_ObjectTransform* __restrict transforms,
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
	const Maths::Vec4F32 basePoseS = Maths::Const<Maths::Vec4F32>::One();
	
	for ( U32 i = 0; i < count; ++i )
	{
		const C_ObjectTransform localTransform = transforms[i];
		
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
		const Maths::VF32 weightS = Maths::W( scaleWithWeight );
		const Maths::Vec4F32 normS = scaleWithWeight * Maths::Rcp( weightS );
		const Maths::VF32 blendS = Maths::Max( one - ( weightS * oneOverDefaultThreshold ), zero );
		const Maths::Vec4F32 finalScale = Maths::Lerp( normS, basePoseS, blendS );

		transforms[i].SetRotation( finalRotation );
		transforms[i].GetTranslationField() = finalTranslation;
		transforms[i].GetScaleField() = finalScale;
	}
}	

void C_ObjectTransform::Normalise(
	C_ObjectTransform* __restrict transforms,
	U32 count
)
{
	const Maths::Vector4 constants = s_normalisationConstants;
	const Maths::VF32 oneOverDefaultThreshold = Maths::X(constants);
	const Maths::VF32 oneOverDefaultThresholdSquared = Maths::Y(constants);

	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	const Maths::Quaternion identityR = Maths::Const<Maths::Quaternion>::Identity();
	const Maths::Quaternion zeroR = Maths::Const<Maths::Quaternion>::Zero();
	const Maths::Vec4F32 zeroT = Maths::MakeVec4F32( Maths::Const<Maths::Vector3>::Zero(), one );
	const Maths::Vec4F32 oneS = Maths::Const<Maths::Vec4F32>::One();
	
	for ( U32 i = 0; i < count; ++i )
	{
		const C_ObjectTransform localTransform = transforms[ i ];
		
		const Maths::Quaternion rotation = localTransform.GetRotation();
		const Maths::VF32 weightRSquared = Maths::LengthSquared( rotation );
		const Maths::Quaternion normR = rotation * Maths::RcpSqrt( weightRSquared );
		const Maths::VF32 blendR = Maths::Max( one - ( weightRSquared * oneOverDefaultThresholdSquared ), zero );
		const Maths::Quaternion finalRotation = Maths::NearestLerpNormalised( normR, identityR, blendR );
		
		const Maths::Vec4F32 transWithWeight = localTransform.GetTranslationField().GetValueAndWeight();
		const Maths::VF32 weightT = Maths::W( transWithWeight );
		const Maths::Vec4F32 normT = transWithWeight * Maths::Rcp( weightT );
		const Maths::VF32 blendT = Maths::Max( one - ( weightT * oneOverDefaultThreshold ), zero );
		const Maths::Vec4F32 finalTranslation = Maths::Lerp( normT, zeroT, blendT );

		const Maths::Vec4F32 scaleWithWeight = localTransform.GetScaleField().GetValueAndWeight();
		const Maths::VF32 weightS = Maths::W( scaleWithWeight );
		const Maths::Vec4F32 normS = scaleWithWeight * Maths::Rcp( weightS );
		const Maths::VF32 blendS = Maths::Max( one - ( weightS * oneOverDefaultThreshold ), zero );
		const Maths::Vec4F32 finalScale = Maths::Lerp( normS, oneS, blendS );
				
		transforms[i].SetRotation( finalRotation );
		transforms[i].GetTranslationField() = finalTranslation;
		transforms[i].GetScaleField() = finalScale;
	}
}


} // end Animation
