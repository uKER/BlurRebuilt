#ifndef ANIMATION_BLENDFUNCTIONS_H
#define ANIMATION_BLENDFUNCTIONS_H

#include <animation/common/BlendModes.h>

/**
@field BlendFunctions.h

Atomic data field blending functions.
*/

namespace Animation
{
	class C_QuaternionField;
	class C_Vector4Field;
	class C_Vector3Field;
	class C_ScalarField;
	class C_JointTransform;

	//
	// Single field blends.
	//
	
	/// Blend the single quaternion field @a src into @a dest using blend weight @a alpha.
	void BlendQuaternion( E_BlendMode blendMode, C_QuaternionField* dest, const C_QuaternionField* src, const Maths::VF32& alpha );
	
	/// Blend the single vector3 field @a src into @a dest using blend weight @a alpha.
	void BlendVector3( E_BlendMode blendMode, C_Vector3Field* dest, const C_Vector3Field* src, const Maths::VF32& alpha );
	
	/// Blend the single scalar field @a src into @a dest using blend weight @a alpha.
	void BlendScalar( E_BlendMode blendMode, C_ScalarField* dest, const C_ScalarField* src, const Maths::VF32& alpha );

	//
	// Array blends.
	//

	/// Blend the the array of @a count JointTransform fields @a src into @a dest using blend weight @a alpha.
	void BlendJointTransformArray( E_BlendMode blendMode, C_JointTransform* dest, const C_JointTransform* src, U32 count, const Maths::VF32& alpha );
	
	/// Blend the the array of @a count quaternion fields @a src into @a dest using blend weight @a alpha.
	void BlendQuaternionArray( E_BlendMode blendMode, C_QuaternionField* dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha );
	
	/// Blend the the array of @a count vector4 fields @a src into @a dest using blend weight @a alpha.
	void BlendVector4Array( E_BlendMode blendMode, C_Vector4Field* dest, const C_Vector4Field* src, U32 count, const Maths::VF32& alpha );
	
	/// Blend the the array of @a count vector3 fields @a src into @a dest using blend weight @a alpha.
	void BlendVector3Array( E_BlendMode blendMode, C_Vector3Field* dest, const C_Vector3Field* src, U32 count, const Maths::VF32& alpha );
	
	/// Blend the the array of @a count scalar fields @a src into @a dest using blend weight @a alpha.
	void BlendScalarArray( E_BlendMode blendMode, C_ScalarField* dest, const C_ScalarField* src, U32 count, const Maths::VF32& alpha );


	//
	// Indirect array blends.
	//
	
	/// Blend the the array of @a count quaternion fields @a src into the destination fields pointed to by the pointers in array @a dest using blend weight @a alpha.
	void BlendQuaternionArrayIndirect( E_BlendMode blendMode, C_QuaternionField** dest, const C_QuaternionField* src, U32 count, const Maths::VF32& alpha );
	
	/// Blend the the array of @a count vector3 fields @a src into the destination fields pointed to by the pointers in array @a dest using blend weight @a alpha.
	void BlendVector3ArrayIndirect( E_BlendMode blendMode, C_Vector3Field** dest, const C_Vector3Field* src, U32 count, const Maths::VF32& alpha );
	
	/// Blend the the array of @a count scalar fields @a src into the destination fields pointed to by the pointers in array @a dest using blend weight @a alpha.
	void BlendSCalarArrayIndirect( E_BlendMode blendMode, C_ScalarField** dest, const C_ScalarField* src, U32 count, const Maths::VF32& alpha );
}

#endif

