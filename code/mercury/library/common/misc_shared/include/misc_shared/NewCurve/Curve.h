#ifndef CURVE_CURVE_H
#define CURVE_CURVE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename VECTYPE>
	struct ALIGN16 C_Knot
	{
		typedef VECTYPE VecType;

		F32 GetU(U32 knotIndex, U32 nKnots) const { return (F32)knotIndex/((F32)nKnots-1); }
		void SetU(F32 u) { BIZ_ASSERT(!"Cannot set u position!"); }

		VECTYPE	value;		//Knot value
	};

	template<typename VECTYPE>
	struct ALIGN16 C_KnotU
	{
		typedef VECTYPE VecType;

		F32 GetU(U32 knotIndex, U32 nKnots) const { return u; }
		void SetU(F32 u_) { u = u_;}

		F32		u;
		VECTYPE	value;		//Knot value
	};

	template<typename VECTYPE>
	class I_Curve
	{
	public:
		typedef	VECTYPE	VecType;

		///Destructor.
		virtual ~I_Curve();

		virtual VECTYPE Evaluate(F32 u) const = 0;

		virtual VECTYPE EvaluateTangent(F32 u) const;

		virtual F32 GetMinU() const = 0;

		virtual F32 GetMaxU() const = 0;

		virtual VECTYPE GetVertexValue(U32 index) const = 0;

		virtual const void* AcquireVertexData(U32 index) const = 0;

		virtual void ReleaseVertex() const = 0;

		virtual S32 GetNumVertices() const = 0;
	};

	template<typename VECTYPE>
	class I_InterpolatingCurve : public I_Curve<VECTYPE>
	{
	public:
		I_InterpolatingCurve() {}
		virtual ~I_InterpolatingCurve() {}

		virtual VECTYPE EvaluateLinear(F32 u) const = 0;

		virtual F32 GetArcLengthLinear() const = 0;
	};

	template<typename VECTYPE, typename STORAGE>
	class C_InterpolatingCurveBase: public I_InterpolatingCurve<VECTYPE>
	{
	public:
		C_InterpolatingCurveBase();

		virtual ~C_InterpolatingCurveBase();

		virtual VECTYPE EvaluateLinear(F32 u) const;

		virtual F32 GetMinU() const;

		virtual F32 GetMaxU() const;

		virtual VECTYPE GetVertexValue(U32 index) const;

		virtual const void* AcquireVertexData(U32 index) const;

		virtual void ReleaseVertex() const;

		virtual S32 GetNumVertices() const;

		virtual F32 GetArcLengthLinear() const;

		virtual F32 GetMinSegmentLength() const;

		void SetData(STORAGE* pData);

		STORAGE* GetData();

		const STORAGE* GetData() const;

	protected:
		typedef	STORAGE	StorageType;

		void _Process(E_ProcessU processU);
		
		F32							m_arcLengthLinear;
		F32							m_minSegmentLength;
		STORAGE*					m_pData;
	};

	template<typename VECTYPE>
	class I_ApproximatingCurve : public I_Curve<VECTYPE>
	{
	public:
		I_ApproximatingCurve() {}
		virtual ~I_ApproximatingCurve() {}
	};

	template<typename VECTYPE, typename STORAGE>
	class C_ApproximatingCurveBase : public I_ApproximatingCurve<VECTYPE>
	{
	public:
		C_ApproximatingCurveBase();

		virtual ~C_ApproximatingCurveBase();

		void SetData(STORAGE* pData);

		STORAGE* GetData();

		const STORAGE* GetData() const;

	protected:
		STORAGE*					m_pData;
	};

	#include "Curve.inl"
}

#endif
