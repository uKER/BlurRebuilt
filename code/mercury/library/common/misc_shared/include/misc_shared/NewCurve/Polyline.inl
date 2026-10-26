template<typename VECTYPE, typename STORAGE>
C_PolyLine<VECTYPE, STORAGE>::C_PolyLine(C_PolyLineFlags flags)
: C_InterpolatingCurveBase<VECTYPE, STORAGE>()
{
	m_flags = flags;
}

template<typename VECTYPE, typename STORAGE>
C_PolyLine<VECTYPE, STORAGE>::~C_PolyLine()
{
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_PolyLine<VECTYPE, STORAGE>::Evaluate(F32 u) const
{
	return Helpers::KnotLerp<VECTYPE, STORAGE>(Base::m_pData, u);
}

template<typename VECTYPE, typename STORAGE>
bool C_PolyLine<VECTYPE, STORAGE>::Process()
{
	U32 nKnots = Base::m_pData->GetNumKnots();
	if (nKnots < 2)
		return false;

	Base::_Process(m_flags.calcKnotU);
	return true;
}

