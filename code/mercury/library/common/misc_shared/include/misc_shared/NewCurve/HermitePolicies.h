#ifndef CURVE_HERMITE_POLICIES_H
#define CURVE_HERMITE_POLICIES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	//For 1D curves with fixed angular tangents.
	template<typename VECTYPE, typename STORAGE>
	class C_HermiteCtrlPt1DTanAngle
	{
	public:
		typedef VECTYPE VecType;

		void GenerateCtrlPts(
			VecType& ctrlPt0, 
			VecType& ctrlPt1, 
			const typename STORAGE::KnotType& knot0,
			F32 knot0u,
			const typename STORAGE::KnotType& knot1,
			F32 knot1u) const
		{
			//Preserve tangent angle by dividing by the knot interval. To the user the tangent angle remains
			//constant as a point is moved in the x axis.return u1 - u0;
			F32 period = knot1u - knot0u;
			ctrlPt0 = Helpers::OutTangentToControlPoint<VECTYPE>(knot0.value, knot0.tangentOut, period);
			ctrlPt1 = Helpers::InTangentToControlPoint<VECTYPE>(knot1.value, knot1.tangentIn, period);
		}

		VecType GenerateLinearPreInfinity(
			const typename STORAGE::KnotType& knot,
			F32 knotu,
			F32 u) const
		{
			F32 dist = knotu - u;
			F32 result = knot.value + dist*-knot.tangentIn;
			return result;
		}

		VecType GenerateLinearPostInfinity(
			const typename STORAGE::KnotType& knot,
			F32 knotu,
			F32 u) const
		{
			F32 dist = u - knotu;
			F32 result = knot.value + dist*knot.tangentIn;
			return result;
		}
	};

	//Default control point policy.
	template<typename VECTYPE, typename STORAGE>
	class C_HermiteCtrlPtDefault
	{
	public:
		typedef VECTYPE VecType;

		void GenerateCtrlPts(
			VecType& ctrlPt0, 
			VecType& ctrlPt1, 
			const typename STORAGE::KnotType& knot0,
			F32 knot0u,
			const typename STORAGE::KnotType& knot1,
			F32 knot1u) const
		{
			//Non angular tangent.
			//F32 period = knot1u - knot0u;
			ctrlPt0 = Helpers::OutTangentToControlPoint<VECTYPE>(knot0.value, knot0.tangentOut, knot0.tangentScale);
			ctrlPt1 = Helpers::InTangentToControlPoint<VECTYPE>(knot1.value, knot1.tangentIn, knot1.tangentScale);
		}

		VECTYPE GenerateLinearPreInfinity(
			const typename STORAGE::KnotType& knot,
			F32 knotu,
			F32 u) const
		{
			F32 dist = knotu - u;
			VECTYPE normalised = knot.tangentIn / Curve::Helpers::Magnitude(knot.tangentIn);
			VECTYPE result = knot.value + normalised * -dist;
			return result;
		}

		VECTYPE GenerateLinearPostInfinity(
			const typename STORAGE::KnotType& knot,
			F32 knotu,
			F32 u) const
		{
			F32 dist = u - knotu;
			VECTYPE normalised = knot.tangentOut / Curve::Helpers::Magnitude(knot.tangentOut);
			VECTYPE result = knot.value + normalised * dist;
			return result;
		}
	};
}

#endif
