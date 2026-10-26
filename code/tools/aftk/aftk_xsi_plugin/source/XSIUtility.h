#pragma once

namespace XSITrans
{
	INLINE bool IsNumberValid(F32 number, F32 absNumericRange = 20000);

	INLINE bool IsNumberValid(F32 number);

	bool XsiToAftkMatrix44(
		Aftk::Matrix44& aftkMatrix,
		const XSI::MATH::CMatrix4& xsiMatrix, 
		F32 absNumericRange = 20000);

	void AftkToXsiMatrix44(
		XSI::MATH::CMatrix4& xsiMatrix,
		Aftk::Matrix44& aftkMatrix);
}

#include "XSIUtility.inl"

