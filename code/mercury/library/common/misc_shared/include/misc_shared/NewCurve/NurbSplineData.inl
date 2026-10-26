template<typename VECTYPE>
C_NurbSplineData<VECTYPE>::C_NurbSplineData()
{
}

template<typename VECTYPE>
C_NurbSplineData<VECTYPE>::~C_NurbSplineData()
{
}

template<typename VECTYPE>
C_NurbsCv<VECTYPE> C_NurbSplineData<VECTYPE>::GetCv(U32 index) const
{
	return m_cvs[index];
}

template<typename VECTYPE>
void C_NurbSplineData<VECTYPE>::SetCv(U32 index, const C_NurbsCv<VECTYPE>& cv)
{
	m_cvs[index] = cv;
}

template<typename VECTYPE>
F32 C_NurbSplineData<VECTYPE>::GetKnot(U32 index) const
{
	return m_knots[index];
}

template<typename VECTYPE>
void C_NurbSplineData<VECTYPE>::SetKnot(U32 index, F32 knot)
{
	m_knots[index] = knot;
}

template<typename VECTYPE>
S32 C_NurbSplineData<VECTYPE>::GetNumCvs() const
{
	return m_cvs.Size();
}

template<typename VECTYPE>
S32 C_NurbSplineData<VECTYPE>::GetNumKnots() const
{
	return m_knots.Size();
}

template<typename VECTYPE>
Btl::Vector<C_NurbsCv<VECTYPE> >& C_NurbSplineData<VECTYPE>::GetCvs()
{
	return m_cvs;
}

template<typename VECTYPE>
const Btl::Vector<C_NurbsCv<VECTYPE> >& C_NurbSplineData<VECTYPE>::GetCvs() const
{
	return m_cvs;
}

template<typename VECTYPE>
KnotVector& C_NurbSplineData<VECTYPE>::GetKnots()
{
	return m_knots;
}

template<typename VECTYPE>
const KnotVector& C_NurbSplineData<VECTYPE>::GetKnots() const
{
	return m_knots;
}
