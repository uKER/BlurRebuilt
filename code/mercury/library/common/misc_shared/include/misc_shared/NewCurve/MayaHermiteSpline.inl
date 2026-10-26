template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::C_MayaHermiteSpline(C_MayaHermiteSplineFlags flags)
: C_InterpolatingCurveBase<VECTYPE, STORAGE>()
{
	m_flags = flags;

	m_preInfinity = E_InfinityType_Constant;
	m_postInfinity = E_InfinityType_Constant;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::~C_MayaHermiteSpline()
{
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
VECTYPE C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::Evaluate(F32 u) const
{
	S32 iKnot0, iKnot1;
	return EvaluateReturnAdjacent(iKnot0, iKnot1, u);
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
VECTYPE C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::EvaluateReturnAdjacent(S32& iKnot0, S32& iKnot1, F32 u) const
{
	F32 filteredU(u);
	if (u <= Base::GetMinU())
	{
		if (m_preInfinity == E_InfinityType_Constant)
			return ConstantPreInfinity(u);
		else if (m_preInfinity == E_InfinityType_Linear)
			return LinearPreInfinity(u);
		else
			filteredU = InfinityFilterU(u, m_preInfinity);
	}
	else if (u >= Base::GetMaxU())
	{
		if (m_postInfinity == E_InfinityType_Constant)
			return ConstantPostInfinity(u);
		else if (m_postInfinity == E_InfinityType_Linear)
			return LinearPostInfinity(u);
		else
			filteredU = InfinityFilterU(u, m_postInfinity);
	}

	//Get adjacent knots.
	S32 nKnots = Base::m_pData->GetNumKnots();
	iKnot0 = Helpers::GetKnotInterval(Base::m_pData, filteredU);
	S32 max = nKnots;
	BIZ_ASSERT(iKnot0 < max);

	if(iKnot0 < 0)
		return VECTYPE();

	iKnot1 = iKnot0 + 1;

	if(iKnot1 >= max)
	{
		VECTYPE r = Base::m_pData->AcquireKnot(iKnot0).value;
		Base::m_pData->ReleaseKnot();
		return r;
	}

	//Start point
	const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(iKnot0);
	//End point
	const typename STORAGE::KnotType& knot1 = Base::m_pData->AcquireKnot(iKnot1);

	if (knot0.tangentOutType == E_TangentType_Stepped)
	{
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
		return InfinityFilterResult(knot0.value, u);
	}
	else if (knot0.tangentOutType == E_TangentType_SteppedNext)
	{
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
		return InfinityFilterResult(knot1.value, u);
	}

	F32 knot0u = knot0.GetU(iKnot0, max);
	F32 knot1u = knot1.GetU(iKnot1, max);
	F32 period = knot1u - knot0u;
	//Calculate parameter for curve segment, let's call it t.
	F32 t = (filteredU - knot0u) / period;
	if (t >= 1.f)
	{
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
		return InfinityFilterResult(knot1.value, u);
	}
	else if (t <= FLT_EPSILON)
	{
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
		return InfinityFilterResult(knot0.value, u);
	}

	if ((knot0.tangentOutType == E_TangentType_Clamped || 
		knot1.tangentInType == E_TangentType_Clamped) && period < 0.01f)
	{
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
		VECTYPE d = knot1.value - knot0.value;
		return InfinityFilterResult(d * t, u);
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
	return InfinityFilterResult(Helpers::Load<VECTYPE>(tvec * kBezierBasis * g), u);
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
void C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::Process()
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
void C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::SetPreInfinity(E_InfinityType type)
{
	m_preInfinity = type;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
void C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::SetPostInfinity(E_InfinityType type)
{
	m_postInfinity = type;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
E_InfinityType C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::GetPreInfinity() const
{
	return m_preInfinity;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
E_InfinityType C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::GetPostInfinity() const
{
	return m_postInfinity;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
F32 C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::InfinityFilterU(
	F32 u, E_InfinityType type) const
{
	if (u >= Base::GetMinU() && u <= Base::GetMaxU())
		return u;

	switch(type)
	{
	case E_InfinityType_CycleWithOffset:
	case E_InfinityType_Cycle:
	{
		F32 duration = Base::GetMaxU() - Base::GetMinU();
		F32 f = fmod(fabsf(u), duration);
		F32 cycleFrac = f / duration;
		//Reverse cycle direction?
		if (u < 0)
			cycleFrac = 1.f - cycleFrac;
		u = cycleFrac * duration + Base::GetMinU();
		break;
	}

	case E_InfinityType_Oscillate:
	{
		F32 distanceFromCycle = fabsf(Base::GetMinU() - u);
	
		F32 duration = Base::GetMaxU() - Base::GetMinU();
		F32 cycleCount = Maths::Floor(distanceFromCycle / duration);

		F32 f = fmod(distanceFromCycle, duration);
		F32 cycleFrac = f / duration;

		U32 cycleCountI = (U32)(cycleCount);

		//Alternate direction between repeated segments.
		if (cycleCountI % 2 != 0)
			cycleFrac = 1.f - cycleFrac;

		u = cycleFrac * duration + Base::GetMinU();
		break;
	}
	default:
		break;
	};

	return u;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
VECTYPE C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::InfinityFilterResult(VECTYPE result, F32 u) const
{
	if (u < Base::GetMinU() && m_preInfinity == E_InfinityType_CycleWithOffset)
	{
		const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(0);
		const typename STORAGE::KnotType& knot1 = Base::m_pData->AcquireKnot(Base::m_pData->GetNumKnots()-1);

		F32 duration = Base::GetMaxU() - Base::GetMinU();
		F32 distanceFromCycle = Base::GetMinU() - u;
		F32 cycleCount = 1.f + distanceFromCycle / duration;
		cycleCount = Maths::Floor(cycleCount);

		VECTYPE offset = ((knot1.value - knot0.value) * -cycleCount);
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
		return offset + result;
	}
	else if (u > Base::GetMaxU() && m_postInfinity == E_InfinityType_CycleWithOffset)
	{
		const typename STORAGE::KnotType& knot0 = Base::m_pData->AcquireKnot(0);
		const typename STORAGE::KnotType& knot1 = Base::m_pData->AcquireKnot(Base::m_pData->GetNumKnots()-1);

		F32 duration = Base::GetMaxU() - Base::GetMinU();
		F32 distanceFromCycle = u - Base::GetMaxU();
		F32 cycleCount = 1.f + distanceFromCycle / duration;
		cycleCount = Maths::Floor(cycleCount);

		VECTYPE offset = ((knot1.value - knot0.value) * cycleCount);
		Base::m_pData->ReleaseKnot();
		Base::m_pData->ReleaseKnot();
		return offset + result;
	}

	return result;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
VECTYPE C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::ConstantPreInfinity(F32 u) const
{
	const typename STORAGE::KnotType& knot = Base::m_pData->AcquireKnot(0);
	VECTYPE result = knot.value;
	Base::m_pData->ReleaseKnot();
	return result;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
VECTYPE C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::ConstantPostInfinity(F32 u) const
{
	const typename STORAGE::KnotType& knot = Base::m_pData->AcquireKnot(Base::m_pData->GetNumKnots()-1);
	VECTYPE result = knot.value;
	Base::m_pData->ReleaseKnot();
	return result;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
VECTYPE C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::LinearPreInfinity(F32 u) const
{
	const typename STORAGE::KnotType& knot = Base::m_pData->AcquireKnot(0);
	VECTYPE result = GenerateLinearPreInfinity(
		knot, 
		knot.GetU(0, Base::m_pData->GetNumKnots()),
		u);
	Base::m_pData->ReleaseKnot();
	return result;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
VECTYPE C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::LinearPostInfinity(F32 u) const
{
	S32 nKnots = Base::m_pData->GetNumKnots();
	S32 knotIdx = Base::m_pData->GetNumKnots()-1;
	const typename STORAGE::KnotType& knot = Base::m_pData->AcquireKnot(nKnots-1);
	VECTYPE result = GenerateLinearPostInfinity(
		knot, 
		knot.GetU(knotIdx, nKnots),
		u);
	Base::m_pData->ReleaseKnot();
	return result;
}

template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
void C_MayaHermiteSpline<VECTYPE, STORAGE, CTRLPOINT_POLICY>::CalculateKnotTangents(U32 knotIndex, bool in, bool out)
{
	//Create phantom in tangent for i=0 given that there is no knot at i=-1;
	if (knotIndex == 0)
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
	//Create phantom out tangent for i=n-1 given that there is no knot at i=n;
	else if (knotIndex == Base::m_pData->GetNumKnots()-1)
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