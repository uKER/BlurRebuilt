template<typename VECTYPE>
I_Curve<VECTYPE>::~I_Curve()
{
}

template<typename VECTYPE>
VECTYPE I_Curve<VECTYPE>::EvaluateTangent(F32 u) const
{
	const F32 kHalfDelta = 0.025f;
	F32 x0 = u-kHalfDelta;
	F32 x1 = u+kHalfDelta;
	VECTYPE y0 = Evaluate(x0);
	VECTYPE y1 = Evaluate(x1);
	VECTYPE dy = y1 - y0;
	F32 dx = x1 - x0;
	return dy /dx;
}

template<typename VECTYPE, typename STORAGE>
C_InterpolatingCurveBase<VECTYPE, STORAGE>::C_InterpolatingCurveBase()
: m_pData(0), m_arcLengthLinear(0)
{
}

template<typename VECTYPE, typename STORAGE>
C_InterpolatingCurveBase<VECTYPE, STORAGE>::~C_InterpolatingCurveBase()
{
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_InterpolatingCurveBase<VECTYPE, STORAGE>::EvaluateLinear(F32 u) const
{
	return Helpers::KnotLerp<VECTYPE, STORAGE>(m_pData, u);
}

template<typename VECTYPE, typename STORAGE>
F32 C_InterpolatingCurveBase<VECTYPE, STORAGE>::GetMinU() const
{
	if (!m_pData || m_pData->GetNumKnots() == 0)
		return 0;

	F32 u = m_pData->AcquireKnot(0).GetU(0, m_pData->GetNumKnots());
	m_pData->ReleaseKnot();
	return u;
}

template<typename VECTYPE, typename STORAGE>
F32 C_InterpolatingCurveBase<VECTYPE, STORAGE>::GetMaxU() const
{
	if (!m_pData || m_pData->GetNumKnots() == 0)
		return 0;

	F32 u = m_pData->AcquireKnot(m_pData->GetNumKnots()-1).GetU(m_pData->GetNumKnots()-1, m_pData->GetNumKnots());
	m_pData->ReleaseKnot();
	return u;
}

template<typename VECTYPE, typename STORAGE>
void C_InterpolatingCurveBase<VECTYPE, STORAGE>::SetData(STORAGE* pData)
{
	m_pData = pData;
}

template<typename VECTYPE, typename STORAGE>
STORAGE* C_InterpolatingCurveBase<VECTYPE, STORAGE>::GetData()
{
	return m_pData;
}

template<typename VECTYPE, typename STORAGE>
const STORAGE* C_InterpolatingCurveBase<VECTYPE, STORAGE>::GetData() const
{
	return m_pData;
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_InterpolatingCurveBase<VECTYPE, STORAGE>::GetVertexValue(U32 index) const
{
	VECTYPE tmp = m_pData->AcquireKnot(index).value;
	m_pData->ReleaseKnot();
	return tmp;
}

template<typename VECTYPE, typename STORAGE>
const void* C_InterpolatingCurveBase<VECTYPE, STORAGE>::AcquireVertexData(U32 index) const
{
	return &m_pData->AcquireKnot(index);
}

template<typename VECTYPE, typename STORAGE>
void C_InterpolatingCurveBase<VECTYPE, STORAGE>::ReleaseVertex() const
{
	m_pData->ReleaseKnot();
}

template<typename VECTYPE, typename STORAGE>
S32 C_InterpolatingCurveBase<VECTYPE, STORAGE>::GetNumVertices() const
{
	return m_pData->GetNumKnots();
}

template<typename VECTYPE, typename STORAGE>
F32 C_InterpolatingCurveBase<VECTYPE, STORAGE>::GetArcLengthLinear() const
{
	return m_arcLengthLinear;
}

template<typename VECTYPE, typename STORAGE>
F32 C_InterpolatingCurveBase<VECTYPE, STORAGE>::GetMinSegmentLength() const
{
	return m_minSegmentLength;
}

template<typename VECTYPE, typename STORAGE>
void C_InterpolatingCurveBase<VECTYPE, STORAGE>::_Process(E_ProcessU processU)
{
	m_arcLengthLinear = Helpers::CalcArcLengthLinear(this);
	m_minSegmentLength = Helpers::FindMinSegmentLength(this);

	switch(processU)
	{
	case E_ProcessU_Regular:
		Helpers::SetRegularU(m_pData);
		break;

	case E_ProcessU_ArcLengthLinear:
		Helpers::SetArcLengthLinearU(m_pData, m_arcLengthLinear);
		break;

	case E_ProcessU_None:
	default:
		break;
	};
}


template<typename VECTYPE, typename STORAGE>
C_ApproximatingCurveBase<VECTYPE, STORAGE>::C_ApproximatingCurveBase()
: m_pData(0)
{
}

template<typename VECTYPE, typename STORAGE>
C_ApproximatingCurveBase<VECTYPE, STORAGE>::~C_ApproximatingCurveBase()
{
}

template<typename VECTYPE, typename STORAGE>
void C_ApproximatingCurveBase<VECTYPE, STORAGE>::SetData(STORAGE* pData)
{
	m_pData = pData;
}

template<typename VECTYPE, typename STORAGE>
STORAGE* C_ApproximatingCurveBase<VECTYPE, STORAGE>::GetData()
{
	return m_pData;
}

template<typename VECTYPE, typename STORAGE>
const STORAGE* C_ApproximatingCurveBase<VECTYPE, STORAGE>::GetData() const
{
	return m_pData;
}

