template<typename VECTYPE>
C_ArcIterator<VECTYPE>::C_ArcIterator(
	const I_Curve<VECTYPE>& curve,
	const C_Reparameterise& table, 
	F32 arcSegmentLength)
:
m_pCurve(&curve),
m_pTable(&table),
m_arcSegmentLength(arcSegmentLength)
{
	BIZ_ASSERT(arcSegmentLength != 0);

	m_arcLength = m_pTable->GetArcLength();
	if (arcSegmentLength > m_arcLength)
		arcSegmentLength = m_arcLength;
	m_numSegments = (U32)(Maths::Ceil(m_arcLength / m_arcSegmentLength));
	Reset();
}

template<typename VECTYPE>
C_ArcIterator<VECTYPE>::C_ArcIterator(const C_ArcIterator<VECTYPE>& rhs)
{
	m_pCurve = rhs.m_pCurve;
	m_pTable = rhs.m_pTable;
	m_currentSegment = rhs.m_currentSegment;

	m_numSegments = rhs.m_numSegments;
	m_currentValue = rhs.m_currentValue;
}

template<typename VECTYPE>
C_ArcIterator<VECTYPE>::~C_ArcIterator()
{
}

template<typename VECTYPE>
const C_ArcIterator<VECTYPE>& C_ArcIterator<VECTYPE>::operator=(const C_ArcIterator<VECTYPE>& rhs)
{
	m_pCurve = rhs.m_pCurve;
	m_pTable = rhs.m_pTable;
	m_currentSegment = rhs.m_currentSegment;

	m_numSegments = rhs.m_numSegments;
	m_currentValue = rhs.m_currentValue;
	return *this;
}

template<typename VECTYPE>
C_ArcIterator<VECTYPE>::operator const VECTYPE*()
{
	if (m_currentSegment <= m_numSegments)
		return &m_currentValue;

	return 0;
}

template<typename VECTYPE>
const VECTYPE* C_ArcIterator<VECTYPE>::operator ++()
{
	if (m_currentSegment <= m_numSegments)
	{
		++m_currentSegment;
		F32 distance = m_arcSegmentLength * m_currentSegment;
		if (distance > m_arcLength)
			distance = m_arcLength;
		F32 param = m_pTable->GetU(distance);
		m_currentValue = m_pCurve->Evaluate(param);
		return &m_currentValue;
	}

	return 0;
}

template<typename VECTYPE>
const VECTYPE* C_ArcIterator<VECTYPE>::operator ++(int) 
{
	const VECTYPE* pTmp = ++(*this);
	return pTmp;
}
			
template<typename VECTYPE>
void C_ArcIterator<VECTYPE>::Reset()
{
	if (m_pTable->GetNumPoints() != 0)
	{
		m_currentSegment = 0;
		m_currentValue = m_pCurve->Evaluate(m_pTable->GetUByIndex(0));
	}
	else
		m_currentSegment = 0xffffffff;
}

template<typename VECTYPE>
U32 C_ArcIterator<VECTYPE>::GetNumPoints() const
{
	return m_numSegments+1;
}