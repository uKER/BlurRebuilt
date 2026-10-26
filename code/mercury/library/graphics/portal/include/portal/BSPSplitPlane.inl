
// C'tor.
__forceinline C_BSPSplitPlane::C_BSPSplitPlane()
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
__forceinline C_BSPSplitPlane::~C_BSPSplitPlane()
{
	return;
}

// -----------------------------------------------------------------

// Deserialise a BSP split plane object from the stream.
template <typename _T>
void C_BSPSplitPlane::Deserialise(_T& rkStream)
{
	rkStream >> m_Plane;
	rkStream >> m_Front;
	rkStream >> m_Back;
	return;
}

// -----------------------------------------------------------------

// Return the back region.
__forceinline NOALIAS const C_BSPRegion& C_BSPSplitPlane::GetBack() const __restrict
{
	return m_Back;
}

// -----------------------------------------------------------------

// Return the front region.
__forceinline NOALIAS	const C_BSPRegion& C_BSPSplitPlane::GetFront() const __restrict
{
	return m_Front;
}

// -----------------------------------------------------------------

// Return the plane at the split plane.
__forceinline NOALIAS const Maths::C_Plane& C_BSPSplitPlane::GetPlane() const __restrict
{
	return m_Plane;
}

