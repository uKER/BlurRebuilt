template<typename VECTYPE, typename STORAGE>
C_NurbSpline<VECTYPE, STORAGE>::C_NurbSpline(U32 degree)
: C_ApproximatingCurveBase<VECTYPE, STORAGE>(), 
m_degree(degree)
{
}

template<typename VECTYPE, typename STORAGE>
C_NurbSpline<VECTYPE, STORAGE>::~C_NurbSpline()
{
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_NurbSpline<VECTYPE, STORAGE>::Evaluate(F32 u) const
{
	BIZ_ASSERT(Base::m_pData);
	
	//Scale normalised u value to the 0..nSpans-1 range.
	if (u < 0)
		u = 0;
	else if (u >= 1.f)
		u = 1.f - 1e-7f;
	//t is our scaled parameter value.
	U32 nSpans = GetNumSpans();
	F32 span = u * nSpans;

	VECTYPE total;
	Curve::Helpers::Zero(total);

	U32 iSpan = U32(span);
	if (iSpan == nSpans)
		--iSpan;

	F32 segStart = GetKnot(iSpan+m_degree);
	F32 segEnd = GetKnot(iSpan+m_degree+1);
	F32 period = segEnd - segStart;
	F32 uSpan = span - (F32)iSpan;
	F32 t = segStart + period * uSpan;

	for (U32 k = 0; k <= m_degree; k++)
	{
		C_NurbsCv<VECTYPE> cv =  Base::m_pData->GetCv(iSpan+k);
		total = total + cv.value * CoxDeBoor(iSpan+k, m_degree, t) * cv.weight;
	}

	return total;
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_NurbSpline<VECTYPE, STORAGE>::EvaluateLinear(F32 u) const
{
	//Not currently supported for NURB Splines
	return Evaluate(u);
}

template<typename VECTYPE, typename STORAGE>
F32 C_NurbSpline<VECTYPE, STORAGE>::GetMinU() const
{
	return 0;
}

template<typename VECTYPE, typename STORAGE>
F32 C_NurbSpline<VECTYPE, STORAGE>::GetMaxU() const
{
	return 1.f;
}

template<typename VECTYPE, typename STORAGE>
const VECTYPE& C_NurbSpline<VECTYPE, STORAGE>::AcquireVertexValue(U32 index) const
{
	return Base::m_pData->GetCvs()[index].value;
}

template<typename VECTYPE, typename STORAGE>
const void* C_NurbSpline<VECTYPE, STORAGE>::AcquireVertexData(U32 index) const
{
	return &Base::m_pData->GetCvs()[index].value;
}

template<typename VECTYPE, typename STORAGE>
void C_NurbSpline<VECTYPE, STORAGE>::ReleaseVertex() const
{
}

template<typename VECTYPE, typename STORAGE>
S32 C_NurbSpline<VECTYPE, STORAGE>::GetNumVertices() const
{
	return Base::m_pData->GetNumCvs();
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_NurbSpline<VECTYPE, STORAGE>::GetVertexValue(U32 index) const
{
	return Base::m_pData->GetCvs()[index].value;
}

template<typename VECTYPE, typename STORAGE>
U32 C_NurbSpline<VECTYPE, STORAGE>::GetDegree() const
{
	return m_degree;
}

template<typename VECTYPE, typename STORAGE>
F32 C_NurbSpline<VECTYPE, STORAGE>::GetKnot(U32 index) const
{
	if (m_flags.generatePhantomKnots)
	{
		//Cox-DeBoor requires two additional knot entries. Our not vector is effectively expanded
		//via this function.
		U32 nPhysicalKnots = Base::m_pData->GetNumKnots();
		U32 nLogicalKnots = nPhysicalKnots+2;
		BIZ_ASSERT(index < nLogicalKnots);

		//Expand front of vector.
		if (index == 0)
		{
			F32 d = Base::m_pData->GetKnot(1) - Base::m_pData->GetKnot(0);
			return Base::m_pData->GetKnot(0) - d;
		}
		//Expand end of vector
		else if (index == nLogicalKnots-1)
		{
			F32 d = Base::m_pData->GetKnot(nPhysicalKnots-1) - Base::m_pData->GetKnot(nPhysicalKnots-2);
			return Base::m_pData->GetKnot(nPhysicalKnots-1) + d;
		}

		return Base::m_pData->GetKnot(index-1);
	}
	//Else we've got sufficient knots to index directly.
	return Base::m_pData->GetKnot(index);
}

template<typename VECTYPE, typename STORAGE>
U32 C_NurbSpline<VECTYPE, STORAGE>::GetNumKnots() const
{
	//Add two phantom knots.
	if (m_flags.generatePhantomKnots)
		return Base::m_pData->GetNumKnots()+2;
	else
		return Base::m_pData->GetNumKnots();
}

template<typename VECTYPE, typename STORAGE>
U32 C_NurbSpline<VECTYPE, STORAGE>::GetNumSpans() const
{
	return GetNumKnots() - 2 * m_degree - 1;
}

template<typename VECTYPE, typename STORAGE>
void C_NurbSpline<VECTYPE, STORAGE>::Process()
{
	if (Base::m_pData->GetNumKnots() < Base::m_pData->GetNumCvs() + (S32)m_degree+1)
		m_flags.generatePhantomKnots = true;

	if (GetNumKnots() < Base::m_pData->GetNumCvs() + m_degree+1)
	{
		BIZ_ASSERT(!"Invalid spline!");
		return;
	}
}

template<typename VECTYPE, typename STORAGE>
F32 C_NurbSpline<VECTYPE, STORAGE>::CoxDeBoor(U32 i, U32 degree, F32 t) const
{
	F32 temp1, temp2;
	if (i + degree >= GetNumKnots())
		//Invalid spline.
		return 0.f;

	if (degree == 0) 
	{
		if (GetKnot(i) <= t && t < GetKnot(i+1))
			return 1.0;
		return 0.0;
	}
  
	if (t < GetKnot(i) || t > GetKnot(i+degree+1)) 
		return 0.0;
  
	if (GetKnot(i+degree) == GetKnot(i))
		temp1 = 0.0;
	else 
		temp1 = (t - GetKnot(i)) / (GetKnot(i+degree) - GetKnot(i)) * 
			CoxDeBoor(i, degree - 1, t);
  
	if (GetKnot(i+degree+1)== GetKnot(i+1)) 
		temp2 = 0.0;
	else 
	    temp2 = (GetKnot(i+degree+1) - t) / (GetKnot(i+degree+1) - GetKnot(i+1)) * 
			CoxDeBoor(i + 1, degree - 1, t);

	return temp1 + temp2;
}
