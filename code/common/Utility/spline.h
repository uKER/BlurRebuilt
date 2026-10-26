
namespace UtMaths
{

// This class doesn't hold its own data, as it is designed to evaluate splines
// that already exist but lack these functions.
template <typename T>
class BSpline
{
public:
	typedef T PointType;

	BSpline() : m_pCvs(0), m_pKnots(0), m_n(0) {}

	// n = number of spline sections = nCvs - 1
	// (n + 1) control points 0 .. n
	// (n + 2) knots -1 .. n + 1
	BSpline(const PointType* pCvs, const float* pKnots, int nCvs) :
		m_pCvs(pCvs),
		m_pKnots(pKnots),
		m_n(nCvs - 1)
	{}

	void Initialise(const PointType* pCvs, const float* pKnots, int nCvs)
	{
		m_pCvs = pCvs;
		m_pKnots = pKnots;
		m_n = nCvs - 1;
	}

	PointType Position(float u) const
	{
		// If outside the range, return endpoint. Not strictly correct but better than returning nonsense.
		if (u < GetMinKnotValue()) return P(0);
		else if (u >= GetMaxKnotValue()) return P(m_n);

		int i = GetSection(u);
		return Q(i, u);
	}

	MNormal<typename PointType::VectorType> Tangent(float u)
	{
		if (u < GetMinKnotValue()) return Normalised(dP(0));
		else if (u >= GetMaxKnotValue()) return Normalised(dP(m_n - 1));

		int i = GetSection(u);
		return Normalised(dQ(i, u));
	}

	float GetMinKnotValue() const {return t(-1);}
	float GetMaxKnotValue() const {return t(m_n + 1);}
	
private:
	const PointType* m_pCvs; 
	const float* m_pKnots; 
	int m_n;

	int GetSection(float u) const
	{
		// to do: If n is large, faster to search by interval bisection.
		// Or keep last section used and start with that.
		for (int i = 1; i < m_n - 1; i++)
		{
			if (u >= t(i) && u < t(i + 1))
				return i;
		}

		assert(0);
		return -1;
	}

	// get control point i
	const PointType& P(int i) const 
	{
		assert(i >= 0 && i <= m_n); 
		return m_pCvs[i];
	}

	// control point for tangents
	MVector<typename PointType::VectorType> dP(int i) const 
	{
		assert(i >= 0 && i < m_n); 
		return (m_pCvs[i + 1] - m_pCvs[i]) / (t(i + 2) - t(i - 2));
	}

	float t(int i) const
	{
		// repeat the first and last knots
		if (i < -1) i = -1;
		if (i > m_n + 1) i = m_n + 1;
		return m_pKnots[i + 1];
	}

	PointType Q(int i, float u) const
	{
		MVector<PointType::VectorType> total(0.f, 0.f, 0.f);

		for (int k = 0; k <= 3; k++)
			total += N(i + k - 1, 3, u) * MakeVector(P(i + k - 1));

		return MakePoint(total);
	}

	MVector<typename PointType::VectorType> dQ(int i, float u) const
	{
		MVector<PointType::VectorType> total(0.f, 0.f, 0.f);

		for (int k = 0; k <= 2; k++)
			total += 3.f * N(i + k, 2, u) * MakeVector(dP(i + k - 1));

		return total;
	}

	float N(int i, int k, float u) const
	{
		if (k == 0)
		{
			if (u >= t(i - 2) && u < t(i - 1))
				return 1.f;
			else
				return 0.f;
		}
		else
		{
			// to do: this will calculate the same function several times
			// in the course of the recursion.
			float f1 = (u - t(i - 2)) * N(i, k - 1, u);
			float f2 = (t(i + k - 2) - t(i - 2));
			float f3 = (t(i + k - 1) - u) * N(i + 1, k - 1, u);
			float f4 = (t(i + k - 1) - t(i - 1));
			float f = 0.f;

			if (f1 > 0.f) f += f1 / f2;
			if (f3 > 0.f) f += f3 / f4;

			return f;
		}
	}
};

template <typename T>
class CatmullRomSpline
{
public:
	typedef T PointType;

	CatmullRomSpline() : m_pCvs(0), m_n(0) {}

	CatmullRomSpline(const PointType* pCvs, int nCvs) :
		m_pCvs(pCvs),
		m_n(nCvs - 1)
	{}

	void Initialise(const PointType* pCvs, int nCvs)
	{
		m_pCvs = pCvs;
		m_n = nCvs - 1;
	}

	PointType Position(float u) const
	{
		int i = (int) floor(u);
		return Q(i, fmod(u, 1.f));
	}

	float GetMinKnotValue() const {return 0.f;}
	float GetMaxKnotValue() const {return float(m_n);}

private:
	const PointType* m_pCvs; 
	int m_n;

	// get control point i
	const PointType& P(int i) const 
	{
		// assume that the first and last points are repeated
		if (i < 0) i = 0;
		if (i > m_n) i = m_n;

		return m_pCvs[i];
	}

	PointType Q(int i, float u) const
	{
		MVector4F x(1.0, u, u * u, u * u * u);
		MMatrix44 p;
		for (int j = 0; j < 4; j++)
			p[j] = MakeHomogeneous(P(i - 1 + j));

		MMatrix44 cr;
		Set(&cr[0], 0.f, 2.f, 0.f, 0.f); 
		Set(&cr[1], -1.f, 0.f, 1.f, 0.f); 
		Set(&cr[2], 2.f, -5.f, 4.f, -1.f); 
		Set(&cr[3], -1.f, 3.f, -3.f, 1.f);

		return MakePoint(x * cr * p * 0.5f);
	}
};

///////////////////////////////////////////////////////////////////////

// Calculate the length of a spline and evaluate position as a function of distance along the spline.
// This is done by creating a table and interpolating linearly between subdivisions.
template <typename SplineType>
class SplineLengthConverter
{
public:

	SplineLengthConverter() : m_pSpline(0) {}

	// pass in spline and the number of points to evaluate
	SplineLengthConverter(const SplineType* spline, int nSubDivisions) {Initialise(spline, nSubDivisions);}

	void Initialise(const SplineType* spline, int nSubDivisions);

	// return length of spline up to the position given
	float GetLength(float u) const;

	// pass in length, get spline parameter
	float ConvertLength(float length) const;

	// get the total length of the spline
	float TotalLength() const {return m_length;}

private:
	void AddPoint(float u);

	typename SplineType::PointType m_position;
	Ut::UtVector<Ut::DynamicVector<Ut::Pair<float, float> > > m_lengthTable;
	const SplineType* m_pSpline;
	float m_length;
};

template <typename SplineType>
void SplineLengthConverter<SplineType>::Initialise(const SplineType* spline, int nSubDivisions)
{
	m_pSpline = spline;

	float uMin = m_pSpline->GetMinKnotValue();
	float uMax = m_pSpline->GetMaxKnotValue();
	float dU = (uMax - uMin) / float(nSubDivisions);
	m_length = 0;
	m_position = m_pSpline->Position(uMin);

	m_lengthTable.Clear();
	m_lengthTable.PushBack(Ut::MakePair(uMin, m_length));

	for (float u = uMin + dU; u < uMax; u += dU)
		AddPoint(u);

	AddPoint(uMax);
}

template <typename SplineType>
void SplineLengthConverter<SplineType>::AddPoint(float u)
{
	SplineType::PointType newPosition = m_pSpline->Position(u);
	m_length += Magnitude(newPosition - m_position);
	m_position = newPosition;

	m_lengthTable.PushBack(Ut::MakePair(u, m_length));
}

template <typename SplineType>
float SplineLengthConverter<SplineType>::ConvertLength(float length) const
{
	for (int i = 1; i < m_lengthTable.Size(); i++)
	{
		if (length < m_lengthTable[i].second)
		{
			float d = (m_lengthTable[i].second - m_lengthTable[i - 1].second);
			if (d <= 0.f) return m_lengthTable[i].first;
			float f = (length - m_lengthTable[i - 1].second) / d;
			return m_lengthTable[i - 1].first * (1.f - f) + m_lengthTable[i].first * f;
		}
	}

	return m_lengthTable.Back().first;
}

template <typename SplineType>
float SplineLengthConverter<SplineType>::GetLength(float u) const
{
	for (int i = 1; i < m_lengthTable.Size(); i++)
	{
		if (u < m_lengthTable[i].first)
		{
			float d = (m_lengthTable[i].first - m_lengthTable[i - 1].first);
			if (d <= 0.f) return m_lengthTable[i].second;
			float f = (length - m_lengthTable[i - 1].first) / d;
			return m_lengthTable[i - 1].second * (1.f - f) + m_lengthTable[i].second * f;
		}
	}

	return m_lengthTable.Back().second;
}

}