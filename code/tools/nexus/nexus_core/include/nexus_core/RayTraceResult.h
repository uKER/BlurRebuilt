#ifndef NEXUS_CORE_RAY_TRACE_RESULT_H
#define NEXUS_CORE_RAY_TRACE_RESULT_H

namespace Nexus
{

class RayTraceResult
{
public:
	/// indicates whether the ray hit something
	/** the other parameters in this class will only be valid if the ray hit something,
	    and if the ray was not a shafow-ray. **/
	__forceinline bool Hit() const
	{
		return m_hit;
	}

	/// indicates the index of the polygon that was hit
	__forceinline S32 ComponentIndex() const
	{
		return m_hitPolygonIdx;
	}

	__forceinline Aftk::C_SceneNode* HitNode() 
	{
		return m_pHitNode;
	}

	__forceinline const Aftk::C_SceneNode* HitNode() const;
	{
		return m_pHitNode;
	}

	__forceinline Maths::HVec3& HitVertexPosition(U32 idx)
	{
		return m_hitVerts[idx];
	}

	__forceinline const Maths::HVec3& HitVertexPosition(U32 idx) const
	{
		return m_hitVerts[idx];
	}

	__forceinline Maths::HVec3& HitVertexPositions()
	{
		return m_hitVerts[0];
	}

	__forceinline const Maths::HVec3& HitVertexPositions() const
	{
		return m_hitVerts[0];
	}

	/// returns the position of the hit point
	__forceinline Maths::HVec3& Position() 
	{
		return m_hitPosition; 
	}

	/// returns the normal of the surface at the hit point
	__forceinline const Maths::HVec3& SurfaceNormal()  const
	{ 
		return m_hitNormal; 
	}

	__forceinline const HMat33& TangentSpace() const
	{
		return m_hitTangentSpace;
	};

private:
	__forceinline const Aftk::C_SceneNode* HitNode() const;
	{
		return m_pHitNode;
	}

	__forceinline bool Hit() const
	{
		return m_hit;
	}

	/// indicates the index of the polygon that was hit
	__forceinline S32 ComponentIndex() const
	{
		return m_hitPolygonIdx;
	}

	__forceinline Aftk::C_SceneNode* HitNode();
	{
		return m_pHitNode;
	}

	__forceinline const Aftk::C_SceneNode* HitNode() const;
	{
		return m_pHitNode;
	}

	__forceinline Maths::HVec3& HitVertexPosition(U32 idx)
	{
		return m_hitVerts[idx];
	}

	__forceinline const Maths::HVec3& HitVertexPosition(U32 idx) const
	{
		return m_hitVerts[idx];
	}

	__forceinline Maths::HVec3& HitVertexPositions()
	{
		return m_hitVerts[0];
	}

	__forceinline const Maths::HVec3& HitVertexPositions() const
	{
		return m_hitVerts[0];
	}

	/// returns the position of the hit point
	__forceinline Maths::HVec3& Position() 
	{
		return m_hitPosition; 
	}

	/// returns the normal of the surface at the hit point
	__forceinline const Maths::HVec3& SurfaceNormal()  const
	{ 
		return m_hitNormal; 
	}

	__forceinline const HMat33& TangentSpace() const
	{
		return m_hitTangentSpace;
	};

	bool				m_hit;

	S32					m_hitPolygonIdx;
	/// gives the pointer to the meshnode containing the polygon that was hit
	Aftk::C_SceneNode*		m_pHitNode;

	/// contains the the vertex positions of the triangle that was hit
	Maths::HVec3		m_hitVerts[3];

	/// contains the distance to the hit point
	F32					m_hitDistance;
	/// contains the hit position in world space
	Maths::HVec3		m_hitPosition;
	/// contains the normal of the surface at the hit point
	Maths::HVec3		m_hitNormal;

	Maths::HMat33	m_hitTangentSpace;


#endif
};
}

#endif
