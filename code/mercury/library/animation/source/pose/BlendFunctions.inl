#ifndef ANIMATION_BLENDFUNCTIONS_INL
#define ANIMATION_BLENDFUNCTIONS_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{
	typedef void (*JointTransformArrayBlendFuncPtr)( C_JointTransform* dest, const C_JointTransform* src, U32 count, const Maths::VF32& alpha );
	typedef void (*QuaternionArrayIndirectBlendFuncPtr)( C_QuaternionField** dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha );
	typedef void (*QuaternionArrayBlendFuncPtr)( C_QuaternionField* dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha );
	typedef void (*Vec4F32ArrayIndirectBlendFuncPtr)( Maths::Vec4F32** dest, const Maths::Vec4F32* src, U32 count, const Maths::VF32& alpha );
	typedef void (*Vec4F32ArrayBlendFuncPtr)( Maths::Vec4F32* dest, const Maths::Vec4F32* src, U32 count, const Maths::VF32& alpha );
	
	struct C_BlendFunctionTable
	{
		JointTransformArrayBlendFuncPtr		jointTransformArrayBlend;
		QuaternionArrayBlendFuncPtr			quaternionArrayBlend;
		QuaternionArrayIndirectBlendFuncPtr	quaternionArrayIndirectBlend;
		Vec4F32ArrayBlendFuncPtr			vec4F32ArrayBlend;
		Vec4F32ArrayIndirectBlendFuncPtr	vec4F32ArrayIndirectBlend;
	};
	
	extern C_BlendFunctionTable gBlendFuncs[ E_BlendMode_Count ];
	
	//
	// Single field blends.
	//

	inline void BlendQuaternion( E_BlendMode blendMode, C_QuaternionField* dest, const C_QuaternionField* src, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].quaternionArrayBlend)( dest, src, 1, alpha );
	}
	
	inline void BlendVector3( E_BlendMode blendMode, C_Vector3Field* dest, const C_Vector3Field* src, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].vec4F32ArrayBlend)( (Maths::Vec4F32*) dest, (const Maths::Vec4F32*) src, 1, alpha );
	}
	
	inline void BlendScalar( E_BlendMode blendMode, C_ScalarField* dest, const C_ScalarField* src, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].vec4F32ArrayBlend)( (Maths::Vec4F32*) dest, (const Maths::Vec4F32*) src, 1, alpha );
	}

	//
	// Array blends.
	//
	
	inline void BlendJointTransformArray( E_BlendMode blendMode, C_JointTransform* dest, const C_JointTransform* src, U32 count, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].jointTransformArrayBlend)( dest, src, count, alpha );
	}
	
	inline void BlendQuaternionArray( E_BlendMode blendMode, C_QuaternionField* dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].quaternionArrayBlend)( dest, src, count, alpha );
	}

	inline void BlendVector4Array( E_BlendMode blendMode, C_Vector4Field* dest, const C_Vector4Field* src, U32 count, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].vec4F32ArrayBlend)( (Maths::Vec4F32*) dest, (const Maths::Vec4F32*) src, count * 2, alpha );
	}
	
	inline void BlendVector3Array( E_BlendMode blendMode, C_Vector3Field* dest, const C_Vector3Field* src, U32 count, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].vec4F32ArrayBlend)( (Maths::Vec4F32*) dest, (const Maths::Vec4F32*) src, count, alpha );
	}
	
	inline void BlendScalarArray( E_BlendMode blendMode, C_ScalarField* dest, const C_ScalarField* src, U32 count, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].vec4F32ArrayBlend)( (Maths::Vec4F32*) dest, (const Maths::Vec4F32*) src, count, alpha );
	}

	//
	// Indirect array blends.
	//
	
	inline void BlendQuaternionArrayIndirect( E_BlendMode blendMode, C_QuaternionField** dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].quaternionArrayIndirectBlend)( dest, src, count, alpha );
	}
	
	inline void BlendVector3ArrayIndirect( E_BlendMode blendMode, C_Vector3Field** dest, const C_Vector3Field* src, U32 count, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].vec4F32ArrayIndirectBlend)( (Maths::Vec4F32**) dest, (const Maths::Vec4F32*) src, count, alpha );
	}
	
	inline void BlendScalarArrayIndirect( E_BlendMode blendMode, C_ScalarField** dest, const C_ScalarField* src, U32 count, const Maths::VF32& alpha )
	{
		(*gBlendFuncs[blendMode].vec4F32ArrayIndirectBlend)( (Maths::Vec4F32**) dest, (const Maths::Vec4F32*) src, count, alpha );
	}
}


#endif
