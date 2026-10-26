#include "StdAfx.h"
#include <system/perf.h>
#include "gravity/sdk/Scene.h"
#include "gravity/sdk/WaterQuery.h"
#include "gravity/sdk/Body.h"

#include "gravity/broadphase/Object.inl"
#include "gravity/broadphase/DistributionTree.h"

#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/SPUManager.h"
//#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/narrowphase/RayTriangle.inl"
#include "gravity/narrowphase/OptimisedAABBTree.inl"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/solver2/lcp.h"
#include "gravity/sdk/FluidEvents.h"
#include "gravity/sdk/KinematicController.h"
#include "gravity/sdk/AnimatedShapeController.h"
#include "gravity/sharedphysics/Integrator.h"

#include "gravity/sharedphysics/RigidBody.inl"
#include "system/perf/perfevent.h"

#if !defined(PLATFORM_SPU) || defined(SPU_FILL_CACHE)
#include "gravity/sdk/Scene_FillCache.inl"
#endif

#if !defined (PLATFORM_SPU) || defined (SPU_PHANTOM_MESH_LIB)
#include "gravity/sdk/CollisionPhantom.h"
#endif




#if (!defined(PLATFORM_SPU)) || defined(SPU_RAYTRACE)
JQ_DECLARE_DUAL_MODULE(RayTraceModule, Gravity::DeferredRayTrace, gravity_raytrace, "gravity.raytrace");
//JQ_DECLARE_PPU_MODULE(RayTraceModule, Gravity::DeferredRayTrace);
//JQ_DECLARE_SPU_MODULE(RayTraceModule2, Gravity::DeferredRayTrace, gravity_raytrace, "gravity.raytrace");


#endif

#if (!defined(PLATFORM_SPU)) || defined(SPU_TRANSPARENT_RAYTRACE)
JQ_DECLARE_DUAL_MODULE(TransparentRayTraceModule, Gravity::DeferredTransparentRayTrace, gravity_transparentraytrace, "gravity.transparentraytrace");
//JQ_DECLARE_PPU_MODULE(TransparentRayTraceModule, Gravity::DeferredTransparentRayTrace);
//JQ_DECLARE_SPU_MODULE(TransparentRayTraceModule2, Gravity::DeferredTransparentRayTrace, gravity_transparentraytrace, "gravity.transparentraytrace");
#endif

#if (!defined(PLATFORM_SPU)) || defined(SPU_WATER_QUERY)
JQ_DECLARE_DUAL_MODULE(WaterQueryModule, Gravity::DeferredWaterQuery, gravity_waterquery, "gravity.waterquery");
//JQ_DECLARE_PPU_MODULE(WaterQueryModule, Gravity::DeferredWaterQuery);
//JQ_DECLARE_SPU_MODULE(WaterQueryModule2, Gravity::DeferredWaterQuery, gravity_waterquery, "gravity.waterquery");
#endif


#if (!defined PLATFORM_SPU) || defined (SPU_MESH_COLLISION)
JQ_DECLARE_DUAL_MODULE(MeshCollisionModule, Gravity::CollideMesh, gravity_meshcollision, "gravity.meshcollision");
//JQ_DECLARE_PPU_MODULE(MeshCollisionModule, Gravity::CollideMesh);
//JQ_DECLARE_SPU_MODULE(MeshCollisionModule2, Gravity::CollideMesh, gravity_meshcollision, "gravity.meshcollision");
#endif

#if (!defined PLATFORM_SPU) || defined (SPU_MESH_DISPATCH)
JQ_DECLARE_PPU_MODULE(MeshCollisionDispatchModule, Gravity::DispatchMeshCollision);
#endif

#if (!defined PLATFORM_SPU) || defined (SPU_PHANTOM_MESH_COLLISION)
JQ_DECLARE_DUAL_MODULE(PhantomMeshCollisionModule, Gravity::CollidePhantomMesh, gravity_phantommeshcollision, "gravity.phantommeshcollision");
//JQ_DECLARE_PPU_MODULE(MeshCollisionModule, Gravity::CollideMesh);
//JQ_DECLARE_SPU_MODULE(MeshCollisionModule2, Gravity::CollideMesh, gravity_meshcollision, "gravity.meshcollision");
#endif

#if (!defined PLATFORM_SPU) || defined (SPU_GROUNDPLANE_COLLISION)
JQ_DECLARE_DUAL_MODULE(GroundPlaneCollisionModule, Gravity::CollideGroundPlane, gravity_groundplanecollision, "gravity.groundplanecollision");
//JQ_DECLARE_PPU_MODULE(GroundPlaneCollisionModule, Gravity::CollideGroundPlane);
//JQ_DECLARE_SPU_MODULE(GroundPlaneCollisionModule2, Gravity::CollideGroundPlane, gravity_groundplanecollision, "gravity.groundplanecollision");
#endif


#if (!defined PLATFORM_SPU) || defined (SPU_DYNAMIC_COLLISION)
JQ_DECLARE_DUAL_MODULE(DynamicCollisionModule, Gravity::CollideDynamic, gravity_dynamic_collision, "gravity.dynamic_collision");
//JQ_DECLARE_PPU_MODULE(DynamicCollisionModule, Gravity::CollideDynamic);
//JQ_DECLARE_SPU_MODULE(DynamicCollisionModule2, Gravity::CollideDynamic, gravity_dynamic_collision, "gravity.dynamic_collision");
#endif

#if (!defined PLATFORM_SPU) || defined (SPU_DYNAMIC_PHANTOM_COLLISION)
JQ_DECLARE_DUAL_MODULE(DynamicPhantomCollisionModule, Gravity::CollidePhantomDynamic, gravity_dynamic_phantom_collision, "gravity.dynamic_phantom_collision");
//JQ_DECLARE_PPU_MODULE(DynamicPhantomCollisionModule, Gravity::CollidePhantomDynamic);
//JQ_DECLARE_SPU_MODULE(DynamicPhantomCollisionModule2, Gravity::CollidePhantomDynamic, gravity_dynamic_phantom_collision, "gravity.dynamic_phantom_collision");
#endif

#if !defined(PLATFORM_SPU) || defined (SPU_FLUID_COLLISION)
JQ_DECLARE_DUAL_MODULE(FluidCollisionModule, Gravity::FluidCollision, gravity_fluid_collision, "gravity.fluidCollision");
//JQ_DECLARE_PPU_MODULE(FluidCollisionModule, Gravity::FluidCollision);
//JQ_DECLARE_SPU_MODULE(FluidCollisionModule2, Gravity::FluidCollision, gravity_fluid_collision, "gravity.fluidCollision");
#endif

#if (!defined PLATFORM_SPU) || defined (SPU_CONTINUOUS_COLLISION)
JQ_DECLARE_DUAL_MODULE(ContinuousCollisionModule, Gravity::DeferredContinuousCollisionDetection, gravity_continuous_collision, "gravity.continuous_collision");
//JQ_DECLARE_SPU_MODULE(ContinuousCollisionModule2, Gravity::DeferredContinuousCollisionDetection, gravity_continuous_collision, "gravity.continuous_collision");
//JQ_DECLARE_PPU_MODULE(ContinuousCollisionModule, Gravity::DeferredContinuousCollisionDetection );
#endif

#if ((!defined (PLATFORM_SPU)) || (defined (SPU_SOLVER)))
JQ_DECLARE_DUAL_MODULE(SolverModule, Gravity::Solve, gravity_solver, "gravity.solver");
//JQ_DECLARE_PPU_MODULE(SolverModule, Gravity::Solve);
//JQ_DECLARE_SPU_MODULE(SolverModule2, Gravity::Solve, gravity_solver, "gravity.solver");
#endif

#if ((!defined (PLATFORM_SPU)) || (defined (SPU_RAGDOLL_CONTROLLER)))
JQ_DECLARE_DUAL_MODULE(UpdateRagdollControllerModule, Gravity::UpdateRagdollController, gravity_ragdoll_controller, "gravity.ragdoll_controller");
//JQ_DECLARE_PPU_MODULE(UpdateRagdollControllerModule, Gravity::UpdateRagdollController);
//JQ_DECLARE_SPU_MODULE(UpdateRagdollControllerModule2, Gravity::UpdateRagdollController, gravity_ragdoll_controller, "gravity.ragdoll_controller");
#endif

#if ((!defined (PLATFORM_SPU)) || (defined (SPU_POST_INTEGRATE)))
JQ_DECLARE_DUAL_MODULE(PostIntegrateModule, Gravity::PostIntegrate, gravity_post_integrate, "gravity.post_integrate");
//JQ_DECLARE_PPU_MODULE(PostIntegrateModule, Gravity::PostIntegrate);
//JQ_DECLARE_SPU_MODULE(PostIntegrateModule2, Gravity::PostIntegrate, gravity_post_integrate, "gravity.post_integrate");
#endif

#if ((!defined (PLATFORM_SPU)) || (defined (SPU_KINEMATIC_UPDATE)))
JQ_DECLARE_DUAL_MODULE(KinematicControllerModule, Gravity::KinematicControllerUpdate, gravity_kinematic, "gravity.kinematic");
//JQ_DECLARE_PPU_MODULE(KinematicControllerModule, Gravity::KinematicControllerUpdate);
//JQ_DECLARE_SPU_MODULE(KinematicControllerModule2, Gravity::KinematicControllerUpdate, gravity_kinematic, "gravity.kinematic");

#endif

#if ((!defined (PLATFORM_SPU)) || (defined (SPU_INTEGRATE)))
JQ_DECLARE_DUAL_MODULE(IntegrateModule, Gravity::IntegrateBodies, gravity_integrate, "gravity.integrate");
//JQ_DECLARE_PPU_MODULE(IntegrateModule, Gravity::IntegrateBodies);
//JQ_DECLARE_SPU_MODULE(IntegrateModule2, Gravity::IntegrateBodies, gravity_integrate, "gravity.integrate");
#endif 


#if ((!defined (PLATFORM_SPU)) || (defined (SPU_ANIMATED_CONTROLLER)))
JQ_DECLARE_DUAL_MODULE(AnimatedShapeControllerModule, Gravity::AnimatedShapeControllerUpdate, gravity_animated_controller, "gravity.animated_controller");
//JQ_DECLARE_PPU_MODULE(AnimatedShapeControllerModule, Gravity::AnimatedShapeControllerUpdate);
//JQ_DECLARE_SPU_MODULE(AnimatedShapeControllerModule, Gravity::AnimatedShapeControllerUpdate, gravity_animated_controller, "gravity.animated_controller");

#endif


#if ((!defined (PLATFORM_SPU)) || (defined (SPU_BROAD_PHASE)))
JQ_DECLARE_PPU_MODULE(BroadPhaseModule, Gravity::DoBroadPhase);
#endif

#if ((!defined (PLATFORM_SPU)) || (defined (SPU_BROAD_PHASE)))
JQ_DECLARE_PPU_MODULE(WakeUpBroadPhaseModule, Gravity::DoWakeUpBroadPhase);
#endif

#if ((!defined (PLATFORM_SPU)) || (defined (SPU_CLOTH_COLLISION)))
JQ_DECLARE_PPU_MODULE(ClothCollisionModule, Gravity::DoClothCollision);
#endif




namespace Gravity
{

#if ((!defined (PLATFORM_SPU)) || (defined (SPU_CLOTH_COLLISION)))
	void DoClothCollision(Scheduler::Task* pTask)
	{

		PERF_EVENT("Gravity.DoClothCollision");
		Spu::MemBaseMarker marker;
		//Do something in here...
		const U32 size = 512;
#ifndef PLATFORM_SPU
		void* data[size];
#else
		void* data = Spu::Alloc(size * sizeof(void*));
#endif
		SharedPhysics::C_SafeStack<true> stack(data, size);
		C_ClothCollisionQuery* pQuery = (C_ClothCollisionQuery*)&pTask->ParamData[0];
		for(U32 a = 0; a < pQuery->m_Size; ++a)
		{
			Spu::MemBaseMarker innerMarker;
			Gravity::C_Object* pDynamicObject = (Gravity::C_Object*)Spu::FetchAllocAsync(pQuery->m_pDynamicObject[a], sizeof(C_Object), 2);
			Gravity::C_Object* pClothObject = (Gravity::C_Object*)Spu::FetchAllocAsync(pQuery->m_pClothObject[a], sizeof(C_Object) + sizeof(SharedPhysics::C_GraphNode), 2);
			Spu::Wait(2);
			Collision::C_CollisionObject obj = pDynamicObject->CollisionObject();
			SharedPhysics::C_GraphNode* pClothNode = (SharedPhysics::C_GraphNode*)(pClothObject + 1);

			U32 clothType = Spu::Get((U32*)&pClothObject->m_pShape->m_ShapeType);

			if(clothType == Collision::E_Cloth_Shape)
			{
				Collision::C_ClothShape* pClothShape = (Collision::C_ClothShape*)Spu::FetchAlloc(pClothObject->m_pShape, sizeof(Collision::C_ClothShape));
				pQuery->m_pManager->CollideClothDynamic(&obj, pClothNode, pClothShape, stack, *pQuery->m_pStream);
			}
			else if (clothType == Collision::E_Rope_Shape)
			{
				//Do rope stuff here!!!
			}
		}
	}

#endif

#if ((!defined (PLATFORM_SPU)) || (defined (SPU_BROAD_PHASE)))


	inline void C_Scene::GenerateCollisionTasksCloth(C_ObjectWrapper* pList, C_Object& obj, C_ClothCollisionQuery& query,
		C_DistributionNode* pNode)
	{
		while(pList)
		{
			C_Object* pObj(pList->m_pObject);

			if(pObj->m_TransformedBox.IntersectsInAxisBox(obj.m_TransformedBox, pNode->m_bbox) && 
				obj.m_pShape->CanCollide(pObj->m_pShape))
			{
				query.m_pClothObject[query.m_Size] = &obj;
				query.m_pDynamicObject[query.m_Size++] = pObj;
				if(query.m_Size == ClothCollisionSize)
				{
					Scheduler::AddTask(ClothCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_ClothCollisionQuery));				
					query.m_Size = 0;
				}
			}
			pList = pList->m_pNext;
		}
	}

	inline void C_Scene::GenerateCollisionTasksCloth2(C_ObjectWrapper* pList, C_Object& obj, C_ClothCollisionQuery& query,
		C_DistributionNode* pNode)
	{
		while(pList)
		{
			C_Object* pObj(pList->m_pObject);

			if(pObj->m_TransformedBox.IntersectsInAxisBox(obj.m_TransformedBox, pNode->m_bbox) && 
				obj.m_pShape->CanCollide(pObj->m_pShape))
			{
				query.m_pClothObject[query.m_Size] = pObj;
				query.m_pDynamicObject[query.m_Size++] = &obj;
				if(query.m_Size == ClothCollisionSize)
				{
					Scheduler::AddTask(ClothCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_ClothCollisionQuery));				
					query.m_Size = 0;
				}
			}
			pList = pList->m_pNext;
		}
	}

	inline void C_Scene::GenerateCollisionTasks(C_ObjectWrapper* pList, C_Object& obj, C_CollisionQuery& query,
		C_DistributionNode* pNode, const U32& maxCount)
	{
		while(pList)
		{
			C_Object* pObj(pList->m_pObject);

			if(pObj->m_TransformedBox.IntersectsInAxisBox(obj.m_TransformedBox, pNode->m_bbox) && 
				obj.m_pShape->CanCollide(pObj->m_pShape))
			{
				query.m_pColliders[query.m_NumPairs++] = m_pCollisionManager->CreateCollider(obj.m_pGraphNode, pObj->m_pGraphNode);
				if(query.m_NumPairs == maxCount)
				{
					Scheduler::AddTask(DynamicCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
					query.m_NumPairs = 0;
				}				
			}
			pList = pList->m_pNext;
		}
	}

	inline void C_Scene::GenerateCollisionTasksKinematic(C_ObjectWrapper* pList, C_Object& obj, C_CollisionQuery& query,
		C_DistributionNode* pNode, const U32& maxCount)
	{
		while(pList)
		{
			C_Object* pObj(pList->m_pObject);

			if(pObj->m_TransformedBox.IntersectsInAxisBox(obj.m_TransformedBox, pNode->m_bbox) && 
				obj.m_pShape->CanCollide(pObj->m_pShape))
			{
				query.m_pColliders[query.m_NumPairs++] = m_pCollisionManager->CreateCollider(pObj->m_pGraphNode, obj.m_pGraphNode);
				if(query.m_NumPairs == maxCount)
				{
					Scheduler::AddTask(DynamicCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
					query.m_NumPairs = 0;
				}				
			}
			pList = pList->m_pNext;
		}
	}


	void DoWakeUpBroadPhase(Scheduler::Task* pTask)
	{
		PERF_EVENT("Gravity.WakeUpBroadPhaseTask");

		void* data[512];
		SharedPhysics::C_SafeStack<true> stack(data, 512);
		BroadPhaseTask* pBroadPhaseTask = (BroadPhaseTask*)&pTask->ParamData[0];
		
		C_CollisionQuery query;
		//Maths::Store(time, &query.m_Time);
		query.m_pStream = pBroadPhaseTask->m_pStream;
		query.m_pManager = pBroadPhaseTask->m_pManager;
		query.m_NumPairs = 0;

		C_FluidQuery fluidQuery;
		fluidQuery.m_Size = 0;
		fluidQuery.m_pManager = pBroadPhaseTask->m_pManager;
		fluidQuery.m_pFluidBuffer = pBroadPhaseTask->m_pFluidBuffer;

		C_ClothCollisionQuery clothQuery;
		clothQuery.m_pManager = pBroadPhaseTask->m_pManager;
		clothQuery.m_pStream = pBroadPhaseTask->m_pClothStream;
		clothQuery.m_Size = 0;

		const U32 dynamicCollisionCount = MaxCollisionQuery;
		const U32 fluidCollisionCount = MaxFluidQuery;

		Scheduler::GroupHandle group = pBroadPhaseTask->m_pScene->GetInternalGroupHandle();
		Scheduler::PolicyHandle policy = pBroadPhaseTask->m_pScene->GetInternalTaskPolicy();

		for(U32 a = 0; a < pBroadPhaseTask->m_TaskSize; ++a)
		{
			//Gravity::C_DistributionNode* pNode = pBroadPhaseTask->m_Nodes[a];
			Gravity::C_Body* pBody = pBroadPhaseTask->m_pBodies[a];
			Gravity::C_Object* pObj = pBody->GetObject();
			/*clothQuery.m_pDynamicObject = pObj;
			clothQuery.m_pClothObject = pObj;*/

			if(pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris))
			{
				//Debris wake-up
				pBroadPhaseTask->m_pScene->DebrisWakeUpComparison(pObj, query, dynamicCollisionCount, fluidQuery,
					fluidCollisionCount, stack);
				
			}
			else if (pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Cloth))
			{
				//Cloth wake-up!!!
				pBroadPhaseTask->m_pScene->WakeUpCloth(pObj, clothQuery, stack);
			}
			else if (pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Rope))
			{
				//Rope wake-up!!!
			}
			else if (!pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			{
				//Normal object wake-up
				pBroadPhaseTask->m_pScene->WakeUpComparison(pObj, 
					query, dynamicCollisionCount, fluidQuery, fluidCollisionCount, clothQuery, stack);
			}
		}

		if(fluidQuery.m_Size > 0)
		{
			Scheduler::AddTask(FluidCollisionModule, NULL, NULL, group, policy, &fluidQuery, sizeof(C_FluidQuery)); 
		}

		if(query.m_NumPairs > 0)
		{
			Scheduler::AddTask(DynamicCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_CollisionQuery)); 
		}

		if(clothQuery.m_Size > 0)
		{
			Scheduler::AddTask(ClothCollisionModule, NULL, NULL, group, policy, &clothQuery, sizeof(C_ClothCollisionQuery));
		}
	}


	void DoBroadPhase(Scheduler::Task* pTask)
	{
		PERF_EVENT("Gravity.BroadPhaseTask");

		void* data[512];
		SharedPhysics::C_SafeStack<true> stack(data, 512);
		BroadPhaseTask* pBroadPhaseTask = (BroadPhaseTask*)&pTask->ParamData[0];
		
		C_CollisionQuery query;
		//Maths::Store(time, &query.m_Time);
		query.m_pStream = pBroadPhaseTask->m_pStream;
		query.m_pManager = pBroadPhaseTask->m_pManager;
		query.m_NumPairs = 0;

		C_FluidQuery fluidQuery;
		fluidQuery.m_Size = 0;
		fluidQuery.m_pManager = pBroadPhaseTask->m_pManager;
		fluidQuery.m_pFluidBuffer = pBroadPhaseTask->m_pFluidBuffer;

		C_ClothCollisionQuery clothQuery;
		clothQuery.m_pManager = pBroadPhaseTask->m_pManager;
		clothQuery.m_pStream = pBroadPhaseTask->m_pClothStream;
		clothQuery.m_Size = 0;
		

		const U32 dynamicCollisionCount = MaxCollisionQuery;
		const U32 fluidCollisionCount = MaxFluidQuery;

		Scheduler::GroupHandle group = pBroadPhaseTask->m_pScene->GetInternalGroupHandle();
		Scheduler::PolicyHandle policy = pBroadPhaseTask->m_pScene->GetInternalTaskPolicy();

		for(U32 a = 0; a < pBroadPhaseTask->m_TaskSize; ++a)
		{
			//Gravity::C_DistributionNode* pNode = pBroadPhaseTask->m_Nodes[a];
			Gravity::C_Body* pBody = pBroadPhaseTask->m_pBodies[a];
			Gravity::C_Object* pObj = pBody->GetObject();
			/*clothQuery.m_pClothObject = pObj;
			clothQuery.m_pDynamicObject = pObj;*/

			if(pObj->m_ObjectState == Gravity::E_ActiveObject)
			{
				pBroadPhaseTask->m_pScene->ActiveObjectComparison(pObj, 
					query, dynamicCollisionCount, fluidQuery, fluidCollisionCount, clothQuery, stack);
			}
			else if (pObj->m_ObjectState == Gravity::E_KinematicObject)
			{
				pBroadPhaseTask->m_pScene->KinematicObjectComparison(pObj, 
					query, dynamicCollisionCount, fluidQuery, fluidCollisionCount, clothQuery, stack);
			}
			else if (pObj->m_ObjectState == Gravity::E_ActiveDebrisObject)
			{
				pBroadPhaseTask->m_pScene->DebrisComparison(pObj, query, dynamicCollisionCount, fluidQuery,
					fluidCollisionCount, stack);
			}
			else if(pObj->m_ObjectState == Gravity::E_ActiveClothObject)
			{
				pBroadPhaseTask->m_pScene->ClothComparison(pObj, clothQuery, stack);
			}
		}

		if(fluidQuery.m_Size > 0)
		{
			Scheduler::AddTask(FluidCollisionModule, NULL, NULL, group, policy, &fluidQuery, sizeof(C_FluidQuery)); 
		}

		if(query.m_NumPairs > 0)
		{
			Scheduler::AddTask(DynamicCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_CollisionQuery)); 
		}

		if(clothQuery.m_Size > 0)
		{
			Scheduler::AddTask(ClothCollisionModule, NULL, NULL, group, policy, &clothQuery, sizeof(C_ClothCollisionQuery));
		}
	}

	void C_Scene::WakeUpCloth(Gravity::C_Object* pObj, C_ClothCollisionQuery& clothQuery, SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.WakeUpClothComparison");
		for(U32 a = 0; a < pObj->m_NumNodesContained; ++a)
		{
			Gravity::C_ObjectWrapper& wrapper = pObj->m_Nodes[a];
			Gravity::C_DistributionNode* pNode = wrapper.m_pNode;
			//OK. We're active, so compare with all the active objects after me...
			if(pNode->m_pSleepingObjects || pNode->m_pStaticObjects)
			{
				//PERF_EVENT("Gravity.ActiveNodeComparison");
				GenerateCollisionTasksCloth(pNode->m_pSleepingObjects, *pObj, clothQuery, pNode);
				GenerateCollisionTasksCloth(pNode->m_pStaticObjects, *pObj, clothQuery, pNode);
			}

			//Now compare with parents + children...
			{
				//PERF_EVENT("Gravity.ActiveParentComparison");
				C_DistributionNode* pParent(pNode->m_pParentNode);
				while(pParent)
				{
					if(pParent->m_pSleepingObjects || pParent->m_pStaticObjects)
					{
						//PERF_EVENT("Gravity.ActiveNodeComparison");
						GenerateCollisionTasksCloth(pParent->m_pSleepingObjects, *pObj, clothQuery, pNode);
						GenerateCollisionTasksCloth(pParent->m_pStaticObjects, *pObj, clothQuery, pNode);
					}
					pParent = pParent->m_pParentNode;
				}
			}

			if(pNode->m_pLeftNode)
			{
				//PERF_EVENT("Gravity.ActiveChildComparison");
				S32 size = 2;
				C_DistributionNode* pStackTop = pNode->m_pLeftNode;
				stack.Push(pNode->m_pRightNode, 0);
				stack.Push(pNode->m_pLeftNode, 1);

				while(size--)
				{
					C_DistributionNode* pChild = pStackTop;
					pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
					if(pChild->m_bbox.Intersects(pObj->m_TransformedBox))
					{
						if(pChild->m_pSleepingObjects || pChild->m_pStaticObjects)
						{
							GenerateCollisionTasksCloth(pChild->m_pSleepingObjects, *pObj, clothQuery, pChild);
							GenerateCollisionTasksCloth(pChild->m_pStaticObjects, *pObj, clothQuery, pChild);
						}
						//Fluid comparisons
						if(pChild->m_pLeftNode)
						{
							pStackTop = pChild->m_pLeftNode;
							stack.Push(pChild->m_pRightNode, size);
							size += 2;
							//stack.Push(pChild->m_pLeftNode, size++);
						}
					}
				}
			}
		}
	}


	void C_Scene::WakeUpComparison(Gravity::C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, C_ClothCollisionQuery& clothQuery, SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.WakeUpComparison");
		for(U32 a = 0; a < pObj->m_NumNodesContained; ++a)
		{
			Gravity::C_ObjectWrapper& wrapper = pObj->m_Nodes[a];
			Gravity::C_DistributionNode* pNode = wrapper.m_pNode;
			//OK. We're active, so compare with all the active objects after me...
			if(pNode->m_pSleepingDebrisObjects || pNode->m_pSleepingObjects || pNode->m_pStaticObjects || pNode->m_pFluidMeshObjects
				|| pNode->m_pSleepingClothObjects)
			{
				//PERF_EVENT("Gravity.ActiveNodeComparison");
				GenerateCollisionTasks(pNode->m_pSleepingDebrisObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pStaticObjects, *pObj, query, pNode, dynamicCollisionCount);
				//Fluid comparisons
				GenerateFluidTasks(pNode->m_pFluidMeshObjects, *pObj, fluidQuery, pNode, fluidCollisionCount);
				//Cloth
				GenerateCollisionTasksCloth2(pNode->m_pSleepingClothObjects, *pObj, clothQuery, pNode);
			}

			//Now compare with parents + children...
			{
				//PERF_EVENT("Gravity.ActiveParentComparison");
				C_DistributionNode* pParent(pNode->m_pParentNode);
				while(pParent)
				{
					if(pParent->m_pSleepingObjects || pParent->m_pStaticObjects || 
						pParent->m_pFluidMeshObjects || pParent->m_pSleepingDebrisObjects || pParent->m_pSleepingClothObjects)
					{
						GenerateCollisionTasks(pParent->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pSleepingDebrisObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pStaticObjects, *pObj, query, pNode, dynamicCollisionCount);
						//Fluid comparisons
						GenerateFluidTasks(pParent->m_pFluidMeshObjects, *pObj, fluidQuery, pNode, fluidCollisionCount);
						//Cloth
						GenerateCollisionTasksCloth2(pParent->m_pSleepingClothObjects, *pObj, clothQuery, pNode);
					}
					pParent = pParent->m_pParentNode;
				}
			}

			if(pNode->m_pLeftNode)
			{
				//PERF_EVENT("Gravity.ActiveChildComparison");
				S32 size = 2;
				C_DistributionNode* pStackTop = pNode->m_pLeftNode;
				stack.Push(pNode->m_pRightNode, 0);
				stack.Push(pNode->m_pLeftNode, 1);

				while(size--)
				{
					C_DistributionNode* pChild = pStackTop;
					pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
					if(pChild->m_bbox.Intersects(pObj->m_TransformedBox))
					{
						if(pChild->m_pSleepingObjects || pChild->m_pStaticObjects || pChild->m_pFluidMeshObjects
							|| pChild->m_pSleepingDebrisObjects || pChild->m_pSleepingClothObjects)
						{
							GenerateCollisionTasks(pChild->m_pSleepingDebrisObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateCollisionTasks(pChild->m_pSleepingObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateCollisionTasks(pChild->m_pStaticObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateFluidTasks(pChild->m_pFluidMeshObjects, *pObj, fluidQuery, pChild, fluidCollisionCount);
							GenerateCollisionTasksCloth2(pChild->m_pSleepingClothObjects, *pObj, clothQuery, pChild);
						}
						//Fluid comparisons
						if(pChild->m_pLeftNode)
						{
							pStackTop = pChild->m_pLeftNode;
							stack.Push(pChild->m_pRightNode, size);
							size += 2;
							//stack.Push(pChild->m_pLeftNode, size++);
						}
					}
				}
			}
		}
	}

	void C_Scene::DebrisWakeUpComparison(Gravity::C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.WakeUpDebrisComparison");
		for(U32 a = 0; a < pObj->m_NumNodesContained; ++a)
		{
			Gravity::C_ObjectWrapper& wrapper = pObj->m_Nodes[a];
			Gravity::C_DistributionNode* pNode = wrapper.m_pNode;
			//OK. We're active, so compare with all the active objects after me...
			if(pNode->m_pSleepingObjects || pNode->m_pStaticObjects || pNode->m_pFluidMeshObjects)
			{
				//PERF_EVENT("Gravity.ActiveNodeComparison");
				GenerateCollisionTasks(pNode->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pStaticObjects, *pObj, query, pNode, dynamicCollisionCount);
				//Fluid comparisons
				GenerateFluidTasks(pNode->m_pFluidMeshObjects, *pObj, fluidQuery, pNode, fluidCollisionCount);
			}

			//Now compare with parents + children...
			{
				//PERF_EVENT("Gravity.ActiveParentComparison");
				C_DistributionNode* pParent(pNode->m_pParentNode);
				while(pParent)
				{
					if(pParent->m_pSleepingObjects || pParent->m_pStaticObjects || 
						pParent->m_pFluidMeshObjects)
					{
						GenerateCollisionTasks(pParent->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pStaticObjects, *pObj, query, pNode, dynamicCollisionCount);
						//Fluid comparisons
						GenerateFluidTasks(pParent->m_pFluidMeshObjects, *pObj, fluidQuery, pNode, fluidCollisionCount);
					}
					pParent = pParent->m_pParentNode;
				}
			}

			if(pNode->m_pLeftNode)
			{
				//PERF_EVENT("Gravity.ActiveChildComparison");
				S32 size = 2;
				C_DistributionNode* pStackTop = pNode->m_pLeftNode;
				stack.Push(pNode->m_pRightNode, 0);
				stack.Push(pNode->m_pLeftNode, 1);

				while(size--)
				{
					C_DistributionNode* pChild = pStackTop;
					pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
					if(pChild->m_bbox.Intersects(pObj->m_TransformedBox))
					{
						if(pChild->m_pSleepingObjects || pChild->m_pStaticObjects || pChild->m_pFluidMeshObjects)
						{
							GenerateCollisionTasks(pChild->m_pSleepingObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateCollisionTasks(pChild->m_pStaticObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateFluidTasks(pChild->m_pFluidMeshObjects, *pObj, fluidQuery, pChild, fluidCollisionCount);
						}
						//Fluid comparisons
						if(pChild->m_pLeftNode)
						{
							pStackTop = pChild->m_pLeftNode;
							stack.Push(pChild->m_pRightNode, size);
							size += 2;
							//stack.Push(pChild->m_pLeftNode, size++);
						}
					}
				}
			}
		}
	}


	void C_Scene::ActiveObjectComparison(Gravity::C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, C_ClothCollisionQuery& clothQuery, SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.ActiveObjectComparison");
		for(U32 a = 0; a < pObj->m_NumNodesContained; ++a)
		{
			Gravity::C_ObjectWrapper& wrapper = pObj->m_Nodes[a];
			Gravity::C_DistributionNode* pNode = wrapper.m_pNode;
			//OK. We're active, so compare with all the active objects after me...
			if(wrapper.m_pNext || pNode->m_pSleepingDebrisObjects || pNode->m_pSleepingObjects || pNode->m_pStaticObjects || pNode->m_pFluidMeshObjects
				|| pNode->m_pSleepingClothObjects)
			{
				//PERF_EVENT("Gravity.ActiveNodeComparison");
				GenerateCollisionTasks(wrapper.m_pNext, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pSleepingDebrisObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pStaticObjects, *pObj, query, pNode, dynamicCollisionCount);
				//Fluid comparisons
				GenerateFluidTasks(pNode->m_pFluidMeshObjects, *pObj, fluidQuery, pNode, fluidCollisionCount);
				//Cloth comparisons!
				GenerateCollisionTasksCloth2(pNode->m_pSleepingClothObjects, *pObj, clothQuery, pNode);
			}

			//Now compare with parents + children...

			{
				//PERF_EVENT("Gravity.ActiveParentComparison");
				C_DistributionNode* pParent(pNode->m_pParentNode);
				while(pParent)
				{
					if(pParent->m_pSleepingObjects || pParent->m_pStaticObjects || 
						pParent->m_pActiveObjects || pParent->m_pFluidMeshObjects || 
						pParent->m_pSleepingDebrisObjects || pParent->m_pSleepingClothObjects)
					{
						GenerateCollisionTasks(pParent->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pSleepingDebrisObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pStaticObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pActiveObjects, *pObj, query, pNode, dynamicCollisionCount);
						//Fluid comparisons
						GenerateFluidTasks(pParent->m_pFluidMeshObjects, *pObj, fluidQuery, pNode, fluidCollisionCount);
						//Cloth
						GenerateCollisionTasksCloth2(pParent->m_pSleepingClothObjects, *pObj, clothQuery, pNode);
					}
					pParent = pParent->m_pParentNode;
				}
			}

			if(pNode->m_pLeftNode)
			{
				//PERF_EVENT("Gravity.ActiveChildComparison");
				S32 size = 2;
				C_DistributionNode* pStackTop = pNode->m_pLeftNode;
				stack.Push(pNode->m_pRightNode, 0);
				stack.Push(pNode->m_pLeftNode, 1);

				while(size--)
				{
					C_DistributionNode* pChild = pStackTop;
					pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
					if(pChild->m_bbox.Intersects(pObj->m_TransformedBox))
					{
						if(pChild->m_pSleepingObjects || pChild->m_pStaticObjects || pChild->m_pFluidMeshObjects
							|| pChild->m_pSleepingDebrisObjects || pChild->m_pSleepingClothObjects)
						{
							GenerateCollisionTasks(pChild->m_pSleepingDebrisObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateCollisionTasks(pChild->m_pSleepingObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateCollisionTasks(pChild->m_pStaticObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateFluidTasks(pChild->m_pFluidMeshObjects, *pObj, fluidQuery, pChild, fluidCollisionCount);
							//Cloth
							GenerateCollisionTasksCloth2(pChild->m_pSleepingClothObjects, *pObj, clothQuery, pChild);
						}
						//Fluid comparisons
						if(pChild->m_pLeftNode)
						{
							pStackTop = pChild->m_pLeftNode;
							stack.Push(pChild->m_pRightNode, size);
							size += 2;
							//stack.Push(pChild->m_pLeftNode, size++);
						}
					}
				}
			}
		}
	}

	void C_Scene::ClothComparison(Gravity::C_Object* pObj, C_ClothCollisionQuery& clothQuery, SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.ActiveClothComparison");
		
		for(U32 a = 0; a < pObj->m_NumNodesContained; ++a)
		{
			Gravity::C_ObjectWrapper& wrapper = pObj->m_Nodes[a];
			Gravity::C_DistributionNode* pNode = wrapper.m_pNode;
			GenerateCollisionTasksCloth(pNode->m_pActiveObjects, *pObj, clothQuery, pNode);
			GenerateCollisionTasksCloth(pNode->m_pSleepingObjects, *pObj, clothQuery, pNode);
			GenerateCollisionTasksCloth(pNode->m_pStaticObjects, *pObj, clothQuery, pNode);
			GenerateCollisionTasksCloth(pNode->m_pKinematicObjects, *pObj, clothQuery, pNode);

			//Parents!!!

			C_DistributionNode* pParent(pNode->m_pParentNode);
			while(pParent)
			{
				if(pParent->m_pActiveObjects || pParent->m_pSleepingObjects || pParent->m_pStaticObjects || 
					pParent->m_pKinematicObjects)
				{
					GenerateCollisionTasksCloth(pParent->m_pActiveObjects, *pObj, clothQuery, pNode);
					GenerateCollisionTasksCloth(pParent->m_pSleepingObjects, *pObj, clothQuery, pNode);
					GenerateCollisionTasksCloth(pParent->m_pStaticObjects, *pObj, clothQuery, pNode);
					GenerateCollisionTasksCloth(pParent->m_pKinematicObjects, *pObj, clothQuery, pNode);
				}
				pParent = pParent->m_pParentNode;
			}

			if(pNode->m_pLeftNode)
			{
				//PERF_EVENT("Gravity.ActiveChildComparison");
				S32 size = 2;
				C_DistributionNode* pStackTop = pNode->m_pLeftNode;
				stack.Push(pNode->m_pRightNode, 0);
				stack.Push(pNode->m_pLeftNode, 1);

				while(size--)
				{
					C_DistributionNode* pChild = pStackTop;
					pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
					if(pChild->m_bbox.Intersects(pObj->m_TransformedBox))
					{
						if(pChild->m_pActiveObjects || pChild->m_pSleepingObjects || pChild->m_pStaticObjects || pChild->m_pKinematicObjects)
						{
							GenerateCollisionTasksCloth(pChild->m_pActiveObjects, *pObj, clothQuery, pChild);
							GenerateCollisionTasksCloth(pChild->m_pSleepingObjects, *pObj, clothQuery, pChild);
							GenerateCollisionTasksCloth(pChild->m_pStaticObjects, *pObj, clothQuery, pChild);
							GenerateCollisionTasksCloth(pChild->m_pKinematicObjects, *pObj, clothQuery, pChild);
						}
						//Fluid comparisons
						if(pChild->m_pLeftNode)
						{
							pStackTop = pChild->m_pLeftNode;
							stack.Push(pChild->m_pRightNode, size);
							size += 2;
							//stack.Push(pChild->m_pLeftNode, size++);
						}
					}
				}
			}


		}

	}

	void C_Scene::DebrisComparison(Gravity::C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.ActiveObjectComparison");
		for(U32 a = 0; a < pObj->m_NumNodesContained; ++a)
		{
			Gravity::C_ObjectWrapper& wrapper = pObj->m_Nodes[a];
			Gravity::C_DistributionNode* pNode = wrapper.m_pNode;
			//OK. We're active, so compare with all the active objects after me...
			if(pNode->m_pActiveObjects || pNode->m_pSleepingObjects || pNode->m_pStaticObjects || pNode->m_pFluidMeshObjects)
			{
				//PERF_EVENT("Gravity.ActiveNodeComparison");
				GenerateCollisionTasks(pNode->m_pActiveObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pStaticObjects, *pObj, query, pNode, dynamicCollisionCount);
				//Fluid comparisons
				GenerateFluidTasks(pNode->m_pFluidMeshObjects, *pObj, fluidQuery, pNode, fluidCollisionCount);
			}

			//Now compare with parents + children...

			{
				//PERF_EVENT("Gravity.ActiveParentComparison");
				C_DistributionNode* pParent(pNode->m_pParentNode);
				while(pParent)
				{
					if(pParent->m_pSleepingObjects || pParent->m_pStaticObjects || 
						pParent->m_pActiveObjects || pParent->m_pFluidMeshObjects)
					{
						GenerateCollisionTasks(pParent->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pStaticObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pActiveObjects, *pObj, query, pNode, dynamicCollisionCount);
						//Fluid comparisons
						GenerateFluidTasks(pParent->m_pFluidMeshObjects, *pObj, fluidQuery, pNode, fluidCollisionCount);
					}
					pParent = pParent->m_pParentNode;
				}
			}

			if(pNode->m_pLeftNode)
			{
				//PERF_EVENT("Gravity.ActiveChildComparison");
				S32 size = 2;
				C_DistributionNode* pStackTop = pNode->m_pLeftNode;
				stack.Push(pNode->m_pRightNode, 0);
				stack.Push(pNode->m_pLeftNode, 1);

				while(size--)
				{
					C_DistributionNode* pChild = pStackTop;
					pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
					if(pChild->m_bbox.Intersects(pObj->m_TransformedBox))
					{
						if(pChild->m_pActiveObjects || pChild->m_pSleepingObjects || pChild->m_pStaticObjects || pChild->m_pFluidMeshObjects)
						{
							GenerateCollisionTasks(pChild->m_pActiveObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateCollisionTasks(pChild->m_pSleepingObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateCollisionTasks(pChild->m_pStaticObjects, *pObj, query, pChild, dynamicCollisionCount);
							GenerateFluidTasks(pChild->m_pFluidMeshObjects, *pObj, fluidQuery, pChild, fluidCollisionCount);
						}
						//Fluid comparisons
						if(pChild->m_pLeftNode)
						{
							pStackTop = pChild->m_pLeftNode;
							stack.Push(pChild->m_pRightNode, size);
							size += 2;
							//stack.Push(pChild->m_pLeftNode, size++);
						}
					}
				}
			}
		}
	}

	void C_Scene::KinematicObjectComparison(C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, C_ClothCollisionQuery& clothQuery, SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.KinematicObjectComparison");
		for(U32 a = 0; a < pObj->m_NumNodesContained; ++a)
		{
			Gravity::C_ObjectWrapper& wrapper = pObj->m_Nodes[a];
			Gravity::C_DistributionNode* pNode = wrapper.m_pNode;
			//OK. We're kinematic, so compare with all the sleeping objects...
			if(pNode->m_pSleepingObjects || pNode->m_pActiveObjects || pNode->m_pSleepingDebrisObjects || pNode->m_pActiveDebrisObjects
				|| pNode->m_pSleepingClothObjects)
			{
				GenerateCollisionTasksKinematic(pNode->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pActiveObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasks(pNode->m_pActiveDebrisObjects, *pObj, query, pNode, dynamicCollisionCount);
				GenerateCollisionTasksKinematic(pNode->m_pSleepingDebrisObjects, *pObj, query, pNode, dynamicCollisionCount);

				//Cloth
				GenerateCollisionTasksCloth2(pNode->m_pSleepingClothObjects, *pObj, clothQuery, pNode);
			}

			//Now compare with parents + children...

			{
				C_DistributionNode* pParent(pNode->m_pParentNode);
				while(pParent)
				{
					if(pParent->m_pSleepingObjects || pParent->m_pActiveObjects || pParent->m_pSleepingDebrisObjects || pParent->m_pActiveDebrisObjects
						|| pParent->m_pSleepingClothObjects)
					{
						GenerateCollisionTasksKinematic(pParent->m_pSleepingObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pActiveObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasks(pParent->m_pActiveDebrisObjects, *pObj, query, pNode, dynamicCollisionCount);
						GenerateCollisionTasksKinematic(pParent->m_pSleepingDebrisObjects, *pObj, query, pNode, dynamicCollisionCount);

						//Cloth
						GenerateCollisionTasksCloth2(pParent->m_pSleepingClothObjects, *pObj, clothQuery, pNode);
					}
					pParent = pParent->m_pParentNode;
				}
			}

			{
				if(pNode->m_pLeftNode)
				{
					S32 size = 2;
					C_DistributionNode* pStackTop = pNode->m_pLeftNode;
					stack.Push(pNode->m_pRightNode, 0);
					stack.Push(pNode->m_pLeftNode, 1);

					while(size--)
					{
						C_DistributionNode* pChild = pStackTop;
						pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));
						if(pChild->m_bbox.Intersects(pObj->m_TransformedBox))
						{
							if(pChild->m_pSleepingObjects || pChild->m_pActiveObjects || 
								pChild->m_pActiveDebrisObjects || pChild->m_pSleepingDebrisObjects
								|| pChild->m_pSleepingClothObjects)
							{
								GenerateCollisionTasksKinematic(pChild->m_pSleepingObjects, *pObj, query, pChild, dynamicCollisionCount);
								GenerateCollisionTasks(pChild->m_pActiveObjects, *pObj, query, pChild, dynamicCollisionCount);
								GenerateCollisionTasks(pChild->m_pActiveDebrisObjects, *pObj, query, pChild, dynamicCollisionCount);
								GenerateCollisionTasksKinematic(pChild->m_pSleepingDebrisObjects, *pObj, query, pChild, dynamicCollisionCount);

								//Cloth
								GenerateCollisionTasksCloth2(pChild->m_pSleepingClothObjects, *pObj, clothQuery, pChild);
							}
							if(pChild->m_pLeftNode)
							{
								pStackTop = pChild->m_pLeftNode;
								stack.Push(pChild->m_pRightNode, size);
								size += 2;
								//stack.Push(pChild->m_pLeftNode, size++);
							}
						}
					}
				}
			}
		}

	}


	void C_Scene::ActiveNodeComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount)
	{
		using namespace Collision;
		C_AxisBoxMinMax& box=pNode->m_bbox;
		C_ObjectWrapper* pActiveWrapper=pNode->m_pActiveObjects;
		while(pActiveWrapper)
		{
			//PERF_EVENT("Gravity.ActiveObjectLoop");
			C_Object* pActiveObj=pActiveWrapper->m_pObject;
			//Dynamic object comparisons
			GenerateCollisionTasks(pNode->m_pSleepingObjects, *pActiveObj, query, pNode, dynamicCollisionCount);
			GenerateCollisionTasks(pNode->m_pKinematicObjects, *pActiveObj, query, pNode, dynamicCollisionCount);
			GenerateCollisionTasks(pNode->m_pStaticObjects, *pActiveObj, query, pNode, dynamicCollisionCount);
			GenerateCollisionTasks(pActiveWrapper->m_pNext, *pActiveObj, query, pNode, dynamicCollisionCount);

			//Fluid comparisons
			GenerateFluidTasks(pNode->m_pFluidMeshObjects, *pActiveObj, fluidQuery, pNode, fluidCollisionCount);

			pActiveWrapper=pActiveWrapper->m_pNext;
		}

		
		if(pNode->m_pKinematicObjects)
		{
			//PERF_EVENT("Gravity.SleepingObjectComparisons");
			C_ObjectWrapper* pWrapper(pNode->m_pSleepingObjects);
			while(pWrapper)
			{
				C_Object* pSleepyObj(pWrapper->m_pObject);
				GenerateCollisionTasks(pNode->m_pKinematicObjects, *pSleepyObj, query, pNode, dynamicCollisionCount);
				//Cloth comparisons
				pWrapper = pWrapper->m_pNext;
			}
		}
	}

	void C_Scene::ActiveParentComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount)
	{
		using namespace Collision;
		C_AxisBoxMinMax& box=pNode->m_bbox;
		C_DistributionNode* pParent(pNode->m_pParentNode);
		while(pParent)
		{
			/*if(pParent->m_pParentNode)
				DCBT(0, pParent->m_pParentNode);*/
			C_ObjectWrapper* pParentStatics(pParent->m_pStaticObjects);
			//while(pParentStatics)
			if(pParent->m_pStaticObjects || pParent->m_pKinematicObjects || pParent->m_pSleepingObjects || 
				pParent->m_pActiveObjects || pParent->m_pFluidMeshObjects)
			{
				C_ObjectWrapper* pActives(pNode->m_pActiveObjects);
				while(pActives)
				{
					C_Object* pActiveObject(pActives->m_pObject);

					//Dynamic comparisons

					GenerateCollisionTasks(pParent->m_pSleepingObjects, *pActiveObject, query, pNode, dynamicCollisionCount);
					GenerateCollisionTasks(pParent->m_pKinematicObjects, *pActiveObject, query, pNode, dynamicCollisionCount);
					GenerateCollisionTasks(pParent->m_pStaticObjects, *pActiveObject, query, pNode, dynamicCollisionCount);
					GenerateCollisionTasks(pParent->m_pActiveObjects, *pActiveObject, query, pNode, dynamicCollisionCount);
					
					//Fluid comparisons
					GenerateFluidTasks(pParent->m_pFluidMeshObjects, *pActiveObject, fluidQuery, pNode, fluidCollisionCount);

					pActives = pActives->m_pNext;
				}

				if(pParent->m_pKinematicObjects)
				{
					//PERF_EVENT("Gravity.SleepingObjectComparisons");
					C_ObjectWrapper* pSleepy(pNode->m_pSleepingObjects);
					while(pSleepy)
					{
						C_Object* pSleepyObject(pSleepy->m_pObject);
						GenerateCollisionTasks(pParent->m_pKinematicObjects, *pSleepyObject, query, pNode, dynamicCollisionCount);
						pSleepy = pSleepy->m_pNext;
					}
				}


				if(pNode->m_pStaticObjects || pNode->m_pKinematicObjects || pNode->m_pSleepingObjects || 
					pNode->m_pFluidMeshObjects)
				{
					{
						//PERF_EVENT("Gravity.ParentActiveObjectComparisons");
						C_ObjectWrapper* pParentActive(pParent->m_pActiveObjects);
						while(pParentActive)
						{
							C_Object* pActiveObj(pParentActive->m_pObject);

							GenerateCollisionTasks(pNode->m_pSleepingObjects, *pActiveObj, query, pNode, dynamicCollisionCount);
							GenerateCollisionTasks(pNode->m_pKinematicObjects, *pActiveObj, query, pNode, dynamicCollisionCount);
							GenerateCollisionTasks(pNode->m_pStaticObjects, *pActiveObj, query, pNode, dynamicCollisionCount);

							//Fluid comparisons
							GenerateFluidTasks(pNode->m_pFluidMeshObjects, *pActiveObj, fluidQuery, pNode, fluidCollisionCount);
							pParentActive = pParentActive->m_pNext;
						}
					}

					if(pNode->m_pKinematicObjects)
					{
						//PERF_EVENT("Gravity.ParentSleepingObjectComparisons");
						C_ObjectWrapper* pParentSleeping(pParent->m_pSleepingObjects);
						while(pParentSleeping)
						{
							C_Object* pSleepyObject(pParentSleeping->m_pObject);

							GenerateCollisionTasks(pNode->m_pKinematicObjects, *pSleepyObject, query, pNode, dynamicCollisionCount);
							pParentSleeping = pParentSleeping->m_pNext;
						}
					}
				}
			}
			pParent = pParent->m_pParentNode;
		}
	}

	void C_Scene::ActiveChildComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, SharedPhysics::C_SafeStack<true>& stack)
	{
		using namespace Collision;
		if(pNode->m_pLeftNode)
		{
			S32 size = 2;
			C_DistributionNode* pStackTop = pNode->m_pLeftNode;
			stack.Push(pNode->m_pRightNode, 0);
			stack.Push(pNode->m_pLeftNode, 1);

			while(size--)
			{
				C_DistributionNode* pChild = pStackTop;
				pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size-1));

				if(pChild->IsSleeping() && (pChild->m_pSleepingObjects || 
					pChild->m_pStaticObjects || pChild->m_pFluidMeshObjects))
				{
					if(pNode->m_pKinematicObjects)
					{
						C_ObjectWrapper* pSleepy(pChild->m_pSleepingObjects);
						while(pSleepy)
						{
							C_Object* pSleepyObject(pSleepy->m_pObject);
							GenerateCollisionTasks(pNode->m_pKinematicObjects, *pSleepyObject, query, pChild, dynamicCollisionCount);
							pSleepy = pSleepy->m_pNext;
						}
					}
					C_ObjectWrapper* pActive(pNode->m_pActiveObjects);
					while(pActive)
					{
						C_Object* pActiveObject(pActive->m_pObject);
						GenerateCollisionTasks(pChild->m_pSleepingObjects, *pActiveObject, query, pChild, dynamicCollisionCount);
						GenerateCollisionTasks(pChild->m_pStaticObjects, *pActiveObject, query, pChild, dynamicCollisionCount);
						GenerateFluidTasks(pChild->m_pFluidMeshObjects, *pActiveObject, fluidQuery, pChild, fluidCollisionCount);
						pActive = pActive->m_pNext;
					}
				}
				if(pChild->m_pLeftNode)
				{
					pStackTop = pChild->m_pLeftNode;
					stack.Push(pChild->m_pRightNode, size++);
					stack.Push(pChild->m_pLeftNode, size++);
				}

			}		
		}
		
	}

#endif


#if ((!defined (PLATFORM_SPU)) || (defined (SPU_INTEGRATE)))
	void IntegrateBodies(Scheduler::Task* pTask)
	{
		PERF_EVENT("Gravity.IntegrateTask");
		Spu::MemBaseMarker marker;
		C_IntegrateTask* pUpdateTask = (C_IntegrateTask*)&pTask->ParamData[0];
		const Maths::C_TimeS dt(Maths::Load<Maths::HF32>(&pUpdateTask->m_Time));
		SharedPhysics::C_Integrator integrator;
		Gravity::C_Body** pBodies = (Gravity::C_Body**)Spu::FetchAllocUnaligned(pUpdateTask->m_pBodies, sizeof(Gravity::C_Body*) * pUpdateTask->m_NumBodies);

		SharedPhysics::C_SpuArrayStreamer<Gravity::C_Body, 8, 4> streamer(pBodies, pUpdateTask->m_NumBodies);
		for(U32 a = 0; a < pUpdateTask->m_NumBodies; ++a)
		{
			//Spu::MemBaseMarker innerMarker;
			//C_Body* pBody = (C_Body*)Spu::FetchAlloc(pUpdateTask->m_pBodies[a], sizeof(C_Body));
			C_Body* pBody = streamer.NextElement();
			pBody->Integrate(integrator, dt);
			streamer.WriteBackElement();
			
//#ifdef PLATFORM_SPU
//			Spu::Store(pUpdateTask->m_pBodies[a], pBody, sizeof(C_Body));
//#endif
		}
	}
#endif


#if ((!defined (PLATFORM_SPU)) || (defined (SPU_KINEMATIC_UPDATE)))
	void KinematicControllerUpdate(Scheduler::Task* pTask)
	{
		PERF_EVENT("Gravity.UpdateKinematicController");
		Spu::MemBaseMarker marker;
		C_KinematicUpdateTask* pUpdateTask = (C_KinematicUpdateTask*)&pTask->ParamData[0];
		C_KinematicController** pControllers = (C_KinematicController**)Spu::FetchAllocUnaligned(pUpdateTask->m_pControllers, pUpdateTask->m_NumControllers * sizeof(C_KinematicController*));
		SharedPhysics::C_SpuArrayStreamer<Gravity::C_KinematicController, 8, 4> streamer(pControllers, pUpdateTask->m_NumControllers);
		const Maths::HF32 dt(Maths::Load<Maths::HF32>(&pUpdateTask->m_Time));
		for(U32 a = 0; a < pUpdateTask->m_NumControllers; ++a)
		{
			Spu::MemBaseMarker innerMarker;
			//C_KinematicController* pController = (C_KinematicController*)Spu::FetchAlloc(pUpdateTask->m_pControllers[a], sizeof(C_KinematicController));
			C_KinematicController* pController = streamer.NextElement();
			pController->Update(dt);
			streamer.WriteBackElement();
		}
	}
#endif


#if ((!defined (PLATFORM_SPU)) || (defined (SPU_ANIMATED_CONTROLLER)))
	void AnimatedShapeControllerUpdate(Scheduler::Task* pTask)
	{
		PERF_EVENT("Gravity.AnimatedShapeControllerUpdateTask");
		Spu::MemBaseMarker marker;
		C_AnimatedShapeUpdateTask* pAnimatedShapeTask = (C_AnimatedShapeUpdateTask*)&pTask->ParamData[0];
		C_AnimatedShapeController** pControllers = (Gravity::C_AnimatedShapeController**)Spu::FetchAllocUnaligned(pAnimatedShapeTask->m_pControllers, pAnimatedShapeTask->m_NumControllers * sizeof(C_AnimatedShapeController*) );
		SharedPhysics::C_SpuArrayStreamer<C_AnimatedShapeController, 8, 4> streamer(pControllers, pAnimatedShapeTask->m_NumControllers);
		for(U32 a = 0; a < pAnimatedShapeTask->m_NumControllers; ++a)
		{
			PERF_EVENT("Gravity.AnimatedShapeControllerUpdate");
			Spu::MemBaseMarker innerMarker;
			Gravity::C_AnimatedShapeController* pController = streamer.NextElement();
			Collision::C_CompositeShape* pShape = pController->GetCompositeShape();
			Collision::C_CompositeShape* pSpuCompShape = (Collision::C_CompositeShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CompositeShape));
			pController->UpdateTransform(pSpuCompShape);
			//streamer.WriteBackElement();
		}
	}
#endif


#if ((!defined (PLATFORM_SPU)) || (defined (SPU_POST_INTEGRATE)))
	void PostIntegrate(Scheduler::Task* pTask)
	{
		PERF_EVENT("Gravity.PostIntegrate");
		Spu::MemBaseMarker marker;
		C_PostIntegrateTask* pIntegrateTask = (C_PostIntegrateTask*)&pTask->ParamData[0];
		
		Gravity::C_Body** pBodies = (Gravity::C_Body**)Spu::FetchAllocUnaligned(pIntegrateTask->m_pBodies, sizeof(Gravity::C_Body*) * pIntegrateTask->m_NumBodies);

		SharedPhysics::C_SpuArrayStreamer<Gravity::C_Body, 8, 4> streamer(pBodies, pIntegrateTask->m_NumBodies);
		for(U32 a = 0; a < pIntegrateTask->m_NumBodies; ++a)
		{
			Spu::MemBaseMarker innerMarker;
			C_Body* pBody = streamer.NextElement();
			//C_Body* pBody = (C_Body*)Spu::FetchAlloc(pIntegrateTask->m_pBodies[a], sizeof(C_Body));
			pBody->SynchroniseState();
			pBody->SynchroniseStateFlags();
			pBody->LoadExternalForces();
			pBody->SynchronisePosRot();
			
			pBody->ClearEdgeFlags();
			streamer.WriteBackElement();
//#ifdef PLATFORM_SPU
//			Spu::Store(pIntegrateTask->m_pBodies[a], pBody, sizeof(C_Body));
//#endif
		}
	}
#endif


#if (!defined(PLATFORM_SPU)) || defined(SPU_WATER_QUERY)
	void DeferredWaterQuery(Scheduler::Task* pTask)
	{
#ifdef PLATFORM_SPU
		//DebugBreak();
#endif
		const U32 size = 1024;
#ifdef PLATFORM_SPU
		Spu::MemBaseMarker marker;
		char* data = (char*)Spu::Alloc(size * sizeof(void*));
#else
		char data[size * sizeof(void*)];
#endif
		SharedPhysics::C_SafeStack<true> stack(data, size);
		C_DeferredWaterCommand* deferred = (C_DeferredWaterCommand*)&pTask->ParamData[0];

		Gravity::C_WaterQuery* pQuery = (Gravity::C_WaterQuery*)Spu::FetchAllocAsync(deferred->m_pQuery, sizeof(Gravity::C_WaterQuery), 2);
		Gravity::C_Scene* pScene = (Gravity::C_Scene*)Spu::FetchAllocAsync(deferred->m_pScene, sizeof(Gravity::C_Scene), 2);
		Spu::Wait(2);
		
		pScene->DoWaterQuery(pQuery, stack, false);

#ifdef PLATFORM_SPU
		if(pQuery->IsUnderWater())
		{
			//DebugBreak();
			Spu::Store(deferred->m_pQuery, pQuery, sizeof(Gravity::C_WaterQuery));
			
		}
#endif
	}
#endif


#if (!defined(PLATFORM_SPU)) || defined(SPU_CONTINUOUS_COLLISION)

	void DeferredContinuousCollisionDetection(Scheduler::Task* pTask)
	{
		//DebugBreak();
		Spu::MemBaseMarker marker;
		C_DeferredCCDCommand* deferred = (C_DeferredCCDCommand*)&pTask->ParamData[0];

		Gravity::C_Scene* pScene = (Gravity::C_Scene*)Spu::FetchAlloc(deferred->m_pScene, sizeof(Gravity::C_Scene));
		C_Body* pBody = (Gravity::C_Body*)Spu::FetchAlloc(deferred->m_pBody, sizeof(Gravity::C_Body));

		const U32 size = 1024;
#ifdef PLATFORM_SPU
		char* data = (char*)Spu::Alloc(size * sizeof(void*));
#else
		char data[size * sizeof(void*)];
#endif
		
		SharedPhysics::C_SafeStack<true> stack(data, size);
		SharedPhysics::C_Collider collider;
		SharedPhysics::C_GraphNode* pPpuGraphNode(NULL);

		if(pScene->DoContinuousCollisionDetection(pBody, stack, &collider, &pPpuGraphNode, deferred->m_pStream))
		{
			//Write back all the data!
			BIZ_ASSERT(pPpuGraphNode);
#ifdef PLATFORM_SPU
			deferred->m_pStream = collider.m_StreamPtr.m_pStream;
			deferred->m_pCurrentReadPoint = collider.m_StreamPtr.m_pCurrentReadPoint;
			deferred->m_pMeshNode = pPpuGraphNode;
			SharedPhysics::C_RigidBody* pPpuRigid = &deferred->m_pBody->GetBody();
			SharedPhysics::C_RigidBody* pSpuRigid = &pBody->GetBody();
			Spu::Store(pPpuRigid, pSpuRigid, sizeof(SharedPhysics::C_RigidBody));
			Spu::Store(deferred->ppuAddress, deferred, sizeof(C_DeferredCCDCommand));
#else
			deferred->ppuAddress->m_pStream = collider.m_StreamPtr.m_pStream;
			deferred->ppuAddress->m_pCurrentReadPoint = collider.m_StreamPtr.m_pCurrentReadPoint;
			deferred->ppuAddress->m_pMeshNode = pPpuGraphNode;
#endif
			
		}


		
	}
#endif


#if (!defined(PLATFORM_SPU)) || defined(SPU_RAYTRACE)
	void DeferredRayTrace(Scheduler::Task* pTask)
	{
		Spu::MemBaseMarker marker;
#ifdef PLATFORM_SPU
		//DebugBreak();
#endif
		const U32 size = 1024;
#ifdef PLATFORM_SPU
		char* data = (char*)Spu::Alloc(size * sizeof(void*));
#else
		char data[size * sizeof(void*)];
#endif
		SharedPhysics::C_SafeStack<true> stack(data, size);
		C_DeferredRayCommand* deferred = (C_DeferredRayCommand*)&pTask->ParamData[0];
		/*Collision::C_Ray* ray = Spu::FetchAllocAsync<Collision::C_Ray>(deferred->m_pRay, 1 << 1);
		Gravity::C_Scene* pScene = Spu::FetchAllocAsync<Gravity::C_Scene>(deferred->m_pScene, 1 << 1);
		Spu::Wait(1 << 1);*/

		Collision::C_Ray* ray = (Collision::C_Ray*)Spu::FetchAllocAsync(deferred->m_pRay, sizeof(Collision::C_Ray), 2);
		Gravity::C_Scene* pScene = (Gravity::C_Scene*)Spu::FetchAllocAsync(deferred->m_pScene, sizeof(Gravity::C_Scene), 2);
		Spu::Wait(2);
		
#ifndef PLATFORM_SPU
		if(deferred->m_pCache)
		{
			Collision::C_LineCollisionCache* pCache = deferred->m_pCache;
			pScene->DoRayTrace(*ray, *pCache, stack, false);
		}
		else
#endif
		{
			if(deferred->m_TraceStaticMeshOnly)
			{
				pScene->DoRayTraceMesh(*ray, stack, false);
			}
			else
			{
				pScene->DoRayTrace(*ray, stack, false);
			}

#ifdef PLATFORM_SPU
			if(ray->m_result.m_pShape)
			{
				//DebugBreak();
				Spu::Store(deferred->m_pRay, ray, sizeof(Collision::C_Ray));
			}
#endif
			MT::MemoryFence();
			MT::AtomicSet(&deferred->m_pRay->m_CurrentMask, ray->m_WriteMask);
		}
	}
#endif

	#if (!defined(PLATFORM_SPU)) || defined(SPU_TRANSPARENT_RAYTRACE)
	void DeferredTransparentRayTrace(Scheduler::Task* pTask)
	{
#ifdef PLATFORM_SPU
		//DebugBreak();
#endif
		const U32 size = 1024;
#ifdef PLATFORM_SPU
		Spu::MemBaseMarker marker;
		char* data = (char*)Spu::Alloc(size * sizeof(void*));
#else
		char data[size * sizeof(void*)];
#endif
		SharedPhysics::C_SafeStack<true> stack(data, size);
		C_DeferredTransparentRayCommand* deferred = (C_DeferredTransparentRayCommand*)&pTask->ParamData[0];
		/*Collision::C_Ray* ray = Spu::FetchAllocAsync<Collision::C_Ray>(deferred->m_pRay, 1 << 1);
		Gravity::C_Scene* pScene = Spu::FetchAllocAsync<Gravity::C_Scene>(deferred->m_pScene, 1 << 1);
		Spu::Wait(1 << 1);*/

		Collision::C_TransparentRay* ray = (Collision::C_TransparentRay*)Spu::FetchAllocAsync(deferred->m_pRay, sizeof(Collision::C_TransparentRay), 2);
		Gravity::C_Scene* pScene = (Gravity::C_Scene*)Spu::FetchAllocAsync(deferred->m_pScene, sizeof(Gravity::C_Scene), 2);

		Spu::Wait(2);
		
		Collision::C_RayTracingResult* pPpuResult = ray->m_result;
#ifdef PLATFORM_SPU
		ray->m_result = (Collision::C_RayTracingResult*)Spu::Alloc(sizeof(Collision::C_RayTracingResult) * ray->m_maxNumLayers);
#endif		
		if(deferred->m_TraceStaticMeshOnly)
		{
			pScene->DoTransparentRayTraceMesh(*ray, stack, false);
		}
		else
		{
			pScene->DoTransparentRayTrace(*ray, stack, false);
		}

#ifdef PLATFORM_SPU
		if(ray->m_SolidResult.m_pShape || ray->m_numLayers > 0)
		{
			//DebugBreak();
			Spu::Store(pPpuResult, ray->m_result, sizeof(Collision::C_RayTracingResult) * ray->m_maxNumLayers);
			ray->m_result = pPpuResult;
			Spu::Store(deferred->m_pRay, ray, sizeof(Collision::C_TransparentRay));
			
		}
#endif
		
	}
#endif

#if (!defined PLATFORM_SPU) || defined (SPU_MESH_DISPATCH)
	void DispatchMeshCollision(Scheduler::Task* pTask)
	{
		C_MeshDispatchTask* pDispatchTask = (C_MeshDispatchTask*)&pTask->ParamData[0];
		Gravity::C_Scene* pScene = pDispatchTask->m_pScene;
		pScene->GetMeshContacts();
	}
#endif

#if (!defined PLATFORM_SPU) || defined (SPU_MESH_COLLISION)
	void CollideMesh(Scheduler::Task* pTask)
	{
		using namespace Collision;
		PERF_EVENT("Gravity.CollideMesh");
		Spu::MemBaseMarker marker;
#if 0 //def PLATFORM_SPU
		DebugBreak();
#endif
		C_CollisionQuery* collisionQuery = (C_CollisionQuery*)pTask->ParamData;
		//const Maths::HF32 time(Maths::Load<Maths::HF32>(&collisionQuery->m_Time));

		const U32 stackSize = 1024;
		const U32 epaDataSize = sizeof(EPAFacet) * MaxFacets;
#ifndef PLATFORM_SPU
		char *data = (char*)alloca(stackSize * sizeof(void*));
		char * epaData = (char*)alloca(epaDataSize);
		Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>* pResult = (Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>*)alloca(sizeof(Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>));
		Collision::EPA epa(epaData, epaDataSize);
#else
		char* data = (char*)Spu::Alloc(stackSize * sizeof(void*));
		Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>* pResult = (Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>*)Spu::Alloc(sizeof(Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>));
		EPA epa;
#endif
		

		SharedPhysics::C_SafeStack<true> stack(data, stackSize);
		//Collision::C_CollisionManager* pManager = (Collision::C_CollisionManager*)Spu::FetchAlloc(collisionQuery->m_pManager, sizeof(Collision::C_CollisionManager));
		Collision::C_CollisionManager* pManager = collisionQuery->m_pManager;//(Collision::C_CollisionManager*)Spu::FetchAlloc(collisionQuery->m_pManager, sizeof(Collision::C_CollisionManager));

		//SharedPhysics::C_SpuArrayStreamer<SharedPhysics::C_PersistentContactCollider, 6, 3> streamer((SharedPhysics::C_PersistentContactCollider**)collisionQuery->m_pColliders, collisionQuery->m_NumPairs);
		
		for(U32 a = 0; a < collisionQuery->m_NumPairs; ++a)
		{
			Spu::MemBaseMarker innerMarker;
			pResult->m_NumContactPatches = 0;
			SharedPhysics::C_Collider* pCurrentCollider = (SharedPhysics::C_Collider*)Spu::FetchAlloc(collisionQuery->m_pColliders[a], sizeof(SharedPhysics::C_Collider));
			//SharedPhysics::C_Collider* pCurrentCollider = streamer.NextElement();
			pResult->m_pCurrentCollider = pCurrentCollider;
			//pCurrentCollider->m_StreamPtr.m_pCurrentReadPoint = NULL;

			//Gravity::C_Object* pObjectA((Gravity::C_Object*)Spu::FetchAlloc(collisionQuery->m_pObjectA[a], sizeof(Gravity::C_Object)));
			//Gravity::C_Object* pObjectB((Gravity::C_Object*)Spu::FetchAlloc(collisionQuery->m_pObjectB[a], sizeof(Gravity::C_Object)));

			/*C_Body* pBodyA = (Gravity::C_Body*)Spu::Get((U32*)&pCurrentCollider->m_pNodeA->m_pBody);
			C_Body* pBodyB = (Gravity::C_Body*)Spu::Get((U32*)&pCurrentCollider->m_pNodeB->m_pBody);*/


			C_Body* pBodyA =C_Body::GetBody(pCurrentCollider->m_pNodeA);
			C_Body* pBodyB =C_Body::GetBody(pCurrentCollider->m_pNodeB);

			C_Object* pObjA = (C_Object*)Spu::FetchAllocAsync(&pBodyA->m_Object, sizeof(C_Object), 2);
			C_Object* pObjB = (C_Object*)Spu::FetchAllocAsync(&pBodyB->m_Object, sizeof(C_Object), 2);
			Spu::Wait(2);

			
			C_CollisionObject objA = (pObjA->CollisionObject());
			C_CollisionObject objB = (pObjB->CollisionObject());
			//now call the collision function!

			//SharedPhysics::C_ContactStream* stream = (SharedPhysics::C_ContactStream*)Spu::FetchAllocUnaligned(collisionQuery->m_pStream, sizeof(SharedPhysics::C_ContactStream));
			//pManager->UpdateCollider(pResult->m_pCurrentCollider);
			pManager->CollideMesh(&objB, &objA, *collisionQuery->m_pStream, stack, 
				*pResult, epa);
			//if(pResult->m_pCurrentCollider->m_StreamPtr.m_pStream)
			{
				//pManager->UpdateCollider(pCurrentCollider);
	#ifdef PLATFORM_SPU
				Spu::Store(collisionQuery->m_pColliders[a], pCurrentCollider, sizeof(SharedPhysics::C_Collider));
	#endif
				//streamer.WriteBackElement();
			}
		}
	}
#endif

#if (!defined PLATFORM_SPU) || defined (SPU_PHANTOM_MESH_COLLISION)
	void CollidePhantomMesh(Scheduler::Task* pTask)
	{
		using namespace Collision;
		PERF_EVENT("Gravity.CollidePhantomMesh");
		Spu::MemBaseMarker marker;
#if 0 //def PLATFORM_SPU
		DebugBreak();
#endif
		C_PhantomCollisionQuery* collisionQuery = (C_PhantomCollisionQuery*)pTask->ParamData;

		const U32 stackSize = 1024;
		const U32 epaDataSize = sizeof(EPAFacet) * MaxFacets;
#ifndef PLATFORM_SPU
		char *data = (char*)alloca(stackSize * sizeof(void*));
		char * epaData = (char*)alloca(epaDataSize);
		Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>* pResult = (Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>*)alloca(sizeof(Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>));
		Collision::EPA epa(epaData, epaDataSize);
#else
		char* data = (char*)Spu::Alloc(stackSize * sizeof(void*));
		Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>* pResult = (Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>*)Spu::Alloc(sizeof(Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>));
		EPA epa;
#endif
		pResult->m_pCurrentCollider = NULL;
		

		SharedPhysics::C_SafeStack<true> stack(data, stackSize);
		Collision::C_CollisionManager* pManager = (Collision::C_CollisionManager*)Spu::FetchAlloc(collisionQuery->m_pManager, sizeof(Collision::C_CollisionManager));
		
		for(U32 a = 0; a < collisionQuery->m_NumPairs; ++a)
		{
			Spu::MemBaseMarker innerMarker;
			pResult->m_NumContactPatches = 0;

			Gravity::C_Object* pObjA((Gravity::C_Object*)Spu::FetchAlloc(collisionQuery->m_pObjectA[a], sizeof(Gravity::C_Object)));
			Gravity::C_Object* pObjB((Gravity::C_Object*)Spu::FetchAlloc(collisionQuery->m_pObjectB[a], sizeof(Gravity::C_Object)));

			
			C_CollisionObject objA = (pObjA->CollisionObject());
			C_CollisionObject objB = (pObjB->CollisionObject());
			//now call the collision function!

			pManager->CollideMesh(&objB, &objA, *collisionQuery->m_pStream, stack, 
				*pResult, epa);
		}
	}
#endif


#if (!defined PLATFORM_SPU) || defined (SPU_GROUNDPLANE_COLLISION)
	void CollideGroundPlane(Scheduler::Task* pTask)
	{
		using namespace Collision;
		C_CollisionQuery* collisionQuery = (C_CollisionQuery*)pTask->ParamData;
		//const Maths::HF32 time(Maths::Load<Maths::HF32>(&collisionQuery->m_Time));
#if 0 //def PLATFORM_SPU
		DebugBreak();
#endif
		const U32 stackSize = 1024;
		const U32 epaDataSize = sizeof(EPAFacet) * MaxFacets;
#ifndef PLATFORM_SPU
		char *data = (char*)alloca(stackSize * sizeof(void*));
		char * epaData = (char*)alloca(epaDataSize);
		Collision::C_CollisionResult<_MaxPatches, _MaxContacts>* pResult = (Collision::C_CollisionResult<_MaxPatches, _MaxContacts>*)alloca(sizeof(Collision::C_CollisionResult<_MaxPatches, _MaxContacts>));
		EPA epa(epaData, epaDataSize);
#else
		char* data = (char*)Spu::Alloc(stackSize * sizeof(void*));
		Collision::C_CollisionResult<_MaxPatches, _MaxContacts>* pResult = (Collision::C_CollisionResult<_MaxPatches, _MaxContacts>*)Spu::Alloc(sizeof(Collision::C_CollisionResult<_MaxPatches, _MaxContacts>));
		EPA epa;
#endif
		SharedPhysics::C_SafeStack<true> stack(data, stackSize);
		Collision::C_CollisionManager* pManager = (Collision::C_CollisionManager*)Spu::FetchAlloc(collisionQuery->m_pManager, sizeof(Collision::C_CollisionManager));

		for(U32 a = 0; a < collisionQuery->m_NumPairs; ++a)
		{
			Spu::MemBaseMarker marker;
			pResult->m_NumContactPatches = 0;
			SharedPhysics::C_Collider* pCurrentCollider = (SharedPhysics::C_Collider*)Spu::FetchAlloc(collisionQuery->m_pColliders[a], sizeof(SharedPhysics::C_Collider));
			pResult->m_pCurrentCollider = pCurrentCollider;
			pCurrentCollider->m_StreamPtr.m_pCurrentReadPoint = NULL;

			//C_Body* pBodyA = (Gravity::C_Body*)Spu::Get((U32*)&pCurrentCollider->m_pNodeA->m_pBody);
			//C_Body* pBodyB = (Gravity::C_Body*)Spu::Get((U32*)&pCurrentCollider->m_pNodeB->m_pBody);
			C_Body* pBodyA = C_Body::GetBody(pCurrentCollider->m_pNodeA);
			C_Body* pBodyB = C_Body::GetBody(pCurrentCollider->m_pNodeB);

			C_Object* pObjA = (C_Object*)Spu::FetchAlloc(&pBodyA->m_Object, sizeof(C_Object));
			C_Object* pObjB = (C_Object*)Spu::FetchAlloc(&pBodyB->m_Object, sizeof(C_Object));
			//Gravity::C_Object* pObjectA((Gravity::C_Object*)Spu::FetchAlloc(collisionQuery->m_pObjectA[a], sizeof(Gravity::C_Object)));
			//Gravity::C_Object* pObjectB((Gravity::C_Object*)Spu::FetchAlloc(collisionQuery->m_pObjectB[a], sizeof(Gravity::C_Object)));

			C_CollisionObject objA(pObjA->CollisionObject());
			C_CollisionObject objB(pObjB->CollisionObject());
			//now call the collision function!

			//pManager->UpdateCollider(pResult->m_pCurrentCollider);


			//SharedPhysics::C_ContactStream* stream = (SharedPhysics::C_ContactStream*)Spu::FetchAllocUnaligned(collisionQuery->m_pStream, sizeof(SharedPhysics::C_ContactStream));
			pManager->CollideGroundPlane(&objB, &objA, *collisionQuery->m_pStream, stack, 
				*pResult, epa);
			//if(pResult->m_pCurrentCollider->m_StreamPtr.m_pStream)
			{
				//pManager->UpdateCollider(pCurrentCollider);
	#ifdef PLATFORM_SPU
				Spu::Store(collisionQuery->m_pColliders[a], pCurrentCollider, sizeof(SharedPhysics::C_Collider));
	#endif
			}
		}
	}
#endif

#if ! defined(PLATFORM_SPU) || defined(SPU_FLUID_COLLISION)

	void FluidCollision(Scheduler::Task* pTask)
	{
		PERF_EVENT("Gravity.FluidCollision");
		Spu::MemBaseMarker outerMarker;
		const U32 size = 512;
#ifdef PLATFORM_SPU
		//DebugBreak();
		char* data = (char*)Spu::Alloc(size * sizeof(void*));
#else
		char data[size * sizeof(void*)];
#endif
		SharedPhysics::C_SafeStack<true> stack(data, size);

		const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		C_FluidQuery* pFluidQuery = (C_FluidQuery*)pTask->ParamData;
		Collision::C_CollisionManager* pCollisionManager = pFluidQuery->m_pManager;
		Gravity::C_FluidEventBuffer* pBuffer = (Gravity::C_FluidEventBuffer*)Spu::FetchAllocUnaligned(pFluidQuery->m_pFluidBuffer, sizeof(C_FluidEventBuffer));
		Gravity::C_FluidEvent fluidEvent;

		Collision::C_CollisionFilter allSolidFilter;
		const Maths::HVec3 up(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY());
		const Maths::HF32 ten(SharedPhysics::Constants::Ten());

		SharedPhysics::C_SmallSPUCache spuCache(2);
		Collision::C_Ray ray;

		for(U32 a = 0; a < pFluidQuery->m_Size; ++a)
		{
			Spu::MemBaseMarker marker;
			Gravity::C_Body* pBody = Gravity::C_Body::GetBody(pFluidQuery->m_pObject[a]);
			Gravity::C_Object* pObjA = (C_Object*)Spu::FetchAllocAsync(pFluidQuery->m_pObject[a], sizeof(C_Object), 2);
			Gravity::C_Object* pObjB = (C_Object*)Spu::FetchAllocAsync(pFluidQuery->m_pFluidObject[a], sizeof(C_Object), 2);
			U8* pObjectPtr = (U8*)pFluidQuery->m_pObject[a];
			SharedPhysics::C_RigidBody* pPpuRigid = &pBody->GetBody();
			SharedPhysics::C_RigidBody* pRigid = (SharedPhysics::C_RigidBody*)Spu::FetchAllocAsync(pPpuRigid, sizeof(SharedPhysics::C_RigidBody), 2);

			Maths::HVec3 centroid(Maths::Constants<Maths::HVec3>::Zero());
			Maths::HF32 ratio(Maths::Constants<Maths::HF32>::Zero());
			Spu::Wait(2);

			
			Collision::C_FluidMeshShape* pFluidShape((Collision::C_FluidMeshShape*)Spu::FetchAlloc(pObjB->m_pShape, sizeof(Collision::C_FluidMeshShape)));
			BIZ_ASSERT(pFluidShape->m_ShapeType == Collision::E_FluidMesh_Shape);

			const Maths::HF32 submergedVolume = pCollisionManager->GetSubVolume(pObjA->m_pShape, pFluidShape->m_Vertex, pFluidShape->m_Normal, centroid, ratio);
			if (submergedVolume > epsilon)
			{
				//SharedPhysics::C_GraphNode* pGraphNode = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(pObjA->m_pGraphNode, sizeof(SharedPhysics::C_GraphNode));

				//Test against the fluid mesh shape to see if the submerged centroid is in there...

				const Maths::HF32 distance((pFluidShape->m_Vertex - centroid)| pFluidShape->m_Normal);
				const Maths::HVec3 surfacePoint(centroid + pFluidShape->m_Normal * distance);

				ray.InitialiseRay(centroid, surfacePoint + up * ten, &allSolidFilter); 
				ray.SetDoubleSided(true);

				if(pFluidShape->m_AABBTree.RayTracing<Collision::CollideRayTriangleMesh>(ray, stack, spuCache))
				{
					//const Maths::HF32 buoyancyMagnitude(Maths::Min(pFluidShape->m_Density*submergedVolume, pFluidShape->m_MaxBuoyancy * pRigid->m_Mass));
			
					const Maths::HF32 buoyancyMagnitude(pFluidShape->m_Density*submergedVolume);
					const Maths::HVec3 buoyancyForce = buoyancyMagnitude*pFluidShape->m_Normal;

					const Maths::HF32 partialMass = pRigid->Mass() * ratio;
					const Maths::HVec3 vc(pRigid->PointVelocity(centroid));
					const Maths::HF32 dragStrength(pFluidShape->m_LinearDrag);
					const Maths::HVec3 vel(pFluidShape->m_Current - vc);

					const Maths::HVec3 dragForce = (partialMass*dragStrength)*(vel);

					pRigid->ApplyForceWorldSpace((buoyancyForce * pRigid->BuoyancyCoefficient() + dragForce * pRigid->FluidLinearDragCoefficient()), centroid);
					
					const Maths::HF32 length2 = pObjA->m_TransformedBox.extents | pObjA->m_TransformedBox.extents;
					const Maths::HVec3 dragTorque = (-partialMass*pFluidShape->m_AngularDrag*length2)*pRigid->m_AngularVelocity;
					pRigid->AddTorque(dragTorque * pRigid->FluidAngularDragCoefficient());

					fluidEvent.m_pBody = pObjA->m_pGraphNode;
					Maths::Store(centroid, &fluidEvent.m_SubmergedCentroid);
					Maths::Store(submergedVolume, &fluidEvent.m_SubmergedVolume);
					Maths::Store(surfacePoint, &fluidEvent.m_SurfacePoint);
					pBuffer->AddFluidEvent(fluidEvent);
					U32 fluidState = Spu::Get(&pBody->GetGraphNode()->m_ExtendedNodeFlags);
					fluidState |= SharedPhysics::C_GraphNode::E_Type_Submerged;
					Spu::Set(&pBody->GetGraphNode()->m_ExtendedNodeFlags, fluidState);
#ifdef PLATFORM_SPU
					Spu::Store(pPpuRigid, pRigid, sizeof(SharedPhysics::C_RigidBody));
#endif
				}
			}
		}
	}
#endif

#if ! defined (PLATFORM_SPU) || defined (SPU_DYNAMIC_COLLISION)// || defined (SPU_DYNAMICOVERLAY_COLLISION)


	void UpdateCollider(SharedPhysics::C_PersistentContactCollider* pCollider)
	{
		Spu::MemBaseMarker marker;
		/*SharedPhysics::C_GraphNode* pNodeA(pCollider->m_pNodeA);
		SharedPhysics::C_GraphNode* pNodeB(pCollider->m_pNodeB);*/
		Gravity::C_ConstraintBody* pBodyA = (Gravity::C_ConstraintBody*)Spu::FetchAllocAsync(pCollider->m_pNodeA, sizeof(Gravity::C_ConstraintBody), 10);
		Gravity::C_ConstraintBody* pBodyB = (Gravity::C_ConstraintBody*)Spu::FetchAllocAsync(pCollider->m_pNodeB, sizeof(Gravity::C_ConstraintBody), 10);
		Spu::Wait(10);
		SharedPhysics::C_ConstraintEdge* pExistingConstraint = pBodyA->m_GraphNode.FindPrevConstraint(&pBodyB->m_GraphNode);
		
		//SharedPhysics::C_Collider* pExistingCollider = m_pContactDatabase->FindPrevCollider(pNodeA, pNodeB, index);
		//if(pExistingCollider)
		if(pExistingConstraint)
		{
			SharedPhysics::C_ConstraintEdge* pSpuExistingConstraint = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAlloc(pExistingConstraint, sizeof(SharedPhysics::C_ConstraintEdge));
			
			if(pSpuExistingConstraint->m_pCollider)
			{
				//pExistingCollider->m_State = SharedPhysics::C_Collider::E_Removed;
				//pCollider->m_pConstraintEdge = pExistingCollider->m_pConstraintEdge;
				//pExistingCollider->m_pConstraintEdge = NULL;

				SharedPhysics::C_PersistentContactCollider* pPpuPersistentCollider = (SharedPhysics::C_PersistentContactCollider*)pSpuExistingConstraint->m_pCollider;
				SharedPhysics::C_PersistentContactCollider* pPersistentCollider = (SharedPhysics::C_PersistentContactCollider*)Spu::FetchAlloc(pSpuExistingConstraint->m_pCollider, sizeof(SharedPhysics::C_PersistentContactCollider));
				
				pCollider->m_pConstraintEdge = pExistingConstraint;

				//pExistingConstraint->m_pCollider->m_State = SharedPhysics::C_Collider::E_Removed;
				//pExistingConstraint->m_pCollider->m_pConstraintEdge = NULL;

				
				
	#ifdef USE_PERSISTENT_MANIFOLD
				//Quick sets for SPU
				BIZ_ASSERT((!pPersistentCollider->m_PersistentCollider) || pPersistentCollider->m_PersistentManifold.m_NumContacts <= Collision::MaxManifoldContacts);

				//BIZ_ASSERT(pPersistentCollider->m_PersistentCollider == true);
				if(pPersistentCollider->m_Type != SharedPhysics::C_Collider::E_CloseCollision && pPersistentCollider->m_PersistentCollider)
				{
					pCollider->m_PersistentManifold = pPersistentCollider->m_PersistentManifold;
					if(pCollider->m_pNodeA != pPersistentCollider->m_pNodeA)
					{
						pCollider->m_PersistentManifold.FlipNormals();
					}
					/*pCollider->m_PersistentManifold.RefreshManifoldContacts(pNodeA->m_pBody->GetShape()->m_PosRot,
							pNodeB->m_pBody->GetShape()->m_PosRot);*/

				
					const Maths::HMat43 matA(pBodyA->m_RigidBody.m_OrientationMatrix, pBodyA->m_RigidBody.m_Position);
					const Maths::HMat43 matB(pBodyB->m_RigidBody.m_OrientationMatrix, pBodyB->m_RigidBody.m_Position);

					pCollider->m_PersistentManifold.RefreshManifoldContacts(matA, matB);
				}
					 
	#endif
				Spu::Set((U32*)&pSpuExistingConstraint->m_pCollider->m_State, (U32)SharedPhysics::C_Collider::E_Removed);
				Spu::Set((INTPTR*)&pSpuExistingConstraint->m_pCollider->m_pConstraintEdge, (INTPTR)0);	// [Blur2PC][x64] pointer-wide
			}
		}
	}


	void CollideDynamic(Scheduler::Task* pTask)
	{
		using namespace Collision;
		PERF_EVENT("Gravity.CollideDynamic");
		//DebugBreak();
		C_CollisionQuery* collisionQuery = (C_CollisionQuery*)pTask->ParamData;
		//const Maths::HF32 time(Maths::Load<Maths::HF32>(&collisionQuery->m_Time));

		Spu::MemBaseMarker marker;
		const U32 stackSize = 512;
		const U32 epaDataSize = sizeof(EPAFacet) * MaxFacets;
#ifndef PLATFORM_SPU
		char *data = (char*)alloca(stackSize * sizeof(void*));
		char * epaData = (char*)alloca(epaDataSize);
		Collision::C_CollisionResult<_MaxPatches, _MaxContacts>* pResult = (Collision::C_CollisionResult<_MaxPatches, _MaxContacts>*)alloca(sizeof(Collision::C_CollisionResult<_MaxPatches, _MaxContacts>));
		EPA epa(epaData, epaDataSize);
#else
		char* data = (char*)Spu::Alloc(stackSize * sizeof(void*));
		Collision::C_CollisionResult<_MaxPatches, _MaxContacts>* pResult = (Collision::C_CollisionResult<_MaxPatches, _MaxContacts>*)Spu::Alloc(sizeof(Collision::C_CollisionResult<_MaxPatches, _MaxContacts>));
		EPA epa;
#endif

		SharedPhysics::C_SafeStack<true> stack(data, stackSize);

		//DebugBreak();CollideCompositeShapeResource

		//Collision::C_CollisionManager* pManager = (Collision::C_CollisionManager*)Spu::FetchAlloc(collisionQuery->m_pManager, sizeof(Collision::C_CollisionManager));
		//Don't use any members, so don't need to fetch!!!
		Collision::C_CollisionManager* pManager = collisionQuery->m_pManager;

		
		SharedPhysics::C_SpuArrayStreamer<SharedPhysics::C_PersistentContactCollider, 6, 3> streamer((SharedPhysics::C_PersistentContactCollider**)collisionQuery->m_pColliders, collisionQuery->m_NumPairs);

		for(U32 a = 0; a < collisionQuery->m_NumPairs; ++a)
		{
			Spu::MemBaseMarker marker;
			SharedPhysics::C_PersistentContactCollider* pCurrentCollider = streamer.NextElement();
			//SharedPhysics::C_PersistentContactCollider* pCurrentCollider = (SharedPhysics::C_PersistentContactCollider*)Spu::FetchAlloc(collisionQuery->m_pColliders[a], sizeof(SharedPhysics::C_PersistentContactCollider));
#ifdef USE_PERSISTENT_MANIFOLD
			BIZ_ASSERT(pCurrentCollider->m_PersistentManifold.m_NumContacts < MaxManifoldContacts);
#endif

			//C_Body* pBodyA = (Gravity::C_Body*)Spu::Get((U32*)&pCurrentCollider->m_pNodeA->m_pBody);
			//C_Body* pBodyB = (Gravity::C_Body*)Spu::Get((U32*)&pCurrentCollider->m_pNodeB->m_pBody);
			C_Body* pBodyA = C_Body::GetBody(pCurrentCollider->m_pNodeA);
			C_Body* pBodyB = C_Body::GetBody(pCurrentCollider->m_pNodeB);
			C_Object* pObjA = (C_Object*)Spu::FetchAllocAsync(&pBodyA->m_Object, sizeof(C_Object), 8);
			C_Object* pObjB = (C_Object*)Spu::FetchAllocAsync(&pBodyB->m_Object, sizeof(C_Object), 8);
			Spu::Wait(8);


			//assert(pObjA->m_pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping | SharedPhysics::C_GraphNode::E_Type_Active));
			C_CollisionObject objA(pObjA->CollisionObject());
			C_CollisionObject objB(pObjB->CollisionObject());
			pResult->m_pCurrentCollider = pCurrentCollider;
			pCurrentCollider->m_StreamPtr.m_pCurrentReadPoint = NULL;

			//now call the collision function!
			//DebugBreak();
			
			pManager->Collide(&objA, &objB, *collisionQuery->m_pStream, stack, 
				*pResult, epa);
			//if(pResult->m_pCurrentCollider->m_StreamPtr.m_pStream)
			{
				UpdateCollider(pCurrentCollider);
//#ifdef PLATFORM_SPU
//				Spu::Store(collisionQuery->m_pColliders[a], pCurrentCollider, sizeof(SharedPhysics::C_Collider));
//#endif
				streamer.WriteBackElement();
				//Spu::Store(collisionQuery->m_pColliders[a], pCurrentCollider, sizeof(SharedPhysics::C_PersistentContactCollider));
			}
		}
	}
#endif

#if ! defined (PLATFORM_SPU) || defined (SPU_DYNAMIC_PHANTOM_COLLISION)// || defined (SPU_DYNAMICOVERLAY_COLLISION)


	void CollidePhantomDynamic(Scheduler::Task* pTask)
	{
		using namespace Collision;
		PERF_EVENT("Gravity.CollidePhantomDynamic");
		//DebugBreak();
		C_PhantomCollisionQuery* collisionQuery = (C_PhantomCollisionQuery*)pTask->ParamData;

		Spu::MemBaseMarker marker;
		const U32 stackSize = 512;
		const U32 epaDataSize = sizeof(EPAFacet) * MaxFacets;
#ifndef PLATFORM_SPU
		char *data = (char*)alloca(stackSize * sizeof(void*));
		char * epaData = (char*)alloca(epaDataSize);
		Collision::C_CollisionResult<_MaxPatches, _MaxContacts>* pResult = (Collision::C_CollisionResult<_MaxPatches, _MaxContacts>*)alloca(sizeof(Collision::C_CollisionResult<_MaxPatches, _MaxContacts>));
		EPA epa(epaData, epaDataSize);
#else
		char* data = (char*)Spu::Alloc(stackSize * sizeof(void*));
		Collision::C_CollisionResult<_MaxPatches, _MaxContacts>* pResult = (Collision::C_CollisionResult<_MaxPatches, _MaxContacts>*)Spu::Alloc(sizeof(Collision::C_CollisionResult<_MaxPatches, _MaxContacts>));
		EPA epa;
#endif
		pResult->m_pCurrentCollider = NULL;

		SharedPhysics::C_SafeStack<true> stack(data, stackSize);

		//DebugBreak();

		Collision::C_CollisionManager* pManager = (Collision::C_CollisionManager*)Spu::FetchAlloc(collisionQuery->m_pManager, sizeof(Collision::C_CollisionManager));
		
		for(U32 a = 0; a < collisionQuery->m_NumPairs; ++a)
		{
			Spu::MemBaseMarker marker;
			C_Object* pObjA = (C_Object*)Spu::FetchAlloc(collisionQuery->m_pObjectA[a], sizeof(C_Object));
			C_Object* pObjB = (C_Object*)Spu::FetchAlloc(collisionQuery->m_pObjectB[a], sizeof(C_Object));

			C_CollisionObject objA(pObjA->CollisionObject());
			C_CollisionObject objB(pObjB->CollisionObject());
			
			pManager->Collide(&objA, &objB, *collisionQuery->m_pStream, stack, 
				*pResult, epa);
		}
	}
#endif

#if (!defined (PLATFORM_SPU)) || defined(SPU_SOLVER)

	void Solve(Scheduler::Task* pTask)
	{
//		const U32 size = 1024;
//#ifndef PLATFORM_SPU
//		char data[size*sizeof(void*)];
//#else
//		char* data = (char*)Spu::Alloc(size*sizeof(void*));
//#endif
//		SharedPhysics::C_SafeStack<true> stack(data, size);
		C_SolverIsland* island = (C_SolverIsland*)pTask->ParamData;
		SharedPhysics::C_StaticSPUMemoryAllocator* pAllocator = (SharedPhysics::C_StaticSPUMemoryAllocator*)Spu::FetchAlloc(island->m_pMemoryAllocator, sizeof(SharedPhysics::C_StaticSPUMemoryAllocator));
		//NullBody* pNull = island->m_pNullBody;
		C_Body* pWorldBody = island->m_pWorldBody;
		Solver2::LCP lcp(0.f, 0.f, island->m_NumIterations, island->m_JointIterationFrequency, pAllocator, pWorldBody->GetGraphNode(), &pWorldBody->GetBody());

		
		lcp.SetTimeStep(island->m_TimeStep);
		lcp.LCPSolver(island->m_pRootNode);
	}

#endif


#if (!defined (PLATFORM_SPU)) || defined(SPU_RAGDOLL_CONTROLLER)

	void UpdateRagdollController(Scheduler::Task* pTask)
	{
		PERF_EVENT("Gravity.RagdollControllerModule");
		C_RagdollControllerCommand* pCommand = (C_RagdollControllerCommand*)pTask->ParamData;
		const Maths::HF32 time(Maths::Load<Maths::HF32>(&pCommand->m_Time));

		for(U32 a = 0; a < pCommand->m_NumControllers; ++a)
		{
			Spu::MemBaseMarker marker;
			C_RagdollController* pController = (C_RagdollController*)Spu::FetchAlloc(pCommand->m_pController[a], sizeof(C_RagdollController));
			pController->Update(time);
		}

	}

#endif



#if (!defined(PLATFORM_SPU)) || defined(SPU_RAYTRACE)
	void C_Scene::DoRayTrace(Collision::C_Ray& ray, Collision::C_LineCollisionCache& cache, SharedPhysics::C_SafeStack<true>& stack, bool bAcquireLock)
	{
		using namespace Collision;
		Spu::MemBaseMarker _marker;
		if(bAcquireLock)
			m_ReadWriteLock.ReadLock();
		PERF_EVENT("physics.raytrace");
		//raytrace level, then dynamic objects!
		cache.Trace(ray);
		bool bMeshCollision(false);
		
		SharedPhysics::C_SmallSPUCache spuCache;
		if(m_pGroundPlane && ray.m_Filter.CanCollide(&m_pGroundPlane->m_pShape->m_Filter))
		{
			Collision::IntersectRayPlane(ray, m_pGroundPlane->m_pShape);
		}

		if(ray.m_bFatTrace)
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker innerinnerMarker;
				C_Body* pMeshBody(m_MeshCollisionBodies[a].m_pElement);
				//U32 type = Spu::Get((U32*) &pMeshBody->m_pShape->m_ShapeType);
				//Collision::C_Shape*  pMeshShape(pMeshBody->GetShape());
				//if(type == Collision::E_TriangleMesh_Shape)
				{
					Collision::C_TriangleMeshShape* pTriMeshShape((Collision::C_TriangleMeshShape*)pMeshBody->m_pShape);
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(ray, stack, spuCache))
					{
						ray.m_result.m_pShape = pTriMeshShape;
						ray.m_result.m_pNode = pMeshBody->GetGraphNode();
						bMeshCollision = true;
					}

				}
			}
		}
		else
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker innerinnerMarker;
				C_Body* pMeshBody(m_MeshCollisionBodies[a].m_pElement);
				Collision::C_Shape*  pMeshShape(pMeshBody->GetShape());
				if(pMeshShape->GetShape() == Collision::E_TriangleMesh_Shape)
				{
					Collision::C_TriangleMeshShape* pTriMeshShape((Collision::C_TriangleMeshShape*)pMeshShape);
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideRayTriangleMesh>(ray, stack, spuCache))
					{
						ray.m_result.m_pShape = pTriMeshShape;
						ray.m_result.m_pNode = pMeshBody->GetGraphNode();
						bMeshCollision = true;
					}

				}
			}
		}

		/*for(S32 a = 0; a < this->m_ClothBodies.m_numElements; ++a)
		{
			C_Body* pBody = m_ClothBodies[a].m_pElement;
			C_ClothShape* pCloth = (C_ClothShape*)pBody->GetShape();
			Collision::C_CollisionObject obj(pBody->GetObject()->CollisionObject());
			pCloth->m_Cloth.Raytrace(ray, &obj, stack);
		}*/

		if(bMeshCollision)
		{
			//Get the triangle it hit and add it to the cache!
			Spu::MemBaseMarker innerMarker;
			Collision::C_TriangleMeshShape* pShape((Collision::C_TriangleMeshShape*)ray.m_result.m_pShape);
			Collision::AABBTreeTriangle* tri(pShape->m_AABBTree.m_Triangles.GetTriangles(ray.m_result.m_TriangleIndex));
			cache.AddTriangle<Collision::AABBTreeTriangle>(*tri, ray.m_result.m_pShape, ray.m_result.m_pNode,
				ray.m_result.m_TriangleIndex, ray.m_result.m_CollisionFlags, ray.m_result.m_PhysicsMaterialIndex, ray.m_result.m_EffectMaterialIndex, ray.m_result.m_UserData, 
				pShape->m_AABBTree.m_pVertices, pShape->m_AABBTree.m_pNormals);
		}

		//Now trace distribution tree!
		Gravity::C_DistributionTree* pTree = (Gravity::C_DistributionTree*)Spu::FetchAlloc(m_pTree, sizeof(Gravity::C_DistributionTree));
		pTree->RayTracing(ray, *m_pCollisionManager, stack, spuCache);
		if(bAcquireLock)
			m_ReadWriteLock.ReleaseReadLock();

	}

	void C_Scene::DoRayTrace(Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, bool bAcquireLock)
	{
		using namespace Collision;
		//raytrace level, then dynamic objects!
		PERF_EVENT("physics.raytrace");
		Spu::MemBaseMarker _marker;
		if(bAcquireLock)
			m_ReadWriteLock.ReadLock();
		
		bool bMeshCollision(false);
		SharedPhysics::C_SmallSPUCache spuCache;

		//BodyPtr* pBodyArray = (BodyPtr*)Spu::FetchAlloc(m_MeshCollisionBodies.m_pElems, sizeof(BodyPtr) * m_MeshCollisionBodies.m_numElements);

		if(m_pGroundPlane)
		{
			Spu::MemBaseMarker marker;
			//DebugBreak();
			C_Body* groundPlane = (C_Body*)Spu::FetchAlloc(m_pGroundPlane, sizeof(C_Body));
			Collision::C_PlaneShape* pGroundPlane = (Collision::C_PlaneShape*)Spu::FetchAlloc(groundPlane->m_pShape, sizeof(Collision::C_PlaneShape));
			if(pGroundPlane->m_Filter.CanCollide(&ray.m_Filter))
			{
				Collision::IntersectRayPlane(ray, pGroundPlane);
			}
		}

		if(ray.m_bFatTrace)
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(ray, stack, spuCache))
					{
						ray.m_result.m_pShape = pMeshBody->GetShape();
						ray.m_result.m_pNode = pBodyPtr->GetGraphNode();
						bMeshCollision = true;
					}
				}
			}
			Gravity::C_DistributionTree* pTree = (Gravity::C_DistributionTree*)Spu::FetchAlloc(m_pTree, sizeof(Gravity::C_DistributionTree));
			pTree->FatRayTracing(ray, *m_pCollisionManager, stack, spuCache);
		}
		else
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideRayTriangleMesh>(ray, stack, spuCache))
					{
						ray.m_result.m_pShape = pMeshBody->GetShape();
						ray.m_result.m_pNode = pBodyPtr->GetGraphNode();
						bMeshCollision = true;
					}
				}
			}
			Gravity::C_DistributionTree* pTree = (Gravity::C_DistributionTree*)Spu::FetchAlloc(m_pTree, sizeof(Gravity::C_DistributionTree));
			pTree->RayTracing(ray, *m_pCollisionManager, stack, spuCache);
		}

		//Now trace distribution tree!
		
		if(bAcquireLock)
			m_ReadWriteLock.ReleaseReadLock();
	}
	
	
	void C_Scene::DoRayTraceMesh(Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, bool bAcquireLock)
	{
		using namespace Collision;
		//raytrace level, then dynamic objects!
		Spu::MemBaseMarker _marker;
		if(bAcquireLock)
			m_ReadWriteLock.ReadLock();
		PERF_EVENT("physics.raytrace");
		bool bMeshCollision(false);
		SharedPhysics::C_SmallSPUCache spuCache;

		if(ray.m_bFatTrace)
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{ 
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(ray, stack, spuCache))
					{
						ray.m_result.m_pShape = pMeshBody->GetShape();
						ray.m_result.m_pNode = pBodyPtr->GetGraphNode();
						bMeshCollision = true;
					}
				}
			}
		}
		else
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideRayTriangleMesh>(ray, stack, spuCache))
					{
						ray.m_result.m_pShape = pMeshBody->GetShape();
						ray.m_result.m_pNode = pBodyPtr->GetGraphNode();
						bMeshCollision = true;
					}
				}
			}
		}

		//Now trace distribution tree!
	
		if(bAcquireLock)
			m_ReadWriteLock.ReleaseReadLock();
	}
#endif

#if (!defined(PLATFORM_SPU)) || defined(SPU_TRANSPARENT_RAYTRACE)
	void C_Scene::DoTransparentRayTrace(Collision::C_TransparentRay& ray, SharedPhysics::C_SafeStack<true>& stack, bool bAcquireLock)
	{
		using namespace Collision;
		//raytrace level, then dynamic objects!
		Spu::MemBaseMarker _marker;
		if(bAcquireLock)
			m_ReadWriteLock.ReadLock();
		PERF_EVENT("physics.transparentraytrace");
		bool bMeshCollision(false);
		SharedPhysics::C_SmallSPUCache spuCache;
		
		Gravity::C_DistributionTree* pTree = (Gravity::C_DistributionTree*)Spu::FetchAlloc(m_pTree, sizeof(Gravity::C_DistributionTree));


		//BodyPtr* pBodyArray = (BodyPtr*)Spu::FetchAlloc(m_MeshCollisionBodies.m_pElems, sizeof(BodyPtr) * m_MeshCollisionBodies.m_numElements);

		C_Ray solidRay;
		if(ray.m_bFatTrace)
		{
			solidRay.InitialiseRay(ray.m_start, ray.m_Direction, ray.m_maxDistance, &ray.m_SolidFilter, ray.m_Radius); 
		}
		else
		{
			solidRay.InitialiseRay(ray.m_start, ray.m_end, &ray.m_SolidFilter); 
		}
		solidRay.m_MaxCosAngle = ray.m_MaxCosAngle;

		if(m_pGroundPlane)
		{
			//DebugBreak();
			Spu::MemBaseMarker marker;
			C_Body* groundPlane = (C_Body*)Spu::FetchAlloc(m_pGroundPlane, sizeof(C_Body));
			Collision::C_PlaneShape* pGroundPlane = (Collision::C_PlaneShape*)Spu::FetchAlloc(groundPlane->m_pShape, sizeof(Collision::C_PlaneShape));
			if(solidRay.m_Filter.CanCollide(&pGroundPlane->m_Filter))
			{
				Collision::IntersectRayPlane(solidRay, pGroundPlane);
			}
			
		}

		if(ray.m_bFatTrace)
		{
			//Now trace the mesh
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&solidRay.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(solidRay, stack, spuCache))
					{
						solidRay.m_result.m_pShape = pMeshBody->GetShape();;
						solidRay.m_result.m_pNode  = pBodyPtr->GetGraphNode();
						bMeshCollision = true;
					}
				}
			}

			//Now trace distribution tree for the dynamic objects
			pTree->FatRayTracing(solidRay, *m_pCollisionManager, stack, spuCache);
		}
		else
		{
			//Now trace the mesh
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&solidRay.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideRayTriangleMesh>(solidRay,  stack, spuCache))
					{
						solidRay.m_result.m_pShape = pMeshBody->GetShape();
						solidRay.m_result.m_pNode = pBodyPtr->GetGraphNode();
						bMeshCollision = true;
					}
				}
			}
			//Now trace distribution tree for the dynamic objects
			pTree->RayTracing(solidRay, *m_pCollisionManager, stack, spuCache);
		}

			

		if(solidRay.m_result.m_pShape)
		{
			ray.m_SolidResult = solidRay.m_result;
			ray.m_end = solidRay.m_result.m_contact;
		}
		

		//Do Transparent ray tracing now!

		if(m_pGroundPlane)
		{
			//DebugBreak();
			Spu::MemBaseMarker marker;
			C_Body* groundPlane = (C_Body*)Spu::FetchAlloc(m_pGroundPlane, sizeof(C_Body));
			Collision::C_PlaneShape* pGroundPlane = (Collision::C_PlaneShape*)Spu::FetchAlloc(groundPlane->m_pShape, sizeof(Collision::C_PlaneShape));
			if(ray.m_TransFilter.CanCollide(&pGroundPlane->m_Filter))
			{
				Collision::IntersectRayPlane(ray, pGroundPlane);
			}
		}

		if(ray.m_bFatTrace)
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_TransFilter) && pTriMeshShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(ray, &ray.m_TransFilter, pMeshBody->GetShape(), pBodyPtr->GetGraphNode(), stack, spuCache))
					{
						bMeshCollision = true;
					}
				}
			}

			//Now trace distribution tree!
			pTree->FatRayTracing(ray, &ray.m_TransFilter, *m_pCollisionManager, stack, spuCache);
		}
		else
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_TransFilter) && pTriMeshShape->m_AABBTree.RayTracing<CollideRayTriangleMesh>(ray, &ray.m_TransFilter, pMeshBody->GetShape(), pBodyPtr->GetGraphNode(), stack, spuCache))
					{
						bMeshCollision = true;
					}
				}
			}

			//Now trace distribution tree!
			pTree->RayTracing(ray, &ray.m_TransFilter, *m_pCollisionManager, stack, spuCache);
		}


		

		if(bAcquireLock)
			m_ReadWriteLock.ReleaseReadLock();
	}

	void C_Scene::DoTransparentRayTraceMesh(Collision::C_TransparentRay& ray, SharedPhysics::C_SafeStack<true>& stack, bool bAcquireLock)
	{
		using namespace Collision;
		//raytrace level, then dynamic objects!
		Spu::MemBaseMarker _marker;
		if(bAcquireLock)
			m_ReadWriteLock.ReadLock();
		PERF_EVENT("physics.transparentraytrace");
		bool bMeshCollision(false);
		SharedPhysics::C_SmallSPUCache spuCache;


		//BodyPtr* pBodyArray = (BodyPtr*)Spu::FetchAlloc(m_MeshCollisionBodies.m_pElems, sizeof(BodyPtr) * m_MeshCollisionBodies.m_numElements);

		C_Ray solidRay;
		if(ray.m_bFatTrace)
		{
			solidRay.InitialiseRay(ray.m_start, ray.m_Direction, ray.m_maxDistance, &ray.m_SolidFilter, ray.m_Radius); 
		}
		else
		{
			solidRay.InitialiseRay(ray.m_start, ray.m_end, &ray.m_SolidFilter); 
		}

		if(m_pGroundPlane)
		{
			//DebugBreak();
			Spu::MemBaseMarker marker;
			C_Body* groundPlane = (C_Body*)Spu::FetchAlloc(m_pGroundPlane, sizeof(C_Body));
			Collision::C_PlaneShape* pGroundPlane = (Collision::C_PlaneShape*)Spu::FetchAlloc(groundPlane->m_pShape, sizeof(Collision::C_PlaneShape));
			if(solidRay.m_Filter.CanCollide(&pGroundPlane->m_Filter))
			{
				Collision::IntersectRayPlane(solidRay, pGroundPlane);
			}
			
		}

		if(ray.m_bFatTrace)
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&solidRay.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(solidRay, stack, spuCache))
					{
						solidRay.m_result.m_pShape = pMeshBody->GetShape();;
						solidRay.m_result.m_pNode  = pBodyPtr->GetGraphNode();
						bMeshCollision = true;
					}
				}
			}
		}
		else
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&solidRay.m_Filter) && pTriMeshShape->m_AABBTree.RayTracing<CollideRayTriangleMesh>(solidRay,  stack, spuCache))
					{
						solidRay.m_result.m_pShape = pMeshBody->GetShape();
						solidRay.m_result.m_pNode = pBodyPtr->GetGraphNode();
						bMeshCollision = true;
					}
				}
			}
		}

		if(solidRay.m_result.m_pShape)
		{
			ray.m_SolidResult = solidRay.m_result;
			ray.m_end = solidRay.m_result.m_contact;
		}
		

		//Do Transparent ray tracing now!

		if(m_pGroundPlane)
		{
			//DebugBreak();
			Spu::MemBaseMarker marker;
			C_Body* groundPlane = (C_Body*)Spu::FetchAlloc(m_pGroundPlane, sizeof(C_Body));
			Collision::C_PlaneShape* pGroundPlane = (Collision::C_PlaneShape*)Spu::FetchAlloc(groundPlane->m_pShape, sizeof(Collision::C_PlaneShape));
			if(ray.m_TransFilter.CanCollide(&pGroundPlane->m_Filter))
			{
				Collision::IntersectRayPlane(ray, pGroundPlane);
			}
		}

		if(ray.m_bFatTrace)
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_TransFilter) && pTriMeshShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(ray, &ray.m_TransFilter, pMeshBody->GetShape(), pBodyPtr->GetGraphNode(), stack, spuCache))
					{
						bMeshCollision = true;
					}
				}
			}
		}
		else
		{
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				Spu::MemBaseMarker marker;
				C_Body* pBodyPtr = (C_Body*)Spu::Get((INTPTR*)m_MeshCollisionBodies.m_pElems + a);	// [Blur2PC][x64] pointer-sized elements
				C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pBodyPtr, sizeof(C_Body));
				//U32 type = Spu::Get((U32*)(& pMeshBody->m_pShape->m_ShapeType));
				//Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
				//if(pMeshShape->m_ShapeType == Collision::E_TriangleMesh_Shape)
				{
					//Spu::SetMemBase(pMeshShape);
					Collision::C_TriangleMeshShape* pTriMeshShape = (Collision::C_TriangleMeshShape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_TriangleMeshShape));
					if(pTriMeshShape->m_Filter.CanCollide(&ray.m_TransFilter) && pTriMeshShape->m_AABBTree.RayTracing<CollideRayTriangleMesh>(ray, &ray.m_TransFilter, pMeshBody->GetShape(), pBodyPtr->GetGraphNode(), stack, spuCache))
					{
						bMeshCollision = true;
					}
				}
			}
		}
		if(bAcquireLock)
			m_ReadWriteLock.ReleaseReadLock();
	}

#endif


#if (!defined(PLATFORM_SPU)) || defined(SPU_WATER_QUERY)
	void C_Scene::DoWaterQuery(C_WaterQuery* pQuery, SharedPhysics::C_SafeStack<true>& stack, bool bAcquireLock)
	{
		PERF_EVENT("physics.doWaterQuery");
		Spu::MemBaseMarker _marker;
		if(bAcquireLock)
			m_ReadWriteLock.ReadLock();

		Collision::C_Ray ray;
		const Maths::HVec3 startPoint(pQuery->m_TestPoint);
		const Maths::HVec3 endPoint(startPoint + Maths::Constants<Maths::HVec3>::UnitAxisPositiveY() * SharedPhysics::Constants::Thousand());
		ray.InitialiseRay(startPoint, endPoint, pQuery->GetFilter());
		ray.SetDoubleSided(true);

		SharedPhysics::C_SmallSPUCache spuCache;
		Gravity::C_DistributionTree* pTree = (Gravity::C_DistributionTree*)Spu::FetchAlloc(m_pTree, sizeof(Gravity::C_DistributionTree));
		if(pTree->RayTraceWater(ray, *m_pCollisionManager, stack, spuCache))
		{
			pQuery->m_UnderWater = true;
			pQuery->m_Depth = ray.m_result.m_distance;
			pQuery->m_SurfaceNormal = ray.m_result.m_TriangleNormal;
			pQuery->m_SurfacePoint = ray.m_result.m_contact;
		}

		if(bAcquireLock)
			m_ReadWriteLock.ReleaseReadLock();
	}
#endif


#if !defined PLATFORM_SPU || defined (SPU_CONTINUOUS_COLLISION)
	bool C_Scene::DoContinuousCollisionDetection(Gravity::C_Body* pBody, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_Collider* pCollider,
		SharedPhysics::C_GraphNode** pPpuGraphNode, SharedPhysics::C_ContactStream* pStream)
	{
	
		//DebugBreak();
		using namespace Maths;

		Spu::MemBaseMarker marker;
		const HF32 one(Maths::Constants<HF32>::One());
		const HF32 zero(Maths::Constants<HF32>::Zero());
		const HF32 epsilon(Maths::Constants<HF32>::Epsilon2());
		const HF32 scale(SharedPhysics::Constants::Tenth());
		const HVec3 zeroV(Maths::Constants<HVec3>::Zero());
		HVec3 nor(zeroV);
		HVec3 closestA(zeroV);
		HVec3 closestB(zeroV);

		Collision::C_CollisionObject objA = pBody->m_Object.CollisionObject();
		//Collision::C_ContinuousShape* pShape = (Collision::C_ContinuousShape*)pBody->GetShape();
		Collision::C_ContinuousShape* pShape = (Collision::C_ContinuousShape*)Spu::FetchAlloc(pBody->GetShape(), sizeof(Collision::C_ContinuousShape));
		Collision::C_Shape* pDiscreteShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape->m_pDiscreteShape, sizeof(Collision::C_Shape));
	//	Collision::C_Shape* pContinuousShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape->m_pContinuousPrimitive, sizeof(Collision::C_Shape));
		const HVec3 prePos=pDiscreteShape->Position();
		const HVec3 curPos=pBody->m_RigidBody.m_Position;
		U32 physicsMaterial, effectMaterial;

		const HVec3 r = -(curPos - prePos);
		SharedPhysics::C_GraphNode* pGraphNode(NULL);

		//Collision::C_CollisionManager* pManager = (Collision::C_CollisionManager*)Spu::FetchAlloc(m_pCollisionManager, sizeof(Collision::C_CollisionManager));

		HF32 lambda(one);
		for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
		{
			Spu::MemBaseMarker marker2;
		//	//C_Body* pMeshBody(m_MeshCollisionBodies[a].m_pElement);
		////	DebugBreak();
			BodyPtr* pBodyPtr = (BodyPtr*)Spu::FetchAllocUnaligned(&m_MeshCollisionBodies.m_pElems[a], sizeof(BodyPtr));
			C_Body* pPpuMeshBody = pBodyPtr->m_pElement;
			C_Body* pMeshBody = (C_Body*)Spu::FetchAlloc(pPpuMeshBody, sizeof(C_Body));
			Collision::C_Shape* pMeshShape = (Collision::C_Shape*)Spu::FetchAlloc(pMeshBody->GetShape(), sizeof(Collision::C_Shape));
			
			if(pShape->CanCollide(pMeshShape))
			{
				Collision::C_CollisionObject cObjectB = pMeshBody->m_Object.CollisionObject();
				const Maths::HF32 newLambda = m_pCollisionManager->ContinuousCollideMesh(&cObjectB, &objA, r, nor, closestA, closestB, stack, physicsMaterial, effectMaterial);
				if(newLambda < lambda)
				{
					lambda = newLambda;
			//		//pGraphNode = &pMeshBody->m_GraphNode;
					*pPpuGraphNode = &pPpuMeshBody->m_GraphNode;
					BIZ_ASSERT(*pPpuGraphNode);
				}
			}
		}


		//if(lambda == one)
		//{
		//	//No collision, damp the velocity to simulate the air friction
		//	HF32 rate = 0.99f;
		//	pBody->m_RigidBody.StoreVelocitiesAndPosition(pBody->m_RigidBody.m_LinearVelocity*rate,
		//		pBody->m_RigidBody.m_AngularVelocity*rate, pBody->m_RigidBody.m_Orientation, pBody->m_RigidBody.m_Position);

		//}
		//else
		//{
		if(lambda < one)
		{
			BIZ_ASSERT(*pPpuGraphNode);
			//DebugBreak();
			HVec3 contact(closestA);
			if(lambda == zero)
			{

				//const Maths::HF32 pen((closestB - closestA).Length());
#ifdef PLATFORM_SPU
				Collision::SPUManager manager;
				const Maths::HVec3 support(manager.Support(pShape->m_pContinuousPrimitive, nor));
#else
				const Maths::HVec3 support(pShape->Support(nor));
#endif
				const Maths::HVec3 dif(support - closestA);

				const Maths::HF32 dis(dif | nor);
				const Maths::HVec3 closest(support - nor * dis);

				
				const Maths::HF32 pen(Maths::Max(dis, zero) + epsilon);
				const HVec3 position = prePos - pen * nor; 
				contact = closest;
				
				HF32 rate = 0.99f;
				pBody->m_RigidBody.StoreVelocitiesAndPosition(pBody->m_RigidBody.m_LinearVelocity*rate,
					pBody->m_RigidBody.m_AngularVelocity*rate, pBody->m_RigidBody.m_Orientation, position);

				
				

			}
			else //lambda > zero
			{
				//This will hit in the next frame
				const HVec3 position(prePos - r * lambda);
				/*pBody->m_RigidBody.StoreVelocitiesAndPosition(pBody->m_RigidBody.m_LinearVelocity,
				pBody->m_RigidBody.m_AngularVelocity, pBody->m_RigidBody.m_Orientation, position);*/
				pBody->m_RigidBody.m_Position = position;
				pBody->m_RigidBody.m_CenterOfMass = position + pBody->m_RigidBody.m_CenterOfMassOffset * pBody->m_RigidBody.m_OrientationMatrix;

			}
#ifdef PLATFORM_SPU
			Collision::C_CollisionResult<1, 1>* pResults = (Collision::C_CollisionResult<1, 1>*)Spu::Alloc(sizeof(Collision::C_CollisionResult<1, 1>));
			Collision::C_CollisionResult<1, 1>& results = *pResults;
#else
			Collision::C_CollisionResult<1, 1> results;
#endif

			pGraphNode = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(*pPpuGraphNode, sizeof(SharedPhysics::C_GraphNode));
			Gravity::C_Body* pMeshBody = (Gravity::C_Body*)Spu::FetchAlloc(pGraphNode->m_pBody, sizeof(Gravity::C_Body));
			Collision::C_CollisionObject meshObj = pMeshBody->m_Object.CollisionObject();
			results.m_pObjectA = &objA;
			results.m_pObjectB = &meshObj;
			results.m_NumContactPatches = 1;
			
			Collision::C_ContactPatch<1>& patch = results.m_ContactPatches[0];

			results.m_ContactPatches[0].m_Normal = nor;
			results.m_ContactPatches[0].m_NumContactPoints = 1;
			Collision::C_ContactPoint& point = results.m_ContactPatches[0].m_ContactPoints[0];
			point.m_ContactPoint = contact;
			point.m_EffectMaterialIndices = effectMaterial;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			
			point.m_PenetrationDepth = 0.f;
			point.m_HashValue = 1;

			/*SharedPhysics::C_Collider* pCollider = m_pCollisionManager->CreateCollider(&pBody->m_GraphNode, pGraphNode);*/
			//SharedPhysics::C_Collider collider(pBody->m_GraphNode, pGraphNode);

			pCollider->SetNodes(&pBody->m_GraphNode, pGraphNode);
			results.m_pCurrentCollider = pCollider;
			
			//DebugBreak();
			Collision::EncodeContacts<1, 1>(results, *pStream, pCollider);

			return true;

		}
		return false;
	}
#endif

#if !defined (PLATFORM_SPU) || defined (SPU_PHANTOM_MESH_LIB)

//pPhantom = SPU ptr. C_ContactStream = SPU address
void C_Scene::GetCollisionPhantomIntersectionMeshImmediate(Gravity::C_CollisionPhantom* pPhantom, 
	SharedPhysics::C_ContactStream& stream)
{
	using namespace Collision;
	PERF_EVENT("Gravity.CollidePhantomMeshImmediate");
	Spu::MemBaseMarker marker;

	const U32 stackSize = 512;
	const U32 epaDataSize = sizeof(EPAFacet) * MaxFacets;
#ifndef PLATFORM_SPU
	char *data = (char*)alloca(stackSize * sizeof(void*));
	char * epaData = (char*)alloca(epaDataSize);
	Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>* pResult = (Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>*)alloca(sizeof(Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>));
	Collision::EPA epa(epaData, epaDataSize);
#else
	char* data = (char*)Spu::Alloc(stackSize * sizeof(void*));
	Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>* pResult = (Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>*)Spu::Alloc(sizeof(Collision::C_CollisionResult<Static_MaxPatches, Static_MaxContacts>));
	EPA epa;
#endif
	pResult->m_pCurrentCollider = NULL;
	

	Gravity::C_Object* pObjA = pPhantom->GetObject();
	C_CollisionObject objA = pObjA->CollisionObject();


	SharedPhysics::C_SafeStack<true> stack(data, stackSize);

	Gravity::C_Body** pBodies = (Gravity::C_Body**)Spu::FetchAllocUnaligned(m_MeshCollisionBodies.m_pElems, sizeof(Gravity::C_Body*) * m_MeshCollisionBodies.m_numElements);

	for(S32 a = 0; a < this->m_MeshCollisionBodies.m_numElements; ++a)
	{
		Spu::MemBaseMarker innerMarker;
		pResult->m_NumContactPatches = 0;
		Gravity::C_Object* pObjB = (Gravity::C_Object*)Spu::FetchAlloc(pBodies[a]->GetObject(), sizeof(Gravity::C_Object));
		C_CollisionObject objB = (pObjB->CollisionObject());

		//NB - m_pCollisionManager is Ppu ptr, but this function really should be a static - it does not require any data members and therefore
		//can safely use a non-DMA'd in pointer to the class.
		m_pCollisionManager->CollideMeshSpuStream(&objB, &objA, stream, stack, 
			*pResult, epa);
	}

	SharedPhysics::C_ContactStreamResult* res = (SharedPhysics::C_ContactStreamResult*)stream.m_pCurrentWritePoint;
	res->m_ValidData = SharedPhysics::E_NullData;
}
#endif


#ifndef PLATFORM_SPU

	bool C_Scene::ContinuousCheckBoundingBox(Collision::C_Shape* pShapeA, Collision::C_Shape* pShapeB,
		const Maths::HVec3 PCREF transA, const Maths::HVec3 PCREF transB, Maths::HF32& lambda,
		Maths::HVec3& normal, Maths::HVec3& contactPoint)
	{
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		Collision::C_OBB obbA, obbB;
		obbA.Centre(pShapeA->m_PosRot.Translation() - transA);
		obbA.Orientation(pShapeA->m_PosRot.Rotation());
		obbA.Epsilon(0.005f);
		obbA.Margin(0.06f);
		obbA.Extent(pShapeA->m_LocalBoundingBox.extents);
		obbA.WarmStart(obbA.Centre());

		obbB.Centre(pShapeB->m_PosRot.Translation() - transB);
		obbB.Orientation(pShapeB->m_PosRot.Rotation());
		obbB.Epsilon(0.005f);
		obbB.Margin(0.06f);
		obbB.Extent(pShapeB->m_LocalBoundingBox.extents);
		obbB.WarmStart(obbB.Centre());

		const Maths::HVec3 r(transB - transA);
		lambda = one;
		//return m_pCollisionManager->ContinuousCheckOBBOBB(obbA, obbB, r, lambda,normal, contactPoint);
		bool bResult = m_pCollisionManager->ContinuousCheckOBBOBB(obbA, obbB, r, lambda,normal, contactPoint);
		contactPoint += lambda * transA;
		return bResult;
	}

#endif

}
