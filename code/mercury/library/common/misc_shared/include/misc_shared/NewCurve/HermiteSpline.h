#ifndef CURVE_HERMITE_SPLINE_H
#define CURVE_HERMITE_SPLINE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename VECTYPE>
	struct ALIGN16 C_HermiteSplineKnotU
	{
		typedef VECTYPE VecType;

		F32 GetU(U32 x, U32 y) const { return u; }
		void SetU(F32 u_) { u = u_; }

		VECTYPE	value;		//Knot value

		//Read and potentially write//
		VECTYPE	tangentIn;	//Knot inbound tangent
		VECTYPE	tangentOut;	//Knot outbound tangent
		F32		tangentScale;
		F32		u;			//Knot position in parameter space with respect to the entire spline.
	};

	template<typename VECTYPE>
	struct ALIGN16 C_HermiteSplineKnot
	{
		typedef VECTYPE VecType;

		F32 GetU(U32 knotIndex, U32 nKnots) const { return (F32)knotIndex/((F32)nKnots-1); }
		void SetU(F32 u) { BIZ_ASSERT(!"Cannot set u position!");}

		//Read only//
		VECTYPE	value;		//Knot value

		//Read and potentially write//
		VECTYPE	tangentIn;	//Knot inbound tangent
		VECTYPE	tangentOut;	//Knot outbound tangent
		F32		tangentScale;
	};

	struct C_HermiteSplineFlags
	{
		C_HermiteSplineFlags() { bits = 0; }
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
	class C_HermiteSpline : public C_InterpolatingCurveBase<VECTYPE, STORAGE>, public CTRLPOINT_POLICY
	{
	public:
		typedef STORAGE StorageType;
		typedef VECTYPE	VecType;
		typedef C_HermiteSplineFlags FlagsType;

		///Constructor.
		C_HermiteSpline(C_HermiteSplineFlags flags);

		///Destructor.
		virtual ~C_HermiteSpline();

		virtual VECTYPE Evaluate(F32 u) const;

		void SetFlags(C_HermiteSplineFlags flags);

		virtual void Process();

		void CalculateKnotTangents(U32 knotIndex, bool in = true, bool out = true, bool closed = false);

	protected:
		typedef C_InterpolatingCurveBase<VECTYPE, STORAGE>	Base;
		C_HermiteSplineFlags	m_flags;
	};

	typedef C_HermiteSpline<Maths::HVec3, C_CurveData<C_HermiteSplineKnotU<Maths::HVec3> > > HermiteSpline3D;
	
	typedef C_HermiteSpline<F32, C_CurveData<C_HermiteSplineKnotU<F32> >, 
		C_HermiteCtrlPt1DTanAngle<F32, C_CurveData<C_HermiteSplineKnotU<F32> > > > HermiteSpline1DAngular;

	class C_Hermite1DData : public C_CurveData<C_HermiteSplineKnotU<F32> >
	{
	};
	
	class C_Hermite1D : public HermiteSpline1DAngular
	{
	public:
		///Constructor.
		C_Hermite1D(C_HermiteSplineFlags flags = C_HermiteSplineFlags()) : HermiteSpline1DAngular(flags) {}

		///Destructor.
		virtual ~C_Hermite1D() {}	

		typedef HermiteSpline1DAngular::VecType VecType;
		typedef HermiteSpline3D::StorageType::KnotType KnotType;
	};

	class C_Hermite3D : public HermiteSpline3D
	{
	public:
		///Constructor.
		C_Hermite3D(C_HermiteSplineFlags flags = C_HermiteSplineFlags()) : HermiteSpline3D(flags) {}

		///Destructor.
		virtual ~C_Hermite3D() {}	

		typedef HermiteSpline3D::VecType VecType;
		typedef HermiteSpline3D::StorageType::KnotType KnotType;
	};

	class C_Hermite3DData : public C_CurveData<C_Hermite3D::KnotType>
	{
	};

	#include "HermiteSpline.inl"
}

#endif
