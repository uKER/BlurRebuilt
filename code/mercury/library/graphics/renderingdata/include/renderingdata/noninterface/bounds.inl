// Bounds
inline void Bounds::SetUniversal()
{
	aabbMin = Maths::Constants<Point3>::NegativeMax();
	aabbMax = Maths::Constants<Point3>::Max();
	distanceMin = -FLT_MAX;
	distanceMax = FLT_MAX;
}

inline void Bounds::SetAABBEmpty()
{
	aabbMin = Maths::Constants<Point3>::Max();
	aabbMax = Maths::Constants<Point3>::NegativeMax();
}

inline void Bounds::SetEmpty()
{
	aabbMin = Maths::Constants<Point3>::Max();
	aabbMax = Maths::Constants<Point3>::NegativeMax();
	distanceMin = FLT_MAX;
	distanceMax = -FLT_MAX;
}

inline void Bounds::SetUnion(const Point3& p)
{
	SetMin(&aabbMin, aabbMin, p);
	SetMax(&aabbMax, aabbMax, p);
}

inline void Bounds::SetAABBTriangle(const Point3& p0, const Point3& p1, const Point3& p2)
{
	SetAABBEmpty();
	SetUnion(p0);
	SetUnion(p1);
	SetUnion(p2);
}

inline void Bounds::SetUnion(const Bounds& b)
{
	SetMin(&aabbMin, aabbMin, b.aabbMin);
	SetMax(&aabbMax, aabbMax, b.aabbMax);
	distanceMin = Maths::Min(distanceMin, b.distanceMin);
	distanceMax = Maths::Max(distanceMax, b.distanceMax);
}

inline void Bounds::SetIntersection(const Bounds& b)
{
	SetMax(&aabbMin, aabbMin, b.aabbMin);
	SetMin(&aabbMax, aabbMax, b.aabbMax);
	distanceMin = Maths::Max(distanceMin, b.distanceMin);
	distanceMax = Maths::Min(distanceMax, b.distanceMax);
}

inline void Bounds::Scale(const F32 s)
{
	Point3 centre = GetCentre();

	aabbMin = ((aabbMin - centre) * s) + centre;
	aabbMax = ((aabbMax - centre) * s) + centre;
}

inline Vector3 Bounds::GetSize() const
{
	return aabbMax - aabbMin;
}

inline Point3 Bounds::GetCentre() const
{
	return Lerp(aabbMax, aabbMin, 0.5f);
}

inline bool operator == (const Bounds& b1, const Bounds& b2)
{
	if (b1.aabbMin != b2.aabbMin) return false;
	if (b1.aabbMax != b2.aabbMax) return false;
	if (b1.distanceMin != b2.distanceMin) return false;
	if (b1.distanceMax != b2.distanceMax) return false;
	return true;
}


