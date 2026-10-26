#ifndef AFTK_MATHS_H
#define AFTK_MATHS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	Vector4F32	ConvertU32ToVector4F32(U32 srcColour);
	U32			ConvertVector4F32ToU32(const Vector4F32 &colourVec);
	Vector4F32	ConvertU64ToVector4F32(U64 srcColour);
	U64			ConvertVector4F32ToU64(const Vector4F32 &colourVec);
	Normal3		ApplyMatrix(const Matrix44& matrix, const Normal3& src);
	Vector3F32	ApplyMatrix(const Matrix44& matrix, const Vector3F32& src);
	Vector3F32	ApplyMatrix(const Matrix44& matrix, const Vector3F32& src);
	Vector4F32	ApplyMatrix(const Matrix44& matrix, const Vector4F32& src);
	Position3	ApplyMatrix(const Matrix44& matrix, const Position3& src);
	Normal3		ApplyTransposeOfMatrixInverseToNormal(const Matrix44& matrix, const Normal3& src);
	Normal3		ApplyMatrixToNormal(const Matrix44& matrix, const Normal3& src);

	template<class T>
	FORCE_INLINE  F32 MagnitudeSquared(T& v)
	{
		return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	}

	template<class T>
	FORCE_INLINE  F32 Magnitude(T& v)
	{
		return (sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]));
	}

	template<class T>
	FORCE_INLINE  void Normalise(T& v)
	{
		F32 magnitude = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
		if (magnitude > 0.f)
		{
			v[0] = v[0] / magnitude;
			v[1] = v[1] / magnitude;
			v[2] = v[2] / magnitude;
		}
		else
		{
			memset(&v, 0, sizeof(T));
		}
	}

	template <class T>
	FORCE_INLINE  S32 Sgn(T v)
	{
		if (v < 0)
		{
			return -1;
		}

		return 1;
	}

	template <class T>
	FORCE_INLINE  bool EqualityTestFScalar(
		T lhs, 
		T rhs,
		F32 epsilon = AFTK_EPSILON)
	{
		T delta = (T)fabs(lhs - rhs);

		if (delta >= epsilon)
		{
			return false;
		}

		return true;
	}

	template <class T, int N>
	FORCE_INLINE  bool EqualityTestFVector(	const UtMaths::BaseVector<T, N>& lhs, 
											const UtMaths::BaseVector<T, N>& rhs,
											F32 epsilon = AFTK_EPSILON)
	{
		T magnitudeSquared = 0;
		for (U32 i = 0; i < N; i++)
		{
			T delta = lhs[i] - rhs[i];
			magnitudeSquared += delta*delta;
		}

		if (magnitudeSquared >= epsilon)
		{
			return false;
		}

		return true;
	}

	FORCE_INLINE  bool EqualityTestFVector(	const Matrix44& lhs, 
											const Matrix44& rhs,
											F32 epsilon = AFTK_EPSILON)
	{
		for (U32 i = 0; i < 4; i++)
			if (!EqualityTestFVector(lhs[i], rhs[i], epsilon)) 
				return false;
		return true;
	}

	template <class T>
	FORCE_INLINE bool EqualityTestVector(
		const Ut::UtVector<Ut::DynamicVector<T> >& lhs, 
		const Ut::UtVector<Ut::DynamicVector<T> >& rhs)
	{
		const int lSize = lhs.Size();
		const int rSize = rhs.Size();

		if (lSize != rSize)
			return false;

		for (int i=0; i<lSize; i++)
			if (!(lhs[i] == rhs[i]))
				return false;

		return true;
	}

	template <class T, int N>
	FORCE_INLINE  bool EqualityTestVector(
		const UtMaths::BaseVector<T, N>& lhs, 
		const UtMaths::BaseVector<T, N>& rhs)
	{	
		if (memcmp(&lhs, &rhs, sizeof(UtMaths::BaseVector<T, N>)) != 0)
			return false;

		return true;
	}

	FORCE_INLINE bool EqualityTestFMatrix(
		const Matrix44& lhs, 
		const Matrix44& rhs,
		F32 epsilon = AFTK_EPSILON)
	{
		for (U32 i = 0; i < 4; i++)
			if (!EqualityTestFVector(lhs[i], rhs[i], AFTK_EPSILON))
				return false;

		return true;
	}

	template <class T, int N>
	FORCE_INLINE  bool GreaterThanTestVector(	
		const UtMaths::BaseVector<T, N>& lhs, 
		const UtMaths::BaseVector<T, N>& rhs)
	{
		for (U32 i = 0; i < N; i++)
			if (lhs[i] < rhs[i]) 
				return false;

		return true;
	}

	template <class T, int N>
	FORCE_INLINE  bool LessThanTestVector(
		const UtMaths::BaseVector<T, N>& lhs, 
		const UtMaths::BaseVector<T, N>& rhs)
	{
		return !GreaterThanTestVector(lhs, rhs);
	}

	//generic lerp

	template <typename T, int N> 
	T VectorLerp(const T& v1, const T& v2, float factor)
	{
		T newVector;
		float oneMinusFactor = 1.0f - factor;

		for (int i=0; i<N; i++)
			newVector[i] = (v1[i] * factor) + (v2[i] * oneMinusFactor);

		return newVector;
	}

	#ifndef GCC
	FORCE_INLINE  bool DoesMatrixHaveNegativeScaling(const Matrix44 &mtx)
	{
		Vector3F32 v1(mtx.At<0>().X(), mtx.At<0>().Y(), mtx.At<0>().Z());
		Vector3F32 v2(mtx.At<1>().X(), mtx.At<1>().Y(), mtx.At<1>().Z());
		Vector3F32 v3(mtx.At<2>().X(), mtx.At<2>().Y(), mtx.At<2>().Z());

		// check and warn about negative scaling on matrices
		Vector3F32 vec = v1 ^ v2;
		F32 f = vec | v3;

		return ( f < 0.0 );
	}
	#endif

#ifdef MATHS_HVEC_VECTOR_H

	inline void TemplateToNightmareMatrix33(Maths::HMat33& lhs, const Matrix44& rhs)
	{
		lhs.AX(Maths::HVec3(rhs[0][0], rhs[0][1], rhs[0][2]));
		lhs.AY(Maths::HVec3(rhs[1][0], rhs[1][1], rhs[1][2]));
		lhs.AZ(Maths::HVec3(rhs[2][0], rhs[2][1], rhs[2][2]));
	}

	inline void TemplateToNightmareMatrix43(Maths::HMat43& lhs, const Matrix44& rhs)
	{
		lhs.AX(Maths::HVec3(rhs[0][0], rhs[0][1], rhs[0][2]));
		lhs.AY(Maths::HVec3(rhs[1][0], rhs[1][1], rhs[1][2]));
		lhs.AZ(Maths::HVec3(rhs[2][0], rhs[2][1], rhs[2][2]));
		lhs.AW(Maths::HVec3(rhs[3][0], rhs[3][1], rhs[3][2]));
	}

	inline void TemplateToNightmareMatrix44(Maths::HMat44& lhs, const Matrix44& rhs)
	{
		lhs.AX(Maths::HVec4(rhs[0][0], rhs[0][1], rhs[0][2], rhs[0][3]));
		lhs.AY(Maths::HVec4(rhs[1][0], rhs[1][1], rhs[1][2], rhs[1][3]));
		lhs.AZ(Maths::HVec4(rhs[2][0], rhs[2][1], rhs[2][2], rhs[2][3]));
		lhs.AW(Maths::HVec4(rhs[3][0], rhs[3][1], rhs[3][2], rhs[3][3]));
	}

	inline void NightmareToTemplateMatrix33(Matrix44& lhs, const Maths::HMat33& rhs)
	{
		lhs[0][0] = rhs.AX().X(); lhs[0][1] = rhs.AX().Y(); lhs[0][2] = rhs.AX().Z(); lhs[0][3] = 0;
		lhs[1][0] = rhs.AY().X(); lhs[1][1] = rhs.AY().Y(); lhs[1][2] = rhs.AY().Z(); lhs[1][3] = 0;
		lhs[2][0] = rhs.AZ().X(); lhs[2][1] = rhs.AZ().Y(); lhs[2][2] = rhs.AZ().Z(); lhs[2][3] = 0;
		lhs[3][0] = 0; lhs[3][1] = 0; lhs[3][2] = 0; lhs[3][3] = 1;
	}
	
	inline void NightmareToTemplateMatrix43(Matrix44& lhs, const Maths::HMat43& rhs)
	{
		lhs[0][0] = rhs.AX().X(); lhs[0][1] = rhs.AX().Y(); lhs[0][2] = rhs.AX().Z(); lhs[0][3] = 0;
		lhs[1][0] = rhs.AY().X(); lhs[1][1] = rhs.AY().Y(); lhs[1][2] = rhs.AY().Z(); lhs[1][3] = 0;
		lhs[2][0] = rhs.AZ().X(); lhs[2][1] = rhs.AZ().Y(); lhs[2][2] = rhs.AZ().Z(); lhs[2][3] = 0;
		lhs[3][0] = rhs.AW().X(); lhs[3][1] = rhs.AW().Y(); lhs[3][2] = rhs.AW().Z(); lhs[3][3] = 1;
	}

	inline void NightmareToTemplateMatrix44(Matrix44& lhs, const Maths::HMat44& rhs)
	{
		lhs[0][0] = rhs.AX().X(); lhs[0][1] = rhs.AX().Y(); lhs[0][2] = rhs.AX().Z(); lhs[0][3] = rhs.AX().W();
		lhs[1][0] = rhs.AY().X(); lhs[1][1] = rhs.AY().Y(); lhs[1][2] = rhs.AY().Z(); lhs[1][3] = rhs.AY().W();
		lhs[2][0] = rhs.AZ().X(); lhs[2][1] = rhs.AZ().Y(); lhs[2][2] = rhs.AZ().Z(); lhs[2][3] = rhs.AZ().W();
		lhs[3][0] = rhs.AW().X(); lhs[3][1] = rhs.AW().Y(); lhs[3][2] = rhs.AW().Z(); lhs[3][3] = rhs.AW().W();
	}

	inline Maths::HVec3 TemplateToNightmare(const Vector3F32& rhs)
	{
		return Maths::HVec3(rhs[0], rhs[1], rhs[2]);
	}

	inline Maths::HVec4 TemplateToNightmare(const Vector4F32& rhs)
	{
		return Maths::HVec4(rhs[0], rhs[1], rhs[2], rhs[3]);
	}

	inline Maths::HVec3 TemplateToNightmare(const Position3& rhs)
	{
		return Maths::HVec3(rhs[0], rhs[1], rhs[2]);
	}

	inline Maths::C_AxisBox TemplateToNightmare(const C_Bounds& rhs)
	{
		return Maths::C_AxisBox(TemplateToNightmare(rhs.GetMin()),TemplateToNightmare(rhs.GetMax()));
	}

	inline Maths::HVec3 TemplateToNightmare(const Normal3& rhs)
	{
		return Maths::HVec3(rhs[0], rhs[1], rhs[2]);
	}

	inline Vector3F32 NightmareToTemplate(const Maths::HVec3& rhs)
	{
		return Vector3F32(rhs[0], rhs[1], rhs[2]);
	}

	inline Vector4F32 NightmareToTemplate(const Maths::HVec4& rhs)
	{
		return Vector4F32(rhs[0], rhs[1], rhs[2], rhs[3]);
	}

	inline bool EqualityTestNightmare(
		const Maths::HVec3& lhs, 
		const Maths::HVec3& rhs,
		F32 epsilon = AFTK_EPSILON)
	{
		F32 magnitudeSquared = 0;

		F32 delta = lhs[0] - rhs[0];
		magnitudeSquared += delta*delta;
		delta = lhs[1] - rhs[1];
		magnitudeSquared += delta*delta;
		delta = lhs[2] - rhs[2];
		magnitudeSquared += delta*delta;

		if (magnitudeSquared >= epsilon)
			return false;
		return true;
	}

	void BuildMatrix(
		Maths::HMat43& out,
		const Maths::HVec3& scalePivot,
		const Maths::HVec3& scale,
		const Maths::HVec3& shear,
		const Maths::HVec3& scalePivotTranslation,
		const Maths::HVec3& rotatePivot,
		const Maths::HMat33& ro,	//rotate orientation matrix
		const Maths::HMat33& r,		// rotation matrix
		const Maths::HVec3& rotatePivotTranslation,
		const Maths::HVec3& translate);

	void BuildMatrix(
		Maths::HMat43& out,
		const Maths::HVec3& scalePivot,
		const Maths::HVec3& scale,
		const Maths::HVec3& shear,
		const Maths::HVec3& scalePivotTranslation,
		const Maths::HVec3& rotatePivot,
		const Maths::HMat33& rotateOrientation,
		const Maths::HVec3& rotate,
		const Maths::HVec3& rotatePivotTranslation,
		const Maths::HVec3& translate,
		E_RotationOrder rotationOrder);


	void BuildJointMatrix(
		Maths::HMat43& out,
		const Maths::HVec3& scale,
		const Maths::HMat33& scaleOrientation,
		const Maths::HMat33& rotate,
		const Maths::HMat33& orientation,
		const Maths::HVec3& inverseScale,
		const Maths::HVec3& translate);

	void BuildJointMatrix(
		Maths::HMat43& out,
		const Maths::HVec3& scale,
		const Maths::HMat33& scaleOrientation,
		const Maths::HVec3& rotate,
		const Maths::HMat33& orientation,
		const Maths::HVec3& inverseScale,
		const Maths::HVec3& translate,
		E_RotationOrder rotationOrder);

	bool GetSShRT(
		Maths::HVec3& outScale,
		Maths::HVec3& outShear,
		Maths::HVec3& outRotate, 
		Maths::HVec3& outTranslate,
		const Maths::HMat43& m,
		Aftk::E_RotationOrder order);

	bool GetScale(
		Maths::HVec3& outScale,
		const Maths::HMat43& m4x3);

	inline void BuildMatrix(
		Matrix44& out,
		const Vector3F32& scalePivot,
		const Vector3F32& scale,
		const Vector3F32& shear,
		const Vector3F32& scalePivotTranslation,
		const Vector3F32& rotatePivot,
		const Matrix44& ro,	//rotate orientation matrix
		const Matrix44& r,		// rotation matrix
		const Vector3F32& rotatePivotTranslation,
		const Vector3F32& translate)
	{
		Maths::HMat43 hOut;

		Maths::HMat33 hRo;
		Aftk::TemplateToNightmareMatrix33(hRo, ro);

		Maths::HMat33 hr;
		Aftk::TemplateToNightmareMatrix33(hr, r);

		BuildMatrix(
			hOut,
			Aftk::TemplateToNightmare(scalePivot),
			Aftk::TemplateToNightmare(scale),
			Aftk::TemplateToNightmare(shear),
			Aftk::TemplateToNightmare(scalePivotTranslation),
			Aftk::TemplateToNightmare(rotatePivot),
			hRo,
			hr,
			Aftk::TemplateToNightmare(rotatePivotTranslation),
			Aftk::TemplateToNightmare(translate));

		Aftk::NightmareToTemplateMatrix43(out, hOut);
	}

	inline void BuildMatrix(
		Matrix44& out,
		const Vector3F32& scalePivot,
		const Vector3F32& scale,
		const Vector3F32& shear,
		const Vector3F32& scalePivotTranslation,
		const Vector3F32& rotatePivot,
		const Matrix44& rotateOrientation,
		const Vector3F32& rotate,
		const Vector3F32& rotatePivotTranslation,
		const Vector3F32& translate,
		E_RotationOrder rotationOrder)
	{
		Maths::HMat43 hOut;

		Maths::HMat33 hRotateOrientation;
		Aftk::TemplateToNightmareMatrix33(hRotateOrientation, rotateOrientation);

		BuildMatrix(
			hOut,
			Aftk::TemplateToNightmare(scalePivot),
			Aftk::TemplateToNightmare(scale),
			Aftk::TemplateToNightmare(shear),
			Aftk::TemplateToNightmare(scalePivotTranslation),
			Aftk::TemplateToNightmare(rotatePivot),
			hRotateOrientation,
			Aftk::TemplateToNightmare(rotate),
			Aftk::TemplateToNightmare(rotatePivotTranslation),
			Aftk::TemplateToNightmare(translate),
			rotationOrder);

		Aftk::NightmareToTemplateMatrix43(out, hOut);
	}

	inline void BuildJointMatrix(
		Matrix44& out,
		const Vector3F32& scale,
		const Matrix44& scaleOrientation,
		const Matrix44& rotate,
		const Matrix44& orientation,
		const Vector3F32& inverseScale,
		const Vector3F32& translate)
	{
		Maths::HMat43 hOut;

		Maths::HMat33 hScaleOrientation;
		Aftk::TemplateToNightmareMatrix33(hScaleOrientation, scaleOrientation);

		Maths::HMat33 hOrientation;
		Aftk::TemplateToNightmareMatrix33(hOrientation, orientation);

		Maths::HMat33 hRotate;
		Aftk::TemplateToNightmareMatrix33(hRotate, rotate);

		BuildJointMatrix(
			hOut,
			Aftk::TemplateToNightmare(scale),
			hScaleOrientation,
			hRotate,
			hOrientation,
			Aftk::TemplateToNightmare(inverseScale),
			Aftk::TemplateToNightmare(translate));

		Aftk::NightmareToTemplateMatrix43(out, hOut);
	}

	inline void BuildJointMatrix(
		Matrix44& out,
		const Vector3F32& scale,
		const Matrix44& scaleOrientation,
		const Vector3F32& rotate,
		const Matrix44& orientation,
		const Vector3F32& inverseScale,
		const Vector3F32& translate,
		E_RotationOrder rotationOrder)
	{
		Maths::HMat43 hOut;

		Maths::HMat33 hScaleOrientation;
		Aftk::TemplateToNightmareMatrix33(hScaleOrientation, scaleOrientation);

		Maths::HMat33 hOrientation;
		Aftk::TemplateToNightmareMatrix33(hOrientation, orientation);

		BuildJointMatrix(
			hOut,
			Aftk::TemplateToNightmare(scale),
			hScaleOrientation,
			Aftk::TemplateToNightmare(rotate),
			hOrientation,
			Aftk::TemplateToNightmare(inverseScale),
			Aftk::TemplateToNightmare(translate),
			rotationOrder);

		Aftk::NightmareToTemplateMatrix43(out, hOut);
	}
	#endif

}

#endif