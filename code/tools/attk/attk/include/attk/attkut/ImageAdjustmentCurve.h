#ifndef ATTKUT_IMAGE_ADJUSTMENT_CURVE_H
#define ATTKUT_IMAGE_ADJUSTMENT_CURVE_H


#ifdef _MSC_VER_
#pragma once
#endif

#include <core/maths/utmaths/maths.h>

namespace AttkUt
{
	typedef UtMaths::MPoint<UtMaths::VectorXY<F32> > Point2;

	template <typename VertexType>
	class C_ImageAdjustmentCurve
	{
		typedef Ut::UtVector< Ut::DynamicVector<VertexType> >ControlVertexArray;
		typedef Ut::UtVector< Ut::DynamicVector<F32> >KnotArray;

	public:
		C_ImageAdjustmentCurve();

		C_ImageAdjustmentCurve(
			const C_ImageAdjustmentCurve<VertexType>& rhs);

		~C_ImageAdjustmentCurve();

		const C_ImageAdjustmentCurve& operator=(
			const C_ImageAdjustmentCurve& rhs);

		U32 AddControlVertex(const VertexType& ctrlvtx);
		VertexType& GetControlVertex(U32 index);
		const VertexType& GetControlVertex(U32 index) const;

		U32 AddKnot(F32 knot);
		F32 GetKnot(U32 index) const;

		void SetCurveDegree(U32 degree);
		U32 GetCurveDegree() const;

		ControlVertexArray& GetControlVertexArray();
		const ControlVertexArray& GetControlVertexArray() const;

		KnotArray& GetKnotArray();
		const KnotArray& GetKnotArray() const;

	private:
		U32 m_CurveDegree;
		ControlVertexArray m_ControlVertices;
		KnotArray m_Knots;
	};

	/* helper class for evaluating curves:
	 * As we need to evaluate the curve at a given x (not the curve parameter t)
	 *   the correct approach would be to solve the curve for t, then use it to get y.
	 * We don't need to be so precise, and so evaluate the curve at a number of intervals
	 * governed by 'numSubdivisions', to generate a piecewise linear approximation to the curve.
	 */
	template <typename VertexType>
	class C_EvaluateImageAdjustmentCurve
	{
	public:
		C_EvaluateImageAdjustmentCurve();

		~C_EvaluateImageAdjustmentCurve();

		bool Initialise(
			const C_ImageAdjustmentCurve<VertexType>& curve,
			U32 numSubdivisions);

		void Destroy();

		VertexType Evaluate(
			F32 x) const;

		U32 GetNumSubdivisions() const;

	private:
		U32 m_NumSubdivisions;
		Ut::UtVector< Ut::DynamicVector<VertexType> > m_ApproximationCurve;

	};

	C_ImageAdjustmentCurve<Point2> CreateThresholdCurve(U32 threshold);
	C_ImageAdjustmentCurve<Point2> CreateContrastEnhancementCurve(U32 pivot, U32 width);
}

#include "ImageAdjustmentCurve.inl"

#endif

