template<typename VECTYPE, typename STORAGE>
C_BSpline<VECTYPE, STORAGE>::C_BSpline()
: C_ApproximatingCurveBase<VECTYPE, STORAGE>()
{
}

template<typename VECTYPE, typename STORAGE>
C_BSpline<VECTYPE, STORAGE>::~C_BSpline()
{
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_BSpline<VECTYPE, STORAGE>::Evaluate(F32 u) const
{
	//Spline interpolation begins once we have a cv count equal to the degree of the curve.
	//We have a valid curve of zero length with 3 cvs in this case (we're a cubic curve). Once
	//a fourth cv is added we actually gain a length to interpolate over.

	//range over which spline is interpolated.
	S32 nCvs = Base::m_pData->GetNumKnots();
	const S32 degree(3);
	S32 nSpans = nCvs - degree;
	if(nSpans < 0)
		return VECTYPE();

	F32 spanU = u * nSpans;
	S32 span = (S32)spanU;
	//Edge case, consider last point to be at the end of the last span, not the beginning of the
	//next (non existent) span.
	if (u >= 1.f)
		--span;

	S32 i0 = span+1;
	S32 end = i0 + 2;

	if(end >= nCvs)
	{
		//Invalid curve.
		return VECTYPE();	
	}

	const typename STORAGE::KnotType& p_1 = Base::m_pData->AcquireKnot(i0-1);
	const typename STORAGE::KnotType& p0 = Base::m_pData->AcquireKnot(i0);
	const typename STORAGE::KnotType& p1 = Base::m_pData->AcquireKnot(i0+1);
	const typename STORAGE::KnotType& p2 = Base::m_pData->AcquireKnot(i0+2);

	//Calculate parameter for curve segment, let's call it t.
	F32 t = spanU - (F32)span;
	F32 t2 = t * t;
	F32 t3 = t2 * t;
	//Create Blend parameter vector.
	Maths::HVec4 tvec(t3, t2, t, 1);

	//Create geometry matrix.
	Maths::HMat44 g;
	Helpers::BuildGeometryMatrix(
		g, 
		p_1.value, 
		p0.value,
		p1.value,
		p2.value);

	//x = [t^3 t^2 t 1] .  M(basis) . [p_1.value]
	//                                [p0.value]
	//                                [p1.value]
	//                                [p2.value] 
	
	Base::m_pData->ReleaseKnot();
	Base::m_pData->ReleaseKnot();
	Base::m_pData->ReleaseKnot();
	Base::m_pData->ReleaseKnot();
	return Helpers::Load<VECTYPE>(tvec * kBSplineBasis * g);
}

template<typename VECTYPE, typename STORAGE>
F32 C_BSpline<VECTYPE, STORAGE>::GetMinU() const
{
	return 0;
}
template<typename VECTYPE, typename STORAGE>
F32 C_BSpline<VECTYPE, STORAGE>::GetMaxU() const
{
	return 1.f;
}

template<typename VECTYPE, typename STORAGE>
const VECTYPE& C_BSpline<VECTYPE, STORAGE>::AcquireVertexValue(U32 index) const
{
	return Base::m_pData->AcquireKnot(index).value;
}

template<typename VECTYPE, typename STORAGE>
const void* C_BSpline<VECTYPE, STORAGE>::AcquireVertexData(U32 index) const
{
	return &Base::m_pData->AcquireKnot(index);
}

template<typename VECTYPE, typename STORAGE>
void C_BSpline<VECTYPE, STORAGE>::ReleaseVertex() const
{
	Base::m_pData->ReleaseKnot();
}

template<typename VECTYPE, typename STORAGE>
VECTYPE C_BSpline<VECTYPE, STORAGE>::GetVertexValue(U32 index) const
{
	VECTYPE tmp = Base::m_pData->AcquireKnot(index).value;
	Base::m_pData->ReleaseKnot();
	return tmp;
}

template<typename VECTYPE, typename STORAGE>
S32 C_BSpline<VECTYPE, STORAGE>::GetNumVertices() const
{
	return Base::m_pData->GetNumKnots();
}

template<typename VECTYPE, typename STORAGE>
void C_BSpline<VECTYPE, STORAGE>::Process()
{
}
