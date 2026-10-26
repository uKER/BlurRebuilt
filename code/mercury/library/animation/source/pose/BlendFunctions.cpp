#include "stdafx.h"
#include "BlendFunctions.h"
#include "BlendFunctions.inl"
#include <animation/pose/PoseFields.h>
#include <animation/pose/PoseFields.inl>
//#include <core/maths/hvec/new/IncludeGeneral.h>
//#include <core/maths/hvec/new/IncludeGeneral.inl>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>
#include <animation/common/JointTransform.h>

/*
	Each blend function is a binary operation where the LHS is updated to the result of (LHS <op> RHS).
		
	Replace:
		LHS = If RHS.w > 0 then (RHS.xyz,1) else LHS
		TODO: set weight to RHS.w, not 1?
		
	Accumulate:
		LHS = If RHS.W > 0 then LHS + (RHS.xyz,RHS.w) * weight else LHS
		
	Lerp:
		LHS = if RHS.w > 0 then (lerp(LHS.xyz,RHS.xyz,weight), 1) else LHS
		
	Add:
		// Leave LHS weight unchanged or replace it with one?
		LHS = if RHS.w > 0 then (LHS.xyz + RHS.xyz * weight, 1) else LHS
*/

namespace Animation
{

#define BLEND_INLINE FORCE_INLINE

BLEND_INLINE Maths::Vec4F32 ReplaceW( const Maths::Vec4F32& v, const Maths::VF32& w )
{
	return Maths::MakeVec4F32( Maths::MakeVector3( v ), w );
}

// Result = lhs + rhs * alpha
BLEND_INLINE Maths::Quaternion AccumulateQuats(
	const Maths::Quaternion& lhsQ,
	const Maths::Quaternion& rhsQ,
	const Maths::VF32& alpha,
	const Maths::VF32& zero
)
{
	const Maths::VF32 rhsWeightSquared = Maths::LengthSquared(rhsQ);
	const Maths::Quaternion blendedQ = Maths::NearMulAdd( rhsQ, alpha, lhsQ );
	const Maths::Quaternion resultQ = Maths::Select( rhsWeightSquared > zero, blendedQ, lhsQ );
	return resultQ;
}

// Result = lhs + rhs * alpha
BLEND_INLINE  Maths::Vec4F32 AccumulateVec4s(
	const Maths::Vec4F32& lhs,
	const Maths::Vec4F32& rhs,
	const Maths::VF32& alpha,
	const Maths::VF32& zero,
	const Maths::VF32& one
)
{
	const Maths::Vec4F32 blended = Maths::MulAdd( rhs, alpha, lhs );
	const Maths::VF32 rhsWeight = Maths::W(rhs);
	const Maths::Vec4F32 result = Maths::Select( rhsWeight > zero, blended, lhs );
	return result;
}
/*
	result = nlerp(LHS.q, RHS.q / |RHS.q|, alpha * |RHS.q|)
*/
BLEND_INLINE  Maths::Quaternion LerpQuats(
	const Maths::Quaternion& lhsQ,
	const Maths::Quaternion& rhsQ,
	const Maths::VF32& alpha,
	const Maths::VF32& zero
)
{
	const Maths::VF32 rhsW = Maths::Length(rhsQ);
	const Maths::Quaternion rhsNormQ = rhsQ * Maths::Rcp( rhsW );
	const Maths::Quaternion blendedQ = Maths::NearestLerpNormalised( lhsQ, rhsNormQ, alpha * rhsW );
	const Maths::Quaternion resultQ = Maths::Select( rhsW > zero, blendedQ, lhsQ ); 
	return resultQ;
}

/*
	result = LHS.v + (RHS.v / RHS.w - LHS.v) * alpha * RHS.w
*/
BLEND_INLINE  Maths::Vec4F32 LerpVec4s(
	const Maths::Vec4F32& lhs,
	const Maths::Vec4F32& rhs,
	const Maths::VF32& alpha,
	const Maths::VF32& zero,
	const Maths::VF32& one
)
{
	const Maths::VF32 rhsWeight = Maths::W( rhs );
	const Maths::Vec4F32 rhsNormalised = rhs * Maths::Rcp(rhsWeight);
	const Maths::Vec4F32 blended = ReplaceW( Maths::Lerp( lhs, rhsNormalised, alpha * rhsWeight ), one );
	const Maths::Vec4F32 result = Maths::Select( rhsWeight > zero, blended, lhs );
	return result;
}

BLEND_INLINE  Maths::Quaternion AddQuats(
	const Maths::Quaternion& lhsQ,
	const Maths::Quaternion& rhsQ,
	const Maths::VF32& alpha,
	const Maths::VF32& zero
)
{
	const Maths::VF32 rhsWeight = Maths::Length( rhsQ );
	const Maths::Quaternion rhsNormQ = rhsQ * Maths::Rcp( rhsWeight );
	const Maths::Quaternion blendedQ = Maths::NearestLerpNormalised( lhsQ, rhsNormQ * lhsQ, alpha * rhsWeight );
	const Maths::Quaternion resultQ = Maths::Select( rhsWeight > zero, blendedQ, lhsQ );
	return resultQ;
}

// if rhs.w > 0 then
//	result.xyz = rhs.xyz * alpha + lhs.xyz
//	result.w = 1
// else
//	result = lhs
BLEND_INLINE  Maths::Vec4F32 AddVec4s(
	const Maths::Vec4F32& lhs,
	const Maths::Vec4F32& rhs,
	const Maths::VF32& alpha,
	const Maths::VF32& zero,
	const Maths::VF32& one
)
{		
	// replacing weights with one - OK? should be... must have been prenormalised anyway.
	// NB. assuming that RHS is non-normalised, with weight premultiplied in.
	const Maths::VF32 rhsWeight = Maths::W( rhs );
	const Maths::Vec4F32 blended = ReplaceW(Maths::MulAdd( rhs, alpha, lhs ), one);
	const Maths::Vec4F32 result = Maths::Select( rhsWeight > zero, blended, lhs );
	return result;
}

//
// Joint transform blending.
//

/*
	Assumes that LHS with weight of zero has been set to zero.
*/
void BlendJointTransformArray_Accumulate( C_JointTransform* dest, const C_JointTransform* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Quaternion q = AccumulateQuats(
			dest[i].GetRotationField(),
			src[i].GetRotationField(),
			alpha,
			zero
		);
		dest[i].GetRotationField() = q;
		
		const Maths::Vec4F32 t = AccumulateVec4s(
			dest[i].GetTranslationField().GetValueAndWeight(),
			src[i].GetTranslationField().GetValueAndWeight(),
			alpha,
			zero,
			one
		);
		dest[i].GetTranslationField() = t;
		
		const Maths::Vec4F32 s = AccumulateVec4s(
			dest[i].GetScaleField().GetValueAndWeight(),
			src[i].GetScaleField().GetValueAndWeight(),
			alpha,
			zero,
			one
		);
		dest[i].GetScaleField() = s;
	}
}

/*
	Assumes that LHS is valid.
	
	If RHS is undefined, then LHS remains unchanged.
	 
*/
void BlendJointTransformArray_Lerp( C_JointTransform* dest, const C_JointTransform* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	const Maths::Quaternion identityQuat = Maths::Const< Maths::Quaternion >::Identity();
	
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Quaternion q = LerpQuats(
			dest[i].GetRotationField(),
			src[i].GetRotationField(),
			alpha,
			zero
		);
		dest[i].GetRotationField() = q;
		
		const Maths::Vec4F32 t = LerpVec4s(
			dest[i].GetTranslationField().GetValueAndWeight(),
			src[i].GetTranslationField().GetValueAndWeight(),
			alpha,
			zero,
			one
		);
		dest[i].GetTranslationField() = t;
		
		const Maths::Vec4F32 s = LerpVec4s(
			dest[i].GetScaleField().GetValueAndWeight(),
			src[i].GetScaleField().GetValueAndWeight(),
			alpha,
			zero,
			one
		);
		dest[i].GetScaleField() = s;		
	}
}

/*
	Assumes that LHS is valid.
	Doesn't actually use alpha - blend factors are calculated using weights of RHS fields.
	When LHS is undefined, the result is undefined. Additive blending must always have a valid base layer to blend onto.
*/
void BlendJointTransformArray_Add( C_JointTransform* dest, const C_JointTransform* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	const Maths::Quaternion identityQuat = Maths::Const< Maths::Quaternion >::Identity();
	
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Quaternion q = AddQuats(
			dest[i].GetRotationField(),
			src[i].GetRotationField(),
			alpha,
			zero
		);
		dest[i].GetRotationField() = q;
		
		const Maths::Vec4F32 t = AddVec4s(
			dest[i].GetTranslationField().GetValueAndWeight(),
			src[i].GetTranslationField().GetValueAndWeight(),
			alpha,
			zero,
			one
		);
		dest[i].GetTranslationField() = t;

		const Maths::Vec4F32 s = AddVec4s(
			dest[i].GetScaleField().GetValueAndWeight(),
			src[i].GetScaleField().GetValueAndWeight(),
			alpha,
			zero,
			one
		);
		dest[i].GetScaleField() = s;
	}
}

//
// Linear quaternion array blending.
//

void BlendQuaternionArray_Replace( C_QuaternionField* dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha )
{
	for ( U32 i=  0; i < count; ++i )
	{
		dest[i] = src[i];
	}
}

void BlendQuaternionArray_Accumulate( C_QuaternionField* dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Quaternion q = AccumulateQuats(
			dest[i],
			src[i],
			alpha,
			zero
		);
		dest[i] = q;
	}
}

void BlendQuaternionArray_Lerp( C_QuaternionField* dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Quaternion q = LerpQuats(
			dest[i],
			src[i],
			alpha,
			zero
		);
		dest[i] = q;
	}
}

void BlendQuaternionArray_Add( C_QuaternionField* dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Quaternion q = AddQuats(
			dest[i],
			src[i],
			alpha,
			zero
		);
		dest[i] = q;
	}
}



//
// Indirect quaternion array blending.
//

void BlendQuaternionArrayIndirect_Accumulate( C_QuaternionField** dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Quaternion q = AccumulateQuats(
			*(dest[i]),
			src[i],
			alpha,
			zero
		);
		*(dest[i]) = q;
	}
}

void BlendQuaternionArrayIndirect_Lerp( C_QuaternionField** dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Quaternion q = LerpQuats(
			*(dest[i]),
			src[i],
			alpha,
			zero
		);
		*(dest[i]) = q;
	}
}

void BlendQuaternionArrayIndirect_Add( C_QuaternionField** dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Quaternion q = AddQuats(
			*(dest[i]),
			src[i],
			alpha,
			zero
		);
		*(dest[i]) = q;
	}
}

//
// Vec4F32 indirect array blending.
//

void BlendVec4F32ArrayIndirect_Accumulate( Maths::Vec4F32** dest, const Maths::Vec4F32* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Vec4F32 v = AccumulateVec4s(
			*dest[i],
			src[i],
			alpha,
			zero,
			one
		);
		*dest[i] = v;
	}
}

void BlendVec4F32ArrayIndirect_Lerp( Maths::Vec4F32** dest, const Maths::Vec4F32* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Vec4F32 v = LerpVec4s(
			*dest[i],
			src[i],
			alpha,
			zero,
			one
		);
		*dest[i] = v;
	}
}

void BlendVec4F32ArrayIndirect_Add( Maths::Vec4F32** dest, const Maths::Vec4F32* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Vec4F32 v = AddVec4s(
			*dest[i],
			src[i],
			alpha,
			zero,
			one
		);
		*dest[i] = v;
	}
}

//
// Vec4F32 linear array blending.
//

void BlendVec4F32Array_Accumulate( Maths::Vec4F32* dest, const Maths::Vec4F32* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Vec4F32 v = AccumulateVec4s(
			dest[i],
			src[i],
			alpha,
			zero,
			one
		);
		dest[i] = v;
	}
}

void BlendVec4F32Array_Lerp( Maths::Vec4F32* dest, const Maths::Vec4F32* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Vec4F32 v = LerpVec4s(
			dest[i],
			src[i],
			alpha,
			zero,
			one
		);
		dest[i] = v;
	}
}

void BlendVec4F32Array_Add( Maths::Vec4F32* dest, const Maths::Vec4F32* src, U32 count, const Maths::VF32& alpha )
{
	const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
	const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
	for ( U32 i=  0; i < count; ++i )
	{
		const Maths::Vec4F32 v = AddVec4s(
			dest[i],
			src[i],
			alpha,
			zero,
			one
		);
		dest[i] = v;
	}
}

C_BlendFunctionTable gBlendFuncs[ E_BlendMode_Count ] =
{
	{
		&BlendJointTransformArray_Accumulate,
		&BlendQuaternionArray_Accumulate,
		&BlendQuaternionArrayIndirect_Accumulate,
		&BlendVec4F32Array_Accumulate,
		&BlendVec4F32ArrayIndirect_Accumulate
	},
	{
		&BlendJointTransformArray_Lerp,
		&BlendQuaternionArray_Lerp,
		&BlendQuaternionArrayIndirect_Lerp,
		&BlendVec4F32Array_Lerp,
		&BlendVec4F32ArrayIndirect_Lerp
	},
	{
		&BlendJointTransformArray_Add,
		&BlendQuaternionArray_Add,
		&BlendQuaternionArrayIndirect_Add,
		&BlendVec4F32Array_Add,
		&BlendVec4F32ArrayIndirect_Add
	}
};

#undef BLEND_INLINE

} // end Animation
