#ifndef EDITOR_MATHS_HELPERS_H
#define EDITOR_MATHS_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >			Vector3;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >			Vector4;
	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4, 4> > Matrix44;

	FORCE_INLINE Maths::HMat44 ConvertToHMat44(
		const Matrix44& rhs)
	{
		return Maths::HMat44(
			Maths::HVec4(rhs[0][0], rhs[0][1], rhs[0][2], rhs[0][3]),
			Maths::HVec4(rhs[1][0], rhs[1][1], rhs[1][2], rhs[1][3]),
			Maths::HVec4(rhs[2][0], rhs[2][1], rhs[2][2], rhs[2][3]),
			Maths::HVec4(rhs[3][0], rhs[3][1], rhs[3][2], rhs[3][3]));
	}
}

#endif
