#ifndef NEWCONTACTDATABASE_H
#define NEWCONTACTDATABASE_H

#include "gravity/sharedphysics/ContactStream.h"
#include "gravity/sharedphysics/Select.h"
#include "gravity/sharedphysics/Collider.h"
#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/sharedphysics/RadixSort.h"

namespace SharedPhysics
{
	class C_WarmStartForces
	{
	public:
		C_WarmStartForces(const Maths::HF32 PCREF forceNR, const Maths::HF32 PCREF forceF0, const Maths::HF32 PCREF forceF1) : m_Forces(forceNR, forceF0, forceF1)
		{
		}
		C_WarmStartForces() : m_Forces(Maths::Constants<Maths::HVec3>::Zero())
		{
		}

		C_WarmStartForces(const C_WarmStartForces& f) : m_Forces(f.m_Forces)
		{
		}

		C_WarmStartForces& operator = (const C_WarmStartForces& f)
		{
			m_Forces = f.m_Forces;
			return *this;
		}

		C_WarmStartForces& operator = (const Maths::HVec3& f)
		{
			m_Forces = f;
			return *this;
		}

		operator Maths::HVec3() const
		{
			return m_Forces;
		}

		const Maths::HF32 NormalForce() const
		{
			return m_Forces.X();
		}
		
		void NormalForce(const Maths::HF32 PCREF forceNR)
		{
			m_Forces.X(forceNR);
		}

		const Maths::HF32 StaticFrictionForce() const
		{
			return m_Forces.Y();
		}

		void StaticFrictionForce(const Maths::HF32 PCREF forceF0)
		{
			m_Forces.Y(forceF0);
		}

		const Maths::HF32 DyamicFrictionForce() const
		{
			return m_Forces.Z();
		}

		void DynamicFrictionForce(const Maths::HF32 PCREF forceF1)
		{
			m_Forces.Z(forceF1);
		}

		void ZeroForces()
		{
			m_Forces = Maths::Constants<Maths::HVec3>::Zero();
		}

	private:
		Maths::HVec3 m_Forces;
	};
	//The database representation of a single point of contact
	class C_ContactPointDB : public C_BlockItem<C_ContactPointDB>
	{
	public:
		MathsStorage::Float3 m_ContactPoint;
		MathsStorage::Float3 m_WarmstartForces;
		F32 m_PenetrationDepth;
		U32 m_HashValue;
		U32 m_PhysicsMaterialIndices;
		U32 m_EffectMaterialIndices;
		C_ContactPointDB* m_pPrev;
		C_ContactPointDB() : m_pPrev(NULL)
		{
		}
	};

	class C_ContactPatchDB : public C_BlockItem<C_ContactPatchDB>
	{
	public:
		MathsStorage::Float3 m_Normal;
		C_ContactPointDB* m_pContactPoints;
		C_ContactPatchDB* m_pPrev;
		U32 m_NumContactPoints;

		C_ContactPatchDB() : m_pContactPoints(NULL), m_pPrev(NULL), m_NumContactPoints(0)
		{
		}

		void AddContactPoint(C_ContactPointDB* pPoint)
		{
			C_ContactPointDB* pNext(Fast_Select<C_ContactPointDB>(m_pContactPoints == NULL, pPoint, m_pContactPoints));
			pPoint->m_pNext = m_pContactPoints;
			pNext->m_pPrev = pPoint;
			pPoint->m_pPrev = NULL;
			m_pContactPoints = pPoint;
		}

		void RemoveContactPatch(C_ContactPointDB* pPoint)
		{
			C_ContactPointDB* pNext(Fast_Select<C_ContactPointDB>(pPoint->m_pNext == NULL, pPoint, pPoint->m_pNext));
			C_ContactPointDB* pPrev(Fast_Select<C_ContactPointDB>(pPoint->m_pNext == NULL, pPoint, pPoint->m_pPrev));
			m_pContactPoints = Fast_Select<C_ContactPointDB>(m_pContactPoints == pPoint, pPoint->m_pNext, m_pContactPoints);
			pPrev->m_pNext = pPoint->m_pNext;
			pNext->m_pPrev = pPoint->m_pPrev;
			pPoint->m_pNext = NULL;
			pPoint->m_pPrev = NULL;
		}
	};

	class C_CollisionResultDB : public C_BlockItem<C_CollisionResultDB>
	{
	public:
		//A witness point in body A and B's local space. Transform both of these into 
		//world space and compare their position. If close enough, we can re-use these contacts
		Maths::HVec3 m_WitnessPointLocalA, m_WitnessPointLocalB;
		C_GraphNode* m_pNodeA, *m_pNodeB;
		C_ContactPatchDB* m_pContactPatches;
		U32 m_NumPatches;

		//NB - as an optimisation for searching, m_IndexA <= m_IndexB always!!!!
		U32 m_IndexA, m_IndexB;

		C_CollisionResultDB() : m_pNodeA(NULL), m_pNodeB(NULL), m_pContactPatches(NULL), m_NumPatches(0),
			m_WitnessPointLocalA(Maths::UNDEFINED), m_WitnessPointLocalB(Maths::UNDEFINED)
		{
		}

		void AddContactPatch(C_ContactPatchDB* pPatch)
		{
			C_ContactPatchDB* pNext(Fast_Select<C_ContactPatchDB>(m_pContactPatches == NULL, pPatch, m_pContactPatches));
			pPatch->m_pNext = m_pContactPatches;
			pNext->m_pPrev = pPatch;
			pPatch->m_pPrev = NULL;
			m_pContactPatches = pPatch;
		}

		void RemoveContactPatch(C_ContactPatchDB* pPatch)
		{
			C_ContactPatchDB* pNext(Fast_Select<C_ContactPatchDB>(pPatch->m_pNext == NULL, pPatch, pPatch->m_pNext));
			C_ContactPatchDB* pPrev(Fast_Select<C_ContactPatchDB>(pPatch->m_pNext == NULL, pPatch, pPatch->m_pPrev));
			m_pContactPatches = Fast_Select<C_ContactPatchDB>(m_pContactPatches == pPatch, pPatch->m_pNext, m_pContactPatches);
			pPrev->m_pNext = pPatch->m_pNext;
			pNext->m_pPrev = pPatch->m_pPrev;
			pPatch->m_pNext = NULL;
			pPatch->m_pPrev = NULL;
		}

		NOALIAS inline bool operator < (const C_CollisionResultDB & other) const
		{
			return (m_IndexA < other.m_IndexA) | ((m_IndexA == other.m_IndexA) & (m_IndexB < other.m_IndexB));
		}

		NOALIAS inline bool operator <= (const C_CollisionResultDB & other) const
		{
			return (m_IndexA <= other.m_IndexA) | ((m_IndexA == other.m_IndexA) & (m_IndexB <= other.m_IndexB));
		}

		NOALIAS inline bool operator > (const C_CollisionResultDB & other) const
		{
			return (m_IndexA > other.m_IndexA) | ((m_IndexA == other.m_IndexA) & (m_IndexB > other.m_IndexB));
		}

		NOALIAS inline bool operator >= (const C_CollisionResultDB & other) const
		{
			return (m_IndexA >= other.m_IndexA) | ((m_IndexA == other.m_IndexA) & (m_IndexB >= other.m_IndexB));
		}

		NOALIAS inline bool operator == (const C_CollisionResultDB & other) const
		{
			return (m_IndexA == other.m_IndexA) & (m_IndexB == other.m_IndexB);
		}

		NOALIAS inline bool operator != (const C_CollisionResultDB & other) const
		{
			return (m_IndexA != other.m_IndexA) | (m_IndexB != other.m_IndexB);
		}

		S32 Compare(const C_CollisionResultDB& other) const
		{
			S32 compOne(((m_IndexA > other.m_IndexA) - (m_IndexA < other.m_IndexA))<<1);
			S32 compTwo(((m_IndexA == other.m_IndexA) & (m_IndexB > other.m_IndexB)) 
				- ((m_IndexA == other.m_IndexA) & (m_IndexB < other.m_IndexB)));
			return compOne + compTwo;
		}

		S32 CompareFirstElement(const U32 index) const
		{
			return (m_IndexA > index) - (m_IndexA < index);
		}
	};

	class C_CollisionResultDBReverse : public C_BlockItem<C_CollisionResultDBReverse>
	{
	public:
		C_CollisionResultDB* m_pResult;

		C_CollisionResultDBReverse(C_CollisionResultDB* ptr = NULL) : m_pResult(ptr)
		{
		}

		NOALIAS inline bool operator < (const C_CollisionResultDBReverse& other) const
		{
			//return *other.m_pResult < *m_pResult;
			return (m_pResult->m_IndexB < other.m_pResult->m_IndexB) | 
				((m_pResult->m_IndexB == other.m_pResult->m_IndexB) & 
				(m_pResult->m_IndexA < other.m_pResult->m_IndexA));

		}
		
		NOALIAS inline bool operator <= (const C_CollisionResultDBReverse& other) const
		{
			return (m_pResult->m_IndexB < other.m_pResult->m_IndexB) | 
				((m_pResult->m_IndexB == other.m_pResult->m_IndexB) & 
				(m_pResult->m_IndexA <= other.m_pResult->m_IndexA));
		}

		NOALIAS inline bool operator > (const C_CollisionResultDBReverse& other) const
		{
			return (m_pResult->m_IndexB > other.m_pResult->m_IndexB) | 
				((m_pResult->m_IndexB == other.m_pResult->m_IndexB) & 
				(m_pResult->m_IndexA > other.m_pResult->m_IndexA));
		}

		NOALIAS inline bool operator >= (const C_CollisionResultDBReverse& other) const
		{
			return (m_pResult->m_IndexB > other.m_pResult->m_IndexB) | 
				((m_pResult->m_IndexB == other.m_pResult->m_IndexB) & 
				(m_pResult->m_IndexA >= other.m_pResult->m_IndexA));
		}

		NOALIAS inline bool operator == (const C_CollisionResultDBReverse& other) const
		{
			return *other.m_pResult == *m_pResult;
		}

		NOALIAS inline bool operator != (const C_CollisionResultDBReverse& other) const
		{
			return *other.m_pResult != *m_pResult;
		}

		S32 CompareFirstElement(const U32 index) const
		{
			return (m_pResult->m_IndexB > index) - (m_pResult->m_IndexB < index);
		}
	};

	//TODO - need to hook the colliders into the contact database. Additionally, the contact database needs
	//to have a pool of memory to store the contacts between sleeping/sleeping and sleeping/static objects
	//in. This is going to be done using the above data structures in a manager. The static/sleeping portion of the 
	//contact database need not be double-buffered, but MUST be able to search for all collisions involving a given
	//object...

#define SLEEPY_RESULT_BLOCK_SIZE 32
#define SLEEPY_PATCH_BLOCK_SIZE 64
#define SLEEPY_POINT_BLOCK_SIZE 256
#define ACTIVE_RESULT_BLOCK_SIZE 64

	class C_SleepyContactManager
	{
	public:
		C_AllocationManager<C_CollisionResultDB, SLEEPY_RESULT_BLOCK_SIZE, false> m_SleepyResultManager;
		C_AllocationManager<C_CollisionResultDBReverse, SLEEPY_RESULT_BLOCK_SIZE, false> m_SleepyResultReverseManager;
		C_AllocationManager<C_ContactPatchDB, SLEEPY_PATCH_BLOCK_SIZE, false> m_SleepyPatchManager;
		C_AllocationManager<C_ContactPointDB, SLEEPY_POINT_BLOCK_SIZE, false> m_SleepyPointManager;
		

		//Getters!

		NOALIAS inline C_CollisionResultDB* GetFreeCollisionResult()
		{
			C_CollisionResultDB* pResult = m_SleepyResultManager.GetNextFreeItem();
			new (pResult) C_CollisionResultDB;
			return pResult;
		}

		NOALIAS inline C_CollisionResultDBReverse* GetFreeCollisionResultRv()
		{
			C_CollisionResultDBReverse* pResult = m_SleepyResultReverseManager.GetNextFreeItem();
			return pResult;
		}

		NOALIAS inline C_ContactPatchDB* GetFreeCollisionPatch()
		{
			C_ContactPatchDB* pPatch = m_SleepyPatchManager.GetNextFreeItem();
			new (pPatch) C_ContactPatchDB;
			return pPatch;
		}

		NOALIAS inline C_ContactPointDB* GetFreeCollisionPoint()
		{
			C_ContactPointDB* pPoint = m_SleepyPointManager.GetNextFreeItem();
			new (pPoint) C_ContactPointDB;
			return pPoint;
		}

		//Releasers!

		NOALIAS inline void PutToFree(C_CollisionResultDB* pResult)
		{
			m_SleepyResultManager.PushToFree(pResult);
		}

		NOALIAS inline void PutToFree(C_CollisionResultDBReverse* pResult)
		{
			m_SleepyResultReverseManager.PushToFree(pResult);
		}

		NOALIAS inline void PutToFree(C_ContactPatchDB* pPatch)
		{
			m_SleepyPatchManager.PushToFree(pPatch);
		}

		NOALIAS inline void PutToFree(C_ContactPointDB* pPoint)
		{
			m_SleepyPointManager.PushToFree(pPoint);
		}
	};

	class C_ActiveResultRvPtr
	{
	public:
		C_Collider* m_pCollider;
		C_ActiveResultRvPtr(C_Collider* pCollider = NULL) : m_pCollider(pCollider)
		{
		}

		NOALIAS inline U64 GetRadix64()
		{
			return (U64)((((U64)m_pCollider->m_IndexB) << 32) | m_pCollider->m_IndexA);
		}

		NOALIAS inline U32 GetRadix32()
		{
			return (U32)(((m_pCollider->m_IndexB & 0xffff) << 16) | (m_pCollider->m_IndexA & 0xffff));
		}


		NOALIAS inline bool operator < (const C_ActiveResultRvPtr& other) const
		{
			return (m_pCollider->m_IndexB < other.m_pCollider->m_IndexB) | 
				((m_pCollider->m_IndexB == other.m_pCollider->m_IndexB) & 
				(m_pCollider->m_IndexA < other.m_pCollider->m_IndexA));
		}

		NOALIAS inline bool operator <= (const C_ActiveResultRvPtr& other) const
		{
			return (m_pCollider->m_IndexB < other.m_pCollider->m_IndexB) | 
				((m_pCollider->m_IndexB == other.m_pCollider->m_IndexB) & 
				(m_pCollider->m_IndexA <= other.m_pCollider->m_IndexA));
		}

		NOALIAS inline bool operator > (const C_ActiveResultRvPtr& other) const
		{
			return (m_pCollider->m_IndexB > other.m_pCollider->m_IndexB) | 
				((m_pCollider->m_IndexB == other.m_pCollider->m_IndexB) & 
				(m_pCollider->m_IndexA > other.m_pCollider->m_IndexA));
		}

		NOALIAS inline bool operator >= (const C_ActiveResultRvPtr& other) const
		{
			return (m_pCollider->m_IndexB > other.m_pCollider->m_IndexB) | 
				((m_pCollider->m_IndexB == other.m_pCollider->m_IndexB) & 
				(m_pCollider->m_IndexA >= other.m_pCollider->m_IndexA));
		}

		NOALIAS inline bool operator == (const C_ActiveResultRvPtr& other) const
		{
			return *m_pCollider == *other.m_pCollider;
		}

		NOALIAS inline bool operator != (const C_ActiveResultRvPtr& other) const
		{
			return *m_pCollider != *other.m_pCollider;
		}
	};

	class C_ActiveResultPtr
	{
	public:
		C_Collider* m_pCollider;
		C_ActiveResultPtr(C_Collider* pCollider = NULL) : m_pCollider(pCollider)
		{
		}

		NOALIAS inline U64 GetRadix64()
		{
			return (U64)((((U64)m_pCollider->m_IndexA) << 32) | m_pCollider->m_IndexB);
		}

		NOALIAS inline U32 GetRadix32()
		{
			return (U32)(((m_pCollider->m_IndexA & 0xffff) << 16) | (m_pCollider->m_IndexB & 0xffff));
		}


		NOALIAS inline bool operator < (const C_ActiveResultPtr& other) const
		{
			return *m_pCollider < *other.m_pCollider;
		}

		NOALIAS inline bool operator <= (const C_ActiveResultPtr& other) const
		{
			return *m_pCollider <= *other.m_pCollider;
		}

		NOALIAS inline bool operator > (const C_ActiveResultPtr& other) const
		{
			return *m_pCollider > *other.m_pCollider;
		}

		NOALIAS inline bool operator >= (const C_ActiveResultPtr& other) const
		{
			return *m_pCollider >= *other.m_pCollider;
		}

		NOALIAS inline bool operator == (const C_ActiveResultPtr& other) const
		{
			return *m_pCollider == *other.m_pCollider;
		}

		NOALIAS inline bool operator != (const C_ActiveResultPtr& other) const
		{
			return *m_pCollider != *other.m_pCollider;
		}
	};


	//typedef C_PointerWrapper<C_Collider> C_ActiveResultPtr;

	class C_ActiveContactManager
	{
		friend class C_ContactDatabase;
		//C_AllocationManager<C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false> m_ActiveManager;
		/*MemoryAllocator<C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false> m_ActiveManager;
		C_MemoryIterator<C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false> m_ActiveIterator;*/
		C_PersistentContactCollider* m_pColliders;

		MemoryAllocator<C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false> m_MeshManager;
		C_MemoryIterator<C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false> m_MeshIterator;


		//MemoryAllocator<C_CollisionInstance, ACTIVE_RESULT_BLOCK_SIZE, true> m_CollisionInstances;
		//C_MemoryIterator<C_CollisionInstance, ACTIVE_RESULT_BLOCK_SIZE, true> m_CollisionInstanceIter;
		//Only 1 array required. C_Colliders always have m_IndexA smaller than m_IndexB!
		//However, the arrays are double-buffered...
		SortedArray<C_ActiveResultPtr> m_ActiveArray;
		SortedArray<C_ActiveResultRvPtr> m_ActiveRvArray;
		U32 m_NumCollidingPairs;
		U32 m_NumMeshPairs;
		U32 m_MaxColliders;
		//MT::Lock m_Lock;

	public:

		C_ActiveContactManager(U32 maxColliders) : /*m_ActiveIterator(&m_ActiveManager),*/ m_MeshIterator(&m_MeshManager), m_NumCollidingPairs(0),
			m_NumMeshPairs(0), m_MaxColliders(maxColliders)/*, m_Lock("ContactMgr")*/
		{
			m_pColliders = new C_PersistentContactCollider[maxColliders];
		}

		~C_ActiveContactManager()
		{
			delete [] m_pColliders;
		}

		NOALIAS inline void Reset()
		{
			//m_ActiveIterator.Reset();
			m_MeshIterator.Reset();
			//m_ActiveArray.m_numElements = 0;
			//m_ActiveRvArray.m_numElements = 0;
			m_NumCollidingPairs = 0;
			m_NumMeshPairs = 0;
			//m_CollisionInstanceIter.Reset();
		}

		NOALIAS inline C_PersistentContactCollider* GetCollider(U32 index)
		{
			return &m_pColliders[index];
		}


		NOALIAS inline C_PersistentContactCollider* GetCollider()
		{
			U32 index = (U32)MT::AtomicIncrement((S32*)&m_NumCollidingPairs);
			BIZ_ASSERT(index < m_MaxColliders);
			if(index < m_MaxColliders)
			{
				C_PersistentContactCollider* pCollider = &m_pColliders[index];
				pCollider->m_StreamPtr.Reset();
				return pCollider;
			}
			return NULL;
		}

		/*NOALIAS inline C_Collider* GetColliderNoLock()
		{
			U32 index = (U32)MT::AtomicIncrement((S32*)&m_NumCollidingPairs);
			BIZ_ASSERT(index < m_MaxColliders);
			if(index < m_MaxColliders)
			{
				C_Collider* pCollider = &m_pColliders[index];
				pCollider->m_StreamPtr.Reset();
				return pCollider;
			}
			return NULL;
		}  */

		NOALIAS inline C_Collider* GetMeshCollider()
		{
			++m_NumMeshPairs;
			C_Collider* pCollider(m_MeshIterator.Create());
			pCollider->m_StreamPtr.Reset();
			return pCollider;
		}


		U32 GetNumberCollidingPairs() const
		{
			return m_NumCollidingPairs;
		}

		U32 GetNumberMeshPairs() const
		{
			return m_NumMeshPairs;
		}

		/*C_MemoryIterator<C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false> GetIterator()
		{
			return C_MemoryIterator<C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false>(&m_ActiveManager);
		}*/

		C_MemoryIterator<C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false> GetMeshIterator()
		{
			return C_MemoryIterator<C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false>(&m_MeshManager);
		}



		/*NOALIAS __forceinline void FreeCollider(C_Collider* pCollider)
		{
			m_ActiveManager.PushToFree(pCollider);
		}*/

		NOALIAS inline U32 NumColliders() const
		{
			return m_NumCollidingPairs;//m_ActiveArray.m_numElements;
		}

	/*	NOALIAS inline C_Collider* Collider(U32 index) const
		{
			C_ActiveResultPtr& ptr(m_ActiveArray[index]);
			return ptr.m_pCollider;
		}

		NOALIAS inline C_Collider* RvCollider(U32 index) const
		{
			C_ActiveResultRvPtr& ptr(m_ActiveRvArray[index]);
			return ptr.m_pCollider;
		}*/

		void Resize(const U32 numContacts)
		{
			m_ActiveArray.Resize(numContacts);
			m_ActiveRvArray.Resize(numContacts);
		}

		NOALIAS inline void AddToDatabase(C_Collider* pCollider)
		{
			C_ActiveResultPtr ptr(pCollider);
			m_ActiveArray.AddToEnd(&ptr);
			C_ActiveResultRvPtr ptr2(pCollider);
			m_ActiveRvArray.AddToEnd(&ptr2);
		}

		void SortDatabase(C_S32SafeStack& stack, SharedPhysics::C_RadixSorter64<C_ActiveResultPtr>* pSorter);

		C_Collider* FindCollider(C_Collider* pCollider, S32& index) const;

		NOALIAS inline S32 FindFirstIndex( C_GraphNode* pNode ) const
		{
			C_Collider tempCollider;
			tempCollider.m_IndexA = pNode->m_Index;
			tempCollider.m_IndexB = 0;
			C_ActiveResultPtr ptr(&tempCollider);
			S32 ipos;
			m_ActiveArray.FindIndex(&ptr, ipos);
			return ipos;
		}

		NOALIAS inline S32 FindFirstIndexRv( C_GraphNode* pNode ) const
		{
			C_Collider tempCollider;
			tempCollider.m_IndexA = 0;
			tempCollider.m_IndexB = pNode->m_Index;
			C_ActiveResultRvPtr ptr(&tempCollider);
			S32 ipos;
			m_ActiveRvArray.FindIndex(&ptr, ipos);
			return ipos;
		}

		NOALIAS inline void RemoveContact(S32 index)
		{
			m_ActiveArray.Remove(index, 1);
		}
	};

	typedef C_PointerWrapper<C_CollisionResultDB> C_ResultPtr;
	typedef C_PointerWrapper<C_CollisionResultDBReverse> C_ResultRvPtr;
	

	class C_Collider;

	class C_ContactDatabase
	{
	public:
		//Contact Database split into 2 parts - [Sleeping/slepping, Sleeping/static], [Contacts involving at least 1 dynamic body]
		//First - the sleepy part...
		/*C_SleepyContactManager m_SleepyManager;
		//Two sorted arrays, one sorting by the major factor, the other sorting by the minor factor...
		SortedArray<C_ResultPtr> m_SleepyContactArray;
		SortedArray<C_ResultRvPtr> m_SleepyContactRvArray;*/

		//SharedPhysics::C_RadixSorter64<C_ActiveResultPtr>* m_pRadixSorter;
		C_ActiveContactManager m_ActiveContact1, m_ActiveContact2;

		//The active part...
		C_ActiveContactManager * m_pCurrentBuffer, *m_pPrevBuffer;

		C_ContactDatabase(U32 maxColliders) : m_ActiveContact1(maxColliders), m_ActiveContact2(maxColliders), m_pCurrentBuffer(&m_ActiveContact1), m_pPrevBuffer(&m_ActiveContact2)
		{
			//m_pRadixSorter = new SharedPhysics::C_RadixSorter64<C_ActiveResultPtr>;
		}

		~C_ContactDatabase()
		{
			//delete m_pRadixSorter;
		}

		NOALIAS inline void FlipBuffers()
		{
			C_ActiveContactManager * pBuff(m_pCurrentBuffer);
			m_pCurrentBuffer = m_pPrevBuffer;
			m_pPrevBuffer = pBuff;
			m_pCurrentBuffer->Reset();
		}

		NOALIAS inline C_PersistentContactCollider* GetFreeCollider()
		{
			return m_pCurrentBuffer->GetCollider();
		}

		NOALIAS inline C_Collider* GetFreeMeshCollider()
		{
			return m_pCurrentBuffer->GetMeshCollider();
		}


	/*	NOALIAS inline C_Collider* GetFreeColliderNoLock()
		{
			return m_pCurrentBuffer->GetColliderNoLock();
		}*/



		NOALIAS inline void AddContact(C_Collider* pCollider)
		{			
			m_pCurrentBuffer->AddToDatabase(pCollider);
		}

		NOALIAS inline C_Collider* FindPrevCollider(C_GraphNode* pNodeA, C_GraphNode* pNodeB, S32& index)
		{
			C_Collider collider(pNodeA, pNodeB);
			return m_pPrevBuffer->FindCollider(&collider, index);
		}

		NOALIAS inline void RemovePrevContact(S32 index)
		{
			m_pPrevBuffer->RemoveContact(index);
		}

		NOALIAS inline void RemoveCollider(C_Collider* pCollider)
		{
			//assert(pCollider->m_State == C_Collider::E_Normal);
			pCollider->m_State = C_Collider::E_Removed;
		}

		NOALIAS inline void SortDatabase(C_S32SafeStack& stack)
		{
			//m_pPrevBuffer->SortDatabase(stack, m_pRadixSorter);
			m_pCurrentBuffer->SortDatabase(stack,NULL);// m_pRadixSorter);
		}

		//Makes a given contact sleepy - pushes this contact into the contact database
		//NOALIAS inline void MakeSleepyContact(C_Collider* pCollider);

		//Writes all sleeepy contacts involving a given graph node into the contact stream...
		//This not only inserts new C_Colliders into the current buffer of the contact database, but removes
		//the corresponding contacts from the two buffers in the sleepy contact array
		//void WriteAllSleepyContactsToStream(C_GraphNode* pGraphNode, C_ContactStream& stream);
		//Remove all contacts from the sleepy database involving a given object (e.g. the object has been removed from the simulation)
		//void PurgeAllContacts(C_GraphNode* pGraphNode);

		NOALIAS inline void SumKineticEnergies(SharedPhysics::C_ContactStream& clothStream);

		/*NOALIAS inline void ReleaseSleepyContact(C_CollisionResultDB* pResult);

		NOALIAS inline void ReleaseSleepyContact(C_CollisionResultDBReverse* pResultRv);

		NOALIAS inline void AddSleepyContact(C_CollisionResultDB* pResult)
		{
			C_CollisionResultDBReverse* pReverse(m_SleepyManager.GetFreeCollisionResultRv());
			pReverse->m_pResult = pResult;
			C_ResultPtr result(pResult);
			C_ResultRvPtr rvResult(pReverse);
			m_SleepyContactArray.Insert(&result);
			m_SleepyContactRvArray.Insert(&rvResult);
		}*/

		NOALIAS inline SortedArray<C_ActiveResultPtr>& GetActiveContacts()
		{
			return m_pCurrentBuffer->m_ActiveArray;
		}

		//Clear left-over contacts, wake up sleepy objects and push constraint edges/points/contact constraints
		//to free!
		//Must be a method in constraint graph manager!!!!
		//void ClearLeftoverContacts(C_ConstraintGraphManager* pManager);
	};

};

#endif
