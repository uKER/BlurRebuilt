#ifndef CURVE_CURVE_HELPERS_H
#define CURVE_CURVE_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/UtMaths.h>	// [Blur2PC] the Load<> specialisations below name UtMaths::MVector

namespace Curve
{
	namespace Helpers
	{
		template<typename VECTYPE>
		INLINE VECTYPE ControlPointToOutTangent(VECTYPE point, VECTYPE control, F32 tangentScale);

		template<typename VECTYPE>
		INLINE VECTYPE ControlPointToInTangent(VECTYPE point, VECTYPE control, F32 tangentScale);

		template<typename VECTYPE>
		INLINE VECTYPE OutTangentToControlPoint(VECTYPE point, VECTYPE tangent, F32 tangentScale);

		template<typename VECTYPE>
		INLINE VECTYPE InTangentToControlPoint(VECTYPE point, VECTYPE tangent, F32 tangentScale);

		template<typename VECTYPE>
		INLINE VECTYPE OutTangentToControlPoint1d(VECTYPE point, VECTYPE tangent, F32 tangentScale, F32 periodScale);

		template<typename VECTYPE>
		INLINE VECTYPE InTangentToControlPoint1d(VECTYPE point, VECTYPE tangent, F32 tangentScale, F32 periodScale);

		INLINE Maths::HVec3 OutTangentToControlPoint1dTo3d(Maths::HVec3 point, F32 tanAngle, F32 tangentScale, F32 periodScale);

		INLINE Maths::HVec3 InTangentToControlPoint1dTo3d(Maths::HVec3 point, F32 tanAngle, F32 tangentScale, F32 periodScale);

		INLINE Maths::HVec2 OutTangentToControlPoint1dTo2d(Maths::HVec2 point, F32 tanAngle, F32 tangentScale, F32 periodScale);

		INLINE Maths::HVec2 InTangentToControlPoint1dTo2d(Maths::HVec2 point, F32 tanAngle, F32 tangentScale, F32 periodScale);

		INLINE void BuildGeometryMatrix(
			Maths::HMat44& m,
			const Maths::HVec3& p0,
			const Maths::HVec3& p1,
			const Maths::HVec3& p2,
			const Maths::HVec3& p3);

		INLINE void BuildGeometryMatrix(
			Maths::HMat44& m,
			const Maths::HVec2& p0,
			const Maths::HVec2& p1,
			const Maths::HVec2& p2,
			const Maths::HVec2& p3);

		INLINE void BuildGeometryMatrix(
			Maths::HMat44& m,
			const F32 p0,
			const F32 p1,
			const F32 p2,
			const F32 p3);

		INLINE void BuildGeometryMatrix(
			Maths::HMat44& m,
			const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& p0,
			const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& p1,
			const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& p2,
			const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& p3);

		INLINE F32 Magnitude(F32 v);

		INLINE F32 Magnitude(const Maths::HVec2& v);

		INLINE F32 Magnitude(const Maths::HVec3& v);

		INLINE F32 Magnitude(const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& v);

		INLINE void Zero(F32& v);

		INLINE void Zero(Maths::HVec2& v);

		INLINE void Zero(Maths::HVec3& v);

		INLINE void Zero(UtMaths::MVector<UtMaths::VectorXYZ<F32> >& v);

		template <typename T>
		INLINE T Load(const Maths::HVec4 PCREF v);

		template <>
		INLINE Maths::HVec2 Load<Maths::HVec2>(const Maths::HVec4 PCREF v);
		 
		template <>
		INLINE Maths::HVec3 Load<Maths::HVec3>(const Maths::HVec4 PCREF v);

		template <>
		INLINE  UtMaths::MVector<UtMaths::VectorXYZ<F32> > Load<UtMaths::MVector<UtMaths::VectorXYZ<F32> > >(const Maths::HVec4 PCREF v);

		///Binary search for the interval that u falls within.
		template<typename STORAGE>
		S32 GetKnotInterval(const STORAGE* pData, F32 u);

		template<typename VECTYPE, typename STORAGE>
		VECTYPE KnotLerp(const STORAGE* pData, F32 u);

		template<typename CURVE>
		F32 CalcDuration(const CURVE* pCurve);

		template<typename CURVE>
		F32 CalcArcLengthLinear(const CURVE* pCurve);

		template<typename CURVE>
		F32 FindMinSegmentLength(const CURVE* pCurve);

		INLINE F32 GetLinearSegmentLength(const Maths::HVec2& prev, const Maths::HVec2& next);

		INLINE F32 GetLinearSegmentLength(const Maths::HVec3& prev, const Maths::HVec3& next);

		INLINE F32 GetLinearSegmentLength(const F32& prev, const F32& next);

		template<typename STORAGE>
		void SetRegularU(STORAGE* pData);

		template<typename STORAGE>
		void SetArcLengthLinearU(STORAGE* pData, F32 arcLength);

		#include "CurveHelpers.inl"
	}
}

#endif
