#ifndef ANIMATION_CORE_SPLINES_H
#define ANIMATION_CORE_SPLINES_H

#ifdef _MSC_VER
#pragma once
#endif

///
/// \file
/// B-Spline evaluation policies.
///

namespace Animation
{

/// \brief Cubic B-spline evaluation policy.
class C_CubicBSpline
{
public:
	enum
	{
		kDegree	= 3
	};

	/// Evaluate a cubic B-spline segment at time t, where knots[2] <= t < knots[3].
	template <typename TPoint, typename TKnot>
	static TPoint Evaluate(const TKnot knots[6], const TPoint controlPoints[4], TKnot t);

	/// Evaluate coefficients for a cubic B-spline segment at time t, where knots[2] <= t < knots[3].
	template <typename TKnot>
	static void EvaluateCoefficients(const TKnot knots[6], TKnot t, TKnot outCoefficients[4]);

};

/// \brief Quadratic B-spline evaluation policy.
class C_QuadraticBSpline
{
public:
	enum
	{
		kDegree	= 2
	};

	/// Evaluate a quadratic B-spline segment at time t, where knots[1] <= t < knots[2].
	template <typename TPoint, typename TKnot>
	static TPoint Evaluate(const TKnot knots[4], const TPoint controlPoints[3], TKnot t);

	/// Evaluate coefficients for a quadratic B-spline segment at time t, where knots[1] <= t < knots[2].
	template <typename TKnot>
	static void EvaluateCoefficients(const TKnot knots[4], TKnot t, TKnot outCoefficients[3]);

};

/// \brief Linear B-spline evaluation policy.
class C_LinearBSpline
{
public:
	enum
	{
		kDegree	= 1
	};

	/// Evaluate a linear B-spline segment at time t, where knots[0] <= t < knots[1].
	template <typename TPoint, typename TKnot>
	static TPoint Evaluate(const TKnot knots[2], const TPoint controlPoints[3], TKnot t);

	/// Evaluate coefficients for a linear B-spline segment at time t, where knots[0] <= t < knots[1].
	template <typename TKnot>
	static void EvaluateCoefficients(const TKnot knots[2], TKnot t, TKnot outCoefficients[2]);

};

} // end Animation

#include "Splines.inl"

#endif
