template<typename VECTYPE>
C_CurveBounds<VECTYPE>::C_CurveBounds()
{
}

template<typename VECTYPE>
C_CurveBounds<VECTYPE>::~C_CurveBounds()
{
}

template<typename VECTYPE>
void C_CurveBounds<VECTYPE>::Process(const I_Curve<VECTYPE>* pCurve, const C_Reparameterise* pReparam)
{
	if (pCurve->GetNumVertices() == 0)
		return;

	VECTYPE bbMax(-FLT_MAX);
	VECTYPE bbMin(FLT_MAX);

	if (pReparam)
	{
		for (U32 i = 0; i < pReparam->GetNumPoints(); ++i)
		{
			VECTYPE v = ReparamHelpers::GetPointValue(*pCurve, *pReparam, i);
			bbMin = Maths::Min(bbMin, v);
			bbMax = Maths::Max(bbMax, v);
		}
	}
	else
	{
		for (U32 i = 0; i < (U32)pCurve->GetNumVertices(); ++i)
		{
			VECTYPE v = pCurve->GetVertexValue(i);
			bbMin = Maths::Min(bbMin, v);
			bbMax = Maths::Max(bbMax, v);
		}
	}

	m_aabb = Maths::C_AxisBox(bbMin, bbMax);
}

template<typename VECTYPE>
const Maths::C_AxisBox& C_CurveBounds<VECTYPE>::GetAabb() const
{
	return m_aabb;
}
