#ifndef NEWRAY_H
#define NEWRAY_H

#include "gravity/sharedphysics/GraphNode.h"
#include "gravity/narrowphase/CollisionFilter.h"

namespace SharedPhysics
{
	class C_GraphNode;
};

namespace Gravity
{
	class C_Body;
}

namespace Collision
{

class C_Shape;

class C_RayTracingResult
{
public:

	C_RayTracingResult() : m_contact(Maths::UNDEFINED), m_normal(Maths::UNDEFINED), m_distance(Maths::UNDEFINED),
		m_pShape(NULL), m_pNode(NULL), m_TriangleIndex(-1), m_UserData(0), m_PhysicsMaterialIndex(0), m_EffectMaterialIndex(0), m_bNewContact(false)
	{
	}

	void ClearContacts()
	{
		m_bNewContact = false;
	}

	bool ValidContact() const
	{
		return m_bNewContact;
	}

	bool Hit() const
	{
		return m_pShape != NULL;
	}

	const Maths::HVec3 HitPoint() const
	{
		return m_contact;
	}

	const Maths::C_Normal Normal() const
	{
		return m_normal;
	}

	const Maths::C_Normal TriangleNormal() const
	{
		return m_TriangleNormal;
	}

	const Maths::HF32 Distance() const
	{
		return m_distance;
	}

	const Collision::C_Shape* GetShape() const
	{
		return m_pShape;
	}

	const SharedPhysics::C_GraphNode* GetGraphNode() const
	{
		return m_pNode;
	}
	
	const Gravity::C_Body* GetBody() const
	{
		return (m_pNode != NULL) ? m_pNode->m_pBody : NULL;
	}

	const U32 GetHitTriangleIndex() const
	{
		return m_TriangleIndex;
	}

	const U16 GetPhysicsMaterial() const
	{
		return m_PhysicsMaterialIndex;
	}

	const U16 GetEffectMaterial() const
	{
		return m_EffectMaterialIndex;
	}

	const U32 GetUserData() const
	{
		return m_UserData;
	}

	//...More to come ?


	Maths::HVec3 m_contact;
	Maths::C_Normal m_normal;			// Normal of ray 'sphere intersect' for vehicle system.
	Maths::C_Normal m_TriangleNormal;	// Normal of hit triangle.
	Maths::HF32 m_distance;
	C_Shape* m_pShape;
	SharedPhysics::C_GraphNode* m_pNode;
	//Used if we hit a triangle, otherwise -1 for non-triangle hits!
	S32 m_TriangleIndex;
	U32 m_UserData;
	U32 m_CollisionFlags;
	U16 m_PhysicsMaterialIndex;
	U16 m_EffectMaterialIndex;
	bool m_bNewContact;
};




class ALIGN16 C_Ray
{
public:
	C_Ray() : m_start(Maths::UNDEFINED), m_end(Maths::UNDEFINED), m_Radius(Maths::UNDEFINED), 
		m_Direction(Maths::UNDEFINED), m_maxDistance(Maths::UNDEFINED), m_MaxCosAngle(Maths::UNDEFINED), m_bFatTrace(false), m_bDoubleSided(false)
	{
	}
	C_Ray(const C_Ray& _ray) : m_start(_ray.m_start), m_end(_ray.m_end), m_Radius(_ray.m_Radius), m_Direction(_ray.m_Direction),
		m_Filter(_ray.m_Filter), m_result(_ray.m_result), m_bFatTrace(_ray.m_bFatTrace), m_MaxCosAngle(_ray.m_MaxCosAngle),
		m_maxDistance(_ray.m_maxDistance)
	{
	}

	C_Ray& operator = (const C_Ray& _ray)
	{
		m_start=_ray.m_start;
		m_end=_ray.m_end;
		m_Radius = _ray.m_Radius;
		m_Direction = _ray.m_Direction;
		m_Filter=_ray.m_Filter;
		m_result=_ray.m_result;
		m_bFatTrace = _ray.m_bFatTrace;
		m_MaxCosAngle = _ray.m_MaxCosAngle;
		return *this;
	}

	void InitialiseRay(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const C_CollisionFilter* pFilter,
		U32 currentMask = 0, U32 writeMask = 1)
	{
		m_start = start;
		m_end = end;
		m_Direction = Maths::C_Normal(end - start);
		m_result.m_contact = end;
		m_result.m_distance = Maths::Constants<Maths::HF32>::Max();
		m_Filter = *pFilter;
		m_bFatTrace = false;
		m_result.m_pShape = NULL;
		m_result.m_pNode = NULL;
		m_Radius = Maths::Constants<Maths::HF32>::Zero();
		m_MaxCosAngle = Maths::Constants<Maths::HF32>::Zero();
		m_WriteMask = writeMask;
		m_CurrentMask = currentMask;
	}

	void InitialiseRay(	const Maths::HVec3 PCREF start,
						const Maths::C_Normal PCREF direction, 
						const Maths::HF32 PCREF maxDistance,
						const C_CollisionFilter* pFilter, 
						const Maths::HF32 PCREF radius,
						U32 currentMask = 0, U32 writeMask = 1)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		m_start = start;
		m_end = start + direction * maxDistance;
		m_Direction = direction;
		m_maxDistance = maxDistance;
		m_Radius = radius;
		
		m_result.m_contact = start + direction * (maxDistance + radius);
		m_result.m_distance = maxDistance; // needed to avoid intersections past the end of the ray
		m_result.m_pShape = NULL;
		m_result.m_pNode = NULL;

		m_Filter = *pFilter;
		m_bFatTrace = radius > zero;
		m_MaxCosAngle = zero;
		m_WriteMask = writeMask;
		m_CurrentMask = currentMask;
	}

	bool HasFinished()
	{
		return MT::AtomicGet(&m_CurrentMask) == m_WriteMask;
	}

	void SetMaxCosAngle(const Maths::HF32 PCREF maxCosAngle)
	{
		m_MaxCosAngle = maxCosAngle;
	}

	C_RayTracingResult& GetResult()
	{
		return m_result;
	}

	const C_RayTracingResult& GetResult() const
	{
		return m_result;
	}

	void SetCollisionFilter(const Collision::C_CollisionFilter* pFilter)
	{
		m_Filter = *pFilter;
	}

	void SetDoubleSided(bool bDoubleSided)
	{
		m_bDoubleSided = bDoubleSided;
	}

	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::RGBA PCREF color);

	C_RayTracingResult m_result;
	Maths::HVec3 m_start;
	Maths::HVec3 m_end;
	Maths::HF32 m_Radius;
	Maths::C_Normal m_Direction;
	Maths::HF32 m_maxDistance;
	Maths::HF32 m_MaxCosAngle;
	C_CollisionFilter m_Filter;
	U32 m_WriteMask, m_CurrentMask;
	bool m_bFatTrace;
	bool m_bDoubleSided;
};

class ALIGN16 C_TransparentRay
{
public:
	C_TransparentRay(U32 maxLayers = 4) : m_start(Maths::UNDEFINED), m_end(Maths::UNDEFINED), m_Radius(Maths::UNDEFINED), 
		m_Direction(Maths::UNDEFINED), m_maxDistance(Maths::UNDEFINED), m_MaxCosAngle(Maths::UNDEFINED), m_bFatTrace(false), m_bDoubleSided(false),
		m_numLayers(0), m_maxNumLayers(maxLayers), m_result(NULL)
	{
		m_result = new C_RayTracingResult[maxLayers];
	}

	C_TransparentRay(const C_TransparentRay& _ray) : m_start(_ray.m_start),m_end(_ray.m_end), m_Radius(_ray.m_Radius),
		m_Direction(_ray.m_Direction), m_maxDistance(_ray.m_maxDistance), m_MaxCosAngle(_ray.m_MaxCosAngle), m_SolidFilter(_ray.m_SolidFilter), m_TransFilter(_ray.m_TransFilter), 
		m_bFatTrace(_ray.m_bFatTrace), m_numLayers(_ray.m_numLayers), m_maxNumLayers(_ray.m_maxNumLayers)
	{
#ifndef PLATFORM_SPU
		if (m_maxNumLayers > 0)
		{
			m_result=new C_RayTracingResult[m_maxNumLayers];
			for(U32 i=0; i<m_numLayers; ++i)
			{
				m_result[i] = _ray.m_result[i];
			}
		}
		else
		{
			m_result = NULL;
		}
#endif
		m_SolidResult = _ray.m_SolidResult;
	}

	~C_TransparentRay()
	{
#ifndef PLATFORM_SPU
		delete[] m_result;
#endif
	}


	C_TransparentRay& operator = (const C_TransparentRay& _ray)
	{
		m_start=_ray.m_start;
		m_end=_ray.m_end;
		m_Radius = _ray.m_Radius;
		m_Direction = _ray.m_Direction;
		m_maxDistance = _ray.m_maxDistance;
		m_MaxCosAngle = _ray.m_MaxCosAngle;
		m_SolidFilter=_ray.m_SolidFilter;
		m_TransFilter=_ray.m_TransFilter;
		//m_result=_ray.m_result;
		m_bFatTrace = _ray.m_bFatTrace;
		m_numLayers = _ray.m_numLayers;
		m_maxNumLayers = _ray.m_maxNumLayers;

#ifndef PLATFORM_SPU
		if (m_result != NULL)
		{
			delete[] m_result;
		}

		if (m_maxNumLayers > 0)
		{
			m_result=new C_RayTracingResult[m_maxNumLayers];
			for(U32 i =0; i<m_numLayers; ++i)
			{
				m_result[i] = _ray.m_result[i];
			}
		}
		else
		{
			m_result = NULL;
		}
#else
		m_result = _ray.m_result;
#endif 
		m_SolidResult = _ray.m_SolidResult;
		return *this;
	}

	void StoreResult(C_RayTracingResult& res)
	{
		for(U32 i=0; i<m_numLayers; ++i)
		{
			C_RayTracingResult& result = m_result[i];
			if(res.Distance() < result.Distance())
			{
				//swap
				for(U32 j = m_maxNumLayers - 1; j>i; j--)
				{
					m_result[j] = m_result[j-1];
				}
				m_result[i]=res;
				m_numLayers = Maths::Min(m_numLayers + 1, m_maxNumLayers);
				return;
			}
		}

		if(m_numLayers < m_maxNumLayers)
		{
			m_result[m_numLayers++] = res;
		}
	}

	void InitialiseRay(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const C_CollisionFilter* pSolidFilter, const C_CollisionFilter* pTransFilter)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		m_start = start;
		m_end = end;
		m_Direction = Maths::C_Normal(end - start);
		
		m_SolidFilter = *pSolidFilter;
		m_TransFilter = *pTransFilter;
		m_bFatTrace = false;
		m_numLayers = 0;
		m_SolidResult.m_pShape = NULL;
		m_SolidResult.m_pNode = NULL;
		m_Radius = zero;
		m_MaxCosAngle = zero;
		
	}

	void SetNumLayers(const U32 numLayer)
	{
		if(m_result)
			delete[] m_result;

		m_maxNumLayers = numLayer;

		if (numLayer > 0)
		{
			m_result = new C_RayTracingResult[numLayer];
		}
		else
		{
			m_result = NULL;
		}
	}

	void InitialiseRay(	const Maths::HVec3 PCREF start,
						const Maths::C_Normal PCREF direction, 
						const Maths::HF32 PCREF maxDistance,
						const C_CollisionFilter* pSolidFilter,
						const C_CollisionFilter* pTransFilter,
						const Maths::HF32 PCREF radius)
	{
		m_start = start;
		m_end = start + direction * maxDistance;
		m_Direction = direction;
		m_maxDistance = maxDistance;
		m_Radius = radius;
		m_numLayers = 0;
		m_SolidFilter = *pSolidFilter;
		m_TransFilter = *pTransFilter;
		m_bFatTrace = true;
		m_MaxCosAngle = Maths::Constants<Maths::HF32>::Zero();
		m_SolidResult.m_pShape = NULL;
		m_SolidResult.m_pNode = NULL;
	}

	void SetMaxCosAngle(const Maths::HF32 PCREF maxCosAngle)
	{
		m_MaxCosAngle = maxCosAngle;
	}

	C_RayTracingResult* GetResult()
	{
		return m_result;
	}

	void SetCollisionSolidFilter(const Collision::C_CollisionFilter* pFilter)
	{
		m_SolidFilter = *pFilter;
	}

	void SetCollisionTransparantFilter(const Collision::C_CollisionFilter* pFilter)
	{
		m_TransFilter = *pFilter;
	}

	void SetDoubleSided(bool bDoubleSided)
	{
		m_bDoubleSided = bDoubleSided;
	}

	C_RayTracingResult* m_result;
	C_RayTracingResult m_SolidResult;
	Maths::HVec3 m_start;
	Maths::HVec3 m_end;
	Maths::HF32 m_Radius;
	Maths::C_Normal m_Direction;
	Maths::HF32 m_maxDistance;
	Maths::HF32 m_MaxCosAngle;
	C_CollisionFilter m_SolidFilter;
	C_CollisionFilter m_TransFilter;
	bool m_bFatTrace;
	bool m_bDoubleSided;
	U32 m_maxNumLayers;
	U32 m_numLayers;
};

};

#endif
