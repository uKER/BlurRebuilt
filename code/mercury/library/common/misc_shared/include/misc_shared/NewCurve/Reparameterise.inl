
inline C_Reparameterise::C_Reparameterise()
{
}

inline C_Reparameterise::~C_Reparameterise()
{
}

inline F32 C_Reparameterise::GetArcLengthByIndex(U32 index) const
{
	return m_arcLengthByUInterval[index];
}

inline U32 C_Reparameterise::GetNumPoints() const
{
	return (U32)m_arcLengthByUInterval.Size();
}

inline C_ReparamPoint C_Reparameterise::GetPoint(U32 index) const
{
	BIZ_ASSERT(index < (U32)m_arcLengthByUInterval.Size());
	return C_ReparamPoint(GetUByIndex(index), m_arcLengthByUInterval[index]);
}

inline F32 C_Reparameterise::GetUByIndex(U32 index) const
{
	return index * m_uScale;
}

template<typename VECTYPE>
void C_Reparameterise::BuildTable(const I_Curve<VECTYPE>* pCurve, U32 nSubDivisions)
{
	m_arcLengthByUInterval.Clear();
	m_uByLengthInterval.Clear();

	if (nSubDivisions == 0)
		return;

	m_uMax = pCurve->GetMaxU();
	m_uMin = pCurve->GetMinU();

	m_arcLengthByUInterval.Resize(nSubDivisions);
	m_uByLengthInterval.Resize(nSubDivisions);

	F32 dU = m_uMax - m_uMin;
	m_uScale = 1.f / ((F32)(m_arcLengthByUInterval.Size()) - 1.f) * dU;

	m_nSubDivisions = (float)nSubDivisions;

	{
		F32 dU = (m_uMax - m_uMin) / F32(nSubDivisions);
		F32 arcLength = 0;

		//First point on curve.
		VECTYPE value = pCurve->Evaluate(m_uMin);
		//Add table entry for first point.
		m_arcLengthByUInterval[0] = 0;
		U32 i = 1;
		for (F32 u = m_uMin + dU; i < nSubDivisions-1; u += dU, ++i)
		{
			VECTYPE newValue = pCurve->Evaluate(u);
			arcLength += Helpers::Magnitude(newValue - value);
			value = newValue;

			m_arcLengthByUInterval[i] = arcLength;
		}
		//Add final point manually to avoid floating point error.
		{
			VECTYPE newValue = pCurve->Evaluate(m_uMax);
			arcLength += Helpers::Magnitude(newValue - value);
			value = newValue;

			m_arcLengthByUInterval[i] = arcLength;
		}
	}

	m_arcLength = GetArcLength(m_uMax);

	{
		S32 n = m_uByLengthInterval.Size();

		for (int i = 0; i < n - 1; i++)
		{
			float arcLength = (m_arcLength * i) / (n - 1);
			m_uByLengthInterval[i] = FindUInterval(arcLength, 0, n);
		}

		m_uByLengthInterval[n - 1] = m_arcLengthByUInterval.Size() - 1;
	}
}


