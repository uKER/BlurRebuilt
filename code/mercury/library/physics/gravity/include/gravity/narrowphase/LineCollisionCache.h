#ifndef LINE_COLLISION_CACHE_H
#define LINE_COLLISION_CACHE_H
#include "gravity/narrowphase/Ray.h"
#include <core/threads.h>
#include <system/debug/I_DebugRender.h>

namespace Collision
{
#define DEFAULT_CACHE_SIZE 4
	struct ALIGN16 C_CachedTriangle
	{
		Maths::HVec3 m_VertexA;
		Maths::HVec3 m_VertexB;
		Maths::HVec3 m_VertexC;
		Maths::C_Normal m_Normal;
		U32 m_TriangleIndex;
		U32 m_CollisionFlags;
		U16 m_PhysicsMaterialFlags;
		U16 m_EffectMaterialFlags;
		U32 m_UserData;
		C_Shape* m_pShape;
		SharedPhysics::C_GraphNode* m_pGraphNode;

		C_CachedTriangle() : m_VertexA(Maths::UNDEFINED), m_VertexB(Maths::UNDEFINED), m_VertexC(Maths::UNDEFINED), m_Normal(Maths::UNDEFINED)
		{
		}
	};

	class ALIGN16 C_SingleTriangleCache
	{
	public:
		C_CachedTriangle m_Triangle;
		bool m_bHit;
		C_SingleTriangleCache() : m_bHit(false)
		{
		}
	};

	//Implemented as a circular buffer...
	class ALIGN16 C_LineCollisionCache
	{
	public:
		Collision::C_Shape* m_pPlaneShape;
		C_CachedTriangle* m_pTriangles;
		U32 m_Low, m_High;
		U32 m_CacheSize;
		bool m_bExternallyOwned;

	
#ifndef PLATFORM_SPU
		C_LineCollisionCache(const U32 _cacheSize = DEFAULT_CACHE_SIZE) : m_Low(0), m_High(0), 
			m_CacheSize(_cacheSize), m_pPlaneShape(NULL), m_bExternallyOwned(false)
		{

			m_pTriangles = new C_CachedTriangle[_cacheSize];
		}
#endif
		C_LineCollisionCache(C_CachedTriangle* pTriangles, const U32 triangleCount) : m_pTriangles(pTriangles), 
			m_Low(0), m_High(0), m_CacheSize(triangleCount), m_pPlaneShape(NULL), m_bExternallyOwned(true)
		{
		}

		~C_LineCollisionCache()
		{
#ifndef PLATFORM_SPU
			if(!m_bExternallyOwned)
			{
				delete [] m_pTriangles;
			}
			m_pTriangles = NULL;
#endif
		}

		void Reset()
		{
			m_Low = m_High = 0;
			m_pPlaneShape = NULL;
		}

		//NB - only used when we are filling a cache through a fill cache function!
		bool Full() const
		{
			return m_High == m_CacheSize;
		}

		void AddPlane(Collision::C_Shape* pPlane)
		{
			m_pPlaneShape = pPlane;
		}

#ifndef PLATFORM_SPU
#ifndef CFG_CONVERTER
		void DebugRender(DbUtil::I_DebugRender* pRender) const;
#endif
#endif

		bool Trace(Collision::C_Ray& ray, C_SingleTriangleCache* pHitTriangle = NULL);


		template <typename Triangle>
		void AddTriangle(Triangle& triangle, C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, const U32 triangleIndex, const U32 collisionFlags, const U16 physicsMaterial, const U16 effectMaterial, const U32 userData,
			const MathsStorage::Fixed3_S16<14>* const __restrict pVerts, const MathsStorage::HenDN3* const __restrict normals)
		{
			/*if(Full())
				return;*/
			/*const MathsStorage::Float3& vertA(pVerts[triangle.m_IndexA]);
			const MathsStorage::Float3& vertB(pVerts[triangle.m_IndexB]);
			const MathsStorage::Float3& vertC(pVerts[triangle.m_IndexC]);*/
			const MathsStorage::HenDN3& norm(triangle.m_SurfaceNormal);
			//Find out if the is a slot for the triangle
			C_CachedTriangle& freeTri(m_pTriangles[m_High++]);
			//m_High = (m_High + 1)%m_CacheSize;
			//m_Low = Maths::Select(m_High == m_Low, (m_Low + 1)%m_CacheSize, m_Low);

			/*freeTri.m_VertexA = Maths::Load<Maths::HVec3>(&vertA);
			freeTri.m_VertexB = Maths::Load<Maths::HVec3>(&vertB);
			freeTri.m_VertexC = Maths::Load<Maths::HVec3>(&vertC);*/

			freeTri.m_VertexA = Maths::HVec3(&pVerts[triangle.m_IndexA]);
			freeTri.m_VertexB = Maths::HVec3(&pVerts[triangle.m_IndexB]);
			freeTri.m_VertexC = Maths::HVec3(&pVerts[triangle.m_IndexC]);
			freeTri.m_Normal = Maths::C_Normal(Maths::Load<Maths::HVec3>(&norm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			freeTri.m_TriangleIndex = triangleIndex;
			freeTri.m_CollisionFlags = collisionFlags;
			freeTri.m_PhysicsMaterialFlags = physicsMaterial;
			freeTri.m_EffectMaterialFlags = effectMaterial;
			freeTri.m_UserData = userData;
			freeTri.m_pShape = pShape;
			freeTri.m_pGraphNode = pGraphNode;	
		}

		void AddTriangle(const Maths::HVec3 PCREF vertA, const Maths::HVec3 PCREF vertB, const Maths::HVec3 PCREF vertC,
			const Maths::C_Normal PCREF norm, C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
			const U32 triangleIndex, const U32 collisionFlags, const U16 physicsMaterialFlags, const U32 effectMaterialFlags, const U32 userData)
		{
			/*if(Full())
				return;  */
			//Find out if the is a slot for the triangle
			C_CachedTriangle& freeTri(m_pTriangles[m_High++]);
			//m_High = (m_High + 1)%m_CacheSize;

			//m_Low = Maths::Select(m_High == m_Low, (m_Low + 1)%m_CacheSize, m_Low);

			freeTri.m_VertexA = vertA;
			freeTri.m_VertexB = vertB;
			freeTri.m_VertexC = vertC;
			freeTri.m_Normal = norm;
			freeTri.m_TriangleIndex = triangleIndex;
			freeTri.m_CollisionFlags = collisionFlags;
			freeTri.m_PhysicsMaterialFlags = physicsMaterialFlags;
			freeTri.m_EffectMaterialFlags = (U16)effectMaterialFlags;
			freeTri.m_UserData = userData;
			freeTri.m_pShape = pShape;
			freeTri.m_pGraphNode = pGraphNode;	
		}

	};
};

#endif
