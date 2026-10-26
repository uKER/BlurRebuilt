#ifndef ANIMATION_CURVETYPES_H
#define ANIMATION_CURVETYPES_H

#ifdef _MSC_VER
#pragma once
#endif

///
/// \file CurveTypes.h
///
/// Defines the types of curve stored in a C_Cycle.
///

namespace Animation
{
	/// Varying curve types.
	enum E_VaryingCurveType
	{
		E_VaryingCurveType_First = 0,
		E_VaryingCurveType_VaryingVector3F32	= 0,
		E_VaryingCurveType_VaryingQuaternion32,
		E_VaryingCurveType_VaryingScalarF32,
		E_VaryingCurveType_VaryingQuaternion48,
		E_VaryingCurveType_VaryingVector3U16,
		E_VaryingCurveType_VaryingVector3U8,
		E_VaryingCurveType_Count,
		E_VaryingCurveType_Invalid = E_VaryingCurveType_Count
	};

	inline E_VaryingCurveType& operator ++ (E_VaryingCurveType& curveType)
	{
		curveType = static_cast<E_VaryingCurveType>(curveType + 1);
		return curveType;
	}

	inline E_VaryingCurveType operator ++ (E_VaryingCurveType curveType, int)
	{
		return static_cast<E_VaryingCurveType>(curveType + 1);
	}
	
	/// Constant curve types.
	enum E_ConstantCurveType
	{
		E_ConstantCurveType_ConstantVector3F32	= 0,
		E_ConstantCurveType_ConstantScalarF32,
		E_ConstantCurveType_ConstantQuaternion48,		
		E_ConstantCurveType_Count,
		E_ConstantCurveType_Invalid = E_ConstantCurveType_Count
	};
	
	inline E_ConstantCurveType& operator ++ (E_ConstantCurveType& curveType)
	{
		curveType = static_cast<E_ConstantCurveType>(curveType + 1);
		return curveType;
	}

	inline E_ConstantCurveType operator ++ (E_ConstantCurveType curveType, int)
	{
		return static_cast<E_ConstantCurveType>(curveType + 1);
	}

}

#endif
