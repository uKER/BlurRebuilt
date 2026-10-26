#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H

#include "gravity/narrowphase/ResultSize.h"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/LineCollisionCache.h"
#include "gravity/narrowphase/EPA.h"

namespace Gravity
{
	class C_Scene;
};

namespace SharedPhysics
{
	template <bool bFixedSize> class C_SafeStack;

	class C_Collider;
};

namespace Solver2
{
	class C_ConstraintGraphManager;
}

namespace Collision
{
class C_Ray;
class C_ClothShape;
class C_RopeShape;

class C_CollisionManager
{
public:
	friend class Gravity::C_Scene;

	C_CollisionManager(U32 streamSize, U32 maxColliders) : m_StreamA(streamSize), m_StreamB(streamSize), m_ClothStream(streamSize),
		m_pCurrentStream(&m_StreamA), m_pPrevStream(&m_StreamB)
	{
		m_pContactDatabase = new SharedPhysics::C_ContactDatabase(maxColliders);
		m_pDefaultEpa = new EPA;
	}

	~C_CollisionManager()
	{
		delete m_pContactDatabase;
		delete m_pDefaultEpa;
	}

	void* operator new (size_t size)
	{
		return BizAlignedMalloc(size, 128);
	}

	void* operator new [] (size_t size)
	{
		return BizAlignedMalloc(size, 128);
	}

	void operator delete(void* ptr)
	{
		BizAlignedFree(ptr);
	}

	void operator delete [] (void* ptr)
	{
		BizAlignedFree(ptr);
	}

	//void UpdateCollider(SharedPhysics::C_Collider* pCollider);

	void PatchColliders(Gravity::C_BodyList& wakingList);

	void PatchWakingColliders(Gravity::C_BodyList& interferedList);

	template<int MaxPatches, int MaxContacts>
	bool CollideMesh( const C_CollisionObject* const  __restrict a, const C_CollisionObject*  const __restrict b, SharedPhysics::C_SafeStack<true>& stack, C_CollisionResult<MaxPatches, MaxContacts>& results,
		EPA& m_Epa);

	


	/*template<int MaxPatches, int MaxContacts>
	bool Collide(const Collision::C_CollisionObject *__restrict const a, const Collision::C_CollisionObject *__restrict const b, C_CollisionResult<MaxPatches, MaxContacts>& results);*/

	void Collide(C_CollisionObject* a, C_CollisionObject* b, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionResult<_MaxPatches, _MaxContacts>& result, 
		EPA& m_Epa);
	void CollideMesh(C_CollisionObject* a, C_CollisionObject* b, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>& results, 
		EPA& m_Epa);

	void CollideMeshSpuStream(C_CollisionObject* a, C_CollisionObject* b, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>& results, 
		EPA& m_Epa);


	Maths::HF32 ContinuousCollideMesh(C_CollisionObject* a, C_CollisionObject* b, const Maths::HVec3 PCREF r, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, SharedPhysics::C_SafeStack<true>& stack,
		U32& physicsMaterial, U32& effectMaterial);

	bool CollideGroundPlane(C_CollisionObject* a, C_CollisionObject* b, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
		EPA& m_Epa);
	
	bool Collide(const C_Shape* const a, const C_Shape*  const b,  Collision::C_CollisionResult<_MaxPatches, _MaxContacts>& result, 
		SharedPhysics::C_SafeStack<true>& stack, EPA& m_Epa, bool bFlip, Collision::C_ContactPatch<_MaxContacts>& patch);
	/*bool CollideMesh(const  C_ShapeWrapper* const a, const C_ShapeWrapper*  const b, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionResult<_MaxPatches, _MaxContacts>& results,
		EPA& m_Epa);*/
	
	
	bool RayTrace(const C_CollisionObject* const __restrict a, C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);
	bool FatRayTrace(const C_CollisionObject* const __restrict a, C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);

	bool RayTraceWater(const C_CollisionObject* const __restrict a, C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack , SharedPhysics::C_SmallSPUCache& spuCache);
	bool RayTraceWater( const C_Shape* __restrict const pPpuShape,Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache );

	bool RayTrace(const C_CollisionObject* const __restrict a, C_TransparentRay& ray, C_CollisionFilter* pFilter, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);
	bool FatRayTrace(const C_CollisionObject* const __restrict a, C_TransparentRay& ray, C_CollisionFilter* pFilter, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);

	
	/*SharedPhysics::C_Collider* GetCollider(const SharedPhysics::C_GraphNode* const __restrict a, const SharedPhysics::C_GraphNode* const __restrict b);*/
	template <class Triangle>
	bool CollideTriangle(const Maths::HVec3* const __restrict verts, const Triangle* const __restrict tri,
		const U32 numTriangle, SharedPhysics::C_Collider* collider);
	SharedPhysics::C_ContactStream GetContactStream();

	bool CollideClothDynamic(C_CollisionObject* obj, SharedPhysics::C_GraphNode* pClothNode, 
		C_ClothShape* pCloth, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_ContactStream& clothContactStream);

	bool CollideRopeDynamic(C_CollisionObject* obj, SharedPhysics::C_GraphNode* pRopeNode, 
		C_RopeShape* pRope, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_ContactStream& ropeContactStream);

	void CreateColliders(SharedPhysics::C_ConstContactStreamPtr& stream, Solver2::C_ConstraintGraphManager& constraintManager);

	void FillCache(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
		Collision::C_LineCollisionCache* pCache, Collision::C_CRAxisBox& box, Collision::C_CollisionFilter* pFilter, 
		const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF maxCosAngle, SharedPhysics::C_SafeStack<true>& stack, 
		S32 size, SharedPhysics::C_SmallSPUCache& spuCache);

	Maths::HF32 GetSubVolume(Collision::C_Shape* pShape, const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio);

	Maths::HF32 GetSubVolume2(Collision::C_Shape* pShape, const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio);

	static void TransformShapeAndBox(Collision::C_Shape* pShape, const Maths::HMat43& matrix, Collision::C_CRAxisBox& box);

	static void TransformShape(Collision::C_Shape* pShape, const Maths::HMat43& matrix);

	static void TransformShapeLocal(Collision::C_Shape* pShape, const Maths::HMat43& matrix);

	static void DestructShape(Collision::C_Shape* pShape);

	
	EPA *m_pDefaultEpa;
	void FlipBuffers()
	{
		SharedPhysics::C_ContactStream* pBuf(m_pCurrentStream);
		m_pCurrentStream = m_pPrevStream;
		m_pPrevStream = pBuf;
		m_pCurrentStream->Reset();
		m_pContactDatabase->FlipBuffers();
	}

	

	void ReverseContacts(C_CollisionResult<_MaxPatches, _MaxContacts>& results)
	{
		for(U32 a= 0; a < results.m_NumContactPatches; ++a)
		{
			results.m_ContactPatches[a].m_Normal = -results.m_ContactPatches[a].m_Normal;
		}
	}

	SharedPhysics::C_Collider* CreateCollider(SharedPhysics::C_GraphNode* a, SharedPhysics::C_GraphNode* b,
		SharedPhysics::C_Collider::E_ColliderType type = SharedPhysics::C_Collider::E_RigidCollision);

	/*SharedPhysics::C_Collider* CreateColliderNoLock(SharedPhysics::C_GraphNode* a, SharedPhysics::C_GraphNode* b,
		SharedPhysics::C_Collider::E_ColliderType type = SharedPhysics::C_Collider::E_RigidCollision);*/

	SharedPhysics::C_Collider* CreateColliderMesh(SharedPhysics::C_GraphNode* a, SharedPhysics::C_GraphNode* b,
						SharedPhysics::C_Collider::E_ColliderType type = SharedPhysics::C_Collider::E_RigidCollision);

	template <U32 MaxPatches, U32 MaxContacts, U32 DesiredPatches, U32 DesiredContacts>
	void StoreContacts(const C_CollisionObject* const  __restrict a, const C_CollisionObject*  const __restrict b, SharedPhysics::C_ContactStream& stream, C_CollisionResult<MaxPatches, MaxContacts>& results);

	template <U32 MaxPatches, U32 MaxContacts, U32 DesiredPatches, U32 DesiredContacts>
	void StoreContactsSpu(const C_CollisionObject* const  __restrict a, const C_CollisionObject*  const __restrict b, SharedPhysics::C_ContactStream& stream, C_CollisionResult<MaxPatches, MaxContacts>& results);
	
	template <U32 MaxPatches, U32 MaxContacts, U32 DesiredPatches, U32 DesiredContacts>
	void StoreContactsCharacter(const C_CollisionObject* const a, 
	const C_CollisionObject*  const b, SharedPhysics::C_ContactStream& stream, 
	C_CollisionResult<MaxPatches, MaxContacts>& results);

	bool ContinuousCheckOBBOBB(Collision::C_OBB& obbA, Collision::C_OBB& obbB,
		const Maths::HVec3 PCREF r, Maths::HF32& lambda,Maths::HVec3& normal, Maths::HVec3& contactPoint);


	
	template <U32 MaxPatches, U32 MaxContacts, U32 DesiredPatches, U32 DesiredContacts>
	NOALIAS SharedPhysics::C_Collider* StoreContactsGJK(const C_CollisionObject* const  __restrict a, const C_CollisionObject*  const __restrict b, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_Collider* pExistingCollider, C_CollisionResult<MaxPatches, MaxContacts>& results);
	SharedPhysics::C_ContactStream m_StreamA, m_StreamB, m_ClothStream;
	SharedPhysics::C_ContactStream* m_pCurrentStream, *m_pPrevStream;
	SharedPhysics::C_ContactDatabase* m_pContactDatabase;
	U32 m_HashValueRandomisation;
	//SharedPhysics::C_MemoryIterator<SharedPhysics::C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false> m_Iterator;
	SharedPhysics::C_MemoryIterator<SharedPhysics::C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false> m_MeshIterator;
	U32 m_NumCollidingPairs;
	U32 m_NumMeshPairs;


};

};

#endif

