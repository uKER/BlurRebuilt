template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
C_HermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::C_HermiteSpline(C_HermiteSplineFlags flags)
: C_InterpolatingCurveBase<VECTYPE, STORAGE>()
{
	m_flags = flags;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
C_HermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::~C_HermiteSpline()
{
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
VECTYPE C_HermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::Evaluate(F32 u) const
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
		
	//Intermediate points.
	VECTYPE control0;
	VECTYPE control1;
	GenerateCtrlPts(control0, control1, knot0, knot0u, knot1, knot1u);

	F32 t2 = t * t;
	F32 t3 = t2 * t;
	//Create Blend parameter vector.
	Maths::HVec4 tvec(t3, t2, t, 1);

	Maths::HMat44 g;
	//Create geometry matrix.
	Helpers::BuildGeometryMatrix(
		g, 
		knot0.value, 
		control0,
		control1,
		knot1.value);
	//x = [t^3 t^2 t 1] .  M(basis) . [knot0.value]
	//                                [control0]
	//                                [control1]
	//                                [knot1.value] 

	Base::m_pData->ReleaseKnot();
	Base::m_pData->ReleaseKnot();
	return Helpers::Load<VECTYPE>(tvec * kBezierBasis * g);
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
void C_HermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::SetFlags(C_HermiteSplineFlags flags)
{
	m_flags = flags;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
void C_HermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::Process()
{
	U32 nKnots = Base::m_pData->GetNumKnots();
	if (nKnots < 2)
		return;

	Base::_Process(m_flags.calcKnotU);

	if (m_flags.calcTangents)
		for(U32 i = 0; i < nKnots; ++i)
			CalculateKnotTangents(i);
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
void C_HermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::CalculateKnotTangents(U32 knotIndex, bool in, bool out, bool closed)
{
	//Create phantom in tangent for i=0 given that there is no knot at i=-1;
	if (knotIndex == 0)
	{
		if (closed)
		{
			if (Base::m_pData->GetNumKnots() < 3)
				return;

			const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(Base::m_pData->GetNumKnots()-2);
			const typename STORAGE::KnotType& knot2 = Base::m_pData->AcquireKnot(1);
			//Auto generated in and out-tangents are the same in both direction and magnitude.
			//So our curve exhibits G1 (tangents same direction) and C1 continuity (tangents same direction AND magnitude).
			typename STORAGE::KnotType&		 knot1 = Base::m_pData->AcquireKnot(0);
			VECTYPE tan = knot2.value - knot0.value;
			if (in) 
				knot1.tangentIn = tan;
			if (out) 
				knot1.tangentOut = tan;
			Base::m_pData->CommitKnot();
			Base::m_pData->ReleaseKnot();
			Base::m_pData->ReleaseKnot();
		}
		else
		{
			typename STORAGE::KnotType&			knot0 = Base::m_pData->AcquireKnot(0);
			const typename STORAGE::KnotType&	knot1 = Base::m_pData->AcquireKnot(1);
			VECTYPE tan = knot1.value - knot0.value;
			if (in) 
				knot0.tangentIn = tan;
			if (out) 
				knot0.tangentOut = tan;
			Base::m_pData->ReleaseKnot();
			Base::m_pData->CommitKnot();
			return;
		}
	}
	//Create phantom out tangent for i=n-1 given that there is no knot at i=n;
	else if (knotIndex == (U32)(Base::m_pData->GetNumKnots()-1))
	{
		if (closed)
		{
			if (Base::m_pData->GetNumKnots() < 3)
				return;

			const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(knotIndex-1);
			const typename STORAGE::KnotType& knot2 = Base::m_pData->AcquireKnot(1);
			//Auto generated in and out-tangents are the same in both direction and magnitude.
			//So our curve exhibits G1 (tangents same direction) and C1 continuity (tangents same direction AND magnitude).
			typename STORAGE::KnotType&		 knot1 = Base::m_pData->AcquireKnot(knotIndex);
			VECTYPE tan = knot2.value - knot0.value;
			if (in) 
				knot1.tangentIn = tan;
			if (out) 
				knot1.tangentOut = tan;
			Base::m_pData->CommitKnot();
			Base::m_pData->ReleaseKnot();
			Base::m_pData->ReleaseKnot();
		}
		else
		{
			const typename STORAGE::KnotType&	knot0 = Base::m_pData->AcquireKnot(knotIndex-1);
			typename STORAGE::KnotType&			knot1 = Base::m_pData->AcquireKnot(knotIndex);
			VECTYPE tan = knot1.value - knot0.value;
			if (in) 
				knot1.tangentIn = tan;
			if (out) 
				knot1.tangentOut = tan;
			Base::m_pData->CommitKnot();
			Base::m_pData->ReleaseKnot();
		}
	}
	else
	{
		const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(knotIndex-1);
		const typename STORAGE::KnotType& knot2 = Base::m_pData->AcquireKnot(knotIndex+1);
		//Auto generated in and out-tangents are the same in both direction and magnitude.
		//So our curve exhibits G1 (tangents same direction) and C1 continuity (tangents same direction AND magnitude).
		typename STORAGE::KnotType&		 knot1 = Base::m_pData->AcquireKnot(knotIndex);
		VECTYPE tan = knot2.value - knot0.value;
		if (in) 
			knot1.tangentIn = tan;
		if (out) 
			knot1.tangentOut = tan;
		Base::m_pData->CommitKnot();
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
	}
}