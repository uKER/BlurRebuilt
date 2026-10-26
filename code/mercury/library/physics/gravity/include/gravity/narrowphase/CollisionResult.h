#ifndef COLLISIONRESULT_H
#define COLLISIONRESULT_H

#include "gravity/narrowphase/AxisBox.h"
#include "gravity/narrowphase/Shape.h"
#include "gravity/sharedphysics/GraphNode.h"
#include "gravity/sharedphysics/ContactDatabase.h"
#include "gravity/sharedphysics/ContactStream.h"
//#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/collisionproxy.h"
//#include "gravity/narrowphase/EPA.h"

/*
 * Basic currency of collision results. This is the flattened, uncompressed collision result structure
 * which is filled by the collision detection. Following its creation, it will be subject to contact reduction
 * and will then be pushed into a memory-efficient contact stream, which will pack the data into a flattened byte-array
 */

namespace Collision
{
	//using namespace Maths;

	class C_ClothContact;
	class EPA;

	template<class Storage, int MaxElements> class C_Cluster;

	class C_ContactPoint
	{
	public:
		C_ContactPoint() : m_ContactPoint(Maths::UNDEFINED)
		{
		}
		Maths::HVec3 m_ContactPoint;
		F32 m_PenetrationDepth;
		//High 16-bit = bodyA material index, Low 16-bit = bodyB material index
		U32 m_PhysicsMaterialIndices;
		U32 m_EffectMaterialIndices;
		U32 m_HashValue;
	};

	template <int MaxContacts>
	class C_ContactPatch
	{
	public:
		C_ContactPatch() : m_Normal(Maths::UNDEFINED), m_NumContactPoints(0)
		{
		}
		C_ContactPoint m_ContactPoints[MaxContacts];
		Maths::C_Normal m_Normal;
		U32 m_NumContactPoints;
		void* m_pCluster;

		template <class ClusterObj>
		NOALIAS FORCE_INLINE ClusterObj* Cluster()
		{
			return ((ClusterObj*)m_pCluster);
		}

		NOALIAS FORCE_INLINE void Cluster(void* cluster)
		{
			m_pCluster = cluster;
		}
	};

	class C_CollisionObject
	{
	public:
		C_Shape* m_pShape;
		C_Shape* m_pSpuShape;
		U32 m_Index;
		C_CRAxisBox m_TransformedBox;
		Maths::HF32 m_Radius;
		//Maths::HVec3 m_VectorTravelled;

		//A void pointer to the object - object type not declared in this scope so must be void*
		SharedPhysics::C_GraphNode* m_pNodePtr;

		C_CollisionObject(C_Shape* pShape, U32 index, C_CRAxisBox& box, SharedPhysics::C_GraphNode* pNode,
			const Maths::HF32 PCREF time) : 
		m_pShape(pShape), m_Index(index), m_TransformedBox(box), m_pNodePtr(pNode), m_Radius(Maths::UNDEFINED)
		{
		}

		C_CollisionObject(C_Shape* pShape, U32 index, C_CRAxisBox& box, SharedPhysics::C_GraphNode* pNode) : 
		m_pShape(pShape), m_Index(index), m_TransformedBox(box), m_pNodePtr(pNode), m_Radius(Maths::UNDEFINED)
		{
		}

		C_CollisionObject(C_Shape* pShape, C_Shape* ppuShape) : m_pShape(ppuShape), m_Radius(Maths::UNDEFINED)
		{
			pShape->m_LocalBoundingBox.Transform(pShape->m_PosRot, m_TransformedBox);
		}

		C_CollisionObject() : m_Radius(Maths::UNDEFINED)
		{
		}
	};


	template<int MaxPatches, int MaxContacts>
	class C_CollisionResult
	{
	public:
		C_CollisionResult() : m_NumContactPatches(0), m_pCurrentCollider(NULL)
		{
		}
		C_CollisionObject* m_pObjectA, *m_pObjectB;
		C_ContactPatch<MaxContacts> m_ContactPatches[MaxPatches];
		U32 m_NumContactPatches;
		EPA* m_pEpa;
		SharedPhysics::C_Collider* m_pCurrentCollider;

		void AddPatch(C_ContactPatch<MaxContacts>& patch)
		{
			const Maths::HF32 closeNormal = SharedPhysics::Constants::CloseNormal();
			const Maths::HF32 closeContact = SharedPhysics::Constants::CloseContact();
			U32 incr = 0;
			C_ContactPatch<MaxContacts>* patchResult(NULL);
			if(m_NumContactPatches < MaxPatches)
			{
				patchResult = &m_ContactPatches[m_NumContactPatches];
				patchResult->m_NumContactPoints = 0;
				patchResult->m_Normal = patch.m_Normal;
				incr = 1;
			}
			for(U32 p = 0; p < m_NumContactPatches; ++p)
			{
				const Maths::HF32 dot(m_ContactPatches[p].m_Normal | patch.m_Normal);
				if(dot > closeNormal)
				{
					patchResult = &m_ContactPatches[p];
					incr = 0;
					break;
				}
			}
			if(!patchResult)
				return;
			m_NumContactPatches += incr;
			for(U32 b=0; b<patch.m_NumContactPoints; ++b)
			{
				bool bFound = false;
				if(patchResult->m_NumContactPoints >= MaxContacts)
					break;
				for(U32 c = 0; c < patchResult->m_NumContactPoints; ++c)
				{
					const Maths::HVec3 dif(patchResult->m_ContactPoints[c].m_ContactPoint - patch.m_ContactPoints[b].m_ContactPoint);
					const Maths::HF32 d(dif | dif);
					if(d < closeContact)
					{
						bFound = true;
						break;
					}
				}
				if(!bFound)
					patchResult->m_ContactPoints[patchResult->m_NumContactPoints++]=patch.m_ContactPoints[b];
			}
		}
	};

	template<class Storage, int MaxElements = 32>
	class C_Cluster
	{
	public:
		C_Cluster() : m_Centroid(Maths::UNDEFINED), m_OldCentroid(Maths::UNDEFINED), m_FClusterSize(Maths::UNDEFINED), processed(false)
		{
		}
		Maths::HVec3 m_Centroid;
		Maths::HVec3 m_OldCentroid;
		Storage m_Cluster[MaxElements];
		U32 m_ClusterSize;
		Maths::HF32 m_FClusterSize;
		bool processed;
	};

	template <int MaxPatches, int MaxContacts, int DesiredPatches, int DesiredContacts>
	void ReduceContactsAndPackToStream( C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactStream& stream,
		SharedPhysics::C_Collider* pCollider);

	template <int MaxPatches, int MaxContacts, int DesiredPatches, int DesiredContacts>
	void ReduceContactsAndPackToStreamSpu( C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactStream& stream,
		SharedPhysics::C_Collider* pCollider);

	template <int MaxPatches, int MaxContacts, int DesiredPatches, int DesiredContacts>
	void ReduceContactsAndPackToDatabase(C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactDatabase& database);

	template <int MaxPatches, int MaxContacts>
	void EncodeContacts(const C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactStream& stream,
		SharedPhysics::C_Collider* pCollider);

	template <int MaxPatches, int MaxContacts>
	void EncodeContactsSpu(const C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactStream& stream,
		SharedPhysics::C_Collider* pCollider);

	void EncodeContact(const C_ClothContact* pContacts, const U32 numContacts, SharedPhysics::C_ContactStream& stream,
		SharedPhysics::C_Collider* pCollider);

	template <int MaxPatches, int MaxContacts>
	void EncodeContacts(const C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactDatabase& database);


};

#endif
