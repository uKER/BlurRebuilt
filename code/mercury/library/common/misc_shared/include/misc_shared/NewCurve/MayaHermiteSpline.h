#ifndef CURVE_MAYA_HERMITE_SPLINE_H
#define CURVE_MAYA_HERMITE_SPLINE_H

#ifdef _MSC_VER
#pragma once
#endif

//Useful discussion here.
//http://www.fxshare.com/boards/lofiversion/index.php/t213323.html

namespace Curve
{
	enum E_InfinityType
	{
		//The Constant setting maintains the value of the end keys. 
		//This is the default setting for animation curves in Maya. 
		E_InfinityType_Constant,
		//The Cycle setting repeats the animation curve as a copy infinitely. 
		E_InfinityType_Cycle,
		//The Cycle with Offset setting repeats the animation curve infinitely, except it appends the 
		//cycled curve’s last key’s value to the value of the first key’s original curve. 
		E_InfinityType_CycleWithOffset,
		//The Oscillate setting repeats the animation curve by reversing its values, and therefore shape, 
		//with each cycle, creating an alternating backwards and forwards effect. 
		E_InfinityType_Oscillate,
		//The Linear setting extrapolates the value of the first key using its tangent information. It
		//projects a linear curve infinitely beyond. 
		E_InfinityType_Linear,
	};

	//Note: In Maya, Plateau, Fixed and Flat tangent types are used to ensure automatic tangent calculate
	//happens as expected. We don't need to replicate these here as the correct tangents will have
	//already been provided by Maya.
	enum E_TangentType
	{
		E_TangentType_Unknown,
		//Clamped Tangent.
		//Specifying a clamped tangent creates an animation curve that has the characteristics of linear and 
		//spline curves. 
		//
		//The keys’ tangents will be splines unless the value of two adjacent keys are very close. In this case,
		//the out tangent of the first key and the in tangent of the second key will be interpolated as linear. 
		//
		//When animating a skeleton in a walk-cycle, you may notice a slipping of the pelvic and foot joint 
		//position. This occurs because the value of the keys on the curve are either the same or close to 
		//(within tolerance of) the value of the other. Maya assigns a default spline tangent to describe 
		//what occurs between these keys. This is why you get the joint positions slipping. 
		//
		//By setting the tangent type to clamped for these keys, the slipping is corrected and the tangents
		//becomes a combination of linear and spline. 
		//
		//In the Maya environment preferences, Clamped is the default tangent type. 
		E_TangentType_Clamped,
		//Stepped Tangent.
		//Specifying a stepped tangent creates an animation curve whose out tangent is a flat curve. 
		//The curve segment is flat (horizontal), so the value changes at the key without gradation. 
		E_TangentType_Stepped,
		//Stepped Next
		//This tangent type differs from regular Stepped tangents in that its interpolation values jump
		//immediately to that of the next key rather than holding the value of the current key until the
		//next key is reached. 
		E_TangentType_SteppedNext,
	};


	template<typename VECTYPE>
	struct ALIGN16 C_MayaHermiteSplineKnot
	{
		typedef VECTYPE VecType;

		F32 GetU(U32 knotIndex, U32 nKnots) const { return (F32)knotIndex/((F32)nKnots-1); }
		void SetU(F32 u) { BIZ_ASSERT(!"Cannot set u position!");}

		VECTYPE				value;		//Knot value
		E_TangentType		tangentInType;
		VECTYPE				tangentIn;	//Knot inbound tangent
		E_TangentType		tangentOutType;
		VECTYPE				tangentOut;	//Knot outbound tangent
		F32					tangentScale;
	};

	template<typename VECTYPE>
	struct ALIGN16 C_MayaHermiteSplineKnotU
	{
		typedef VECTYPE VecType;

		F32 GetU(U32 knotIndex, U32 nKnots) const { return u; }
		void SetU(F32 u_) { u = u_; }

		VECTYPE				value;		//Knot value
		E_TangentType		tangentInType;
		VECTYPE				tangentIn;	//Knot inbound tangent
		E_TangentType		tangentOutType;
		VECTYPE				tangentOut;	//Knot outbound tangent
		F32					tangentScale;
		F32					u;			//Knot position in parameter space with respect to the entire spline.
	};


	struct C_MayaHermiteSplineFlags
	{
		C_MayaHermiteSplineFlags() { bits = 0; }
		union
		{
			U32 bits;
			struct
			{
				E_ProcessU calcKnotU : 4;
				bool calcTangents : 1;	// Automatically calculate tangents?
			};
		};
	};


	template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY = C_HermiteCtrlPtDefault<VECTYPE, STORAGE> >
	class C_MayaHermiteSpline : public C_InterpolatingCurveBase<VECTYPE, STORAGE>, public CTRLPOINT_POLICY
	{
	public:
		typedef STORAGE StorageType;
		typedef VECTYPE	VecType;
		typedef C_MayaHermiteSplineFlags FlagsType;

		///Constructor.
		C_MayaHermiteSpline(C_MayaHermiteSplineFlags flags);

		///Destructor.
		virtual ~C_MayaHermiteSpline();

		virtual VECTYPE Evaluate(F32 u) const;

		VECTYPE EvaluateReturnAdjacent(S32& iKnot0, S32& iKnot1, F32 u) const;

		void Process();

		void SetPreInfinity(E_InfinityType type);

		void SetPostInfinity(E_InfinityType type);

		E_InfinityType GetPreInfinity() const;

		E_InfinityType GetPostInfinity() const;

		void CalculateKnotTangents(U32 knotIndex, bool in = true, bool out = true);

	protected:
		F32 InfinityFilterU(F32 u, E_InfinityType type) const;

		VECTYPE InfinityFilterResult(VECTYPE result, F32 u) const;

		VECTYPE ConstantPreInfinity(F32 u) const;
		VECTYPE ConstantPostInfinity(F32 u) const;

		VECTYPE LinearPreInfinity(F32 u) const;
		VECTYPE LinearPostInfinity(F32 u) const;

		typedef C_InterpolatingCurveBase<VECTYPE, STORAGE>	Base;
		C_MayaHermiteSplineFlags				m_flags;
		Maths::HMat44							m_basis;

		E_InfinityType					m_preInfinity;
		E_InfinityType					m_postInfinity;
	};

	typedef C_MayaHermiteSpline<F32, C_CurveData<C_MayaHermiteSplineKnotU<F32> >, 
		C_HermiteCtrlPt1DTanAngle<F32, C_CurveData<C_MayaHermiteSplineKnotU<F32> > > > MayaHermiteSpline1DAngular;

	#include "MayaHermiteSpline.inl"
}

#endif
