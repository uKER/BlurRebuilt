template<typename KNOT>
C_CurveData<KNOT>::C_CurveData()
: m_stackSize(0)
{
}

template<typename KNOT>
C_CurveData<KNOT>::~C_CurveData()
{
	BIZ_ASSERT(m_stackSize == 0);
}

template<typename KNOT>
KNOT& C_CurveData<KNOT>::AcquireKnot(U32 index)
{
	++m_stackSize;
	BIZ_ASSERT(m_stackSize <= CURVE_ACQUIRE_MAX);
	return m_knots[index];
}

template<typename KNOT>
void C_CurveData<KNOT>::ReleaseKnot() const
{
	--m_stackSize;
	BIZ_ASSERT(m_stackSize >= 0);
}

template<typename KNOT>
void C_CurveData<KNOT>::CommitKnot()
{
	--m_stackSize;
	BIZ_ASSERT(m_stackSize >= 0);
}

template<typename KNOT>
void C_CurveData<KNOT>::CommitKnots()
{
	m_stackSize = 0;
}

template<typename KNOT>
const KNOT& C_CurveData<KNOT>::AcquireKnot(U32 index) const
{
	++m_stackSize;
	BIZ_ASSERT(m_stackSize <= CURVE_ACQUIRE_MAX);
	return m_knots[index];
}

template<typename KNOT>
S32 C_CurveData<KNOT>::GetNumKnots() const
{
	return m_knots.Size();
}

template<typename KNOT>
Btl::Vector<KNOT>& C_CurveData<KNOT>::GetKnots()
{
	return m_knots;
}

template<typename KNOT>
const Btl::Vector<KNOT>& C_CurveData<KNOT>::GetKnots() const
{
	return m_knots;
}



template<typename KNOT, typename DATA_POLICY>
C_CurveDataProxy<KNOT, DATA_POLICY>::C_CurveDataProxy()
: m_stackSize(0)
{
}

template<typename KNOT, typename DATA_POLICY>
C_CurveDataProxy<KNOT, DATA_POLICY>::~C_CurveDataProxy()
{
	assert(m_stackSize == 0);
}

template<typename KNOT, typename DATA_POLICY>
KNOT& C_CurveDataProxy<KNOT, DATA_POLICY>::AcquireKnot(U32 index)
{
	++m_stackSize;
	BIZ_ASSERT(m_stackSize <= CURVE_ACQUIRE_MAX);

	KNOT& knot = m_acquired[m_stackSize-1].knot;
	DATA_POLICY::LoadSourceKnot(knot, index);
	return knot;
}

template<typename KNOT, typename DATA_POLICY>
const KNOT& C_CurveDataProxy<KNOT, DATA_POLICY>::AcquireKnot(U32 index) const
{
	++m_stackSize;
	BIZ_ASSERT(m_stackSize <= CURVE_ACQUIRE_MAX);

	KNOT& knot = m_acquired[m_stackSize-1].knot;
	(const_cast<C_CurveDataProxy<KNOT, DATA_POLICY>*>(this))->LoadSourceKnot(knot, index);
	return knot;
}

template<typename KNOT, typename DATA_POLICY>
void C_CurveDataProxy<KNOT, DATA_POLICY>::ReleaseKnot() const
{
	--m_stackSize;
	BIZ_ASSERT(m_stackSize >= 0);
}

template<typename KNOT, typename DATA_POLICY>
void C_CurveDataProxy<KNOT, DATA_POLICY>::CommitKnot()
{
	--m_stackSize;
	DATA_POLICY::StoreSourceKnot(m_acquired[m_stackSize].knot, m_acquired[m_stackSize].index);
}

template<typename KNOT, typename DATA_POLICY>
void C_CurveDataProxy<KNOT, DATA_POLICY>::CommitKnots()
{
	for (S32 i = 0; i < m_stackSize; ++i)
		DATA_POLICY::StoreSourceKnot(m_acquired[i].knot, m_acquired[i].index);

	m_stackSize = 0;
}

template<typename KNOT, typename DATA_POLICY>
S32 C_CurveDataProxy<KNOT, DATA_POLICY>::GetNumKnots() const
{
	return DATA_POLICY::GetNumSourceKnots();
}

