template<typename VECTYPE, typename STORAGE>
C_CardinalSpline<VECTYPE, STORAGE>::C_CardinalSpline(C_CardinalSplineFlags flags)
: C_InterpolatingCurveBase<VECTYPE, STORAGE>()
{
	m_flags = flags;
}

template<typename VECTYPE, typename STORAGE>
C_CardinalSpline<VECTYPE, STORAGE>::~C_CardinalSpline()
{
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_CardinalSpline<VECTYPE, STORAGE>::Evaluate(F32 u) const
{
	//Get adjacent knots.
	S32 nKnots = Base::m_pData->GetNumKnots();
	S32 ka = Helpers::GetKnotInterval(Base::m_pData, u);
	BIZ_ASSERT(ka < nKnots);

	if(ka < 0)
		return VECTYPE();

	S32 kb = ka + 1;

	if(kb >= nKnots)
	{
		VECTYPE r = Base::m_pData->AcquireKnot(ka).value;
		Base::m_pData->ReleaseKnot();
		return r;
	}

	const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(ka);
	const typename STORAGE::KnotType& knot1 = Base::m_pData->AcquireKnot(kb);

	F32 knot0u = knot0.GetU(ka, nKnots);
	F32 knot1u = knot1.GetU(kb, nKnots);
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

	//Create geometry matrix.
	Maths::HMat44 g;
	Helpers::BuildGeometryMatrix(
		g, 
		knot0.value, 
		knot1.value,
		knot0.tangent * knot0.tangentScale,
		knot1.tangent * knot1.tangentScale);

	//x = [t^3 t^2 t 1] .  M(basis) . [p0]
	//                                [p1]
	//                                [r0]
	//                                [r1] 
	
	Base::m_pData->ReleaseKnot();
	Base::m_pData->ReleaseKnot();
	return Helpers::Load<VECTYPE>(tvec * kHermiteBasis * g);
}

template<typename VECTYPE, typename STORAGE>
void C_CardinalSpline<VECTYPE, STORAGE>::Process()
{
	U32 nKnots = Base::m_pData->GetNumKnots();
	if (nKnots < 2)
		return;

	Base::_Process(m_flags.calcKnotU);

	//Automatically calculate tangents. User has no control over tangents when using cardinal splines.
	{
		//Create phantom in tangent for i=0 given that there is no knot at i=-1;
		{
			typename STORAGE::KnotType&			knot0 = Base::m_pData->AcquireKnot(0);
			const typename STORAGE::KnotType&	knot1 = Base::m_pData->AcquireKnot(1);
			knot0.tangent = knot1.value - knot0.value;
			Base::m_pData->ReleaseKnot();
			Base::m_pData->CommitKnot();
		}
		U32 i = 1;
		for(; i < nKnots - 1; ++i)
		{
			const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(i-1);
			const typename STORAGE::KnotType& knot2 = Base::m_pData->AcquireKnot(i+1);
			//Auto generated in and out-tangents are the same in both direction and magnitude.
			//So our curve exhibits G1 (tangents same direction) and C1 continuity (tangents same direction AND magnitude).
			typename STORAGE::KnotType&		 knot1 = Base::m_pData->AcquireKnot(i);
			knot1.tangent = knot2.value - knot0.value;
			Base::m_pData->CommitKnot();
			Base::m_pData->ReleaseKnot();
			Base::m_pData->ReleaseKnot();
		}
		//Create phantom out tangent for i=n-1 given that there is no knot at i=n;
		{
			const typename STORAGE::KnotType&	knot0 = Base::m_pData->AcquireKnot(i-1);
			typename STORAGE::KnotType&			knot1 = Base::m_pData->AcquireKnot(i);
			knot1.tangent = knot1.value - knot0.value;
			Base::m_pData->CommitKnot();
			Base::m_pData->ReleaseKnot();
		}
	}
}

