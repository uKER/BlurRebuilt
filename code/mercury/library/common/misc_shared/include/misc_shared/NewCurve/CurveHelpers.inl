template<typename VECTYPE>
INLINE VECTYPE ControlPointToOutTangent(VECTYPE point, VECTYPE control, F32 tangentScale)
{
	VECTYPE tangent = 3*(control - point);
	if (tangentScale > 0)
		tangent = tangent / tangentScale;
	return tangent;
}

template<typename VECTYPE>
INLINE VECTYPE ControlPointToInTangent(VECTYPE point, VECTYPE control, F32 tangentScale)
{
	VECTYPE tangent = 3*(point - control);
	if (tangentScale > 0)
		tangent = tangent / tangentScale;
	return tangent;
}

template<typename VECTYPE>
INLINE VECTYPE OutTangentToControlPoint(VECTYPE point, VECTYPE tangent, F32 tangentScale)
{
	return point + tangent/3.f*tangentScale;
}

template<typename VECTYPE>
INLINE VECTYPE InTangentToControlPoint(VECTYPE point, VECTYPE tangent, F32 tangentScale)
{
	return point - tangent/3.f*tangentScale;
}

template<typename VECTYPE>
INLINE VECTYPE OutTangentToControlPoint1d(VECTYPE point, VECTYPE tangent, F32 tangentScale, F32 periodScale)
{
	return point + (tangent/(3.f/periodScale));
}

template<typename VECTYPE>
INLINE VECTYPE InTangentToControlPoint1d(VECTYPE point, VECTYPE tangent, F32 tangentScale, F32 periodScale)
{
	return point - (tangent/(3.f/periodScale));
}

INLINE Maths::HVec3 OutTangentToControlPoint1dTo3d(Maths::HVec3 point, F32 tanAngle, F32 tangentScale, F32 periodScale)
{
	F32 x = periodScale/3.f;
	F32 y = tanAngle/(3.f/periodScale);

	return point + Maths::HVec3(x, 0, y);
}

INLINE Maths::HVec3 InTangentToControlPoint1dTo3d(Maths::HVec3 point, F32 tanAngle, F32 tangentScale, F32 periodScale)
{
	F32 x = periodScale/3.f;
	F32 y = tanAngle/(3.f/periodScale);

	return point - Maths::HVec3(x, 0, y);
}

INLINE Maths::HVec2 OutTangentToControlPoint1dTo2d(Maths::HVec2 point, F32 tanAngle, F32 tangentScale, F32 periodScale)
{
	F32 x = periodScale/3.f;
	F32 y = tanAngle/(3.f/periodScale);

	return point + Maths::HVec2(x, y);
}

INLINE Maths::HVec2 InTangentToControlPoint1dTo2d(Maths::HVec2 point, F32 tanAngle, F32 tangentScale, F32 periodScale)
{
	F32 x = periodScale/3.f;
	F32 y = tanAngle/(3.f/periodScale);

	return point - Maths::HVec2(x, y);
}

INLINE void BuildGeometryMatrix(
	Maths::HMat44& m,
	const Maths::HVec3& p0,
	const Maths::HVec3& p1,
	const Maths::HVec3& p2,
	const Maths::HVec3& p3)
{
	m.AX(Maths::HVec4(p0));
	m.AY(Maths::HVec4(p1));
	m.AZ(Maths::HVec4(p2));
	m.AW(Maths::HVec4(p3));
}

INLINE void BuildGeometryMatrix(
	Maths::HMat44& m,
	const Maths::HVec2& p0,
	const Maths::HVec2& p1,
	const Maths::HVec2& p2,
	const Maths::HVec2& p3)
{
	m.AX(Maths::HVec4(p0));
	m.AY(Maths::HVec4(p1));
	m.AZ(Maths::HVec4(p2));
	m.AW(Maths::HVec4(p3));
}

INLINE void BuildGeometryMatrix(
	Maths::HMat44& m,
	const F32 p0,
	const F32 p1,
	const F32 p2,
	const F32 p3)
{
	m.AX(Maths::HVec4(p0, 0, 0, 0));
	m.AY(Maths::HVec4(p1, 0, 0, 0));
	m.AZ(Maths::HVec4(p2, 0, 0, 0));
	m.AW(Maths::HVec4(p3, 0, 0, 0));
}

INLINE void BuildGeometryMatrix(
	Maths::HMat44& m,
	const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& p0,
	const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& p1,
	const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& p2,
	const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& p3)
{
	m.AX(Maths::HVec4(p0.X(), p0.Y(), p0.Z(), 0.f));
	m.AY(Maths::HVec4(p1.X(), p1.Y(), p1.Z(), 0.f));
	m.AZ(Maths::HVec4(p2.X(), p2.Y(), p2.Z(), 0.f));
	m.AW(Maths::HVec4(p3.X(), p3.Y(), p3.Z(), 0.f));
}

INLINE F32 Magnitude(F32 v)
{
	return v;
}

INLINE F32 Magnitude(const Maths::HVec2& v)
{
	return v.Length();
}

INLINE F32 Magnitude(const Maths::HVec3& v)
{
	return v.Length();
}

INLINE F32 Magnitude(const UtMaths::MVector<UtMaths::VectorXYZ<F32> >& v)
{
	return UtMaths::Magnitude(v);
}

INLINE void Zero(F32& v)
{
	v = 0;
}

INLINE void Zero(Maths::HVec2& v)
{
	v = Maths::HVec2(0,0);
}
INLINE void Zero(Maths::HVec3& v)
{
	v = Maths::HVec3(0,0,0);
}

INLINE void Zero(UtMaths::MVector<UtMaths::VectorXYZ<F32> >& v)
{
	memset(&v, 0, sizeof(v));
}

template <>
INLINE F32 Load<F32>(const Maths::HVec4 PCREF v)
{
	return v.X();
}

template <>
INLINE Maths::HVec2 Load<Maths::HVec2>(const Maths::HVec4 PCREF v)
{
	return (Maths::HVec2)v;
}
 
template <>
INLINE Maths::HVec3 Load<Maths::HVec3>(const Maths::HVec4 PCREF v)
{
	return (Maths::HVec3)v;
}

template <>
INLINE UtMaths::MVector<UtMaths::VectorXYZ<F32> > Load<UtMaths::MVector<UtMaths::VectorXYZ<F32> > >(const Maths::HVec4 PCREF v)
{
	return UtMaths::MVector<UtMaths::VectorXYZ<F32> >(v.X(), v.Y(), v.Z());
}

template<typename STORAGE>
S32 GetKnotInterval(const STORAGE* pData, F32 u)
{
	U32 nKnots = pData->GetNumKnots();
	if (nKnots == 0)
		return -1;

	S32 low = 0;
	S32 high = nKnots - 1;
	
	while(high > (low + 1))
	{
		S32 mid = (low + high) >> 1;
		F32 midU = pData->AcquireKnot(mid).GetU(mid, nKnots);
		pData->ReleaseKnot();
		bool bComp(u < midU);
		high = Maths::Select(bComp, mid, high);
		low = Maths::Select(bComp, low, mid);
		/*if(u < midU)
			high = mid;
		else
			low = mid;*/
	}
	
	return low;
}

template<typename VECTYPE, typename STORAGE>
VECTYPE KnotLerp(const STORAGE* pData, F32 u)
{
	//Get adjacent knots.
	S32 nKnots = pData->GetNumKnots();
	S32 ka = Helpers::GetKnotInterval(pData, u);
	BIZ_ASSERT(ka < nKnots);

	if(ka < 0)
		return VECTYPE();

	S32 kb = ka + 1;

	if(kb >= nKnots)
	{
		VECTYPE r = pData->AcquireKnot(ka).value;
		pData->ReleaseKnot();
		return r;
	}

	const typename STORAGE::KnotType& knot0 = pData->AcquireKnot(ka);
	const typename STORAGE::KnotType& knot1 = pData->AcquireKnot(kb);
	F32 knot0u = knot0.GetU(ka, nKnots);
	F32 knot1u = knot1.GetU(kb, nKnots);
	F32 period = knot1u - knot0u;
	F32 t = (u - knot0u) / period;
	if (t > 1.f)
	{
		pData->ReleaseKnot();
		pData->ReleaseKnot();
		return knot1.value;
	}
	else if (t <= FLT_EPSILON)
	{
		pData->ReleaseKnot();
		pData->ReleaseKnot();
		return knot0.value;
	}

	VECTYPE delta = knot1.value - knot0.value;
	VECTYPE result = knot0.value + delta * t;
	pData->ReleaseKnot();
	pData->ReleaseKnot();
	return result;
}

template<typename CURVE>
F32 CalcDuration(const CURVE* pCurve)
{
	BIZ_ASSERT(pCurve);
	return pCurve->GetMaxU() - pCurve->GetMinU();
}

INLINE F32 GetLinearSegmentLength(const Maths::HVec2& prev, const Maths::HVec2& next)
{
	return Maths::Length(next - prev);
}

INLINE F32 GetLinearSegmentLength(const Maths::HVec3& prev, const Maths::HVec3& next)
{
	return Maths::Length(next - prev);
}

INLINE F32 GetLinearSegmentLength(const F32& prev, const F32& next)
{
	return fabsf(next - prev);
}

template<typename CURVE>
F32 CalcArcLengthLinear(const CURVE* pCurve)
{
	S32 nVertices = pCurve->GetNumVertices();
	if (nVertices == 0)
		return 0.f;

	F32 length = 0;
	typename CURVE::VecType prev = pCurve->GetVertexValue(0);
	for(S32 i = 1; i < nVertices; ++i)
	{
		typename CURVE::VecType next = pCurve->GetVertexValue(i);
		length += GetLinearSegmentLength(prev, next);
		prev = next;
	}
	
	return length;
}

template<typename CURVE>
F32 FindMinSegmentLength(const CURVE* pCurve)
{
	S32 nVertices = pCurve->GetNumVertices();
	if (nVertices == 0)
		return 0.f;

	F32 minLength = CalcArcLengthLinear(pCurve);

	typename CURVE::VecType prev = pCurve->GetVertexValue(0);
	for(S32 i = 1; i < nVertices; ++i)
	{
		typename CURVE::VecType next = pCurve->GetVertexValue(i);

		F32 segmentLength = GetLinearSegmentLength(prev, next);

		if (segmentLength > 0.f)
			minLength = Maths::Min(minLength, segmentLength);

		prev = next;
	}
	
	return minLength;
}

template<typename STORAGE>
void SetRegularU(STORAGE* pData)
{
	U32 nKnots = pData->GetNumKnots();
	for (U32 i = 0; i < nKnots; ++i)
	{
		typename STORAGE::KnotType& knot = pData->AcquireKnot(i);
		knot.SetU((F32)i/((F32)nKnots-1));
		pData->CommitKnot();
	}
}

template<typename STORAGE>
void SetArcLengthLinearU(STORAGE* pData, F32 arcLength)
{
	typename STORAGE::KnotType& prev = pData->AcquireKnot(0);
	typename STORAGE::KnotType::VecType prevValue = prev.value;
	prev.SetU(0.0f);
	pData->CommitKnot();

	U32 nKnots = pData->GetNumKnots();
	F32 currentLength = 0;
	for (U32 i = 1; i < nKnots; ++i)
	{
		typename STORAGE::KnotType& knot = pData->AcquireKnot(i);
		currentLength += GetLinearSegmentLength(prevValue, knot.value);
		prevValue = knot.value;
		knot.SetU(currentLength / arcLength);
		pData->CommitKnot();
	}
}
