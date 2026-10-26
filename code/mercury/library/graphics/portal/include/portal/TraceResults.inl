
// Default c'tor.
C_TraceResults::C_TraceResults() :
	m_WorldNormal(),
	m_Origin(),
	m_EndPoint(),
	m_WorldPos(),
	m_fDistance(0.0f),
	m_iFinalRoom((U32)~0),
	m_bHit(false)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_TraceResults::~C_TraceResults()
{
	return;
}

// -----------------------------------------------------------------

// Return the distance along the ray at which the intersection occured.
NOALIAS const Maths::HF32 C_TraceResults::GetDistance() const __restrict
{
	return m_fDistance;
}

// -----------------------------------------------------------------

// Return the end point of the ray.
NOALIAS const Maths::C_Vertex&	C_TraceResults::GetEndPoint() const __restrict
{
	return m_EndPoint;
}

// -----------------------------------------------------------------

// Return the room index of the end point of the ray.
NOALIAS const U32 C_TraceResults::GetFinalRoomIndex() const __restrict
{
	return m_iFinalRoom;
}

// -----------------------------------------------------------------

//! Return the world-space origin of the ray.
NOALIAS const Maths::C_Vertex& C_TraceResults::GetOrigin() const __restrict
{
	return m_Origin;
}

// -----------------------------------------------------------------

// Return the world normal at the point of intersection.
NOALIAS const Maths::C_Normal& C_TraceResults::GetWorldNormal() const __restrict
{
	return m_WorldNormal;
}

// -----------------------------------------------------------------

// Return the world position at the point of intersection.
NOALIAS const Maths::C_Vertex& C_TraceResults::GetWorldPosition() const __restrict
{
	return m_WorldPos;
}

// -----------------------------------------------------------------

// Return true if the ray has hit something.
NOALIAS const bool C_TraceResults::HasHit() const __restrict 
{
	return m_bHit;
}

// -----------------------------------------------------------------

// Set the distance along the ray at which an intersection occurred.
void C_TraceResults::SetDistance(const Maths::HF32 fDistance)
{
	m_fDistance = fDistance;
	return;
}

// -----------------------------------------------------------------

// Set the world-space end point of the ray.
void C_TraceResults::SetEndPoint(const Maths::C_Vertex& rkEndPoint)
{
	m_EndPoint = rkEndPoint;
	return;
}

// -----------------------------------------------------------------

// Set the room index at the end point of the ray.
void C_TraceResults::SetFinalRoomIndex(const U32 iRoomIndex)
{
	m_iFinalRoom = iRoomIndex;
	return;
}

// -----------------------------------------------------------------

// Set the hit flag.
void C_TraceResults::SetHasHit(const bool bHasHit)
{
	m_bHit = bHasHit;
	return;
}

// -----------------------------------------------------------------

// Set the world-space origin of the ray.
void C_TraceResults::SetOrigin(const Maths::C_Vertex& rkOrigin)
{
	m_Origin = rkOrigin;
	return;
}

// -----------------------------------------------------------------

// Set the world normal at the point of intersection.
void C_TraceResults::SetWorldNormal(const Maths::C_Normal &rkNormal)
{
	m_WorldNormal = rkNormal;
	return;
}

// -----------------------------------------------------------------

// Set the world position at the point of intersection.
void C_TraceResults::SetWorldPosition(const Maths::C_Vertex &rkVertex)
{
	m_WorldPos = rkVertex;
	return;
}

// -----------------------------------------------------------------

