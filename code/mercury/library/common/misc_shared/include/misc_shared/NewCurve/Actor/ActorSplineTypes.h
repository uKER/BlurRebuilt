#ifndef CURVE_ACTOR_SPLINE_TYPES_H
#define CURVE_ACTOR_SPLINE_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef TOOL
#include <Xt/XtTool.h>
#else
#include <Xt/XtGame.h>
#endif
#include <Xt/XtStd.h>

namespace Curve
{
	template<typename KNOT>
	class C_XtProxy
	{
	public:
		void SetData(const XtSplineVec3* pXtData)
		{
			m_pXtData = pXtData;

			U32 nKnots = m_pXtData->closed ? m_pXtData->knots.count + 1 : m_pXtData->knots.count;
#ifndef DISABLE_NON_1D_UPARAM
			m_uValues.Resize(nKnots);
#endif
			if (m_pXtData->knots.count == 0)
				return;

			//Calculate linear arc length
			F32 arcLengthLinear = 0;
			{
				typename KNOT::VecType prevValue = Load(m_pXtData->knots[0].position);
				for(S32 i = 1; i < (S32)nKnots; ++i)
				{
					typename KNOT::VecType nextValue = Load(m_pXtData->knots[i%m_pXtData->knots.count].position);
					arcLengthLinear += Helpers::GetLinearSegmentLength(prevValue, nextValue);
					prevValue = nextValue;
				}
			}
#ifndef DISABLE_NON_1D_UPARAM
			//Set u spacing.
			{
				typename KNOT::VecType prevValue = Load(m_pXtData->knots[0].position);
				m_uValues[0] = 0.0f;

				F32 currentLength = 0;
				for (U32 i = 1; i < nKnots; ++i)
				{
					typename KNOT::VecType nextValue = Load(m_pXtData->knots[i%m_pXtData->knots.count].position);
					currentLength += Helpers::GetLinearSegmentLength(prevValue, nextValue);
					prevValue = nextValue;
					m_uValues[i] = currentLength / arcLengthLinear;
				}
			}
#endif
		}

		void LoadSourceKnot(KNOT& knot, U32 index)
		{
			U32 realIndex = index;
			if (m_pXtData->closed)
			{
				BIZ_ASSERT_MSG(m_pXtData->knots.count > 0, "Spline contains no knots!");
				realIndex = index % m_pXtData->knots.count;
			}

			const XtKnotVec3& xtKnot = m_pXtData->knots[realIndex];
			//Load functions come from "#include <core/xtstdx.h>"
			knot.value = Load(xtKnot.position);
			knot.tangentIn = Load(xtKnot.tangentIn);
			knot.tangentOut = Load(xtKnot.tangentOut);
			knot.tangentScale = xtKnot.tangentScale;
			//knot.u = (F32)index / ((F32)(GetNumSourceKnots())-1.f);
#ifndef DISABLE_NON_1D_UPARAM
			knot.SetU(m_uValues[index]);
#endif
		}

		void StoreSourceKnot(const KNOT& knot, U32 index)
		{
			BIZ_ASSERT(!"Xt splines are read only!");
		}

		U32 GetNumSourceKnots() const
		{
			return m_pXtData->closed ? m_pXtData->knots.count+1 : m_pXtData->knots.count;
		}

		XtSplineVec3* GetXtCreationData()
		{
			return m_pXtData;
		}

	private:
		const XtSplineVec3* m_pXtData;
#ifndef DISABLE_NON_1D_UPARAM
		//This needs to die.
		Btl::Vector<F32>	m_uValues;
#endif
	};

	template<typename KNOT>
	class C_XtProxyWithTransform
	{
	public:
		void SetData(const XtSplineVec3* pXtData, const Maths::HMat43& createMat)
		{
			m_pXtData = pXtData;
			m_createMat = createMat;

			U32 nKnots = m_pXtData->closed ? m_pXtData->knots.count+1 : m_pXtData->knots.count;
#ifndef DISABLE_NON_1D_UPARAM
			m_uValues.Resize(nKnots);
#endif
			if (m_pXtData->knots.count == 0)
				return;

			//Calculate linear arc length
			F32 arcLengthLinear = 0;
			{
				typename KNOT::VecType prevValue = Load(m_pXtData->knots[0].position);
				for(S32 i = 1; i < (S32)nKnots; ++i)
				{
					typename KNOT::VecType nextValue = Load(m_pXtData->knots[i%m_pXtData->knots.count].position);
					arcLengthLinear += Helpers::GetLinearSegmentLength(prevValue, nextValue);
					prevValue = nextValue;
				}
			}
#ifndef DISABLE_NON_1D_UPARAM
			//Set u spacing.
			{
				typename KNOT::VecType prevValue = Load(m_pXtData->knots[0].position);
				m_uValues[0] = 0.0f;

				F32 currentLength = 0;
				for (U32 i = 1; i < nKnots; ++i)
				{
					typename KNOT::VecType nextValue = Load(m_pXtData->knots[i%m_pXtData->knots.count].position);
					currentLength += Helpers::GetLinearSegmentLength(prevValue, nextValue);
					prevValue = nextValue;
					m_uValues[i] = currentLength / arcLengthLinear;
				}
			}
#endif
		}

		void LoadSourceKnot(KNOT& knot, U32 index)
		{
			U32 realIndex = index;
			if (m_pXtData->closed)
				realIndex = index % m_pXtData->knots.count;

			const XtKnotVec3& xtKnot = m_pXtData->knots[realIndex];
			//Load functions come from "#include <core/xtstdx.h>"
			knot.value = Load(xtKnot.position) * m_createMat;
			knot.tangentIn = Load(xtKnot.tangentIn) * m_createMat.Rotation();
			knot.tangentOut = Load(xtKnot.tangentOut) * m_createMat.Rotation();
			knot.tangentScale = xtKnot.tangentScale;
			//knot.u = (F32)index / ((F32)(GetNumSourceKnots())-1.f);
#ifndef DISABLE_NON_1D_UPARAM
			knot.SetU(m_uValues[index]);
#endif
		}

		void StoreSourceKnot(const KNOT& knot, U32 index)
		{
			BIZ_ASSERT(!"Xt splines are read only!");
		}

		U32 GetNumSourceKnots() const
		{
			return m_pXtData->closed ? m_pXtData->knots.count+1 : m_pXtData->knots.count;
		}

		XtSplineVec3* GetXtCreationData()
		{
			return m_pXtData;
		}

	private:
		const XtSplineVec3* m_pXtData;
		Maths::HMat43		m_createMat;
#ifndef DISABLE_NON_1D_UPARAM
		//This needs to die
		Btl::Vector<F32>	m_uValues;
#endif
	};


	typedef C_HermiteSplineKnotU<Maths::HVec3> Hermite3DActorKnot;




	class C_Hermite3DActorProxy : public C_XtProxy<Hermite3DActorKnot>
	{
	};


	class C_Hermite3DActorDataProxy : public C_CurveDataProxy< 
				Hermite3DActorKnot, 
				C_XtProxy<Hermite3DActorKnot> 
			>
	{
	};

	class C_Hermite3DActorProxyWithTransform : public C_XtProxyWithTransform<Hermite3DActorKnot>
	{
	};


	class C_Hermite3DActorDataProxyWithTransform : public C_CurveDataProxy< 
				Hermite3DActorKnot, 
				C_XtProxyWithTransform<Hermite3DActorKnot> 
			>
	{
	};




	class C_Hermite3DActor : public 
		C_HermiteSpline<
			Maths::HVec3, 
			C_Hermite3DActorDataProxy
		>
	{
	public:
		typedef HermiteSpline3D::VecType VecType;
		typedef HermiteSpline3D::StorageType::KnotType KnotType;
		typedef C_HermiteSpline<
			Maths::HVec3, 
			C_Hermite3DActorDataProxy
		> Base;

	public:
		///Constructor.
		C_Hermite3DActor(C_HermiteSplineFlags flags = C_HermiteSplineFlags()) : Base(flags) {}

		///Destructor.
		virtual ~C_Hermite3DActor() {}
	};



	class C_ActorCurve3D
	{
	public:
		///Constructor.
		C_ActorCurve3D() :
			m_pCurve(0),
			m_duration(0) {}

		///Destructor.
		~C_ActorCurve3D() 
		{
			delete m_pCurve;
		}

		void SetCurve(I_InterpolatingCurve<Maths::HVec3>* pCurve)
		{
			if (pCurve == m_pCurve)
				return;

			delete m_pCurve;
			m_pCurve = pCurve;
		}

		void Process(U32 nArcSteps)
		{
			m_duration = Helpers::CalcDuration(m_pCurve);
			m_reparam.BuildTable(m_pCurve, nArcSteps);
			Curve::C_CurveBounds<Maths::HVec3> curveBounds;
			curveBounds.Process(m_pCurve, &m_reparam);
			m_aabb = curveBounds.GetAabb();
		}

		const Maths::C_AxisBox& GetAabb() const
		{
			return m_aabb;
		}

		F32 GetArcLength() const
		{
			return m_reparam.GetArcLength();
		}

		F32 GetArcLengthLinear() const
		{
			return m_pCurve->GetArcLengthLinear();
		}

		F32 GetDuration() const
		{
			return m_duration;
		}

		Maths::HVec3 Evaluate(F32 u) const
		{
			return m_pCurve->Evaluate(u);
		}
		
		Maths::HVec3 EvaluateArcLength(F32 arcLength) const
		{
			return m_pCurve->Evaluate(m_reparam.GetU(arcLength));
		}

		I_InterpolatingCurve<Maths::HVec3>* GetCurve()
		{
			return m_pCurve;
		}

		const I_InterpolatingCurve<Maths::HVec3>* GetCurve() const
		{
			return m_pCurve;
		}

		const C_Reparameterise& GetReparam() const
		{
			return m_reparam;
		}

	protected:
		I_InterpolatingCurve<Maths::HVec3>*	m_pCurve;
		C_Reparameterise		m_reparam;
		Maths::C_AxisBox		m_aabb;
		F32						m_arcLength;
		F32						m_arcLengthLinear;
		F32						m_duration;
	};
}

#endif
