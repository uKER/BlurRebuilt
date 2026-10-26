#ifndef ANIMATION_CONSTRAINTTYPES_H
#define ANIMATION_CONSTRAINTTYPES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{
	/// Types of constraint.
	enum E_ConstraintType
	{
		E_ConstraintType_Aim			= 0,
		E_ConstraintType_Orientation,
		E_ConstraintType_Position,
		E_ConstraintType_DrivenKeyRotation,
		E_ConstraintType_DrivenKeyTranslation,
		E_ConstraintType_Custom,
		E_ConstraintType_DynamicAimJoint,
		E_ConstraintType_DynamicSliderJoint,
		E_ConstraintType_Count
	};
	
	inline bool IsStatic( E_ConstraintType constraintType )
	{
		return constraintType < E_ConstraintType_DynamicAimJoint;
	}

	/// Flag indicating which axes are affected by a constraint.
	enum E_AxisFlag
	{
		E_AxisFlag_X	= 1,
		E_AxisFlag_Y	= 2,
		E_AxisFlag_Z	= 4,
		E_AxisFlag_XYZ = E_AxisFlag_X | E_AxisFlag_Y | E_AxisFlag_Z
	};

	/// Method used to derive the "up vector" in aim constraints.
	enum E_UpVectorType
	{
		E_UpVectorType_ModelSpace = 0,		///< Up vector defined as a constant model space vector.
		E_UpVectorType_ReferenceJointSpace,	///< Up vector defined in local space of up vector reference joint.
		E_UpVectorType_AimAtReferenceJoint,	///< Up vector calculated by aiming at the origin of the up vector reference joint.
		E_UpVectorType_Count
	};

	/// Type of driven key input, used to evaluate curve.
	enum E_InputType
	{
		E_InputType_RotationX	= 0,
		E_InputType_RotationY,
		E_InputType_RotationZ,
		E_InputType_TranslationX,
		E_InputType_TranslationY,
		E_InputType_TranslationZ,
		E_InputType_Count
	};

}

#endif

