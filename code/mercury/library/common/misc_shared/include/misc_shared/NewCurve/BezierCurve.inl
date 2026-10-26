template<typename VECTYPE, typename STORAGE>
C_BezierCurve<VECTYPE, STORAGE>::C_BezierCurve(C_BezierCurveFlags flags)
: C_InterpolatingCurveBase<VECTYPE, STORAGE>()
{
	m_flags = flags;
}

template<typename VECTYPE, typename STORAGE>
C_BezierCurve<VECTYPE, STORAGE>::~C_BezierCurve()
{
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_BezierCurve<VECTYPE, STORAGE>::Evaluate(F32 u) const
{
	//Get adjacent knots.
	S32 max = Base::m_pData->GetNumKnots();
	S32 start = Helpers::GetKnotInterval(Base::m_pData, u);

	if(start < 0)
		return VECTYPE();

	S32 end = start + 1;

	if(end >= max)
	{
		VECTYPE r = Base::m_pData->AcquireKnot(start).value;
		Base::m_pData->ReleaseKnot();
		return r;
	}

	//Start point
	const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(start);
	//End point
	const typename STORAGE::KnotType& knot1 = Base::m_pData->AcquireKnot(start+1);

	F32 knot0u = knot0.GetU(start, max);
	F32 knot1u = knot1.GetU(start+1, max);
	F32 period = knot1u - knot0u;
	//Calculate parameter for curve segment, let's call it t.
	F32 t = (u - knot0u) / period;
	if (t > 1.f)
	{
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
		return knot1.value;
	}
	else if (t <= FLT_EPSILON)
	{
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
		return knot0.value;
	}
	F32 t2 = t * t;
	F32 t3 = t2 * t;
	//Create Blend parameter vector.
	Maths::HVec4 tvec(t3, t2, t, 1);

	Maths::HMat44 g;
	//Create geometry matrix.
	Helpers::BuildGeometryMatrix(
		g, 
		knot0.value, 
		knot0.ctrlOut,
		knot1.ctrlIn,
		knot1.value);
	//x = [t^3 t^2 t 1] .  M(basis) . [knot0.value]
	//                                [control0]
	//                                [control1]
	//                                [knot1.value] 

	Base::m_pData->ReleaseKnot();
	Base::m_pData->ReleaseKnot();
	return Helpers::Load<VECTYPE>(tvec * kBezierBasis * g);
}

template<typename VECTYPE, typename STORAGE>
void C_BezierCurve<VECTYPE, STORAGE>::Process()
{
	U32 nKnots = Base::m_pData->GetNumKnots();
	if (nKnots < 2)
		return;

	Base::_Process(m_flags.calcKnotU);

	//Automatically calculate tangents where uninitialised.
	if (m_flags.calcCtrlPoints)
	{
		//Create phantom in tangent for i=0 given that there is no knot at i=-1;
		{
			typename STORAGE::KnotType&			knot0 = Base::m_pData->AcquireKnot(0);
			const typename STORAGE::KnotType&	knot1 = Base::m_pData->AcquireKnot(1);

			knot0.ctrlIn =
				Helpers::InTangentToControlPoint<VECTYPE>(knot0.value, knot1.value - knot0.value, 1.f);
			knot0.ctrlOut =
				Helpers::OutTangentToControlPoint<VECTYPE>(knot0.value, knot1.value - knot0.value, 1.f);

			Base::m_pData->ReleaseKnot();
			Base::m_pData->CommitKnot();
		}
		U32 i = 1;
		for(; i < nKnots - 1; ++i)
		{
			{
				const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(i-1);
				const typename STORAGE::KnotType& knot2 = Base::m_pData->AcquireKnot(i+1);
				//Auto generated in and out-tangents are the same in both direction and magnitude.
				//So our curve exhibits G1 (tangents same direction) and C1 continuity (tangents same direction AND magnitude).
				typename STORAGE::KnotType&		 knot1 = Base::m_pData->AcquireKnot(i);

				knot1.ctrlIn =
					Helpers::InTangentToControlPoint<VECTYPE>(knot1.value, knot2.value - knot0.value, 1.f);
				knot1.ctrlOut = 
					Helpers::OutTangentToControlPoint<VECTYPE>(knot1.value, knot2.value - knot0.value, 1.f);

				Base::m_pData->CommitKnot();
				Base::m_pData->ReleaseKnot();
				Base::m_pData->ReleaseKnot();
			}
		}
		//Create phantom out tangent for i=n-1 given that there is no knot at i=n;
		{
			const typename STORAGE::KnotType&	knot0 = Base::m_pData->AcquireKnot(i-1);
			typename STORAGE::KnotType&			knot1 = Base::m_pData->AcquireKnot(i);

			knot1.ctrlIn =
				Helpers::InTangentToControlPoint<VECTYPE>(knot1.value, knot1.value - knot0.value, 1.f);
			knot1.ctrlOut = 
				Helpers::OutTangentToControlPoint<VECTYPE>(knot1.value, knot1.value - knot0.value, 1.f);

			Base::m_pData->CommitKnot();
			Base::m_pData->ReleaseKnot();
		}
	}
}

