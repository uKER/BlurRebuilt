#ifndef SOLVER2_CONSTRAINTGRAPHMANAGER_H
#define SOLVER2_CONSTRAINTGRAPHMANAGER_H
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/sharedphysics/ConstraintGraph.h"
#include "gravity/sharedphysics/ContactDatabase.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/sharedphysics/Material.h"
#include "gravity/sharedphysics/SafeStack.h"
#include "gravity/sharedphysics/MemoryAllocator.h"

namespace Gravity
{
	class C_EventStream;
	class C_BodyList;
};

namespace Solver2
{
#ifndef PLATFORM_SPU
	static Debug::Logger &g_Log = Debug::GetLogger("Physics.Solver2");
#endif

	typedef SharedPhysics::C_PointerWrapper<SharedPhysics::C_GraphNode> C_GraphNodeWrapper;
	typedef SharedPhysics::C_PointerWrapper<SharedPhysics::C_ConstraintEdge> C_ConstraintEdgeWrapper;
	typedef SharedPhysics::C_PointerWrapper<SharedPhysics::C_ConstraintPoint> C_ConstraintPointWrapper;
	typedef SharedPhysics::C_PointerWrapper<SharedPhysics::C_Constraint> C_ConstraintPtr;

	class C_ConstraintGraphManager
	{
	public:
		SharedPhysics::C_AllocationManager<SharedPhysics::C_ConstraintEdge, 256, true> m_PersistentConstraintEdges;
		//SharedPhysics::C_AllocationManager<SharedPhysics::C_ConstraintTwin, 128, true> m_PersistentTwinConstraints;
		SharedPhysics::C_AllocationManager<SharedPhysics::C_ConstraintPoint, 512, true> m_PersistentConstraintPoints;

		SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintEdge, 256, true> m_VolatileConstraintEdges1;
		//SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintTwin, 128, true> m_VolatileTwinConstraints1;
		SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintPoint, 256, true> m_VolatileConstraintPoints1;

		SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintEdge, 256, true> m_VolatileConstraintEdges2;
		//SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintTwin, 128, true> m_VolatileTwinConstraints2;
		SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintPoint, 256, true> m_VolatileConstraintPoints2;

		SharedPhysics::MemoryAllocator<C_ContactConstraint, 256, false> m_ContactConstraints1;

		SharedPhysics::MemoryAllocator<C_ContactConstraint, 256, false> m_ContactConstraints2;

		SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintEdge, 256, true>* m_pVolatileConstraintEdges;
		//SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintTwin, 128, true>* m_pVolatileTwinConstraints;
		SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintPoint, 256, true>* m_pVolatileConstraintPoints;

		SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintEdge, 256, true>* m_pPrevVolatileConstraintEdges;
		//SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintTwin, 128, true>* m_pPrevVolatileTwinConstraints;
		SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintPoint, 256, true>* m_pPrevVolatileConstraintPoints;

		SharedPhysics::MemoryAllocator<C_ContactConstraint, 256, false>* m_pContactConstraints;
		SharedPhysics::MemoryAllocator<C_ContactConstraint, 256, false>* m_pPrevContactConstraints;

		SharedPhysics::C_AllocationManager<Solver2::C_SphericalConstraint, 16, true> m_SphericalConstraints;
		SharedPhysics::C_AllocationManager<Solver2::C_RevoluteConstraint, 16, true> m_RevoluteConstraints;
		SharedPhysics::C_AllocationManager<Solver2::C_PrismaticConstraint, 16, true> m_PrismaticConstraint;
		SharedPhysics::C_AllocationManager<Solver2::C_PrismaticLimitConstraint, 16, true> m_PrismaticLimitConstraint;
		SharedPhysics::C_AllocationManager<Solver2::C_RevoluteLimitConstraint, 16, true> m_RevoluteLimitConstraint;
		SharedPhysics::C_AllocationManager<Solver2::C_FixedRotationalConstraint, 16, true> m_FixedConstraints;
		SharedPhysics::C_AllocationManager<Solver2::C_SphericalLimitConstraint, 16, true> m_SphericalLimitConstraints;
		SharedPhysics::C_AllocationManager<Solver2::C_DistanceConstraint, 16, true> m_DistanceConstraints;
		SharedPhysics::C_AllocationManager<Solver2::C_TwoRotationalConstraint, 16, true> m_TwoRotationalConstraints;
		SharedPhysics::C_AllocationManager<Solver2::C_CarWheelConstraint, 16, true> m_CarWheelConstraints;
		SharedPhysics::C_AllocationManager<Solver2::C_ProxyConstraint, 16, true> m_ProxyConstraints;

		SharedPhysics::C_MemoryIterator<SharedPhysics::C_ConstraintEdge, 256, true> m_Edges;
		//SharedPhysics::C_MemoryIterator<SharedPhysics::C_ConstraintTwin, 128, true> m_Twins;
		SharedPhysics::C_MemoryIterator<SharedPhysics::C_ConstraintPoint, 256, true> m_Points;
		SharedPhysics::C_MemoryIterator<C_ContactConstraint, 256, false> m_Constraints;


		SharedPhysics::SortedArray<C_GraphNodeWrapper> m_GraphNodeArray;
		SharedPhysics::SortedArray<C_GraphNodeWrapper> m_StaticGraphNodeArray;
		SharedPhysics::SortedArray<C_ConstraintEdgeWrapper> m_ConstraintEdgeArray;
		SharedPhysics::SortedArray<C_ConstraintPtr> m_DebugConstraints;
		SharedPhysics::C_MaterialPair* m_pMaterialMatrix;
		U32 m_NumMaterials;

		C_ConstraintGraphManager(SharedPhysics::C_MaterialPair* pMaterials = NULL, const U32 numMaterials = 0) : 
			m_pMaterialMatrix(pMaterials), m_NumMaterials(numMaterials), m_pVolatileConstraintEdges(&m_VolatileConstraintEdges1), 
			/*m_pVolatileTwinConstraints(&m_VolatileTwinConstraints1),*/ m_pVolatileConstraintPoints(&m_VolatileConstraintPoints1),
			m_pPrevVolatileConstraintEdges(&m_VolatileConstraintEdges2), /*m_pPrevVolatileTwinConstraints(&m_VolatileTwinConstraints2),*/ 
			m_pPrevVolatileConstraintPoints(&m_VolatileConstraintPoints2), m_pContactConstraints(&m_ContactConstraints1), 
			m_pPrevContactConstraints(&m_ContactConstraints2)
		{
		}

		NOALIAS __forceinline void SetMaterials(SharedPhysics::C_MaterialPair* pMaterials, const U32 numMaterials)
		{
			m_pMaterialMatrix = pMaterials;
			m_NumMaterials = numMaterials;
		}

		SharedPhysics::C_ConstraintEdge* GetConstraintEdge(SharedPhysics::C_GraphNode* pNodeA, SharedPhysics::C_GraphNode* pNodeB);

		NOALIAS __forceinline void FlipBuffers()
		{
			SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintEdge, 256, true>* tempCE(m_pVolatileConstraintEdges);
			m_pVolatileConstraintEdges = m_pPrevVolatileConstraintEdges;
			m_pPrevVolatileConstraintEdges = tempCE;

			/*SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintTwin, 128, true>* tempCT(m_pVolatileTwinConstraints);
			m_pVolatileTwinConstraints = m_pPrevVolatileTwinConstraints;
			m_pPrevVolatileTwinConstraints = tempCT;*/

			SharedPhysics::MemoryAllocator<SharedPhysics::C_ConstraintPoint, 256, true>* tempCP(m_pVolatileConstraintPoints);
			m_pVolatileConstraintPoints = m_pPrevVolatileConstraintPoints;
			m_pPrevVolatileConstraintPoints = tempCP;

			//Must be constructed/destructed otherwise the v-table on the constraint will be broken!
			SharedPhysics::MemoryAllocator<C_ContactConstraint, 256, false>* tempCC(m_pContactConstraints);
			m_pContactConstraints = m_pPrevContactConstraints;
			m_pPrevContactConstraints = tempCC;
		}

		void PrepareForNextFrame(Gravity::C_BodyList& activeBodies);

		NOALIAS __forceinline void InsertGraphNode(SharedPhysics::C_GraphNode* __restrict pNode) __restrict
		{
			C_GraphNodeWrapper wrapper(pNode);
			m_GraphNodeArray.Insert(&wrapper);
		}

		void RemoveGraphNode(SharedPhysics::C_GraphNode* __restrict pNode, Gravity::C_EventStream& m_EventStream, bool removeConstraints);

		NOALIAS __forceinline void InsertStaticGraphNode(SharedPhysics::C_GraphNode* __restrict pNode) __restrict
		{
			C_GraphNodeWrapper wrapper(pNode);
			m_StaticGraphNodeArray.Insert(&wrapper);
		}

		void RemoveStaticGraphNode(SharedPhysics::C_GraphNode* __restrict pNode);

		S32 SplitConstraintGroups(SharedPhysics::C_SafeStack<true>& stack, Gravity::C_BodyList& activeBodies, Gravity::C_BodyList& continuousBodies, Gravity::C_BodyList& kinematicBodies, const Maths::HF32 PCREF time);

		void MergeSmallConstraintGroups(Gravity::C_BodyList& activeBodies, Gravity::C_BodyList& continuousBodies, Gravity::C_BodyList& kinematicBodies, const S32 islandThresholdSize);

		S32 SplitConstraintGroupsPostStabilise(SharedPhysics::C_SafeStack<true>& stack);

		void BuildConstraintGraph(SharedPhysics::C_PersistentContactCollider* pColliderArray, const U32 numContacts,
			SharedPhysics::C_MemoryIterator<SharedPhysics::C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false>& meshContacts, const U32 numMeshContacts);

		void BuildClothConstraintGraph(SharedPhysics::C_ContactStream* pClothStream);

		void ClearLeftoverContacts(SharedPhysics::C_ContactDatabase & contactDatabase, Gravity::C_BodyList& wakingList);

		void PutToSleep(SharedPhysics::C_GraphNode* pNode, SharedPhysics::C_ContactDatabase& contactDatabase);

		SharedPhysics::C_MaterialPair& GetMaterialPair(const U32 matIndexA, const U32 matIndexB)
		{
//			static int errCount = 0;
//			if (matIndexA< 0 || matIndexA >= m_NumMaterials)
//			{
//#ifndef PLATFORM_SPU
//				if (errCount++ < 8)
//					LOGWARN(g_Log, "invalid material A index %d (material count %d)", matIndexA, m_NumMaterials);
//#endif
//				matIndexA = 0;
//			}	
//
//			if (matIndexB< 0 || matIndexB >= m_NumMaterials)
//			{
//#ifndef PLATFORM_SPU
//				if (errCount++ < 8)
//					LOGWARN(g_Log, "invalid material B index %d (material count %d)", matIndexB, m_NumMaterials);
//#endif
//				matIndexB = 0;
//			}	
//
//			return m_pMaterialMatrix[(matIndexA * m_NumMaterials) + matIndexB]; 

			BIZ_ASSERT((matIndexA < m_NumMaterials) & (matIndexB < m_NumMaterials));

			U32 indexA = Maths::Select(matIndexA>= m_NumMaterials, (U32)0, matIndexA);
			U32 indexB = Maths::Select(matIndexB>= m_NumMaterials, (U32)0, matIndexB);

			return m_pMaterialMatrix[(indexA * m_NumMaterials) + indexB];
		}

		void RemoveConstraints(SharedPhysics::C_Constraint* pConstraint);
		void RemoveDebugConstraints(SharedPhysics::C_Constraint* pConstraint);
		void AddDebugConstraints(SharedPhysics::C_Constraint* pConstraint);

		private:
			void RemoveConstraints(SharedPhysics::C_GraphNode* __restrict pNode, Gravity::C_EventStream& stream);
		bool ValidConstraintArray();
		void ReleaseConstraint(SharedPhysics::C_Constraint* pConstraint);
		
	};
};

#endif

