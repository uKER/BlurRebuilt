
// Default c'tor -- uninitialised ray.
__forceinline C_Ray::C_Ray() :
	m_fDistance(0.0f),
	m_iSolidRegion((U32)~0)
{
	return;
}

// -----------------------------------------------------------------

// C'tor.
__forceinline C_Ray::C_Ray(const Maths::C_Vertex& rkOrigin, const Maths::HVec3& rkDirection, const Maths::HF32 fDistance) :
	m_Origin(rkOrigin),
	m_EndPoint(rkOrigin + (rkDirection * fDistance)),
	m_Direction(rkDirection),
	m_fDistance(fDistance),
	m_iSolidRegion(0)
{
	return;
}

// -----------------------------------------------------------------

// C'tor.
__forceinline C_Ray::C_Ray(const Maths::C_Vertex& rkOrigin, const Maths::C_Vertex& rkEndPoint) :
	m_Origin(rkOrigin),
	m_EndPoint(rkEndPoint),
	m_Direction(rkEndPoint - rkOrigin),
	m_fDistance(m_Direction.Length()),
	m_iSolidRegion(0)
{
	return;
}

// -----------------------------------------------------------------

// C'tor.
__forceinline C_Ray::C_Ray(const Maths::C_Vertex& rkOrigin, const Maths::HVec3& rkWorldVector) :
	m_Origin(rkOrigin),
	m_EndPoint(rkOrigin + Maths::C_Vertex(rkWorldVector)),
	m_Direction(rkWorldVector),
	m_fDistance(m_Direction.Length()),
	m_iSolidRegion(0)
{
	m_Direction.SafeNormalise();
	return;
}

// -----------------------------------------------------------------

// D'tor.
__forceinline C_Ray::~C_Ray()
{
	return;
}

// -----------------------------------------------------------------

// Return the world-space direction vector for the ray.
NOALIAS const Maths::HVec3& C_Ray::GetDirection() const __restrict
{
	return m_Direction;
}

// -----------------------------------------------------------------

// Return the world-space end point of the ray.
NOALIAS const Maths::C_Vertex& C_Ray::GetEndPoint() const __restrict
{
	return m_EndPoint;
}

// -----------------------------------------------------------------

// Return the maximum distance to consider the ray over.
NOALIAS const Maths::HF32 C_Ray::GetLength() const __restrict
{
	return m_fDistance;
}

// -----------------------------------------------------------------

// Return the origin of the ray.
NOALIAS const Maths::C_Vertex& C_Ray::GetOrigin() const __restrict
{
	return m_Origin;
}

// -----------------------------------------------------------------

// Return the solid region of the ray.
NOALIAS const U32 C_Ray::GetSolidRegion() const
{
	return m_iSolidRegion;
}
