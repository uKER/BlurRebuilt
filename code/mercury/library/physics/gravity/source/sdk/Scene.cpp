#include "StdAfx.h"

#include <system/memory/memorytracker.h>
#include <system/perf.h>

#include "btl/Containers/openhashset.h"

#include "gravity/sdk/Scene.h"
#include "gravity/sdk/Body.h"
#include "gravity/sdk/CharacterController.h"
#include "gravity/sdk/KinematicController.h"
#include "gravity/sdk/AnimatedShapeController.h"
#include "gravity/sdk/ReferenceFrame.h"
#include "gravity/sdk/CollisionCallback.h"
#include "gravity/sdk/EventCallback.h"
#include "gravity/sdk/CollisionPhantom.h"

#include "gravity/broadphase/Object.inl"
#include "gravity/broadphase/DistributionTree.h"

#include "gravity/sharedphysics/RigidBody.inl"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/RayTriangle.inl"
#include "gravity/narrowphase/OptimisedAABBTree.inl"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/narrowphase/linecollisioncache.h"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/narrowphase/AxisBox.inl"

#include "gravity/sharedphysics/contactdatabase.inl"
#include "gravity/sharedphysics/AllocationManager.inl"

#include "gravity/cloths/HighFieldFluidCollision.h"

#include "gravity/solver2/lcp.h"
#include "gravity/Solver2/ConstraintGraphManager.h"
#include "gravity/sharedphysics/contactstreamiterator.h"
#include "gravity/importer/Importer.h"

//Comment to force re-build of this file.

#ifndef PLATFORM_SPU
#include "Btl/Utility/String.h"
#endif

#ifndef PLATFORM_SPU
Debug::Int g_dbNumIterations("Gravity.Solver.NumberIterations", 30, Debug::Range(10, 100));
Debug::Bool g_dbOverrideSolverParams("Gravity.Solver.OverrideSolverParams", false);
Debug::Int g_dbJointIterationFrequency("Gravity.Solver.ConstraintFrequency", 1, Debug::Range(1, 10));
Debug::Int g_dbDynamicCollisionGranularity("Gravity.Collision.DynamicCollisionBatch", 9, Debug::Range(1, 9));
Debug::Int g_dbMeshCollisionGranularity("Gravity.Collision.MeshCollisionBatch", 6, Debug::Range(1, 9));
Debug::Int g_dbIslandSizeThreshold("Gravity.Solver.SmallIslandThreshold", 4, Debug::Range(1, 100));
Debug::Bool g_dbDisableDeactivation("Gravity.DisableDeactivation", false);
Debug::Bool g_dbDoActiveChildComparison("Gravity.DoActiveChildComparison", true);


extern Scheduler::jqModule SolverModule;
extern Scheduler::jqModule WaterQueryModule;
extern Scheduler::jqModule RayTraceModule;
extern Scheduler::jqModule TransparentRayTraceModule;
extern Scheduler::jqModule DynamicCollisionModule;
extern Scheduler::jqModule DynamicPhantomCollisionModule;
extern Scheduler::jqModule MeshCollisionModule;
extern Scheduler::jqModule PhantomMeshCollisionModule;
extern Scheduler::jqModule MeshCollisionModule2;
extern Scheduler::jqModule GroundPlaneCollisionModule;
extern Scheduler::jqModule FluidCollisionModule;
extern Scheduler::jqModule ContinuousCollisionModule;
extern Scheduler::jqModule UpdateRagdollControllerModule;
extern Scheduler::jqModule PostIntegrateModule;
extern Scheduler::jqModule AnimatedShapeControllerModule;
extern Scheduler::jqModule KinematicControllerModule;
extern Scheduler::jqModule IntegrateModule;
extern Scheduler::jqModule BroadPhaseModule;
extern Scheduler::jqModule WakeUpBroadPhaseModule;
extern Scheduler::jqModule MeshCollisionDispatchModule;
extern Scheduler::jqModule ClothCollisionModule;

#endif

const U32 ContactStreamSize = 16192*32;
const U32 MaxColliders = 8192;


namespace Gravity
{

#ifndef PLATFORM_SPU

	C_Scene::C_Scene(SharedPhysics::C_MaterialPair* materialMatrix, const U32 numMaterials) : m_Stack(4096), m_CommandIter(NULL),
		m_SPUAllocator(32, 65536), m_pCollisionCallback(NULL), m_MinTimeStep(1.f/60.f), m_RemainingTime(Maths::Constants<Maths::HF32>::Zero()), m_pGroundPlane(NULL), m_pEventCallback(NULL), m_NumActiveBodies(0),
	  m_pActiveControllers(NULL), m_pActiveAnimatedShapeControllers(NULL), m_pActiveRagdollControllers(NULL), m_NumIterations(g_dbNumIterations), m_JointIterationFrequency(g_dbJointIterationFrequency),
	  m_BodyIndex(0), m_pStaticUserData(NULL)
	{
		m_pTree = new Gravity::C_DistributionTree;
		m_pCollisionManager = new Collision::C_CollisionManager(ContactStreamSize, MaxColliders);

		m_pCommandManager1 = new SharedPhysics::MemoryAllocator<C_DeferredBodyCommand, 32, false>;
		m_pCommandManager2 = new SharedPhysics::MemoryAllocator<C_DeferredBodyCommand, 32, false>;
		m_pContinuousCommandList = new SharedPhysics::MemoryAllocator<C_DeferredCCDCommand, 32, false>;
		m_pTaskBuffer = new SharedPhysics::MemoryAllocator<C_TaskBuffer<TaskSize>, TaskBufferSize, false>;

		m_pCommandManager = m_pCommandManager1;
		m_pOtherCommandManager = m_pCommandManager2;
		m_CommandIter.SetAllocator(m_pCommandManager);
		m_pBodyManager = new SharedPhysics::C_AllocationManager<C_Body, 16, true>;
		m_pSpringManager = new SharedPhysics::C_AllocationManager<SharedPhysics::C_Spring, 4, true>;
		//m_pCollisionQueries = new SharedPhysics::MemoryAllocator<C_CollisionQuery, 256, false>;
		//m_pContactPairs = new SharedPhysics::MemoryAllocator<Gravity::C_ObjectPair, 128>;
		m_pClothPairs = new SharedPhysics::MemoryAllocator<Gravity::C_ObjectPair, 128>;
		//m_pFluidPairs = new SharedPhysics::MemoryAllocator<Gravity::C_ObjectPair, 128>;
//#ifdef USE_SOLVER_2
		m_pGraphManager = new Solver2::C_ConstraintGraphManager(materialMatrix, numMaterials);
		//m_pMultiBody = new Solver2::C_MultiBody();
//#else
//		m_pGraphManager = new Solver::C_ConstraintGraphManager(materialMatrix, numMaterials);
//#endif
		//m_pNullBody = new NullBody;
		m_pWorldBody = new C_Body;
		m_pWorldBody->MakeStatic();
		m_pWorldBody->m_GraphNode.m_IslandIndex = 0;
		m_pWorldBody->FlagInsertedIntoWorld();
		m_pWorldBody->SetScene(this);
		SharedPhysics::C_RigidBody& rigid = m_pWorldBody->m_RigidBody;
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		const Maths::HMat33 identity(Maths::Constants<Maths::HMat33>::Identity());
		rigid.StoreVelocitiesAndPosition(zero, zero, identity, zero);
		m_pGraphManager->InsertGraphNode(&m_pWorldBody->m_GraphNode);

		m_WorldBounds.SetInvalidExtents();

		//m_Group = Scheduler::CreateGroup();
		//m_BroadPhaseGroup = Scheduler::CreateGroup();
		//m_pIslands = new SharedPhysics::MemoryAllocator<C_SolverIsland, 32>;
	}

	/*C_Scene() : m_pFirstCommand(NULL), m_Stack(16192), m_pCollisionCallback(NULL)
	{
		//m_pBodyManager = new C_AllocationManager<C_Body, 64, false>;
	}*/
	C_Scene::~C_Scene()
	{
		//Wait for all deferred tasks to be completed!!!
		
		//Scheduler::DestroyGroup(m_BroadPhaseGroup);
		//Scheduler::DestroyGroup(m_Group);

		Scheduler::Flush(m_BroadPhaseGroup);
		Scheduler::Flush(m_Group);
		//Lock out any deferred raytraces
		m_ReadWriteLock.WriteLock();


		//Clear the stuff the raytraces query against
		this->m_MeshCollisionBodies.m_numElements = 0;

		ProcessCommands();
		//delete m_pCollisionQueries;
		delete m_pTree;
		m_pTree = NULL;

		
		//Release the write lock
		m_ReadWriteLock.ReleaseWriteLock();

		//delete m_pIslands;

		//Clean up all the other structures
		delete m_pCollisionManager;
		delete m_pBodyManager;
		delete m_pSpringManager;
		//delete m_pContactPairs;
		delete m_pClothPairs;
		//delete m_pFluidPairs;
		delete m_pCommandManager1;
		delete m_pCommandManager2;
		delete m_pGraphManager;
		delete m_pContinuousCommandList;
		//delete m_pMultiBody;
		//delete m_pNullBody;
		delete m_pWorldBody;
		delete m_pTaskBuffer;
	}

	void C_Scene::SetInternalGroupHandle(Scheduler::GroupHandle handle)
	{
		m_Group = handle;
	}

	void C_Scene::SetBroadPhaseGroupHandle(Scheduler::GroupHandle handle)
	{
		m_BroadPhaseGroup = handle;
	}

	void C_Scene::SetStaticUserData(void* pUserData)
	{
		m_pStaticUserData = pUserData;
	}

	void* C_Scene::GetStaticUserData()
	{
		return m_pStaticUserData;
	}

	//Call after resetting the level in game code...
	void C_Scene::ResetBodyIndex()
	{
		m_BodyIndex = 0;
	}


	void C_Scene::SetNumSolverIterations(const U32 numSolverIterations)
	{
		m_NumIterations = numSolverIterations;
	}

	void C_Scene::SetJointIterationFrequency(const U32 jointIterationFrequency)
	{
		m_JointIterationFrequency = jointIterationFrequency;
	}

	void C_Scene::SetGroundPlane(Gravity::C_Body* pGroundPlane)
	{
		BIZ_ASSERT(pGroundPlane != m_pGroundPlane);
		if(m_pGroundPlane)
		{
			RemoveStaticBodyFromWorld(m_pGroundPlane);
		}
		m_pGroundPlane = pGroundPlane;
		if(m_pGroundPlane)
		{
			AddStaticBody(m_pGroundPlane);
		}
	}

	Gravity::C_Body* C_Scene::GetGroundPlane()
	{
		return m_pGroundPlane;
	}

	C_TriggerSystem* C_Scene::GetTriggerSystem()
	{
		return &m_TriggerSystem;
	}


	void C_Scene::CompletePhantomCollisionQuery(SharedPhysics::C_ContactStream& stream, Scheduler::PolicyHandle policy, Scheduler::GroupHandle group)
	{
		Scheduler::Flush(group);
		SharedPhysics::C_ContactStreamResult* res = (SharedPhysics::C_ContactStreamResult*)stream.m_pCurrentWritePoint;
		res->m_ValidData = SharedPhysics::E_NullData;
	}

	void C_Scene::GetCollisionPhantomIntersectionMesh(Gravity::C_CollisionPhantom* pPhantom, 
		SharedPhysics::C_ContactStream& stream, Scheduler::PolicyHandle policy,	Scheduler::GroupHandle group)
	{
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		C_PhantomCollisionQuery query;
		//Maths::Store(time, &query.m_Time);
		query.m_pStream = &stream;
		query.m_pManager = m_pCollisionManager;
		query.m_NumPairs = 0;

		for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
		{
			C_Body* pMeshBody = m_MeshCollisionBodies[a].m_pElement;
			Collision::C_Shape* pShape = pMeshBody->m_pShape;

			if(pShape->CanCollide(pPhantom->m_pShape))
			{
				//Kick off collision detection with world!
				query.m_pObjectA[query.m_NumPairs] = &pPhantom->m_Object;
				query.m_pObjectB[query.m_NumPairs++] = &pMeshBody->m_Object;

				if(query.m_NumPairs == MaxPhantomCollisionQuery)
				{
					Scheduler::AddTask(PhantomMeshCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_CollisionQuery));
					query.m_NumPairs = 0;
				}
			}
		}

		if(query.m_NumPairs > 0)
		{
			Scheduler::AddTask(PhantomMeshCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_CollisionQuery));
			query.m_NumPairs = 0;
		}
	}


	void C_Scene::GetCollisionPhantomIntersection(Gravity::C_CollisionPhantom* pPhantom, 
		SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack,
		Scheduler::PolicyHandle policy,	Scheduler::GroupHandle group)
	{
		//Go through broad phase and get collisions with this body!

		//Loop through all the meshes and get contacts out for them!

		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		C_PhantomCollisionQuery query;
		//Maths::Store(time, &query.m_Time);
		query.m_pStream = &stream;
		query.m_pManager = m_pCollisionManager;
		query.m_NumPairs = 0;

		for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
		{
			C_Body* pMeshBody = m_MeshCollisionBodies[a].m_pElement;
			Collision::C_Shape* pShape = pMeshBody->m_pShape;

			if(pShape->CanCollide(pPhantom->m_pShape))
			{
				//Kick off collision detection with world!
				query.m_pObjectA[query.m_NumPairs] = &pPhantom->m_Object;
				query.m_pObjectB[query.m_NumPairs++] = &pMeshBody->m_Object;

				if(query.m_NumPairs == MaxPhantomCollisionQuery)
				{
					Scheduler::AddTask(PhantomMeshCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_CollisionQuery));
					query.m_NumPairs = 0;
				}
			}
		}

		if(query.m_NumPairs > 0)
		{
			Scheduler::AddTask(PhantomMeshCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_CollisionQuery));
			query.m_NumPairs = 0;
		}

		if(this->m_pTree)
		{
			//Loop through tree and find anything which it hits!!!

			C_DistributionNode* pStackTop = m_pTree->m_pRootNode;
			S32 size = 1;
			stack.Push(m_pTree->m_pRootNode, 0);

			C_Object* pObj = &pPhantom->m_Object;
			Collision::C_CRAxisBox& phantomBox = pObj->m_TransformedBox;

			while(size--)
			{
				C_DistributionNode* pNode = pStackTop;
				pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(0, size - 1));

				


				Collision::C_AxisBoxMinMax& box(pNode->m_bbox);
				if(box.Intersects(pObj->m_TransformedBox))
				{
					if(pNode->m_pLeftNode)
					{
						stack.Push(pNode->m_pRightNode, size++);
						stack.Push(pNode->m_pLeftNode, size++);
						pStackTop = pNode->m_pLeftNode;
					}
					//Loop through the static and kinematic bodies first!!!
					C_ObjectWrapper* pObjWrapper = pNode->m_pStaticObjects;
					while(pObjWrapper)
					{
						C_Object* pStaticObj = pObjWrapper->m_pObject;
						Collision::C_CRAxisBox& staticBox=pStaticObj->m_TransformedBox;
						if(phantomBox.IntersectsInAxisBox(staticBox, box) && pObj->m_pShape->CanCollide(pStaticObj->m_pShape))
						{
							//C_ObjectPair* pair=pObjPair.Create(index++);
							query.m_pObjectA[query.m_NumPairs] = pObj;
							query.m_pObjectB[query.m_NumPairs++] = pStaticObj;
							if(query.m_NumPairs == MaxPhantomCollisionQuery)
							{
								Scheduler::AddTask(DynamicPhantomCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_PhantomCollisionQuery));
								query.m_NumPairs = 0;
							}
						}
						pObjWrapper = pObjWrapper->m_pNext;
					}

					pObjWrapper = pNode->m_pKinematicObjects;
					while(pObjWrapper)
					{
						C_Object* pStaticObj = pObjWrapper->m_pObject;
						Collision::C_CRAxisBox& staticBox=pStaticObj->m_TransformedBox;
						if(phantomBox.IntersectsInAxisBox(staticBox, box) && pObj->m_pShape->CanCollide(pStaticObj->m_pShape))
						{
							//C_ObjectPair* pair=pObjPair.Create(index++);
							query.m_pObjectA[query.m_NumPairs] = pObj;
							query.m_pObjectB[query.m_NumPairs++] = pStaticObj;
							if(query.m_NumPairs == MaxPhantomCollisionQuery)
							{
								Scheduler::AddTask(DynamicPhantomCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_PhantomCollisionQuery));
								query.m_NumPairs = 0;
							}
						}
						pObjWrapper = pObjWrapper->m_pNext;
					}

					pObjWrapper = pNode->m_pActiveObjects;
					while(pObjWrapper)
					{
						C_Object* pStaticObj = pObjWrapper->m_pObject;
						Collision::C_CRAxisBox& staticBox=pStaticObj->m_TransformedBox;
						if(phantomBox.IntersectsInAxisBox(staticBox, box) && pObj->m_pShape->CanCollide(pStaticObj->m_pShape))
						{
							//C_ObjectPair* pair=pObjPair.Create(index++);
							query.m_pObjectA[query.m_NumPairs] = pObj;
							query.m_pObjectB[query.m_NumPairs++] = pStaticObj;
							if(query.m_NumPairs == MaxPhantomCollisionQuery)
							{
								Scheduler::AddTask(DynamicPhantomCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_PhantomCollisionQuery));
								query.m_NumPairs = 0;
							}
						}
						pObjWrapper = pObjWrapper->m_pNext;
					}

					pObjWrapper = pNode->m_pSleepingObjects;
					while(pObjWrapper)
					{
						C_Object* pStaticObj = pObjWrapper->m_pObject;
						Collision::C_CRAxisBox& staticBox=pStaticObj->m_TransformedBox;
						if(phantomBox.IntersectsInAxisBox(staticBox, box) && pObj->m_pShape->CanCollide(pStaticObj->m_pShape))
						{
							//C_ObjectPair* pair=pObjPair.Create(index++);
							query.m_pObjectA[query.m_NumPairs] = pObj;
							query.m_pObjectB[query.m_NumPairs++] = pStaticObj;
							if(query.m_NumPairs == MaxPhantomCollisionQuery)
							{
								Scheduler::AddTask(DynamicPhantomCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_PhantomCollisionQuery));
								query.m_NumPairs = 0;
							}
						}
						pObjWrapper = pObjWrapper->m_pNext;
					}

				}

			}
			if(query.m_NumPairs > 0)
			{
				Scheduler::AddTask(DynamicPhantomCollisionModule, NULL, NULL, group, policy, &query, sizeof(C_PhantomCollisionQuery));
				query.m_NumPairs = 0;
			}
		}		
	}
	
	U32 C_Scene::GetIntersectingBodies(const Collision::C_Sphere& sphere, Collision::C_CollisionFilter* pFilter,
		SharedPhysics::MemoryAllocator<C_Body*>& allocator, SharedPhysics::C_SafeStack<true>& stack)
	{
		return m_pTree->GetIntersectingBodies(sphere, pFilter, allocator, stack);
	}


	void C_Scene::ResizeScene(Collision::C_AxisBoxMinMax PCREF worldBox, const Maths::HVec3 PCREF desiredSize,
		const Maths::HVec3 PCREF maxCellSize, S32 maxDepth)
	{
	/*	m_PhysicsLock.Wait();
		m_pTree->SetDimensions(worldBox, desiredSize, maxDepth);
		m_PhysicsLock.Release();*/
		m_WorldBounds = worldBox;
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());

		pCommand->m_CommandType = C_DeferredBodyCommand::E_ResizeWorld;
		pCommand->m_CommandBundle.SetSceneSize(C_SceneSize(worldBox, desiredSize, maxCellSize, maxDepth));
		AddCommand(pCommand);
	}

	SharedPhysics::C_ConstraintEdge* C_Scene::GetConstraintEdge()
	{
		return m_pGraphManager->m_PersistentConstraintEdges.GetNextFreeItem();
	}

	/*SharedPhysics::C_ConstraintTwin* C_Scene::GetConstraintTwin()
	{
		return m_pGraphManager->m_PersistentTwinConstraints.GetNextFreeItem();
	}*/

	SharedPhysics::C_ConstraintPoint* C_Scene::GetConstraintPoint()
	{
		return m_pGraphManager->m_PersistentConstraintPoints.GetNextFreeItem();
	}

	Gravity::C_DistributionTree* C_Scene::GetBroadPhase()
	{
		return m_pTree;
	}

	C_DeferredBodyCommand* C_Scene::GetCommand()
	{
		return m_CommandIter.Create();
	}

	void C_Scene::AddCommand(C_DeferredBodyCommand* pCommand)
	{
		/*pCommand->m_pNext = m_pFirstCommand;
		m_pFirstCommand = pCommand;*/
		m_CommandIter.m_Mutex.Exit();
	}

	void C_Scene::ProcessCommands()
	{
		PERF_EVENT("Gravity.ProcessCommands");
		m_CommandIter.m_Mutex.Enter();
		const U32 numCommands = m_CommandIter.m_Iterator.NumElements();
		SharedPhysics::C_MemoryIterator<C_DeferredBodyCommand, 32, false> iter(m_pCommandManager);
		SharedPhysics::MemoryAllocator<C_DeferredBodyCommand, 32, false>* pOther =  m_pCommandManager;
		m_pCommandManager = m_pOtherCommandManager;
		m_pOtherCommandManager = pOther;
		m_CommandIter.SetAllocator(m_pCommandManager);
		m_CommandIter.m_Mutex.Exit();

		for(U32 a = 0; a < numCommands; ++a)
		{
			C_DeferredBodyCommand* pCommand = iter.Get();
			if(pCommand->m_CommandType < C_DeferredBodyCommand::E_MaxBodyCommands)
			{
				C_Body* pBody = pCommand->m_CommandBundle.m_pBody;
				pBody->ApplyDeferredCommand(*pCommand);
			}
			else
			{
				//Scene commands!
				ApplyDeferredCommand(*pCommand);
			}
		}
		pOther->Reset();
	}

	void C_Scene::ApplyDeferredCommand(C_DeferredBodyCommand& command)
	{
		//List of events to process!

		switch(command.m_CommandType)
		{
		case C_DeferredBodyCommand::E_AddKinematicController:
			{
				KinematicControllerPtr ptr(command.m_CommandBundle.m_pKinematicController);
				m_KinematicControllers.Insert(&ptr);
				break;
			}
		case C_DeferredBodyCommand::E_RemoveKinematicController:
			{
				KinematicControllerPtr ptr(command.m_CommandBundle.m_pKinematicController);
				m_KinematicControllers.Remove(&ptr);
				break;
			}
		case C_DeferredBodyCommand::E_AddAnimatedShapeController:
			{
				AnimatedShapeControllerPtr ptr(command.m_CommandBundle.m_pAnimatedShapeController);
				m_AnimatedShapeControllers.Insert(&ptr);
				break;
			}
		case C_DeferredBodyCommand::E_RemoveAnimatedShapeController:
			{
				AnimatedShapeControllerPtr ptr(command.m_CommandBundle.m_pAnimatedShapeController);
				m_AnimatedShapeControllers.Remove(&ptr);
				break;
			}
		case C_DeferredBodyCommand::E_AddRagdollController:
			{
				RagdollControllerPtr ptr(command.m_CommandBundle.m_pRagdollController);
				m_RagdollControllers.Insert(&ptr);
				break;
			}
		case C_DeferredBodyCommand::E_RemoveRagdollController:
			{
				RagdollControllerPtr ptr(command.m_CommandBundle.m_pRagdollController);
				m_RagdollControllers.Remove(&ptr);
				break;
			}
		case C_DeferredBodyCommand::E_AddCharacterController:
			{
				ControllerPtr ptr(command.m_CommandBundle.m_pCharacterController);
				m_CharacterControllers.Insert(&ptr);
				break;
			}
		case C_DeferredBodyCommand::E_RemoveCharacterController:
			{
				ControllerPtr ptr(command.m_CommandBundle.m_pCharacterController);
				m_CharacterControllers.Remove(&ptr);
				break;
			}
		case C_DeferredBodyCommand::E_AddConstraint:
			{
				break;
			}
		case C_DeferredBodyCommand::E_RemoveConstraint:
			{
				m_pGraphManager->RemoveConstraints( command.m_CommandBundle.m_pConstraint);
				m_pGraphManager->RemoveDebugConstraints( command.m_CommandBundle.m_pConstraint);
				/*ConstraintPtr ptr(command.m_CommandBundle.m_pConstraint);
				m_Constraints.Remove(&ptr);*/
				break;
			}
		case C_DeferredBodyCommand::E_AddSpring:
			{
				SpringPtr wrapper(command.m_CommandBundle.m_pSpring);
				m_Springs.Insert(&wrapper);
				break;
			}
		case C_DeferredBodyCommand::E_RemoveSpring:
			{
				SpringPtr wrapper(command.m_CommandBundle.m_pSpring);
				m_Springs.Remove(&wrapper);
				m_pSpringManager->PushToFree(command.m_CommandBundle.m_pSpring);
				break;
			}
		case C_DeferredBodyCommand::E_ResizeWorld:
			{
				C_SceneSize sceneSize = command.m_CommandBundle.GetSceneSize();
				m_pTree->SetDimensions(sceneSize.m_WorldBox, sceneSize.m_DesiredSize, sceneSize.m_MaxCellSize, sceneSize.m_MaxDepth);
				break;
			}
		case C_DeferredBodyCommand::E_DeleteImportScene:
			{
				C_ImportScene* pImport = command.m_CommandBundle.m_pImportScene;
				delete pImport;
				break;
			}
		}
	}


	C_Body* C_Scene::CreateBody()
	{
		MT::AutoLock lock(m_BodyLock);
		C_Body* pBody =  m_pBodyManager->GetNextFreeItem();
		pBody->m_BodyFlags = pBody->m_BodyFlags & ~C_Body::E_Type_Removed;
		pBody->SetIndex(m_BodyIndex++);
		return pBody;		
	}

	void C_Scene::ReleaseBody(C_Body* pBody)
	{
		pBody->SetScene(NULL);
		m_BodyLock.Enter();
		m_pBodyManager->PushToFree(pBody);
		m_BodyLock.Exit();
		//delete pBody;
	}

	void C_Scene::AddBody(C_Body* pBody)
	{
		U32 state = pBody->m_Object.m_ObjectState & (~(E_Dirty | E_Removed));
		BIZ_ASSERT((state >= E_ActiveObject && state <= E_SleepingDebrisObject));
		BIZ_ASSERT(!pBody->TestBodyFlags(C_Body::E_Type_Removed | C_Body::E_InsertedIntoWorld));
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddBody;
		pBody->SetScene(this);
		pCommand->m_CommandBundle.m_pBody = pBody;
		AddCommand(pCommand);
		
	}

	void C_Scene::AddStaticBody(C_Body* pBody)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddStaticBody;
		pBody->SetScene(this);
		pCommand->m_CommandBundle.m_pBody = pBody;
		AddCommand(pCommand);
	}


	void C_Scene::RemoveBody(C_Body* pBody, C_BodyRemoveCallback* pCallback)
	{
		//BodyPtr wrapper(pBody);
		//m_Bodies.Remove(&wrapper);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pCommand->m_CommandBundle.m_pScene = this;
		pCommand->m_CommandBundle.m_pUserData = pBody->m_pUsrData;
		pCommand->m_CommandBundle.m_pCallback = pCallback;
		pBody->SetScene(this);
		pBody->SetUserData(NULL); //assuming they won't need this ptr anymore - causes crashes in Nightwing if not set to null
		pBody->SetExternalConstants(NULL);
		AddCommand(pCommand);
		
	}

	void C_Scene::ReleaseImportScene(C_ImportScene* pImportScene)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_DeleteImportScene;
		pCommand->m_CommandBundle.m_pImportScene = pImportScene;
		AddCommand(pCommand);
	}

	void C_Scene::TemporarilyRemoveBody(C_Body* pBody)
	{
		//BodyPtr wrapper(pBody);
		//m_Bodies.Remove(&wrapper);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveBodyNoDelete;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pCommand->m_CommandBundle.m_pScene = this;
		pBody->SetScene(this);
		AddCommand(pCommand);
	}



	void C_Scene::RemoveStaticBody(C_Body* pBody)
	{
		//BodyPtr wrapper(pBody);
		//m_Bodies.Remove(&wrapper);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveStaticBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_pOwningScene = this;
		AddCommand(pCommand);
	}

	void C_Scene::AddClothBody(C_Body* pBody)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddClothBody;
		pBody->SetScene(this);
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Cloth | SharedPhysics::C_GraphNode::E_Type_NoIntegrate);
		
		AddCommand(pCommand);
	}

	void C_Scene::RemoveClothBody(C_Body* pBody)
	{
		//BodyPtr wrapper(pBody);
		//m_Bodies.Remove(&wrapper);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveClothBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_pOwningScene = this;
		AddCommand(pCommand);
	}

	void C_Scene::AddDeformableBody(C_Body* pBody)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddDeformableBody;
		pBody->SetScene(this);
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_GraphNode.SetExtendedNodeFlags(SharedPhysics::C_GraphNode::E_Type_Deformable);
		
		AddCommand(pCommand);
	}

	void C_Scene::RemoveDeformableBody(C_Body* pBody)
	{
		//BodyPtr wrapper(pBody);
		//m_Bodies.Remove(&wrapper);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveDeformableBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_pOwningScene = this;
		AddCommand(pCommand);
	}

	void C_Scene::AddRopeBody(C_Body* pBody)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddRopeBody;
		pBody->SetScene(this);
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Rope | SharedPhysics::C_GraphNode::E_Type_NoIntegrate);
		
		AddCommand(pCommand);
	}

	void C_Scene::RemoveRopeBody(C_Body* pBody)
	{
		//BodyPtr wrapper(pBody);
		//m_Bodies.Remove(&wrapper);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveRopeBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_pOwningScene = this;
		AddCommand(pCommand);
	}


	void C_Scene::AddFluidBody(C_Body* pBody)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddFluidBody;
		pBody->SetScene(this);
		pCommand->m_CommandBundle.m_pBody = pBody;
		AddCommand(pCommand);
	}

	void C_Scene::RemoveFluidBody(C_Body* pBody)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveFluidBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_pOwningScene = this;
		AddCommand(pCommand);

	}

	void C_Scene::AddFluidMeshBody(C_Body* pBody)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddFluidMeshBody;
		pBody->SetScene(this);
		pCommand->m_CommandBundle.m_pBody = pBody;
		AddCommand(pCommand);
	}

	void C_Scene::RemoveFluidMeshBody(C_Body* pBody)
	{

		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveFliudMeshBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_pOwningScene = this;
		AddCommand(pCommand);

	}

	void C_Scene::AddMeshBody(C_Body* pBody)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddMeshBody;
		pBody->SetScene(this);
		pCommand->m_CommandBundle.m_pBody = pBody;
		AddCommand(pCommand);
		/*BodyPtr wrapper(pBody);
		m_MeshCollisionBodies.Insert(&wrapper);
		pBody->FlagInsertedIntoWorld();
		pBody->m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_StaticMesh | SharedPhysics::C_GraphNode::E_Type_Static);
		pBody->SetScene(this);
		m_pGraphManager->InsertGraphNode(&pBody->m_GraphNode);*/
	}

	void C_Scene::RemoveMeshBody(C_Body* pBody)
	{
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveMeshBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->SetScene(this);
		AddCommand(pCommand);
		/*BodyPtr wrapper(pBody);
		m_MeshCollisionBodies.Remove(&wrapper);
		m_pGraphManager->RemoveGraphNode(&pBody->m_GraphNode, m_EventStream);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_ReleaseBody;
		pCommand->m_pBody = pBody;
		pBody->m_pOwningScene = this;
		AddCommand(pCommand);*/
	}

	void C_Scene::AddDebugConstraint(SharedPhysics::C_Constraint* pConstraint)
	{
		/*ConstraintPtr wrapper(pConstraint);
		m_Constraints.Insert(&wrapper);*/
		m_pGraphManager->AddDebugConstraints(pConstraint);
		
	}

	void C_Scene::RemoveConstraint(SharedPhysics::C_Constraint* pConstraint)
	{
	/*	ConstraintPtr wrapper(pConstraint);
		m_Constraints.Remove(&wrapper);*/
		//m_pGraphManager->RemoveDebugConstraints(pConstraint);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveConstraint;
		pCommand->m_CommandBundle.m_pConstraint = pConstraint;	
	}

	const U32 C_Scene::NumConstraints() const
	{
		//return m_Constraints.m_numElements;
		return m_pGraphManager->m_DebugConstraints.m_numElements;
	}

	SharedPhysics::C_Constraint* C_Scene::GetConstraint(const U32 index)
	{
		return m_pGraphManager->m_DebugConstraints[index].m_pElement;
	}

	void C_Scene::ClearConstraints()
	{
		//m_Constraints.m_numElements = 0;
		m_pGraphManager->m_DebugConstraints.m_numElements =0;
	}

	void C_Scene::AddSpring(SharedPhysics::C_Spring* pSpring)
	{

		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddSpring;
		pCommand->m_CommandBundle.m_pSpring= pSpring;
		AddCommand(pCommand);
		
	}

	void C_Scene::RemoveSpring(SharedPhysics::C_Spring* pSpring)
	{

		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveSpring;
		pCommand->m_CommandBundle.m_pSpring = pSpring;
		AddCommand(pCommand);
	}

	const U32 C_Scene::NumSprings() const
	{
		return m_Springs.m_numElements;
	}

	SharedPhysics::C_Spring* C_Scene::GetSpring(const U32 index)
	{
		SpringPtr wrapper = m_Springs[index];
		return wrapper.m_pElement;
	}

	void C_Scene::ClearSprings()
	{
		m_Springs.m_numElements = 0;
	}



	void C_Scene::AddReferenceFrame(C_ReferenceFrame* pReferenceFrame)
	{
		ReferenceFramePtr wrapper(pReferenceFrame);
		m_ReferenceFrames.Insert(&wrapper);
	}

	void C_Scene::RemoveReferenceFrame(C_ReferenceFrame* pReferenceFrame)
	{
		ReferenceFramePtr wrapper(pReferenceFrame);
		m_ReferenceFrames.Remove(&wrapper);
	}

	const U32 C_Scene::NumReferenceFrames() const
	{
		return m_ReferenceFrames.m_numElements;
		
	}

	C_ReferenceFrame* C_Scene::GetReferenceFrame(const U32 index)
	{
		ReferenceFramePtr wrapper = m_ReferenceFrames[index];
		return wrapper.m_pElement;
	}

	void C_Scene::ClearReferenceFrame()
	{
		m_ReferenceFrames.m_numElements = 0;
	}

	void C_Scene::InsertBodyInWorld(C_Body* pBody)
	{
		BIZ_ASSERT(!pBody->TestBodyFlags(C_Body::E_Type_Removed | C_Body::E_InsertedIntoWorld));
		if(pBody->TestBodyFlags(C_Body::E_InsertedIntoWorld | C_Body::E_Type_Removed))
			return;
		//m_ReadWriteLock.WriteLock();

		if(pBody->GetShape()->HasFluid())
		{
			pBody->m_Object.m_ObjectState = E_FluidMeshObject;
		}
		
		if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
		{
			BIZ_ASSERT((pBody->m_Object.m_ObjectState & ~(Gravity::E_Removed | Gravity::E_Dirty)) == Gravity::E_RaytraceOnlyObject);
		}


		BodyPtr wrapper(pBody);
		m_Bodies.Insert(&wrapper);
		pBody->FlagInsertedIntoWorld();
		m_pGraphManager->InsertGraphNode(&pBody->m_GraphNode);
		pBody->SetScene(this);
		m_pTree->AddObject(&pBody->m_Object);

		if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
		{
			m_KinematicBodyList.AddBody(pBody);
		}
		//if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Active))
		else if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			//Add to active list!
			m_ActiveBodyList.AddBody(pBody);
		}
		else if(!(pBody->m_Object.m_ObjectState & Gravity::E_NoCollision))
		{
			m_pTree->AddDirtyObject(&pBody->m_Object);
			BodyPtr bodyPtr(pBody);
			m_OverriddenBodyList.AddToEnd(&bodyPtr);
			pBody->m_GraphNode.m_IslandIndex = 0;
		}
		pBody->SynchroniseState();
		pBody->SynchroniseStateFlags();
	}

	void C_Scene::WakeUpSleepingBodies(Collision::C_CRAxisBox& box, Collision::C_CollisionFilter* pFilter)
	{
		C_DistributionNode* pStackTop = m_pTree->m_pRootNode;
		S32 size = 1;

		while(size--)
		{
			C_DistributionNode* pNode = pStackTop;
			pStackTop = m_Stack.Pop<C_DistributionNode>(Maths::Max(size - 1, 0));

			if(pNode->m_bbox.Intersects(box))
			{
				C_ObjectWrapper* pWrapper = pNode->m_pSleepingObjects;
				while(pWrapper)
				{
					C_ObjectWrapper* pNextWrapper = pWrapper->m_pNext;
					if(pWrapper->m_pObject->m_TransformedBox.Intersects(box))
					{
						C_Body* pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						pBody->m_ExternalState.m_ExternalBodyState &= ~(SharedPhysics::C_GraphNode::E_Type_ZeroInertia);
						pBody->m_ExternalState.m_ExternalBodyState |= SharedPhysics::C_GraphNode::E_Type_Active;
						pBody->_Activate();
						//pWrapper->m_pObject->m_pGraphNode->m_pBody->_Activate();
					}
					pWrapper = pNextWrapper;
				}
				if(pNode->m_pLeftNode)
				{
					pStackTop = pNode->m_pLeftNode;
					m_Stack.Push(pNode->m_pRightNode, size);
					size+= 2;
				}
			}
		}

	}


	void C_Scene::RemoveBodyFromWorld(C_Body* pBody, C_BodyRemoveCallback* pCallback, void* pUserData)
	{
		//this->m_CollisionManager.m_ContactDatabase.PurgeAllContacts(&pBody->m_GraphNode);
		BIZ_ASSERT((pBody->m_BodyFlags & (C_Body::E_Type_Removed)) == 0);
		pBody->m_BodyFlags |= C_Body::E_Type_Removed;

		m_pTree->RemoveObject(&pBody->m_Object);
		
		BodyPtr ptr(pBody);
		m_pGraphManager->RemoveGraphNode(&pBody->m_GraphNode, m_EventStream, true);
		this->m_Bodies.Remove(&ptr);
		pBody->m_BodyFlags &= ~C_Body::E_InsertedIntoWorld;
		pBody->m_BodyFlags &= ~C_Body::E_TemporarilyRemoved;
		pBody->SetScene(NULL);

		WakeUpSleepingBodies(pBody->m_Object.m_TransformedBox, &pBody->m_pShape->m_Filter);
		
		/*m_ActiveBodyList.RemoveBody(pBody);
		m_ContinuousBodyList.RemoveBody(pBody);
		pBody->m_pNext = pBody->m_pPrev = NULL;*/
		if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Continuous))
		{
			m_ContinuousBodyList.RemoveBody(pBody);
		}
		else if (pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			m_ActiveBodyList.RemoveBody(pBody);
		}
		else if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
		{
			m_KinematicBodyList.RemoveBody(pBody);
		}
		pBody->m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Continuous);

		if(pCallback)
		{
			pCallback->BodyRemoved(pBody, pUserData);
		}

		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_ReleaseBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pCommand->m_CommandBundle.m_pScene = this;
		AddCommand(pCommand);
	}


	void C_Scene::RemoveBodyFromWorldNoDelete(C_Body* pBody)
	{
		//this->m_CollisionManager.m_ContactDatabase.PurgeAllContacts(&pBody->m_GraphNode);
		BIZ_ASSERT((pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld) != 0);
		//m_ReadWriteLock.WriteLock();
		m_pTree->RemoveObject(&pBody->m_Object);
		BodyPtr ptr(pBody);
		m_pGraphManager->RemoveGraphNode( &pBody->m_GraphNode, m_EventStream, false);
		this->m_Bodies.Remove(&ptr);
		pBody->m_BodyFlags &= ~C_Body::E_InsertedIntoWorld;
		pBody->m_BodyFlags |= C_Body::E_TemporarilyRemoved;
		pBody->m_pOwningScene = NULL;
		if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Continuous))
		{
			m_ContinuousBodyList.RemoveBody(pBody);
		}
		else if (pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			m_ActiveBodyList.RemoveBody(pBody);
		}
		else if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
		{
			m_KinematicBodyList.RemoveBody(pBody);
		}
		
		
		//m_ReadWriteLock.ReleaseWriteLock();
	}

	void C_Scene::InsertStaticBodyInWorld(C_Body* pBody)
	{
		BIZ_ASSERT(!(pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld));
		//m_ReadWriteLock.WriteLock();
		pBody->SynchronisePosRot();
		pBody->SynchroniseState();
		pBody->SynchroniseStateFlags();
		BodyPtr wrapper(pBody);
		m_StaticBodies.Insert(&wrapper);
		pBody->FlagInsertedIntoWorld();
		m_pGraphManager->InsertStaticGraphNode(&pBody->m_GraphNode);
		m_pTree->AddStaticObject(&pBody->m_Object);
		BodyPtr bodyPtr(pBody);
		m_OverriddenBodyList.AddToEnd(&bodyPtr);
		pBody->m_GraphNode.m_IslandIndex = 0;
		pBody->m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_State_Changed);
		//m_pTree->AddDirtyObject(&pBody->m_Object);
		
		//m_ReadWriteLock.ReleaseWriteLock();
	}


	void C_Scene::RemoveStaticBodyFromWorld(C_Body* pBody)
	{
		BIZ_ASSERT((pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld)!= 0);

		pBody->m_BodyFlags &= ~C_Body::E_InsertedIntoWorld;
		pBody->m_BodyFlags &= ~C_Body::E_TemporarilyRemoved;
		m_pTree->RemoveStaticObject(&pBody->m_Object);
		BodyPtr ptr(pBody);
		m_pGraphManager->RemoveStaticGraphNode(&pBody->m_GraphNode);
		m_StaticBodies.Remove(&ptr);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_ReleaseBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pCommand->m_CommandBundle.m_pScene = this;
		AddCommand(pCommand);
	}

	void C_Scene::InsertMeshBodyInWorld(C_Body* pBody)
	{
		BodyPtr wrapper(pBody);
		m_MeshCollisionBodies.Insert(&wrapper);
		pBody->FlagInsertedIntoWorld();
		pBody->m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_StaticMesh | SharedPhysics::C_GraphNode::E_Type_Static);
		pBody->SetScene(this);
		m_pGraphManager->InsertGraphNode(&pBody->m_GraphNode);
		pBody->m_GraphNode.m_IslandIndex = 0;
		pBody->m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_State_Changed);
		pBody->SynchroniseState();
		pBody->SynchroniseStateFlags();
	}


	void C_Scene::RemoveMeshBodyFromWorld(C_Body* pBody)
	{

		pBody->m_BodyFlags &= ~C_Body::E_InsertedIntoWorld;
		pBody->m_BodyFlags &= ~C_Body::E_TemporarilyRemoved;
		BodyPtr wrapper(pBody);
		m_MeshCollisionBodies.Remove(&wrapper);
		m_pGraphManager->RemoveGraphNode( &pBody->m_GraphNode, m_EventStream, true);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_ReleaseBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pBody->m_pOwningScene = this;
		pCommand->m_CommandBundle.m_pScene = this;
		AddCommand(pCommand);
	}


	void C_Scene::InsertClothBodyInWorld(C_Body* pBody)
	{
		using namespace Collision;
		BIZ_ASSERT(!(pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld));
		BodyPtr wrapper(pBody);
		m_ClothBodies.Insert(&wrapper);
		pBody->FlagInsertedIntoWorld();
		m_pGraphManager->InsertGraphNode(&pBody->m_GraphNode);
		//Add into broad phase world...
		//Needs to go in cloth section!
		pBody->m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Cloth | 
			SharedPhysics::C_GraphNode::E_Type_NoIntegrate);
		if(pBody->IsActive())
		{
			pBody->m_Object.m_ObjectState = Gravity::E_ActiveClothObject;
			m_ActiveBodyList.AddBody(pBody);
		}
		else
		{
			pBody->m_Object.m_ObjectState = Gravity::E_SleepingClothObject;
		}

		if(pBody->GetShape()->m_ShapeType == Collision::E_Cloth_Shape)
		{
			C_ClothShape* pShape=(C_ClothShape*)pBody->GetShape();
			pShape->m_Cloth.GetBoundingBox(pShape->m_LocalBoundingBox);
			pBody->m_Object.m_TransformedBox = pShape->m_LocalBoundingBox;
			m_pTree->AddObject(&pBody->m_Object);
			m_pTree->AddDirtyObject(&pBody->m_Object);
		}
		else
		{
			pBody->m_Object.m_ObjectState = E_NoCollision;
		}
		pBody->SynchroniseState();
		pBody->SynchroniseStateFlags();
	}

	void C_Scene::RemoveClothBodyFromWorld(C_Body* pBody)
	{
		BIZ_ASSERT((pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld) != 0);
		pBody->m_BodyFlags &= ~C_Body::E_InsertedIntoWorld;
		pBody->m_BodyFlags &= ~C_Body::E_TemporarilyRemoved;
		//this->m_CollisionManager.m_ContactDatabase.PurgeAllContacts(&pBody->m_GraphNode);
		//m_Tree.RemoveObject(&pBody->m_Object);
		BodyPtr ptr(pBody);
		m_pGraphManager->RemoveGraphNode(&pBody->m_GraphNode, m_EventStream, true);
		m_ClothBodies.Remove(&ptr);
		m_pTree->RemoveObject(&pBody->m_Object);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_ReleaseBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pCommand->m_CommandBundle.m_pScene = this;
		if(pBody->IsActive())
			m_ActiveBodyList.RemoveBody(pBody);
		AddCommand(pCommand);
	}


	void C_Scene::InsertDeformableBodyInWorld(C_Body* pBody)
	{
		using namespace Collision;
		BIZ_ASSERT(!(pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld));
		BodyPtr wrapper(pBody);
		m_DeformableBodies.Insert(&wrapper);
		pBody->FlagInsertedIntoWorld();
		m_pGraphManager->InsertGraphNode(&pBody->m_GraphNode);
		//Add into broad phase world...
		//Needs to go in cloth section!
		pBody->m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Cloth | 
			SharedPhysics::C_GraphNode::E_Type_NoIntegrate);
		if(pBody->IsActive())
		{
			pBody->m_Object.m_ObjectState = Gravity::E_ActiveClothObject;
			m_ActiveBodyList.AddBody(pBody);
		}
		else
		{
			pBody->m_Object.m_ObjectState = Gravity::E_SleepingClothObject;
		}

		if(pBody->GetShape()->m_ShapeType == Collision::E_Cloth_Shape)
		{
			C_ClothShape* pShape=(C_ClothShape*)pBody->GetShape();
			pShape->m_Cloth.GetBoundingBox(pShape->m_LocalBoundingBox);
			pBody->m_Object.m_TransformedBox = pShape->m_LocalBoundingBox;
			m_pTree->AddObject(&pBody->m_Object);
			m_pTree->AddDirtyObject(&pBody->m_Object);
		}
		else
		{
			pBody->m_Object.m_ObjectState = E_NoCollision;
		}
		pBody->SynchroniseState();
		pBody->SynchroniseStateFlags();
	}

	void C_Scene::RemoveDeformableBodyFromWorld(C_Body* pBody)
	{
		BIZ_ASSERT((pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld) != 0);
		pBody->m_BodyFlags &= ~C_Body::E_InsertedIntoWorld;
		pBody->m_BodyFlags &= ~C_Body::E_TemporarilyRemoved;
		//this->m_CollisionManager.m_ContactDatabase.PurgeAllContacts(&pBody->m_GraphNode);
		//m_Tree.RemoveObject(&pBody->m_Object);
		BodyPtr ptr(pBody);
		m_pGraphManager->RemoveGraphNode(&pBody->m_GraphNode, m_EventStream, true);
		m_DeformableBodies.Remove(&ptr);
		m_pTree->RemoveObject(&pBody->m_Object);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_ReleaseBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pCommand->m_CommandBundle.m_pScene = this;
		if(pBody->IsActive())
			m_ActiveBodyList.RemoveBody(pBody);
		AddCommand(pCommand);
	}

	void C_Scene::InsertRopeBodyInWorld(C_Body* pBody)
	{
		BIZ_ASSERT(!(pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld));

		pBody->Position(Maths::Constants<Maths::HVec3>::Zero());

		BodyPtr wrapper(pBody);
		m_RopeBodies.Insert(&wrapper);
		pBody->FlagInsertedIntoWorld();
		m_pGraphManager->InsertGraphNode(&pBody->m_GraphNode);
		//Add into broad phase world...
		//Needs to go in cloth section!
		
		pBody->m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Rope | 
			SharedPhysics::C_GraphNode::E_Type_NoIntegrate);
		
		if(pBody->IsActive())
		{
			pBody->m_Object.m_ObjectState = Gravity::E_ActiveClothObject;
			m_ActiveBodyList.AddBody(pBody);
		}
		else
		{
			pBody->m_Object.m_ObjectState = Gravity::E_SleepingClothObject;
		}

		if(pBody->GetShape()->m_ShapeType == Collision::E_Rope_Shape)
		{
			Collision::C_RopeShape* pShape=(Collision::C_RopeShape*)pBody->GetShape();
			pShape->m_Rope.GetBoundingBox(pShape->m_LocalBoundingBox);
			pBody->m_Object.m_TransformedBox = pShape->m_LocalBoundingBox;
			m_pTree->AddObject(&pBody->m_Object);
			m_pTree->AddDirtyObject(&pBody->m_Object);
		}
		else
		{
			pBody->m_Object.m_ObjectState = E_NoCollision;
		}
		pBody->SynchroniseState();
		pBody->SynchroniseStateFlags();
	}

	void C_Scene::RemoveRopeBodyFromWorld(C_Body* pBody)
	{
		BIZ_ASSERT((pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld) != 0);
		pBody->m_BodyFlags &= ~C_Body::E_InsertedIntoWorld;
		pBody->m_BodyFlags &= ~C_Body::E_TemporarilyRemoved;
		//this->m_CollisionManager.m_ContactDatabase.PurgeAllContacts(&pBody->m_GraphNode);
		//m_Tree.RemoveObject(&pBody->m_Object);
		BodyPtr ptr(pBody);
		m_pGraphManager->RemoveGraphNode(&pBody->m_GraphNode, m_EventStream, true);
		m_RopeBodies.Remove(&ptr);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_ReleaseBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pCommand->m_CommandBundle.m_pScene = this;
		if(pBody->IsActive())
			m_ActiveBodyList.RemoveBody(pBody);
		AddCommand(pCommand);
	}

	void C_Scene::InsertFluidBodyInWorld(C_Body* pBody)
	{
		BIZ_ASSERT(!(pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld));
		BodyPtr wrapper(pBody);
		m_FluidBodies.Insert(&wrapper);
		pBody->FlagInsertedIntoWorld();
		m_pGraphManager->InsertGraphNode(&pBody->m_GraphNode);
		pBody->SynchroniseState();
		pBody->SynchroniseStateFlags();
	}

	void C_Scene::RemoveFluidBodyFromWorld(Gravity::C_Body *pBody)
	{
		BIZ_ASSERT((pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld) != 0);
		pBody->m_BodyFlags &= ~C_Body::E_InsertedIntoWorld;
		pBody->m_BodyFlags &= ~C_Body::E_TemporarilyRemoved;
		BodyPtr ptr(pBody);
		m_pGraphManager->RemoveGraphNode(&pBody->m_GraphNode, m_EventStream, true);
		m_FluidBodies.Remove(&ptr);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_ReleaseBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pCommand->m_CommandBundle.m_pScene = this;
		AddCommand(pCommand);
	}

	void C_Scene::InsertFluidMeshBodyInWorld(C_Body* pBody)
	{
		BIZ_ASSERT(!(pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld));
		BodyPtr wrapper(pBody);
		pBody->m_Object.m_ObjectState = E_FluidMeshObject;
		m_FluidMeshBodies.Insert(&wrapper);
		pBody->FlagInsertedIntoWorld();
		m_pTree->AddFluidMeshObject(&pBody->m_Object);
		m_pGraphManager->InsertGraphNode(&pBody->m_GraphNode);
		pBody->SynchroniseState();
		pBody->SynchroniseStateFlags();
	}

	void C_Scene::RemoveFluidMeshBodyFromWorld(Gravity::C_Body *pBody)
	{
		BIZ_ASSERT((pBody->m_BodyFlags & C_Body::E_InsertedIntoWorld) != 0);
		pBody->m_BodyFlags &= ~C_Body::E_InsertedIntoWorld;
		pBody->m_BodyFlags &= ~C_Body::E_TemporarilyRemoved;
		BodyPtr ptr(pBody);
		m_pGraphManager->RemoveGraphNode(&pBody->m_GraphNode, m_EventStream, true);
		m_FluidMeshBodies.Remove(&ptr);
		m_pTree->RemoveFluidMeshObject(&pBody->m_Object);
		C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_ReleaseBody;
		pCommand->m_CommandBundle.m_pBody = pBody;
		pCommand->m_CommandBundle.m_pScene = this;
		AddCommand(pCommand);
	}


	void C_Scene::AddCharacterController(C_CharacterController* pController)
	{
		/*C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddCharacterController;
		pCommand->m_pCharacterController = pController;
		AddCommand(pCommand);*/
		ControllerPtr ptr(pController);
		m_CharacterControllers.Insert(&ptr);
	}

	void C_Scene::RemoveCharacterController(C_CharacterController* pController)
	{
		/*C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveCharacterController;
		pCommand->m_pCharacterController = pController;
		AddCommand(pCommand);*/
		ControllerPtr ptr(pController);
		m_CharacterControllers.Remove(&ptr);
	}

	void C_Scene::AddKinematicController(C_KinematicController* pController)
	{
		/*C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddKinematicController;
		pCommand->m_pKinematicController = pController;
		AddCommand(pCommand);*/
		pController->SetScene(this);
		pController->Activate();
		KinematicControllerPtr ptr(pController);
		m_KinematicControllers.Insert(&ptr);
	}

	void C_Scene::RemoveKinematicController(C_KinematicController* pController)
	{
		/*C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveKinematicController;
		pCommand->m_pKinematicController = pController;
		AddCommand(pCommand);*/
		pController->Deactivate();
		KinematicControllerPtr ptr(pController);
		m_KinematicControllers.Remove(&ptr);
		pController->SetScene(NULL);

	}

	void C_Scene::AddAnimatedShapeController(C_AnimatedShapeController* pController)
	{
		pController->SetScene(this);
		AnimatedShapeControllerPtr ptr(pController);
		m_AnimatedShapeControllers.Insert(&ptr);
	}

	void C_Scene::RemoveAnimatedShapeController(C_AnimatedShapeController* pController)
	{
		pController->Deactivate();
		AnimatedShapeControllerPtr ptr(pController);
		m_AnimatedShapeControllers.Remove(&ptr);
		pController->SetScene(NULL);
	}

	void C_Scene::AddRagdollController(C_RagdollController* pController)
	{
		/*C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_AddRagdollController;
		pCommand->m_pRagdollController = pController;
		AddCommand(pCommand);*/
		pController->SetScene(this);
		pController->Start();
		RagdollControllerPtr ptr(pController);
		m_RagdollControllers.Insert(&ptr);
		
	}

	void C_Scene::RemoveRagdollController(C_RagdollController* pController)
	{
	/*	C_DeferredBodyCommand* pCommand(m_CommandIter.Create());
		pCommand->m_CommandType = C_DeferredBodyCommand::E_RemoveRagdollController;
		pCommand->m_pRagdollController = pController;
		AddCommand(pCommand);*/
		pController->Deactivate();
		pController->SetScene(NULL);
		RagdollControllerPtr ptr(pController);
		m_RagdollControllers.Remove(&ptr);
	}

	void C_Scene::SetMinTimeStep(const Maths::HF32 PCREF minTimeStep)
	{
		m_MinTimeStep = minTimeStep;
	}

	void C_Scene::ResetSoftBodies()
	{
		//const Maths::HVec3 up(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY());
		//const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		//for(S32 a = 0; a < m_ClothBodies.m_numElements; ++a)
		//{
		//	C_Body* pBody(m_ClothBodies[a].m_pElement);
		//	/*if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping))
		//	{
		//		Collision::C_ClothShape* pCloth = (Collision::C_ClothShape*)pBody->GetShape();
		//		pBody->m_GraphNode.m_TotalKineticEnergy = pBody->m_RigidBody.m_KineticEnergy;
		//		pBody->m_RigidBody.m_KineticEnergy = zero;
		//	}
		//	else
		//	{
		//		Collision::C_ClothShape* pCloth = (Collision::C_ClothShape*)pBody->GetShape();
		//		pBody->m_GraphNode.m_TotalKineticEnergy = up * pCloth->m_Cloth.m_MaxSpeedSq;
		//		pBody->m_RigidBody.m_KineticEnergy = zero;
		//	}*/
		//	Collision::C_ClothShape* pCloth = (Collision::C_ClothShape*)pBody->GetShape();
		//	//Maths::Scalar_Bool con(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping));
		//	//pBody->m_GraphNode.m_TotalKineticEnergy = Maths::Select(con, pBody->m_RigidBody.m_KineticEnergy, up * pCloth->m_Cloth.m_MaxSpeedSq);
		//	//pBody->m_RigidBody.m_KineticEnergy = zero;
		//}

		/*for(S32 a = 0; a < m_DeformableBodies.m_numElements; ++a)
		{
			C_Body* pBody(m_DeformableBodies[a].m_pElement);
			Collision::C_DeformableShape* pDeformable = (Collision::C_DeformableShape*)pBody->GetShape();
			Maths::Scalar_Bool con(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping));
			pBody->m_GraphNode.m_TotalKineticEnergy = Maths::Select(con, pBody->m_RigidBody.m_KineticEnergy, up * pDeformable->m_Deformable.m_MaxSpeedSq);
			pBody->m_RigidBody.m_KineticEnergy = zero;
		}*/
		//for(S32 a = 0; a < m_RopeBodies.m_numElements; ++a)
		//{
		//	C_Body* pBody(m_RopeBodies[a].m_pElement);
		//	/*if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping))
		//	{
		//		pBody->m_GraphNode.m_TotalKineticEnergy = pBody->m_RigidBody.m_KineticEnergy;
		//		pBody->m_RigidBody.m_KineticEnergy = zero;
		//	}
		//	else
		//	{
		//		Collision::C_RopeShape* pRope = (Collision::C_RopeShape*)pBody->GetShape();
		//		pBody->m_GraphNode.m_TotalKineticEnergy = up * pRope->m_Rope.m_MaxSpeedSq;
		//		pBody->m_RigidBody.m_KineticEnergy = zero;
		//	}*/


		//	//Collision::C_RopeShape* pRope = (Collision::C_RopeShape*)pBody->GetShape();
		//	//Maths::Scalar_Bool con(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping));
		//	/*pBody->m_GraphNode.m_TotalKineticEnergy = Maths::Select(con, pBody->m_RigidBody.m_KineticEnergy, up *pRope->m_Rope.m_MaxSpeedSq);
		//	pBody->m_RigidBody.m_KineticEnergy = zero;*/
		//}
	}

	void C_Scene::SimulateFluids(const Maths::HF32 PCREF dt)
	{
		using namespace Collision;
		PERF_EVENT("physics.SimulateFluids");
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		const Maths::HF32 zeroF=Maths::Constants<Maths::HF32>::Zero();

		for(S32 a =0; a < m_FluidBodies.m_numElements; ++a)
		{
			C_Body* pBody=m_FluidBodies[a].m_pElement;
			C_FluidShape* pShape=(C_FluidShape*)pBody->GetShape();
			C_Fluid& fluid=pShape->m_Fluid;
			const Maths::HF32 half(zeroF);
			Maths::HF32 averageHeight(zeroF);
			Maths::HVec3 averageCenterPoint(zero);
			

			for(S32 b=0; b<m_Bodies.m_numElements; ++b)
			{
				C_Body* body=m_Bodies[b].m_pElement;
				Collision::C_AxisBoxMinMax& box=fluid.m_Box;
				averageHeight = zeroF;
				averageCenterPoint = zero;
				C_CollisionObject colObj(body->GetObject()->CollisionObject());
				C_CollisionObject fluidObj(pBody->GetObject()->CollisionObject());
				if(fluid.m_CollisionManager->Collided(&fluid, &colObj, averageHeight))
				{
					fluid.ApplyForce(&colObj, averageHeight);
				}

			}
		}

		for(S32 a=0; a < m_FluidBodies.m_numElements; ++a)
		{
			C_Body* pBody(m_FluidBodies[a].m_pElement);
			//if(pBody->IsActive())
			{
				C_FluidShape* pShape=(C_FluidShape*)pBody->GetShape();
			
				pShape->m_Fluid.Verlet(dt);
			}

		}
	}

	void C_Scene::UpdateSprings()
	{
		PERF_EVENT("physics.Spring");

		for(S32 i=0; i< m_Springs.m_numElements; ++i)
		{
			Maths::HVec3 bodyBTorque(Maths::Constants<Maths::HVec3>::Zero());
			SharedPhysics::C_Spring* pSpring = m_Springs[i].m_pElement;
			Maths::HVec3 force= pSpring->GetSpringForce(bodyBTorque);
		
			if(pSpring->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
			{
				pSpring->m_pRigidB->AddForce(force);
				pSpring->m_pRigidB->AddTorque(bodyBTorque * pSpring->m_pRigidB->m_InertiaTensor);
			}
			if(pSpring->m_pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
			{
				pSpring->m_pRigidA->AddForce(-force);
				pSpring->m_pRigidA->AddTorque(-bodyBTorque * pSpring->m_pRigidA->m_InertiaTensor);
			}
		}
	}

	void C_Scene::Solve(const Maths::HF32 PCREF dt)
	{
		PERF_EVENT("physics.SolveContacts");

		const F32 fTime = dt;
		U32 jointIterationFrequency = (g_dbOverrideSolverParams) ? g_dbJointIterationFrequency : m_JointIterationFrequency;
		U32 numIterations = (g_dbOverrideSolverParams) ? g_dbNumIterations : m_NumIterations;

		C_SolverIsland island;
		island.m_NumIterations = m_NumIterations;
		island.m_JointIterationFrequency = m_JointIterationFrequency;
		if(g_dbOverrideSolverParams)
		{
			island.m_NumIterations = g_dbNumIterations;
			island.m_JointIterationFrequency = g_dbJointIterationFrequency;
		}
		island.m_TimeStep = fTime;
		
		island.m_pMemoryAllocator = &m_SPUAllocator;
		island.m_pWorldBody = m_pWorldBody;

		C_Body* pBody = m_ActiveBodyList.m_pFirstBody;
		while(pBody)
		{
			if(pBody->MustSolve())
			{
				island.m_pRootNode = &pBody->m_GraphNode;
				Scheduler::AddTask(SolverModule, NULL, NULL, m_Group, m_Policy, &island, sizeof(C_SolverIsland));

				/*Solver2::LCP lcp(0.f, 0.f, island.m_NumIterations, island.m_JointIterationFrequency, island.m_pMemoryAllocator, m_pWorldBody->GetGraphNode(), &m_pWorldBody->GetBody());
				lcp.SetTimeStep(island.m_TimeStep);
				lcp.LCPSolver(pBody->GetGraphNode());*/
			}
			pBody = pBody->m_pNext;
		}

		pBody = m_ContinuousBodyList.m_pFirstBody;
		while(pBody)
		{
			if(pBody->MustSolve())
			{
				island.m_pRootNode = &pBody->m_GraphNode;
				Scheduler::AddTask(SolverModule, NULL, NULL, m_Group, m_Policy, &island, sizeof(C_SolverIsland));
			}
			pBody = pBody->m_pNext;
		}

		UpdateControllers1(dt);

		Scheduler::Flush(m_Group);
	}

	void C_Scene::Integrate(const Maths::C_TimeS PCREF sTime)
	{
		{
			PERF_EVENT("Gravity.Integration");
			C_IntegrateTask task;
			const Maths::HF32 time(sTime.Get());
			Maths::Store(time, &task.m_Time);
			task.m_NumBodies = 0;
			
			SharedPhysics::C_MemoryIterator<C_TaskBuffer<TaskSize>, TaskBufferSize, false > iter(m_pTaskBuffer);
			{
				C_TaskBuffer<TaskSize>* pTask = iter.Create();
				task.m_pBodies = (C_Body**)pTask->m_pElements;
			}

			C_Body* pBody = m_ActiveBodyList.m_pFirstBody;

			Btl::OpenHashSet<Scheduler::GroupHandle> groupSet;

			while(pBody)
			{
				C_Body* pNextBody = pBody->m_pNext;
				//C_Body* pBody(m_Bodies[a].m_pElement);
				if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoIntegrate))
				{
					BIZ_ASSERT(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));
					if(pBody->m_pIntegrationCallback != NULL)
					{
						pBody->ApplyForcesAndTorques(time);
						Scheduler::GroupHandle groupToFlush = pBody->m_pIntegrationCallback->BeginIntegrationTask(m_Group, m_Policy, time);
						groupSet.Insert(groupToFlush);
					}
					else
					{
						task.m_pBodies[task.m_NumBodies++] = pBody;
						if(task.m_NumBodies == MaxIntegrationSize)
						{
							Scheduler::AddTask(IntegrateModule, NULL, NULL, m_Group, m_Policy, &task, sizeof(C_IntegrateTask));
							task.m_NumBodies = 0;
							C_TaskBuffer<TaskSize>* pTask = iter.Create();
							task.m_pBodies = (C_Body**)pTask->m_pElements;
						}
						//pBody->Integrate(m_Integrator, time);
					}
				}
				pBody = pNextBody;
			}

			pBody = m_ContinuousBodyList.m_pFirstBody;
			while(pBody)
			{
				C_Body* pNextBody = pBody->m_pNext;
				//Check whether the the kinetic energy is above or below the threshold
				if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoIntegrate))
				{
					//pBody->Integrate(m_Integrator, time);
					task.m_pBodies[task.m_NumBodies++] = pBody;
					if(task.m_NumBodies == MaxIntegrationSize)
					{
						Scheduler::AddTask(IntegrateModule, NULL, NULL, m_Group, m_Policy, &task, sizeof(C_IntegrateTask));
						task.m_NumBodies = 0;
						C_TaskBuffer<TaskSize>* pTask = iter.Create();
						task.m_pBodies = (C_Body**)pTask->m_pElements;
					}
				}				
				pBody = pNextBody;
			}

			if(task.m_NumBodies > 0)
			{
				Scheduler::AddTask(IntegrateModule, NULL, NULL, m_Group, m_Policy, &task, sizeof(C_IntegrateTask));
				task.m_NumBodies = 0;
			}

			SharedPhysics::C_S32SafeStack stack((S32*)m_Stack.m_pData, m_Stack.m_Size);

			//Do trigger stuff...
			m_TriggerSystem.GenerateEvents(stack);

			{
				//Flip buffers here so we can record continuous contacts for next frame!!!
				PERF_EVENT("physics.NextFrame1");
				m_pCollisionManager->FlipBuffers();
				m_FluidEventBuffer.Reset();
			}

			Scheduler::Flush(m_Group);

			for (Btl::OpenHashSet<Scheduler::GroupHandle>::Iterator i = groupSet.Begin(); i != groupSet.End(); ++i)
				Scheduler::Flush(*i);

			//Post integration for the car, so don't need to do continuous body List
			pBody = m_ActiveBodyList.m_pFirstBody;
			while(pBody)
			{
				C_Body* pNextBody = pBody->m_pNext;
				if((!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoIntegrate)) & (pBody->m_pIntegrationCallback != NULL))
					pBody->m_pIntegrationCallback->PostIntegration(time);
				else if(pBody->IsContinuousShape() && pBody->IsContinuous())
				{
					pBody->m_GraphNode.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Continuous);
					m_ActiveBodyList.RemoveBody(pBody);
					m_ContinuousBodyList.AddBody(pBody);
				}
				pBody = pNextBody;
			}

			pBody = m_ContinuousBodyList.m_pFirstBody;
			while(pBody)
			{
				C_Body* pNextBody = pBody->m_pNext;
				if(!pBody->IsContinuous())
				{
					pBody->m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Continuous);
					//Add to continuous list...
					m_ContinuousBodyList.RemoveBody(pBody);
					m_ActiveBodyList.AddBody(pBody);
				}
				pBody = pNextBody;
			}
		}
		//Queue up fluid enter and exit events - can probably move this back in parallel with
		//integration because it isn't accessing same data as is accessed on SPUs
		CreateFluidEnterExitEvents();
	}

	void C_Scene::UpdateSoftBodies(const Maths::HF32 PCREF dt, bool bFirstRun)
	{
		using namespace Collision;
		PERF_EVENT("Gravity.UpdateSoftBodies");

		//Process all the cloth collisions...for now, just add them into the cloth collision array...

		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());

		for(S32 a = 0; a < m_ClothBodies.m_numElements; ++a)
		{
			C_Body* pBody(m_ClothBodies[a].m_pElement);
			if(pBody->IsActive())
			{
				if(pBody->GetShape()->m_ShapeType == Collision::E_Cloth_Shape)
				{
					C_ClothShape* pShape=(C_ClothShape*)pBody->GetShape();
					pShape->m_Cloth.TimeStep(dt, m_Stack);
					pShape->m_Cloth.GetBoundingBox(pShape->m_LocalBoundingBox);
					pBody->m_Object.m_TransformedBox = pShape->m_LocalBoundingBox;
					const Maths::HVec3 kineticEnergy(pShape->m_Cloth.m_MaxSpeedSq);
					pBody->m_RigidBody.m_KineticEnergy = kineticEnergy * pShape->m_Cloth.m_ReciprocalTotalMass;
					pBody->m_RigidBody.m_LinearVelocity = pBody->m_RigidBody.m_LinearMomentum
						= pBody->m_RigidBody.m_AngularVelocity = pBody->m_RigidBody.m_AngularMomentum = zero;
					pBody->m_GraphNode.m_TotalKineticEnergy = kineticEnergy;
				}
				else
				{
					C_NonCollidableClothShape* pShape=(C_NonCollidableClothShape*)pBody->GetShape();
					pShape->m_Cloth.TimeStep(dt);
					const Maths::HVec3 kineticEnergy(pShape->m_Cloth.m_MaxSpeedSq);
					pBody->m_RigidBody.m_KineticEnergy = kineticEnergy * pShape->m_Cloth.m_ReciprocalTotalMass;
					pBody->m_RigidBody.m_LinearVelocity = pBody->m_RigidBody.m_LinearMomentum
						= pBody->m_RigidBody.m_AngularVelocity = pBody->m_RigidBody.m_AngularMomentum = zero;
					pBody->m_GraphNode.m_TotalKineticEnergy = kineticEnergy;
				}
				
			}
		}

		for(S32 a = 0; a < m_DeformableBodies.m_numElements; ++a)
		{
			C_Body* pBody(m_DeformableBodies[a].m_pElement);
			if(pBody->IsActive())
			{
				C_DeformableShape* pShape=(C_DeformableShape*)pBody->GetShape();
				pShape->m_Deformable.TimeStep(dt, m_Stack);
				pShape->m_Deformable.GetBoundingBox(pShape->m_LocalBoundingBox);
				pBody->m_Object.m_TransformedBox = pShape->m_LocalBoundingBox;
				pBody->m_RigidBody.m_KineticEnergy = Maths::HVec3(pShape->m_Deformable.m_MaxSpeedSq);
			}
		}

		for(S32 a = 0; a < m_RopeBodies.m_numElements; ++a)
		{
			C_Body* pBody(m_RopeBodies[a].m_pElement);
			if(pBody->IsActive())
			{
				if(pBody->GetShape()->m_ShapeType == Collision::E_Rope_Shape)
				{
					C_RopeShape* pShape=(C_RopeShape*)pBody->GetShape();
					pShape->m_Rope.TimeStep(dt, m_Stack);
					pShape->m_Rope.GetBoundingBox(pShape->m_LocalBoundingBox);
					pBody->m_Object.m_TransformedBox = pShape->m_LocalBoundingBox;
					pBody->m_RigidBody.m_KineticEnergy = Maths::HVec3(pShape->m_Rope.m_MaxSpeedSq);
					pBody->m_RigidBody.m_LinearVelocity = pBody->m_RigidBody.m_LinearMomentum
						= pBody->m_RigidBody.m_AngularVelocity = pBody->m_RigidBody.m_AngularMomentum = zero;
				}
				else
				{
					C_NonCollidableRopeShape* pShape = (C_NonCollidableRopeShape*)pBody->GetShape();
					pShape->m_Rope.TimeStep(dt);
					pBody->m_RigidBody.m_KineticEnergy = Maths::HVec3(pShape->m_Rope.m_MaxSpeedSq);
					pBody->m_RigidBody.m_LinearVelocity = pBody->m_RigidBody.m_LinearMomentum
						= pBody->m_RigidBody.m_AngularVelocity = pBody->m_RigidBody.m_AngularMomentum = zero;
				}
			}
		}

		m_pCollisionManager->m_ClothStream.Reset();

	}

	void C_Scene::ClearConstraintList(Gravity::C_BodyList& list)
	{
		Gravity::C_Body* pBody = list.m_pFirstBody;
		while(pBody)
		{
			pBody->m_GraphNode.m_pVolatileConstraints = NULL;
			pBody->m_GraphNode.m_pVolatileTwinConstraints = NULL;
			pBody->m_GraphNode.m_pCloseConstraints = NULL;
			pBody->m_GraphNode.m_pCloseTwinConstraints = NULL;
			pBody->m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
			pBody = pBody->m_pNext;
		}
	}

	void C_Scene::ClearConstraintListReset(Gravity::C_BodyList& list)
	{
		Gravity::C_Body* pBody = list.m_pFirstBody;
		while(pBody)
		{
			Gravity::C_Body* pNextBody = pBody->m_pNext;
			pBody->m_GraphNode.m_pVolatileConstraints = NULL;
			pBody->m_GraphNode.m_pVolatileTwinConstraints = NULL;
			pBody->m_GraphNode.m_pCloseConstraints = NULL;
			pBody->m_GraphNode.m_pCloseTwinConstraints = NULL;
			pBody->m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
			pBody->m_pNext = pBody->m_pPrev = NULL;
			pBody = pNextBody;
		}
		list.m_pFirstBody = NULL;
	}



	void C_Scene::Simulate(const Maths::C_TimeS PCREF time)
	{
		MEMORY_TRACKER_PUSH("Gravity.Scene.Simulate");
		PERF_EVENT("physics.OuterSimulate");
		//m_PhysicsLock.Wait();
		m_EventStream.Reset();
		bool bFirstRun = true;
		Maths::HF32 remainingTime = time.Get() + m_RemainingTime;
		if(m_MinTimeStep >= 0.f)
		{
			while(remainingTime > m_MinTimeStep)
			{
				_Simulate(m_MinTimeStep, bFirstRun);
				remainingTime -= m_MinTimeStep;
			}
		}
		else
		{
			_Simulate(remainingTime, bFirstRun);
			remainingTime = Maths::Constants<Maths::HF32>::Zero();
		}
		//m_NextSimulate = remainingTime;
		//_Simulate(remainingTime, bFirstRun);
		m_RemainingTime = remainingTime;
		for(S32 a = 0; a < this->m_CharacterControllers.m_numElements; ++a)
		{
			m_CharacterControllers[a].m_pElement->ResetState();
		}
		//m_PhysicsLock.Release();

		if(m_pEventCallback)
		{
			PERF_EVENT("physics.EventCallback");

			SharedPhysics::C_MemoryIterator<C_BodyEvent> iter = GetEventIterator();
			m_pEventCallback->ProcessEvents(iter, GetNumEvents());
		}

		{
			//Kinematic body don't have external forces, so don't need to clean up
			C_Body* pBody = m_ActiveBodyList.m_pFirstBody;
			while(pBody)
			{
				BIZ_ASSERT(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));
				pBody->ClearExternalForces();
				pBody = pBody->m_pNext;
			}

			pBody = m_ContinuousBodyList.m_pFirstBody;
			while(pBody)
			{
				BIZ_ASSERT(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));
				pBody->ClearExternalForces();
				pBody = pBody->m_pNext;
			}
		}


		MEMORY_TRACKER_POP("Gravity.Scene.Simulate");
	}


	void C_Scene::UpdateShapes()
	{
		U32 numActiveBodies = 0;
		{
			C_PostIntegrateTask task;
			task.m_NumBodies = 0;
			SharedPhysics::C_MemoryIterator<C_TaskBuffer<TaskSize>, TaskBufferSize, false > iter(m_pTaskBuffer);
			{
				C_TaskBuffer<TaskSize>* pTask = iter.Create();
				task.m_pBodies = (C_Body**)pTask->m_pElements;
			}
			PERF_EVENT("physics.Synchronise1");
			C_Body* pBody = m_ActiveBodyList.m_pFirstBody;
			while(pBody)
			{
				if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_AnimatedShape))
				{
					BIZ_ASSERT(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));
					task.m_pBodies[task.m_NumBodies++] = pBody;
					if(task.m_NumBodies == MaxPostIntegrateSize)
					{
						Scheduler::AddTask(PostIntegrateModule, NULL, NULL, m_Group, m_Policy, &task, sizeof(C_PostIntegrateTask));
						task.m_NumBodies = 0;
						C_TaskBuffer<TaskSize>* pTask = iter.Create();
						task.m_pBodies = (C_Body**)pTask->m_pElements;
					}

					++numActiveBodies;
				}
				
				pBody = pBody->m_pNext;
			}

			pBody = m_ContinuousBodyList.m_pFirstBody;
			while(pBody)
			{
				if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_AnimatedShape))
				{
					task.m_pBodies[task.m_NumBodies++] = pBody;
					//pBody->ClearEdgeFlags();
					//pBody->SynchronisePosRot();	
					if(task.m_NumBodies == MaxPostIntegrateSize)
					{
						Scheduler::AddTask(PostIntegrateModule, NULL, NULL, m_Group, m_Policy, &task, sizeof(C_PostIntegrateTask));
						task.m_NumBodies = 0;
						C_TaskBuffer<TaskSize>* pTask = iter.Create();
						task.m_pBodies = (C_Body**)pTask->m_pElements;
					}
					++numActiveBodies;
				}
				
				pBody = pBody->m_pNext;
			}


			pBody = m_KinematicBodyList.m_pFirstBody;
			while(pBody)
			{
				if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_AnimatedShape))
				{
					task.m_pBodies[task.m_NumBodies++] = pBody;
					if(task.m_NumBodies == MaxPostIntegrateSize)
					{
						Scheduler::AddTask(PostIntegrateModule, NULL, NULL, m_Group, m_Policy, &task, sizeof(C_PostIntegrateTask));
						task.m_NumBodies = 0;
						C_TaskBuffer<TaskSize>* pTask = iter.Create();
						task.m_pBodies = (C_Body**)pTask->m_pElements;
					}
					++numActiveBodies;
				}
				
				pBody = pBody->m_pNext;
			}

			Gravity::C_AnimatedShapeController* pController = this->m_pActiveAnimatedShapeControllers;;
			while(pController)
			{
				task.m_pBodies[task.m_NumBodies++] = pController->m_pBody;
				if(task.m_NumBodies == MaxPostIntegrateSize)
				{
					Scheduler::AddTask(PostIntegrateModule, NULL, NULL, m_Group, m_Policy, &task, sizeof(C_PostIntegrateTask));
					task.m_NumBodies = 0;
					C_TaskBuffer<TaskSize>* pTask = iter.Create();
					task.m_pBodies = (C_Body**)pTask->m_pElements;
				}
				++numActiveBodies;
				
				pController = pController->m_pNext;
			}



			if(task.m_NumBodies > 0)
			{
				Scheduler::AddTask(PostIntegrateModule, NULL, NULL, m_Group, m_Policy, &task, sizeof(C_PostIntegrateTask));
			}
		}
		Scheduler::Flush(m_Group);
		m_NumActiveBodies = numActiveBodies;
	}

	void C_Scene::UpdateControllers1(const Maths::HF32 PCREF dt)
	{
		PERF_EVENT("physics.UpdateControllers");
		
		SharedPhysics::C_MemoryIterator<C_TaskBuffer<TaskSize>, TaskBufferSize, false > iter(m_pTaskBuffer);
		{
			C_KinematicUpdateTask updateTask;
			updateTask.m_NumControllers = 0;
			Maths::Store(dt, &updateTask.m_Time);

			{
				C_TaskBuffer<TaskSize>* pTask = iter.Create();
				updateTask.m_pControllers = (C_KinematicController**)pTask->m_pElements;
			}

			C_KinematicController* pController = this->m_pActiveControllers;
			while(pController)
			{
				//pController->Update(dt);
				if(!pController->m_pReferenceBody)
				{
					updateTask.m_pControllers[updateTask.m_NumControllers++] = pController;
					if(updateTask.m_NumControllers == MaxKinematicUpdateSize)
					{
						Scheduler::AddTask(KinematicControllerModule, NULL, NULL, m_Group, m_Policy, &updateTask, sizeof(C_KinematicUpdateTask));
						updateTask.m_NumControllers = 0;
						C_TaskBuffer<TaskSize>* pTask = iter.Create();
						updateTask.m_pControllers = (C_KinematicController**)pTask->m_pElements;
					}
				}
				pController = pController->m_pNext;
			}
			if(updateTask.m_NumControllers)
			{
				Scheduler::AddTask(KinematicControllerModule, NULL, NULL, m_Group, m_Policy, &updateTask, sizeof(C_KinematicUpdateTask));
				updateTask.m_NumControllers = 0;
			}
		}

		{

			C_AnimatedShapeUpdateTask updateAnimatedTask;
			updateAnimatedTask.m_NumControllers = 0;
		//	Maths::Store(dt, &updateTask.m_Time);

			{
				C_TaskBuffer<TaskSize>* pTask = iter.Create();
				updateAnimatedTask.m_pControllers = (C_AnimatedShapeController**)pTask->m_pElements;
			}

			C_AnimatedShapeController* pController = m_pActiveAnimatedShapeControllers;
			while(pController)
			{
				//pController->Update(dt);
				updateAnimatedTask.m_pControllers[updateAnimatedTask.m_NumControllers++] = pController;
				if(updateAnimatedTask.m_NumControllers == MaxAnimatedShapeUpdateSize)
				{
					Scheduler::AddTask(AnimatedShapeControllerModule, NULL, NULL, m_Group, m_Policy, &updateAnimatedTask, sizeof(C_AnimatedShapeUpdateTask));
					updateAnimatedTask.m_NumControllers = 0;
					C_TaskBuffer<TaskSize>* pTask = iter.Create();
					updateAnimatedTask.m_pControllers = (C_AnimatedShapeController**)pTask->m_pElements;
				}
				pController = pController->m_pNext;
			}
			if(updateAnimatedTask.m_NumControllers)
			{
				Scheduler::AddTask(AnimatedShapeControllerModule, NULL, NULL, m_Group, m_Policy, &updateAnimatedTask, sizeof(C_AnimatedShapeUpdateTask));
				updateAnimatedTask.m_NumControllers = 0;
			}

		}
	}



	void C_Scene::UpdateControllers2(const Maths::HF32 PCREF dt)
	{
		PERF_EVENT("physics.UpdateControllers");
		
		SharedPhysics::C_MemoryIterator<C_TaskBuffer<TaskSize>, TaskBufferSize, false > iter(m_pTaskBuffer);
		{
			C_KinematicUpdateTask updateTask;
			updateTask.m_NumControllers = 0;
			Maths::Store(dt, &updateTask.m_Time);

			{
				C_TaskBuffer<TaskSize>* pTask = iter.Create();
				updateTask.m_pControllers = (C_KinematicController**)pTask->m_pElements;
			}

			C_KinematicController* pController = this->m_pActiveControllers;
			while(pController)
			{
				//pController->Update(dt);
				if(pController->m_pReferenceBody)
				{
					updateTask.m_pControllers[updateTask.m_NumControllers++] = pController;
					if(updateTask.m_NumControllers == MaxKinematicUpdateSize)
					{
						Scheduler::AddTask(KinematicControllerModule, NULL, NULL, m_Group, m_Policy, &updateTask, sizeof(C_KinematicUpdateTask));
						updateTask.m_NumControllers = 0;
						C_TaskBuffer<TaskSize>* pTask = iter.Create();
						updateTask.m_pControllers = (C_KinematicController**)pTask->m_pElements;
					}
				}
				pController = pController->m_pNext;
			}
			if(updateTask.m_NumControllers)
			{
				Scheduler::AddTask(KinematicControllerModule, NULL, NULL, m_Group, m_Policy, &updateTask, sizeof(C_KinematicUpdateTask));
				updateTask.m_NumControllers = 0;
			}
		}

		{
			PERF_EVENT("CharacterControllers.Update");
			for(S32 a = 0; a < m_CharacterControllers.m_numElements; ++a)
			{
				m_CharacterControllers[a].m_pElement->Update(dt);
			}
		}
		
		UpdateRagdollControllers(dt);
	}


	bool C_Scene::ValidActiveList()
	{
		C_Body* pBody = m_ActiveBodyList.m_pFirstBody;
		while(pBody)
		{
			BIZ_ASSERT(pBody->IsActive());
			if(pBody->m_pNext)
				BIZ_ASSERT(pBody->m_pNext->m_pPrev == pBody);
			pBody = pBody->m_pNext;
		}
		return true;
	}



	void C_Scene::_Simulate(const Maths::C_TimeS PCREF time, bool& bFirstRun)
	{
		PERF_EVENT("physics.simulate");
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		const Maths::HF32 dt(time.Get());

		//ResetSoftBodies();
		
		WaitForDeferredContacts( dt );
		SharedPhysics::C_ConstContactStreamPtr clothStreamEnd(m_pCollisionManager->m_ClothStream);

		SimulateFluids(dt);
		
		SharedPhysics::C_ConstContactStreamPtr startPtr(m_pCollisionManager->m_pCurrentStream, m_pCollisionManager->m_pCurrentStream->m_pStream);
		m_pCollisionManager->CreateColliders(startPtr, *m_pGraphManager);

		//OK - we've got the contacts, now pass the contacts back to the game
		if(m_pCollisionCallback)
		{
			PERF_EVENT("Gravity.CollisionCallback");
			SharedPhysics::C_ContactStreamIterator iter(m_pCollisionManager->m_pCurrentStream, m_pGraphManager, dt);
			m_pCollisionCallback->ProcessCollisions( iter, SharedPhysics::E_CollisionCallbackPhase_Colliders );
			m_pCollisionCallback->ProcessFluidEvents(m_FluidEventBuffer.m_pFluidEvents, Maths::Min(m_FluidEventBuffer.m_NumFluidEvents, m_FluidEventBuffer.m_MaxFluidEvents));
		}

		SharedPhysics::C_ConstContactStreamPtr endPtr(*m_pCollisionManager->m_pCurrentStream);

		m_pCollisionManager->m_pContactDatabase->SumKineticEnergies(m_pCollisionManager->m_ClothStream);
		{
			PERF_EVENT("Gravity.KineticEnergyStuff");
			Gravity::C_Body* pBody = this->m_WakingBodyList.m_pFirstBody;
			while(pBody)
			{
				if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static) && 
					!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Cloth | SharedPhysics::C_GraphNode::E_Type_Rope))
				{
					pBody->m_GraphNode.m_TotalKineticEnergy = pBody->m_RigidBody.m_KineticEnergy;
					pBody->m_RigidBody.m_KineticEnergy = zero;
				}
				pBody = pBody->m_pNext;
			}
		}

		m_pGraphManager->ClearLeftoverContacts(*m_pCollisionManager->m_pContactDatabase, m_WakingBodyList);
		WakeUpBodies(m_Stack, clothStreamEnd, dt);
		
		m_pCollisionManager->CreateColliders(endPtr, *m_pGraphManager);
		if(m_pCollisionCallback)
		{
			PERF_EVENT("Gravity.CollisionCallback");
			SharedPhysics::C_ContactStreamIterator iter(&endPtr, m_pGraphManager, dt);
			m_pCollisionCallback->ProcessCollisions( iter, SharedPhysics::E_CollisionCallbackPhase_Woken );
		}

		//Calculate spring forces!
		UpdateSprings();
		
		//Clear constraint lists and reset touched body list
		ClearConstraintList(m_ActiveBodyList);
		ClearConstraintList(m_ContinuousBodyList);
		ClearConstraintList(m_KinematicBodyList);
		ClearConstraintListReset(m_TouchedBodyList);

		//Build the constraint graph
		SharedPhysics::C_MemoryIterator<SharedPhysics::C_Collider, 64> meshIterator(m_pCollisionManager->m_pContactDatabase->m_pCurrentBuffer->GetMeshIterator());
		m_pGraphManager->BuildConstraintGraph(m_pCollisionManager->m_pContactDatabase->m_pCurrentBuffer->GetCollider(0), m_pCollisionManager->m_pContactDatabase->m_pCurrentBuffer->GetNumberCollidingPairs(),
			meshIterator, m_pCollisionManager->m_pContactDatabase->m_pCurrentBuffer->GetNumberMeshPairs());

		//Temporarily turned off to try something out!
		//m_pGraphManager->BuildClothConstraintGraph(&m_pCollisionManager->m_ClothStream);
		
		//Identify islands and merge small islands to reduce the overhead of the jobqueue system
		S32 islands = m_pGraphManager->SplitConstraintGroups(m_Stack,m_ActiveBodyList, m_ContinuousBodyList, m_KinematicBodyList, dt);
		m_pGraphManager->MergeSmallConstraintGroups(m_ActiveBodyList, m_ContinuousBodyList, m_KinematicBodyList, (U32)g_dbIslandSizeThreshold);

		//Update all the soft bodies
		UpdateSoftBodies(dt, true);	

		
		//Solve the constraints
		Solve(dt);

		//Step bodies forwards by time
		Integrate(time);

		//Do the continuous collision detection for continuous objects
		DoContinuousCollisionDetection();

		//Update controllers
		UpdateControllers2(dt);

		//Wait for ragdolls to finish
		FinaliseRagdollControllers();

		m_ReadWriteLock.WriteLock();
		{
			PERF_EVENT("physics.rwlock");

			{
				PERF_EVENT("physics.PostIntegration");

				//Deferred commands from game
				
				if(bFirstRun)
					ProcessCommands();
				bFirstRun = false;

				UpdateShapes();

				if(!g_dbDisableDeactivation)
					PutBodiesToSleep();		
			}
			
			UpdateOverriddenBodies();

			//Kick off mesh collision here (moved here to run in parallel with update broad phase)
			{
				C_MeshDispatchTask task;
				task.m_pScene = this;
				Scheduler::AddTask(MeshCollisionDispatchModule, 0, 0, m_Group, 0, &task, sizeof(C_MeshDispatchTask));
			}
			//Update broad phase
			m_pTree->UpdateTree( m_Stack, m_ActiveBodyList, m_ContinuousBodyList, m_KinematicBodyList);

		}
		m_ReadWriteLock.ReleaseWriteLock();
		
		//Do broad phase collision and character raytraces!
		GetContacts( dt );		
	}

	void C_Scene::CreateFluidEnterExitEvents()
	{
		PERF_EVENT("Physics.CreateFluidEnterExitEvents");
		C_Body* pBody = m_ActiveBodyList.m_pFirstBody;
		while(pBody)
		{
			if(pBody->m_GraphNode.HasEnteredWater())
			{
				C_BodyEvent* pEvent = this->m_EventStream.CreateEvent();
				pEvent->SetBody(pBody);
				pEvent->SetEventType(Gravity::C_BodyEvent::E_EnterWater);
			}
			else if (pBody->m_GraphNode.HasExitedWater())
			{
				C_BodyEvent* pEvent = this->m_EventStream.CreateEvent();
				pEvent->SetBody(pBody);
				pEvent->SetEventType(Gravity::C_BodyEvent::E_ExitWater);
			}
			pBody->m_GraphNode.RefreshSubmergedFlags();
			pBody = pBody->m_pNext;
		}

		pBody = m_ContinuousBodyList.m_pFirstBody;
		while(pBody)
		{
			if(pBody->m_GraphNode.HasEnteredWater())
			{
				C_BodyEvent* pEvent = this->m_EventStream.CreateEvent();
				pEvent->SetBody(pBody);
				pEvent->SetEventType(Gravity::C_BodyEvent::E_EnterWater);
			}
			else if (pBody->m_GraphNode.HasExitedWater())
			{
				C_BodyEvent* pEvent = this->m_EventStream.CreateEvent();
				pEvent->SetBody(pBody);
				pEvent->SetEventType(Gravity::C_BodyEvent::E_ExitWater);
			}
			pBody->m_GraphNode.RefreshSubmergedFlags();
			pBody = pBody->m_pNext;
		}
	}
	

	void C_Scene::UpdateOverriddenBodies()
	{
		{
			PERF_EVENT("physics.Synchronise2");

			//Move across external states and apply gravity...
			for(S32 a = 0; a < m_OverriddenBodyList.m_numElements; ++a)
			{
				C_Body* pBody(m_OverriddenBodyList[a].m_pElement);
				if(pBody->TestBodyFlags( C_Body::E_Type_Removed))
					continue;
				if(pBody->TestBodyFlags(C_Body::E_Type_Overridden_PosRot) && 
					pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
				{
					pBody->SynchronisePosRot();
					pBody->m_Object.MarkDirty();
					m_pTree->AddDirtyObject(&pBody->m_Object);
					//Transform broad phase and narrow-phase representations accordingly!
				}
				if(pBody->TestBodyFlags(C_Body::E_Type_Overridden_Velocities))
				{
					pBody->SynchroniseState();
				}
				pBody->ClearBodyFlags(C_Body::E_Type_Overridden_PosRot | C_Body::E_Type_Overridden_Velocities);
				pBody->SynchroniseStateFlags();
			}
			m_OverriddenBodyList.Clear();
		}
	}
//#ifndef CFG_SPEED
//#pragma optimize("", on)
//#endif

	
#endif


#ifndef PLATFORM_SPU

	void C_Scene::DoDeferredRayTrace(Collision::C_Ray* pRay, Collision::C_LineCollisionCache* pCache, Scheduler::GroupHandle group,
		Scheduler::PolicyHandle policy, bool traceStaticMeshOnly)
	{
		C_DeferredRayCommand command;
		command.m_pCache = pCache;
		command.m_pRay = pRay;
		command.m_pScene = this;
		command.m_TraceStaticMeshOnly = traceStaticMeshOnly;
		Scheduler::AddTask(RayTraceModule, NULL, NULL, group, policy, &command, sizeof(C_DeferredRayCommand));
	}

	void C_Scene::DoDeferredTransparentRayTrace(Collision::C_TransparentRay* pRay, Scheduler::GroupHandle group, 
		Scheduler::PolicyHandle policy, bool traceStaticMeshOnly)
	{
		C_DeferredTransparentRayCommand command;
		command.m_pRay = pRay;
		command.m_pScene = this;
		command.m_TraceStaticMeshOnly = traceStaticMeshOnly;
		Scheduler::AddTask(TransparentRayTraceModule, NULL, NULL, group, policy, &command, sizeof(C_DeferredTransparentRayCommand));
	}

	void C_Scene::DoDeferredWaterQuery(C_WaterQuery* pQuery, Scheduler::GroupHandle group,Scheduler::PolicyHandle policy)
	{
		C_DeferredWaterCommand command;
		command.m_pQuery = pQuery;
		command.m_pScene = this;
		Scheduler::AddTask(WaterQueryModule, NULL, NULL, group, policy, &command, sizeof(C_DeferredWaterCommand));
	}

	void C_Scene::SetInternalTaskPolicy(Scheduler::PolicyHandle handle)
	{
		m_Policy = handle;
	}

	Scheduler::GroupHandle C_Scene::GetInternalGroupHandle()
	{
		return m_Group;
	}

	Scheduler::PolicyHandle C_Scene::GetInternalTaskPolicy()
	{
		return m_Policy;
	}

	 

	__forceinline U32 C_Scene::CompareObjectToList(SharedPhysics::C_MemoryIterator<C_ObjectPair, 128>& objectIter, Gravity::C_Object* pObject, 
		Gravity::C_ObjectWrapper* pList, Collision::C_AxisBoxMinMax& box, const U32& numContacts)
	{
		U32 count = numContacts;
		C_ObjectWrapper* pWrapper=pList;
		while(pWrapper)
		{
			C_Object* pOtherObj=pWrapper->m_pObject;
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(pObject->m_TransformedBox.centre, pOtherObj->m_TransformedBox.centre, Maths::RGBA(255, 0, 0));
			if(pObject->m_TransformedBox.IntersectsInAxisBox(pOtherObj->m_TransformedBox, box))/* &&
				pObject->m_pShape->CanCollide(pOtherObj->m_pShape))*/
			{
				if(pObject->m_pShape->CanCollide(pOtherObj->m_pShape))
				{
					C_ObjectPair* pair(objectIter.Create());
					++count;
					pair->m_pObjectA=pObject;
					pair->m_pObjectB=pOtherObj;
				}
			}
			pWrapper=pWrapper->m_pNext;
		}
		return count;
	}


	U32 C_Scene::ActiveNodeComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, U32& clothCount, SharedPhysics::C_MemoryIterator<Gravity::C_ObjectPair, 128>& clothIter)
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

			//Cloth comparisons
			clothCount = CompareObjectToList(clothIter, pActiveObj, pNode->m_pActiveClothObjects, box, clothCount);
			clothCount = CompareObjectToList(clothIter, pActiveObj, pNode->m_pSleepingClothObjects, box, clothCount);

			//Fluid comparisons
			GenerateFluidTasks(pNode->m_pFluidMeshObjects, *pActiveObj, fluidQuery, pNode, fluidCollisionCount);

			pActiveWrapper=pActiveWrapper->m_pNext;
		}

		
		if(pNode->m_pKinematicObjects || pNode->m_pActiveClothObjects)
		{
			//PERF_EVENT("Gravity.SleepingObjectComparisons");
			C_ObjectWrapper* pWrapper(pNode->m_pSleepingObjects);
			while(pWrapper)
			{
				C_Object* pSleepyObj(pWrapper->m_pObject);
				GenerateCollisionTasks(pNode->m_pKinematicObjects, *pSleepyObj, query, pNode, dynamicCollisionCount);
				//Cloth comparisons
				clothCount = CompareObjectToList(clothIter, pSleepyObj, pNode->m_pActiveClothObjects, box, clothCount);
				pWrapper = pWrapper->m_pNext;
			}
		}

		if(pNode->m_pActiveClothObjects || pNode->m_pSleepingClothObjects)
		{
			//PERF_EVENT("Gravity.KinematicObjectComparisons");
			C_ObjectWrapper* pWrapper(pNode->m_pKinematicObjects);
			while(pWrapper)
			{
				C_Object* pKinematicObj(pWrapper->m_pObject);
				clothCount = CompareObjectToList(clothIter, pKinematicObj, pNode->m_pActiveClothObjects, box, clothCount);
				clothCount = CompareObjectToList(clothIter, pKinematicObj, pNode->m_pSleepingClothObjects, box, clothCount);
				pWrapper = pWrapper->m_pNext;
			}
		}

		if(pNode->m_pActiveClothObjects)
		{
			//PERF_EVENT("Gravity.StaticObjectComparisons");
			C_ObjectWrapper* pWrapper(pNode->m_pStaticObjects);
			while(pWrapper)
			{
				C_Object* pStaticObj(pWrapper->m_pObject);
				clothCount = CompareObjectToList(clothIter, pStaticObj, pNode->m_pActiveClothObjects, box, clothCount);
				pWrapper = pWrapper->m_pNext;
			}
		}
		return clothCount;
	}

	U32 C_Scene::ActiveParentComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, U32& clothCount, SharedPhysics::C_MemoryIterator<Gravity::C_ObjectPair, 128>& clothIter)
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
				pParent->m_pActiveObjects || pParent->m_pActiveClothObjects || pParent->m_pSleepingClothObjects || 
				pParent->m_pFluidMeshObjects)
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

					//Cloth comparisons
					clothCount = CompareObjectToList(clothIter, pActiveObject, pParent->m_pActiveClothObjects, box, clothCount);
					clothCount = CompareObjectToList(clothIter, pActiveObject, pParent->m_pSleepingClothObjects, box, clothCount);

					//Fluid comparisons
					GenerateFluidTasks(pParent->m_pFluidMeshObjects, *pActiveObject, fluidQuery, pNode, fluidCollisionCount);

					pActives = pActives->m_pNext;
				}

				if(pParent->m_pKinematicObjects || pParent->m_pActiveClothObjects)
				{
					//PERF_EVENT("Gravity.SleepingObjectComparisons");
					C_ObjectWrapper* pSleepy(pNode->m_pSleepingObjects);
					while(pSleepy)
					{
						C_Object* pSleepyObject(pSleepy->m_pObject);
						GenerateCollisionTasks(pParent->m_pKinematicObjects, *pSleepyObject, query, pNode, dynamicCollisionCount);

						//Cloth comparisons
						clothCount = CompareObjectToList(clothIter, pSleepyObject, pParent->m_pActiveClothObjects, box, clothCount);

						pSleepy = pSleepy->m_pNext;
					}
				}

				if(pNode->m_pActiveClothObjects || pNode->m_pSleepingClothObjects)
				{
					//PERF_EVENT("Gravity.KinematicObjectComparisons");
					C_ObjectWrapper* pKinematic(pParent->m_pKinematicObjects);
					while(pKinematic)
					{
						C_Object* pKinematicObject(pKinematic->m_pObject);
						//Cloth comparisons
						clothCount = CompareObjectToList(clothIter, pKinematicObject, pNode->m_pActiveClothObjects, box, clothCount);
						clothCount = CompareObjectToList(clothIter, pKinematicObject, pNode->m_pSleepingClothObjects, box, clothCount);
						pKinematic = pKinematic->m_pNext;
					}
				}


				if(pNode->m_pStaticObjects || pNode->m_pKinematicObjects || pNode->m_pSleepingObjects || 
					pNode->m_pActiveClothObjects || pNode->m_pSleepingClothObjects || pNode->m_pFluidMeshObjects)
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

							//Cloth comparisons
							clothCount = CompareObjectToList(clothIter, pActiveObj, pNode->m_pActiveClothObjects, box, clothCount);
							clothCount = CompareObjectToList(clothIter, pActiveObj, pNode->m_pSleepingClothObjects, box, clothCount);
							
							//Fluid comparisons
							GenerateFluidTasks(pNode->m_pFluidMeshObjects, *pActiveObj, fluidQuery, pNode, fluidCollisionCount);
							pParentActive = pParentActive->m_pNext;
						}
					}

					if(pNode->m_pKinematicObjects || pNode->m_pActiveClothObjects)
					{
						//PERF_EVENT("Gravity.ParentSleepingObjectComparisons");
						C_ObjectWrapper* pParentSleeping(pParent->m_pSleepingObjects);
						while(pParentSleeping)
						{
							C_Object* pSleepyObject(pParentSleeping->m_pObject);

							GenerateCollisionTasks(pNode->m_pKinematicObjects, *pSleepyObject, query, pNode, dynamicCollisionCount);
							//Cloth comparisons
							clothCount = CompareObjectToList(clothIter, pSleepyObject, pNode->m_pActiveClothObjects, box, clothCount);

							pParentSleeping = pParentSleeping->m_pNext;
						}
					}
					if(pParent->m_pActiveClothObjects || pParent->m_pSleepingClothObjects)
					{
						//PERF_EVENT("Gravity.KinematicObjectComparisons");
						C_ObjectWrapper* pKinematic(pNode->m_pKinematicObjects);
						while(pKinematic)
						{
							C_Object* pKinematicObject(pKinematic->m_pObject);
							//Cloth comparisons
							clothCount = CompareObjectToList(clothIter, pKinematicObject, pParent->m_pActiveClothObjects, box, clothCount);
							clothCount = CompareObjectToList(clothIter, pKinematicObject, pParent->m_pSleepingClothObjects, box, clothCount);
							pKinematic = pKinematic->m_pNext;
						}
					}
					if(pParent->m_pActiveClothObjects)
					{
						C_ObjectWrapper* pStatic(pNode->m_pStaticObjects);
						while(pStatic)
						{
							C_Object* pStaticObject(pStatic->m_pObject);
							//Cloth comparisons
							clothCount = CompareObjectToList(clothIter, pStaticObject, pParent->m_pActiveClothObjects, box, clothCount);
							pStatic = pStatic->m_pNext;
						}
					}

					if(pNode->m_pActiveClothObjects)
					{
						C_ObjectWrapper* pWrapper(pParent->m_pStaticObjects);
						while(pWrapper)
						{
							C_Object* pStaticObject(pWrapper->m_pObject);
							clothCount = CompareObjectToList(clothIter, pStaticObject, pNode->m_pActiveClothObjects, box, clothCount);																
							pWrapper = pWrapper->m_pNext;
						}

					}
				}
			}
			pParent = pParent->m_pParentNode;
		}
		return clothCount;
	}

	U32 C_Scene::ActiveChildComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, U32& clothCount, SharedPhysics::C_MemoryIterator<Gravity::C_ObjectPair, 128>& clothIter,
		SharedPhysics::C_SafeStack<true>& stack)
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

				if(pChild->IsSleeping() && (pChild->m_pSleepingClothObjects || pChild->m_pSleepingObjects || 
					pChild->m_pStaticObjects || pChild->m_pFluidMeshObjects))
				{

					if(pNode->m_pKinematicObjects)
					{
						C_ObjectWrapper* pSleepy(pChild->m_pSleepingObjects);
						while(pSleepy)
						{
							C_Object* pSleepyObject(pSleepy->m_pObject);
							GenerateCollisionTasks(pNode->m_pKinematicObjects, *pSleepyObject, query, pChild, dynamicCollisionCount);
							clothCount = CompareObjectToList(clothIter, pSleepyObject, pNode->m_pActiveClothObjects, pChild->m_bbox, clothCount);																
							pSleepy = pSleepy->m_pNext;
						}
					}
					if(pNode->m_pActiveClothObjects)
					{
						C_ObjectWrapper* pStatic(pChild->m_pStaticObjects);
						while(pStatic)
						{
							C_Object* pStaticObject(pStatic->m_pObject);
							clothCount = CompareObjectToList(clothIter, pStaticObject, pNode->m_pActiveClothObjects, pChild->m_bbox, clothCount);																
							pStatic = pStatic->m_pNext;
						}
					}
					C_ObjectWrapper* pActive(pNode->m_pActiveObjects);
					while(pActive)
					{
						C_Object* pActiveObject(pActive->m_pObject);
						GenerateCollisionTasks(pChild->m_pSleepingObjects, *pActiveObject, query, pChild, dynamicCollisionCount);
						GenerateCollisionTasks(pChild->m_pStaticObjects, *pActiveObject, query, pChild, dynamicCollisionCount);
						GenerateFluidTasks(pChild->m_pFluidMeshObjects, *pActiveObject, fluidQuery, pChild, fluidCollisionCount);
						clothCount = CompareObjectToList(clothIter, pActiveObject, pChild->m_pSleepingClothObjects, pChild->m_bbox, clothCount);																
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
		return clothCount;
		
	}



	U32 C_Scene::SleepingParentComparison(C_DistributionNode* pSleepingNodes, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, U32& clothCount, SharedPhysics::C_MemoryIterator<Gravity::C_ObjectPair, 128>& clothIter)
	{
		using namespace Collision;
		C_DistributionNode* pChild(pSleepingNodes);
		C_DistributionNode* pParent(pSleepingNodes->m_pParentNode);
		while(pParent)
		{
			BIZ_ASSERT(pParent->m_pLeftNode == pChild || pParent->m_pRightNode == pChild);
			pChild = pParent;
			if(pParent->m_pKinematicObjects || pParent->m_pActiveClothObjects)
			{
				C_ObjectWrapper* pSleepy(pSleepingNodes->m_pSleepingObjects);
				while(pSleepy)
				{
					C_Object* pSleepyObject(pSleepy->m_pObject);
					GenerateCollisionTasks(pParent->m_pKinematicObjects, *pSleepyObject, query, pSleepingNodes, dynamicCollisionCount);
					clothCount = CompareObjectToList(clothIter, pSleepyObject, pParent->m_pActiveClothObjects, pSleepingNodes->m_bbox, clothCount);																
					pSleepy = pSleepy->m_pNext;
				}
				if(pParent->m_pActiveClothObjects)
				{
					C_ObjectWrapper* pStatic(pSleepingNodes->m_pStaticObjects);
					while(pStatic)
					{
						C_Object* pStaticObject(pStatic->m_pObject);
						clothCount = CompareObjectToList(clothIter, pStaticObject, pParent->m_pActiveClothObjects, pSleepingNodes->m_bbox, clothCount);																
						pStatic = pStatic->m_pNext;
					}
				}

			}

			if(pSleepingNodes->m_pSleepingObjects || pSleepingNodes->m_pSleepingClothObjects || pSleepingNodes->m_pStaticObjects || pSleepingNodes->m_pFluidMeshObjects)
			{
				C_ObjectWrapper* pActive(pParent->m_pActiveObjects);
				while(pActive)
				{
					C_Object* pActiveObject(pActive->m_pObject);
					GenerateCollisionTasks(pSleepingNodes->m_pSleepingObjects, *pActiveObject, query, pSleepingNodes, dynamicCollisionCount);
					GenerateCollisionTasks(pSleepingNodes->m_pStaticObjects, *pActiveObject, query, pSleepingNodes, dynamicCollisionCount);
					GenerateFluidTasks(pSleepingNodes->m_pFluidMeshObjects, *pActiveObject, fluidQuery, pSleepingNodes, fluidCollisionCount);
					clothCount = CompareObjectToList(clothIter, pActiveObject, pSleepingNodes->m_pSleepingClothObjects, pSleepingNodes->m_bbox, clothCount);																
					pActive = pActive->m_pNext;
				}
			}
			pParent = pParent->m_pParentNode;
		}
		return clothCount;
	}


	//Returns number of cloth collision. For now, can't kick off cloth collision detection as a task yet!!!
	//U32 C_Scene::CombinedPairGeneration(SharedPhysics::C_SafeStack<true>& stack, const Maths::HF32 PCREF time)
	//{
	//	using namespace Collision;
	//	PERF_EVENT("Gravity.BroadPhase_Combined");
	//	using namespace Maths;
	//	
	//	U32 dynamicCollisionCount = g_dbDynamicCollisionGranularity;
	//	U32 fluidCollisionCount = MaxFluidQuery;

	//	C_DistributionNode* pNode=m_pTree->m_pActiveNodes;

	//	C_CollisionQuery query;
	//	//Maths::Store(time, &query.m_Time);
	//	query.m_pStream = m_pCollisionManager->m_pCurrentStream;
	//	query.m_pManager = m_pCollisionManager;
	//	query.m_NumPairs = 0;

	//	C_FluidQuery fluidQuery;
	//	fluidQuery.m_Size = 0;
	//	fluidQuery.m_pManager = m_pCollisionManager;
	//	fluidQuery.m_pFluidBuffer = &m_FluidEventBuffer;

	//	SharedPhysics::C_MemoryIterator<Gravity::C_ObjectPair, 128> clothIter(m_pClothPairs);
	//	U32 clothCount = 0;
	//	
	//	while(pNode)
	//	{
	//		ActiveNodeComparison(pNode, query, dynamicCollisionCount, fluidQuery, fluidCollisionCount/*, clothCount, clothIter*/);
	//		ActiveParentComparison(pNode, query, dynamicCollisionCount, fluidQuery, fluidCollisionCount/*, clothCount, clothIter*/);
	//		//if(g_dbDoActiveChildComparison)
	//		ActiveChildComparison(pNode, query, dynamicCollisionCount, fluidQuery, fluidCollisionCount, /*clothCount, clothIter, */stack);
	//		pNode=pNode->m_pNext;
	//	}

	//	/*if(!g_dbDoActiveChildComparison)
	//	{
	//		C_DistributionNode* pSleepingNodes(m_pTree->m_pSleepingNodes);
	//		while(pSleepingNodes)
	//		{
	//			SleepingParentComparison(pSleepingNodes, query, dynamicCollisionCount, fluidQuery, fluidCollisionCount, clothCount, clothIter);
	//			pSleepingNodes = pSleepingNodes->m_pNext;
	//		}
	//	}*/

	//	if(query.m_NumPairs)
	//	{
	//		Scheduler::AddTask(DynamicCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
	//	}

	//	if(fluidQuery.m_Size)
	//	{
	//		Scheduler::AddTask(FluidCollisionModule, NULL, NULL, m_Group, m_Policy, &fluidQuery, sizeof(C_FluidQuery));
	//	}

	//	return clothCount;
	//}

	U32 C_Scene::CombinedPairGeneration(SharedPhysics::C_SafeStack<true>& stack, const Maths::HF32 PCREF time)
	{
		using namespace Collision;
		PERF_EVENT("Gravity.BroadPhase_Combined");
		using namespace Maths;
		
		/*U32 dynamicCollisionCount = g_dbDynamicCollisionGranularity;
		U32 fluidCollisionCount = MaxFluidQuery;
		C_DistributionNode* pNode=m_pTree->m_pActiveNodes;*/

		U32 clothCount = 0;

		Gravity::BroadPhaseTask task;
		task.m_TaskSize = 0;
		task.m_pFluidBuffer = &m_FluidEventBuffer;
		task.m_pStream = m_pCollisionManager->m_pCurrentStream;
		task.m_pClothStream = &m_pCollisionManager->m_ClothStream;
		task.m_pManager = m_pCollisionManager;
		task.m_pScene = this;
				
		/*while(pNode)
		{
			task.m_Nodes[task.m_TaskSize++] = pNode;
			if(task.m_TaskSize == BroadPhaseTaskSize)
			{
				Scheduler::AddTask(BroadPhaseModule, NULL, NULL, m_BroadPhaseGroup, m_Policy, &task, sizeof(Gravity::BroadPhaseTask));
				task.m_TaskSize = 0;
			}
			pNode=pNode->m_pNext;
		}*/

		Gravity::C_Body* pBody = m_ActiveBodyList.m_pFirstBody;
		while(pBody)
		{
			BIZ_ASSERT(pBody->IsActive());
			if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			{
				task.m_pBodies[task.m_TaskSize++] = pBody;
				if(task.m_TaskSize == BroadPhaseTaskSize)
				{
					Scheduler::AddTask(BroadPhaseModule, NULL, NULL, m_Group/*m_BroadPhaseGroup*/, m_Policy, &task, sizeof(Gravity::BroadPhaseTask));
					task.m_TaskSize = 0;
				}
			}
			pBody = pBody->m_pNext;
		}

		pBody = m_ContinuousBodyList.m_pFirstBody;
		while(pBody)
		{
			BIZ_ASSERT(pBody->IsActive());
			if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			{
				task.m_pBodies[task.m_TaskSize++] = pBody;
				if(task.m_TaskSize == BroadPhaseTaskSize)
				{
					Scheduler::AddTask(BroadPhaseModule, NULL, NULL, m_Group/*m_BroadPhaseGroup*/, m_Policy, &task, sizeof(Gravity::BroadPhaseTask));
					task.m_TaskSize = 0;
				}
			}
			pBody = pBody->m_pNext;
		}

		pBody = m_KinematicBodyList.m_pFirstBody;
		while(pBody)
		{
			BIZ_ASSERT(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject));
			if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
			{
				task.m_pBodies[task.m_TaskSize++] = pBody;
				if(task.m_TaskSize == BroadPhaseTaskSize)
				{
					Scheduler::AddTask(BroadPhaseModule, NULL, NULL, m_Group/*m_BroadPhaseGroup*/, m_Policy, &task, sizeof(Gravity::BroadPhaseTask));
					task.m_TaskSize = 0;
				}
			}
			pBody = pBody->m_pNext;
		}


		if(task.m_TaskSize > 0)
		{
			Scheduler::AddTask(BroadPhaseModule, NULL, NULL, m_Group/*m_BroadPhaseGroup*/, m_Policy, &task, sizeof(Gravity::BroadPhaseTask));
			task.m_TaskSize = 0;
		}

		return clothCount;
	}


	void C_Scene::WaitForDeferredContacts( const Maths::HF32 PCREF time )
	{
		PERF_EVENT("Gravity.WaitForDeferredContacts");
		using namespace Collision;
		using namespace SharedPhysics;

		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		const Maths::HF32 fZero(Maths::Constants<Maths::HF32>::Zero());
		//Scheduler::Flush(m_BroadPhaseGroup);
		Scheduler::Flush(m_Group);
		m_pCollisionManager->PatchColliders(m_WakingBodyList);
		

		//Store back collision data between character controllers and feet collisions
		Collision::C_CollisionResult<_MaxPatches, _MaxContacts> results;
		const Maths::HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
		for(S32 a = 0; a < m_CharacterControllers.m_numElements; ++a)
		{
			C_CharacterController* pController = m_CharacterControllers[a].m_pElement;
			C_CharacterControllerShape* pCharacterShape = pController->m_pCollisionShape;
			
			Collision::C_RayTracingResult& traceResult = pCharacterShape->m_Controller.m_Ray.m_result;
			if(traceResult.Hit())
			{
				C_CollisionObject objA(pController->m_pCharacterBody->m_Object.CollisionObject());
				C_CollisionObject objB(traceResult.m_pNode->m_pBody->m_Object.CollisionObject());

				//Set shape/node...
				results.m_pObjectA = &objA;
				results.m_pObjectB = &objB;

				

				//Maths::C_Normal norm(-traceResult.m_TriangleNormal);

				Maths::HF32 pen(((pCharacterShape->m_Controller.m_RayLength) - traceResult.m_distance));
				//if(traceResult.m_distance > pCharacterShape->m_Controller.m_RayLength)
				/*if(pen < -pCharacterShape->m_Controller.m_DeadZone)
				{
					norm = traceResult.m_TriangleNormal;
					pen = pCharacterShape->m_Controller.m_MinPenetration + pen;
				}*/

				Maths::Scalar_Bool con(pen.LT(-pCharacterShape->m_Controller.m_DeadZone) && traceResult.m_pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Static));
				Maths::C_Normal norm(Maths::Select(con, traceResult.m_TriangleNormal, -traceResult.m_TriangleNormal) );
				pen = Maths::Select(con, pCharacterShape->m_Controller.m_MinPenetration + pen, pen);

				if(pen.GE(fZero))
				{

					SharedPhysics::C_Collider* pCollider = m_pCollisionManager->CreateColliderMesh(objA.m_pNodePtr, objB.m_pNodePtr);

					//pen = Maths::Max(Maths::Constants<Maths::HF32>::Zero(), pen);
					results.m_NumContactPatches = 1;
					
					//Loop through collision result and add in contact for pill!
					Collision::C_ContactPatch<_MaxContacts>& patch = results.m_ContactPatches[0];
					patch.m_NumContactPoints = 1;
					patch.m_Normal = norm;

					Collision::C_ContactPoint& point(patch.m_ContactPoints[0]);
					point.m_ContactPoint = traceResult.m_contact;
					//point.m_PenetrationDepth = pen;
					Maths::Store(pen, &point.m_PenetrationDepth);
					BIZ_ASSERT(objA.m_pShape->m_PhysicsMaterialIndex < m_pGraphManager->m_NumMaterials);
					BIZ_ASSERT(pCharacterShape->m_Controller.m_FeetMaterial < m_pGraphManager->m_NumMaterials);
					point.m_PhysicsMaterialIndices = traceResult.m_PhysicsMaterialIndex << 16 | pCharacterShape->m_Controller.m_FeetMaterial;
					point.m_EffectMaterialIndices = traceResult.m_EffectMaterialIndex << 16 | pCharacterShape->m_Controller.m_EffectMaterial;
					point.m_HashValue = traceResult.m_TriangleIndex + (m_pCollisionManager->m_HashValueRandomisation++);
					Collision::EncodeContacts<_MaxPatches, _MaxContacts>(results, *m_pCollisionManager->m_pCurrentStream, pCollider);

					BIZ_ASSERT(!(pCollider->m_pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static) & pCollider->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static)));

					if(pCollider->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static) & (!pCollider->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered)))
					{
						m_WakingBodyList.AddBody(pCollider->m_pNodeB->m_pBody);
						pCollider->m_pNodeB->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
						pCollider->m_pNodeB->m_TotalKineticEnergy = zero;
						pCollider->m_pNodeB->m_pRigidBody->m_KineticEnergy = zero;
					}
					
					if (pCollider->m_pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static) & (!pCollider->m_pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered)))
					{
						m_WakingBodyList.AddBody(pCollider->m_pNodeA->m_pBody);
						pCollider->m_pNodeA->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
						pCollider->m_pNodeA->m_TotalKineticEnergy = zero;
						pCollider->m_pNodeA->m_pRigidBody->m_KineticEnergy = zero;
					}

					

				/*	BIZ_ASSERT((!pCollider->m_pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static)) || 
					pCollider->m_pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered));

					BIZ_ASSERT((!pCollider->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static)) || 
					pCollider->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered));*/


					//Find this using the constraints in the 
					SharedPhysics::C_ConstraintEdge* pExistingConstraint = pCollider->m_pNodeA->FindPrevConstraint(pCollider->m_pNodeB);
					//SharedPhysics::C_Collider* pExistingCollider = m_pContactDatabase->FindPrevCollider(pNodeA, pNodeB, index);
					//if(pExistingCollider)
					if(pExistingConstraint && pExistingConstraint->m_pCollider)
					{
						//pExistingCollider->m_State = SharedPhysics::C_Collider::E_Removed;
						//pCollider->m_pConstraintEdge = pExistingCollider->m_pConstraintEdge;
						//pExistingCollider->m_pConstraintEdge = NULL;
						pCollider->m_pConstraintEdge = pExistingConstraint;
						pExistingConstraint->m_pCollider->m_State = SharedPhysics::C_Collider::E_Removed;
						pExistingConstraint->m_pCollider->m_pConstraintEdge = NULL;

			/*#ifdef USE_PERSISTENT_MANIFOLD
						pCollider->m_PersistentManifold = pExistingConstraint->m_pCollider->m_PersistentManifold;
						pCollider->m_PersistentManifold.RefreshManifoldContacts(pCollider->m_pNodeA->m_pBody->GetShape()->m_PosRot,
							pCollider->m_pNodeB->m_pBody->GetShape()->m_PosRot);
			#endif*/
					}
				}

			}
		}

		{
			//Book mark stream
			SharedPhysics::C_ConstContactStreamPtr streamStart(&m_pCollisionManager->m_ClothStream, m_pCollisionManager->m_ClothStream.m_pStream);
			CopyClothCollisions(streamStart);
		}
	}

	void C_Scene::CopyClothCollisions(SharedPhysics::C_ConstContactStreamPtr& ptr)
	{
		PERF_EVENT("Gravity.CopyClothCollisions");
		//Cloth collision stuff - transfer cloth data to correct buffers! Perhaps this can be a task later???
		{
			{
				SharedPhysics::C_ParticleContactPair* res = (SharedPhysics::C_ParticleContactPair*)ptr.m_pStream->m_pCurrentWritePoint;
				res->m_ValidData = SharedPhysics::E_NullData;
			}
			SharedPhysics::C_ParticleContactStreamIterator iter(&ptr);
			while(iter.HasNextResult())
			{
				SharedPhysics::C_ParticleContactPair* pResult = iter.NextResult();
				Collision::C_ClothShape* pClothShape = (Collision::C_ClothShape*)pResult->m_pParticlePtr->m_pBody->m_pShape;

				if((pResult->m_pParticlePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Waking)))
				{
					if(pResult->m_pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping) 
						&& !pResult->m_pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Waking) && 
						!pResult->m_pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
					{
						pResult->m_pNodePtr->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered /*| SharedPhysics::C_GraphNode::E_Type_LightWakeUp | SharedPhysics::C_GraphNode::E_Type_Waking*/);
						m_WakingBodyList.AddBody(pResult->m_pNodePtr->m_pBody);
					}
				}
				else if (pResult->m_pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_KinematicObject) 
					& !pResult->m_pParticlePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
				{
					m_WakingBodyList.AddBody(pResult->m_pParticlePtr->m_pBody);
					pResult->m_pParticlePtr->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered | SharedPhysics::C_GraphNode::E_Type_LightWakeUp | SharedPhysics::C_GraphNode::E_Type_Waking);
				}

				
				while(iter.HasNextContact())
				{
					SharedPhysics::C_ParticleContactStorage* pContact = iter.NextContact();
					const Maths::HVec3 contact(Maths::Load<Maths::HVec3>(&pContact->m_Contact));
					const Maths::HF32 pen(Maths::Load<Maths::HF32>(&pContact->m_Penetration));
					const Maths::C_Normal norm(Maths::Load<Maths::HVec3>(&pContact->m_EscapeVector), Maths::C_Normal::UNIT_LENGTH_VECTOR);
					const Maths::C_Normal triangleNormal(Maths::Load<Maths::HVec3>(&pContact->m_TriangleNormal), Maths::C_Normal::UNIT_LENGTH_VECTOR);
					const Maths::HF32 stiffness(Maths::Load<Maths::HF32>(&pContact->m_Stiffness));

					if(pContact->m_Type == Collision::C_ParticleContact::E_TriangleConstraint)
					{
						pClothShape->m_Cloth.AddTriangleContact(pResult->m_pParticlePtr, pResult->m_pNodePtr,
							contact,  norm, triangleNormal, pen,  stiffness, pContact->m_Index);
					}
					else
					{
						pClothShape->m_Cloth.AddVertexContact(pResult->m_pParticlePtr, pResult->m_pNodePtr,
							contact,  norm, triangleNormal, pen,  stiffness, pContact->m_Index);
					}				
				}
			}	
		}
	}


	void C_Scene::GetMeshContacts()
	{
		U32 meshCollisionCount = g_dbMeshCollisionGranularity;
		C_CollisionQuery query;
		query.m_pStream = m_pCollisionManager->m_pCurrentStream;
		query.m_pManager = m_pCollisionManager;
		query.m_NumPairs = 0;
		
		m_pWorldBody->m_GraphNode.m_pVolatileConstraints = NULL;
		m_pWorldBody->m_GraphNode.m_pVolatileTwinConstraints = NULL;
		m_pWorldBody->m_GraphNode.m_pCloseConstraints = NULL;
		m_pWorldBody->m_GraphNode.m_pCloseTwinConstraints = NULL;

		{
			PERF_EVENT("Physics.MeshCollision");
			
			for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
			{
				C_Body* pMeshBody(m_MeshCollisionBodies[a].m_pElement);
				pMeshBody->m_GraphNode.m_pVolatileConstraints = NULL;
				pMeshBody->m_GraphNode.m_pVolatileTwinConstraints = NULL;
				pMeshBody->m_GraphNode.m_pCloseConstraints = NULL;
				pMeshBody->m_GraphNode.m_pCloseTwinConstraints = NULL;
			}

			C_Body* pBody = m_ActiveBodyList.m_pFirstBody;
			while(pBody)
			{
				BIZ_ASSERT(pBody->IsActive());
				for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
				{
					C_Body* pMeshBody(m_MeshCollisionBodies[a].m_pElement);
					if(pBody->m_pShape->CanCollide(pMeshBody->m_pShape))
					{
						query.m_pColliders[query.m_NumPairs++] = m_pCollisionManager->CreateColliderMesh(&pBody->m_GraphNode, &pMeshBody->m_GraphNode);
						BIZ_ASSERT(query.m_pColliders[query.m_NumPairs-1]->m_pNodeB->m_pBody->m_pShape->GetShape() == Collision::E_TriangleMesh_Shape);
						if(query.m_NumPairs == meshCollisionCount)
						{
							Scheduler::AddTask(MeshCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
							query.m_NumPairs = 0;
						}
					}
				}
				
				pBody = pBody->m_pNext;
			}

			pBody = m_ContinuousBodyList.m_pFirstBody;
			while(pBody)
			{
				BIZ_ASSERT(pBody->IsActive());
				for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
				{
					C_Body* pMeshBody(m_MeshCollisionBodies[a].m_pElement);
					if(pBody->m_pShape->CanCollide(pMeshBody->m_pShape))
					{
						query.m_pColliders[query.m_NumPairs++] = m_pCollisionManager->CreateColliderMesh(&pBody->m_GraphNode, &pMeshBody->m_GraphNode);
						BIZ_ASSERT(query.m_pColliders[query.m_NumPairs-1]->m_pNodeB->m_pBody->m_pShape->GetShape() == Collision::E_TriangleMesh_Shape);
						if(query.m_NumPairs == meshCollisionCount)
						{
							Scheduler::AddTask(MeshCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
							query.m_NumPairs = 0;
						}
					}
				}
				pBody = pBody->m_pNext;
			}
			if(query.m_NumPairs)
			{
				Scheduler::AddTask(MeshCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
				query.m_NumPairs = 0;
			}
		}
		
		{
			PERF_EVENT("Physics.PlaneCollision");
			if(m_pGroundPlane)
			{
				m_pGroundPlane->m_GraphNode.m_pVolatileConstraints = NULL;
				m_pGroundPlane->m_GraphNode.m_pVolatileTwinConstraints = NULL;
				m_pGroundPlane->m_GraphNode.m_pCloseConstraints = NULL;
				m_pGroundPlane->m_GraphNode.m_pCloseTwinConstraints = NULL;
				C_Body* pBody = m_ActiveBodyList.m_pFirstBody;
				while(pBody)
				{
					BIZ_ASSERT(pBody->IsActive());
					if(pBody->m_pShape->CanCollide(m_pGroundPlane->m_pShape))
					{
						query.m_pColliders[query.m_NumPairs++]=m_pCollisionManager->CreateColliderMesh(&pBody->m_GraphNode, &m_pGroundPlane->m_GraphNode);
						if(query.m_NumPairs == meshCollisionCount)
						{
							Scheduler::AddTask(GroundPlaneCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
							query.m_NumPairs = 0;
						}
					}
					pBody = pBody->m_pNext;
				}

				pBody = m_ContinuousBodyList.m_pFirstBody;
				while(pBody)
				{
					if(pBody->m_pShape->CanCollide(m_pGroundPlane->m_pShape))
					{
						query.m_pColliders[query.m_NumPairs++]=m_pCollisionManager->CreateColliderMesh(&pBody->m_GraphNode, &m_pGroundPlane->m_GraphNode);
						if(query.m_NumPairs == meshCollisionCount)
						{
							Scheduler::AddTask(GroundPlaneCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
							query.m_NumPairs = 0;
						}
					}
					pBody = pBody->m_pNext;

				}

				if(query.m_NumPairs)
				{
					Scheduler::AddTask(GroundPlaneCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
					query.m_NumPairs = 0;
				}
			}
		}
	}

	void C_Scene::GetContacts( const Maths::HF32 PCREF time )
	{
		using namespace Collision;
		using namespace SharedPhysics;

		//U32 numPairs( m_pTree->CollidedPairGeneration( *m_pContactPairs, m_Stack ) );

		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		U32 dynamicCollisionCount = g_dbDynamicCollisionGranularity;
		PERF_EVENT("Gravity.GetContacts");

		U32 numClothPairs = CombinedPairGeneration(m_Stack, time);

		for(S32 a = 0; a < m_CharacterControllers.m_numElements; ++a)
		{
			C_CharacterController* pController = m_CharacterControllers[a].m_pElement;
			if(pController->GetBody()->IsActive())
			{
				C_CharacterControllerShape* pCharacterShape = pController->m_pCollisionShape;
				pCharacterShape->m_Controller.m_bContactWrittenToStream = false;
				pCharacterShape->m_Controller.m_Ray.m_result.m_pShape = NULL;
				this->DoDeferredRayTrace(&pCharacterShape->m_Controller.m_Ray, NULL,
					m_Group, m_Policy);
			}
		}

		//Scheduler::Flush(m_Group);
	}


	void C_Scene::UpdateRagdollControllers(const Maths::HF32 PCREF time)
	{
		C_RagdollControllerCommand command;
		Maths::Store(time, &command.m_Time);

		C_RagdollController* pController = m_pActiveRagdollControllers;
		while(pController)
		{

			command.m_pController[command.m_NumControllers++] = pController;
			if(command.m_NumControllers == MaxRagdollControllers)
			{
				Scheduler::AddTask(UpdateRagdollControllerModule, NULL, NULL, m_Group, m_Policy, &command, sizeof(C_RagdollControllerCommand));
				command.m_NumControllers = 0;
			}
			pController = pController->m_pNext;
		}


		if(command.m_NumControllers > 0)
		{
			Scheduler::AddTask(UpdateRagdollControllerModule, NULL, NULL, m_Group, m_Policy, &command, sizeof(C_RagdollControllerCommand));
			command.m_NumControllers = 0;
		}
	}

	void C_Scene::FinaliseRagdollControllers()
	{
		Scheduler::Flush(m_Group);
		/*for(S32 a = 0; a < m_RagdollControllers.m_numElements; ++a)
		{
			m_RagdollControllers[a].m_pElement->Synchronise();
		}*/
	}

	void C_Scene::DoContinuousCollisionDetection()
	{
		SharedPhysics::C_MemoryIterator<C_DeferredCCDCommand> iter(m_pContinuousCommandList);
		U32 count = 0;
		C_Body* pBody = m_ContinuousBodyList.m_pFirstBody;
		while(pBody)
		{

			C_DeferredCCDCommand* command = iter.Create();
			command->m_pBody = pBody;
			command->m_pScene = this;
			command->m_pMeshNode = NULL;
			command->m_pStream = m_pCollisionManager->m_pCurrentStream;
			command->ppuAddress = command;
			//Scheduler::AddTask(ContinuousCollisionModule, NULL, NULL, m_Group, m_Policy, command, sizeof(C_DeferredCCDCommand));

			Scheduler::AddTask(ContinuousCollisionModule, NULL, NULL, m_Group, m_Policy, command, sizeof(C_DeferredCCDCommand));
			++count;
			//DoContinuousCollisionDetection(pBody);
			pBody = pBody->m_pNext;
		}

		Scheduler::Flush(m_Group);

		SharedPhysics::C_MemoryIterator<C_DeferredCCDCommand> resultIter(m_pContinuousCommandList);
		for(U32 a = 0; a < count; ++a)
		{
			C_DeferredCCDCommand* command = resultIter.Get();
			if(command->m_pMeshNode)
			{
				//It was a hit...
				SharedPhysics::C_Collider* pCollider = m_pCollisionManager->CreateCollider(&command->m_pBody->m_GraphNode, command->m_pMeshNode);
				pCollider->m_StreamPtr.m_pStream = command->m_pStream;
				pCollider->m_StreamPtr.m_pCurrentReadPoint = command->m_pCurrentReadPoint;
			}
		}

	}


	void C_Scene::WakeUpBodies(SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_ConstContactStreamPtr& clothStreamEnd, const Maths::HF32 PCREF time)
	{
		using namespace SharedPhysics;
		PERF_EVENT("gravity.WakeUpBodies");


		{
			C_Body* pBody = m_ActiveBodyList.m_pFirstBody;
			while(pBody)
			{
				BIZ_ASSERT(pBody->IsActive());
				/*pBody->m_GraphNode.m_pVolatileConstraints = NULL;
				pBody->m_GraphNode.m_pVolatileTwinConstraints = NULL;*/
				pBody->m_GraphNode.WakeUpConstrainedObjects(stack, m_WakingBodyList);
				pBody = pBody->m_pNext;
			}
		}

		{
			C_Body* pBody = m_KinematicBodyList.m_pFirstBody;
			while(pBody)
			{
				/*pBody->m_GraphNode.m_pVolatileConstraints = NULL;
				pBody->m_GraphNode.m_pVolatileTwinConstraints = NULL;*/
				pBody->m_GraphNode.WakeUpConstrainedObjects(stack, m_WakingBodyList);
				pBody = pBody->m_pNext;
			}
		}

		

		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		{
			PERF_EVENT("gravity.WakeUpBodies1");
			C_Body* pBody = m_WakingBodyList.m_pFirstBody;
			while(pBody)
			{
				C_Body* pNextBody = pBody->m_pNext;
				if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Waking) || 
					(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered)
					&& pBody->m_GraphNode.m_Deactivator.MustActivate()))
				{
					//Waking body...
					pBody->m_GraphNode.SetNodeFlags(C_GraphNode::E_Type_Waking);
				}
				else
				{
					m_WakingBodyList.RemoveBody(pBody);
					m_TouchedBodyList.AddBody(pBody);
					pBody->m_GraphNode.m_TotalKineticEnergy = zero;
					pBody->m_GraphNode.m_IslandIndex = 0;
					pBody->m_GraphNode.ClearNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation | C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_Root_Node | C_GraphNode::E_Type_Visited_Wakeup | 
					C_GraphNode::E_Type_Root_Node);
				}
				//pBody->m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
				pBody = pNextBody;
			}
		}

		{
			PERF_EVENT("gravity.WakeUpBodies2");
			Gravity::C_BodyList constrainedWakeUpList;
			C_Body* pBody = m_WakingBodyList.m_pFirstBody;
			while(pBody)
			{
				pBody->m_GraphNode.WakeUpConstrainedObjects(stack, constrainedWakeUpList);
				pBody = pBody->m_pNext;
			}
			pBody = constrainedWakeUpList.m_pFirstBody;
			while(pBody)
			{
				C_Body* pNextBody = pBody->m_pNext;
				constrainedWakeUpList.RemoveBody(pBody);
				m_WakingBodyList.AddBody(pBody);
				pBody = pNextBody;
			}

		}
		
		{
			PERF_EVENT("gravity.WakeUpBodies3");
			m_ReadWriteLock.WriteLock();
			C_Body* pBody = m_WakingBodyList.m_pFirstBody;
			while(pBody)
			{
				C_Body* pNextBody = pBody->m_pNext;
				//if(pBody->m_GraphNode.TestNodeFlags( C_GraphNode::E_Type_Waking ))
				{
					//pBody->m_GraphNode.ClearNodeFlags( (C_GraphNode::E_Type_Waking | C_GraphNode::E_Type_ZeroInertia | C_GraphNode::E_Type_NoddingOff) );
					BIZ_ASSERT(!pBody->m_GraphNode.TestNodeFlags(C_GraphNode::E_Type_Active));
					pBody->m_GraphNode.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia);
					pBody->m_GraphNode.SetNodeFlags( C_GraphNode::E_Type_Active | C_GraphNode::E_State_Changed );
					pBody->m_ExternalState.m_ExternalBodyState = pBody->m_GraphNode.m_NodeFlags & (SharedPhysics::C_GraphNode::E_Type_BodyStateMask);
					//BIZ_ASSERT(!pBody->IsExternalSleeping());
					//BIZ_ASSERT(!pBody->IsExternalStatic());
					/*pBody->m_GraphNode.m_pVolatileConstraints = NULL;
					pBody->m_GraphNode.m_pVolatileTwinConstraints = NULL;*/
					pBody->DoWakeUpBroadPhase(m_pTree);
					//pBody->ZeroExternalForces();
					pBody->LoadExternalForces();
					if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_LightWakeUp))
					{
						pBody->GetGraphNode()->m_Deactivator.Activate();
					}
					C_BodyEvent* pEvent = m_EventStream.CreateEvent();
					pEvent->SetBody(pBody);
					pEvent->SetEventType(C_BodyEvent::E_WakeUp);
					/*m_WakingBodyList.RemoveBody(pBody);
					if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
						m_KinematicBodyList.AddBody(pBody);
					else
						m_ActiveBodyList.AddBody(pBody);*/
					pBody->m_GraphNode.ClearNodeFlags( (C_GraphNode::E_Type_Waking | C_GraphNode::E_Type_ZeroInertia ) );
				}
				//GetContactsForWakingBody(pBody, time);
				pBody = pNextBody;
			}
			m_ReadWriteLock.ReleaseWriteLock();
		}

		



		{
			PERF_EVENT("gravity.WakeUpBodies4");			
			BroadPhaseTask task;
			task.m_TaskSize = 0;
			task.m_pFluidBuffer = &m_FluidEventBuffer;
			task.m_pStream = m_pCollisionManager->m_pCurrentStream;
			task.m_pManager = m_pCollisionManager;
			task.m_pClothStream = &m_pCollisionManager->m_ClothStream;
			task.m_pScene = this;

			C_CollisionQuery query;
			//Maths::Store(time, &query.m_Time);
			query.m_pStream = m_pCollisionManager->m_pCurrentStream;
			query.m_pManager = m_pCollisionManager;
			query.m_NumPairs = 0;

			U32 meshCollisionCount = g_dbMeshCollisionGranularity;


			C_Body* pBody = m_WakingBodyList.m_pFirstBody;
			while(pBody)
			{
				C_Body* pNextBody = pBody->m_pNext;
				//Move from waking to touched...
				m_WakingBodyList.RemoveBody(pBody);
				//BIZ_ASSERT(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject));
				BIZ_ASSERT(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));
				//BIZ_ASSERT(pBody->IsExternalActive());
				//BIZ_ASSERT(!pBody->IsExternalSleeping());
				//BIZ_ASSERT(!pBody->IsExternalStatic());
				m_ActiveBodyList.AddBody(pBody);
				GetContactsForWakingBody(pBody, task, query, time);
				/*if(task.m_TaskSize == BroadPhaseTaskSize)
				{
					Scheduler::AddTask(WakeUpBroadPhaseModule, 0, 0, m_Group, m_Policy, &task, sizeof(BroadPhaseTask));
					task.m_TaskSize = 0;
				}
				if(query.m_NumPairs == meshCollisionCount)
				{
					Scheduler::AddTask(MeshCollisionModule, 0, 0, m_Group, m_Policy, &task, sizeof(BroadPhaseTask));
					query.m_NumPairs = 0;
				}*/
				pBody = pNextBody;
			}

			if(task.m_TaskSize > 0)
			{
				Scheduler::AddTask(WakeUpBroadPhaseModule, 0, 0, m_Group, m_Policy, &task, sizeof(BroadPhaseTask));
				task.m_TaskSize = 0;
			}
			if(query.m_NumPairs > 0)
			{
				Scheduler::AddTask(MeshCollisionModule, 0, 0, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
				query.m_NumPairs = 0;
			}
		}

		Scheduler::Flush(m_Group);		
		m_pCollisionManager->PatchWakingColliders(m_TouchedBodyList);
		m_WakingBodyList.ClearList();
		CopyClothCollisions(clothStreamEnd);		
	}


	void C_Scene::SendDeactivateMessage(C_Body* pBody)
	{
		C_BodyEvent* pEvent = m_EventStream.CreateEvent();
		pEvent->SetBody(pBody);
		pEvent->SetEventType(C_BodyEvent::E_GoToSleep);
	}

	void C_Scene::SendActivateMessage(C_Body* pBody)
	{
		C_BodyEvent* pEvent = m_EventStream.CreateEvent();
		pEvent->SetBody(pBody);
		pEvent->SetEventType(C_BodyEvent::E_WakeUp);
	}

	U32 C_Scene::GetNumActiveBodies() const
	{
		return m_NumActiveBodies;
	}

	//inline void C_Scene::GenerateCollisionTasksNoLock(C_ObjectWrapper* pList, C_Object& obj, C_CollisionQuery& query,
	//	C_DistributionNode* pNode, const U32& maxCount)
	//{
	//	while(pList)
	//	{
	//		C_Object* pObj(pList->m_pObject);

	//		if(pObj->m_TransformedBox.IntersectsInAxisBox(obj.m_TransformedBox, pNode->m_bbox) && 
	//			obj.m_pShape->CanCollide(pObj->m_pShape))
	//		{
	//			query.m_pColliders[query.m_NumPairs++] = m_pCollisionManager->CreateColliderNoLock(obj.m_pGraphNode, pObj->m_pGraphNode);
	//			if(query.m_NumPairs == maxCount)
	//			{
	//				Scheduler::AddTask(DynamicCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
	//				query.m_NumPairs = 0;
	//			}
	//			
	//		}
	//		pList = pList->m_pNext;
	//	}
	//}


	inline void C_Scene::GenerateFluidTasks(C_ObjectWrapper* pList, C_Object& obj, C_FluidQuery& query,
		C_DistributionNode* pNode, const U32& maxCount)
	{
		if(obj.m_pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoBuoyancy))
			return;
		while(pList)
		{
			//DCBT(0, pList->m_pNext);
			C_Object* pObj(pList->m_pObject);
			if(pObj->m_TransformedBox.IntersectsInAxisBox(obj.m_TransformedBox, pNode->m_bbox)&& obj.m_pShape->CanCollide(pObj->m_pShape))
			{
				query.m_pFluidObject[query.m_Size] = pObj;
				query.m_pObject[query.m_Size++] = &obj;
				if(query.m_Size == maxCount)
				{
					Scheduler::AddTask(FluidCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
					query.m_Size = 0;
				}
				
			}
			pList = pList->m_pNext;
		}
	}

	void C_Scene::GetContactsForWakingBody(C_Body* pBody, BroadPhaseTask& task, C_CollisionQuery& query, const Maths::HF32 PCREF time)
	{
		PERF_EVENT("gravity.GetContactsForWakingBodies");
		//Collision::C_CollisionResult<_MaxPatches, _MaxContacts> results;

		U32 dynamicCollisionCount = g_dbDynamicCollisionGranularity;
		U32 meshCollisionCount = g_dbMeshCollisionGranularity;

		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		//Loop through all nodes this body is in...
		C_Object& obj(pBody->m_Object);
		//C_CollisionObject objA(obj.CollisionObject());

		for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
		{
			C_Body* pMeshBody(m_MeshCollisionBodies[a].m_pElement);
			if(pBody->m_pShape->CanCollide(pMeshBody->m_pShape))
			{
				/*C_CollisionObject meshObj(pMeshBody->m_Object.CollisionObject(time));
				m_CollisionManager.CollideMesh(&meshObj, &objA, *m_CollisionManager.m_pCurrentStream, m_Stack, results,
					m_CollisionManager.m_DefaultEpa);*/
				query.m_pColliders[query.m_NumPairs++] = m_pCollisionManager->CreateColliderMesh(obj.m_pGraphNode, &pMeshBody->m_GraphNode);
				if(query.m_NumPairs == meshCollisionCount)
				{
					Scheduler::AddTask(MeshCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
					query.m_NumPairs = 0;
				}
			}
		}

		if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
		{
			task.m_pBodies[task.m_TaskSize++] = pBody;
			if(task.m_TaskSize == BroadPhaseTaskSize)
			{
				Scheduler::AddTask(WakeUpBroadPhaseModule, 0, 0, m_Group, m_Policy, &task, sizeof(BroadPhaseTask));
				task.m_TaskSize = 0;
			}
		}


		/*if(query.m_NumPairs)
		{
			Scheduler::AddTask(MeshCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
			query.m_NumPairs = 0;
		}*/


		//if(pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris))
		//{
		//	for(U32 a = 0; a < obj.m_NumNodesContained; ++a)
		//	{
		//		C_DistributionNode* pNode = obj.m_Nodes[a].m_pNode;
		//		//Loop through all sleeping and static objects...
		//		GenerateCollisionTasks(pNode->m_pSleepingObjects, obj, query, pNode, dynamicCollisionCount);
		//		GenerateCollisionTasks(pNode->m_pStaticObjects, obj, query, pNode, dynamicCollisionCount);

		//		//Go up the tree looking for statics!
		//		C_DistributionNode* pParent = pNode->m_pParentNode;
		//		while(pParent)
		//		{
		//			//GenerateCollisionTasks(pNode->m_pSleepingObjects, obj, query, pNode->m_pParentNode);
		//			GenerateCollisionTasks(pParent->m_pStaticObjects, obj, query, pNode, dynamicCollisionCount);
		//			GenerateCollisionTasks(pParent->m_pSleepingObjects, obj, query, pNode, dynamicCollisionCount);
		//			pParent = pParent->m_pParentNode;
		//		}

		//		if(pNode->m_pLeftNode)
		//		{
		//			S32 size = 2;
		//			m_Stack.Push(pNode->m_pRightNode, 0);
		//			Gravity::C_DistributionNode* pStackTop = pNode->m_pRightNode;
		//			while(size--)
		//			{
		//				C_DistributionNode* pChild = pStackTop;
		//				pStackTop = SharedPhysics::Fast_Select<C_DistributionNode>(pChild->m_pLeftNode != NULL, pChild->m_pLeftNode,
		//					m_Stack.Pop<C_DistributionNode>(Maths::Max(0, size - 1)));

		//				GenerateCollisionTasks(pChild->m_pStaticObjects, obj, query, pChild, dynamicCollisionCount);
		//				GenerateCollisionTasks(pChild->m_pSleepingObjects, obj, query, pChild, dynamicCollisionCount);
		//				if(pChild->m_pLeftNode)
		//				{
		//					m_Stack.Push(pChild->m_pRightNode, size);
		//					size+=2;
		//				}
		//			}
		//		}

		//	}
		//}
		//else
		//{
		//	for(U32 a = 0; a < obj.m_NumNodesContained; ++a)
		//	{
		//		C_DistributionNode* pNode = obj.m_Nodes[a].m_pNode;
		//		//Loop through all sleeping and static objects...
		//		GenerateCollisionTasks(pNode->m_pSleepingObjects, obj, query, pNode, dynamicCollisionCount);
		//		GenerateCollisionTasks(pNode->m_pStaticObjects, obj, query, pNode, dynamicCollisionCount);
		//		GenerateCollisionTasks(pNode->m_pSleepingDebrisObjects, obj, query, pNode, dynamicCollisionCount);

		//		//Go up the tree looking for statics!
		//		C_DistributionNode* pParent = pNode->m_pParentNode;
		//		while(pParent)
		//		{
		//			//GenerateCollisionTasks(pNode->m_pSleepingObjects, obj, query, pNode->m_pParentNode);
		//			GenerateCollisionTasks(pParent->m_pStaticObjects, obj, query, pNode, dynamicCollisionCount);
		//			GenerateCollisionTasks(pParent->m_pSleepingObjects, obj, query, pNode, dynamicCollisionCount);
		//			GenerateCollisionTasks(pParent->m_pSleepingDebrisObjects, obj, query, pNode, dynamicCollisionCount);
		//			pParent = pParent->m_pParentNode;
		//		}

		//		if(pNode->m_pLeftNode)
		//		{
		//			S32 size = 2;
		//			m_Stack.Push(pNode->m_pRightNode, 0);
		//			Gravity::C_DistributionNode* pStackTop = pNode->m_pRightNode;
		//			while(size--)
		//			{
		//				C_DistributionNode* pChild = pStackTop;
		//				pStackTop = SharedPhysics::Fast_Select<C_DistributionNode>(pChild->m_pLeftNode != NULL, pChild->m_pLeftNode,
		//					m_Stack.Pop<C_DistributionNode>(Maths::Max(0, size - 1)));

		//				GenerateCollisionTasks(pChild->m_pStaticObjects, obj, query, pChild, dynamicCollisionCount);
		//				GenerateCollisionTasks(pChild->m_pSleepingObjects, obj, query, pChild, dynamicCollisionCount);
		//				GenerateCollisionTasks(pChild->m_pSleepingDebrisObjects, obj, query, pChild, dynamicCollisionCount);
		//				if(pChild->m_pLeftNode)
		//				{
		//					m_Stack.Push(pChild->m_pRightNode, size);
		//					size+=2;
		//				}
		//			}
		//		}

		//	}
		//}

		//if(query.m_NumPairs)
		//{
		//	Scheduler::AddTask(DynamicCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
		//	query.m_NumPairs = 0;
		//}

		/*{
			if(m_pGroundPlane && pBody->m_pShape->CanCollide(m_pGroundPlane->m_pShape))
			{
				query.m_pColliders[query.m_NumPairs++] = m_pCollisionManager->CreateCollider(&m_pGroundPlane->m_GraphNode, obj.m_pGraphNode);
				if(query.m_NumPairs == meshCollisionCount)
				{
					Scheduler::AddTask(GroundPlaneCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
					query.m_NumPairs = 0;
				}
			}
			if(query.m_NumPairs)
			{
				Scheduler::AddTask(GroundPlaneCollisionModule, NULL, NULL, m_Group, m_Policy, &query, sizeof(C_CollisionQuery));
				query.m_NumPairs = 0;
			}
		}*/

		//Scheduler::Flush(m_Group);
	}

	void C_Scene::PutBodiesToSleep()
	{
		using namespace SharedPhysics;
		PERF_EVENT("gravity.simulate.PutBodiesToSleep");


		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());

		C_Body* pBody = m_ActiveBodyList.m_pFirstBody;		
		while(pBody)
		{
			C_Body* pNextBody = pBody->m_pNext;
			
			pBody->m_GraphNode.ResistDeactivationConstrainedObjects(m_Stack);
			if((pBody->m_GraphNode.TestNodeFlags( C_GraphNode::E_Type_NoddingOff ) || (pBody->m_GraphNode.m_Deactivator.CanDeactivate())))
			{
				//Put the body to sleep!
				BIZ_ASSERT(pBody->m_GraphNode.TestNodeFlags(C_GraphNode::E_Type_Active));
				BIZ_ASSERT(!pBody->m_GraphNode.TestNodeFlags(C_GraphNode::E_Type_Sleeping));
				pBody->m_GraphNode.ClearNodeFlags( C_GraphNode::E_Type_Active | C_GraphNode::E_Type_NoddingOff );
				pBody->m_GraphNode.SetNodeFlags( C_GraphNode::E_Type_Sleeping);
				if(!pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
					pBody->m_Object.PutToSleep();
				pBody->m_ExternalState.m_ExternalBodyState = pBody->m_GraphNode.m_NodeFlags & (SharedPhysics::C_GraphNode::E_Type_BodyStateMask);
				m_pGraphManager->PutToSleep( &pBody->m_GraphNode, *m_pCollisionManager->m_pContactDatabase );
				//if(!pBody->m_GraphNode.TestNodeFlags(C_GraphNode::E_Type_LightWakeUp))
				{
					C_BodyEvent* pEvent = m_EventStream.CreateEvent();
					pEvent->SetBody(pBody);
					pEvent->SetEventType(C_BodyEvent::E_GoToSleep);
				}
				pBody->m_GraphNode.m_TotalKineticEnergy = zero;
				pBody->m_GraphNode.m_IslandIndex = 0;
				pBody->m_RigidBody.m_KineticEnergy = zero;
				pBody->m_RigidBody.m_LinearVelocity = pBody->m_RigidBody.m_LinearMomentum = pBody->m_RigidBody.m_AngularVelocity = pBody->m_RigidBody.m_AngularMomentum = zero;
				pBody->m_RigidBody.CopyVelocityToBVector();
				m_ActiveBodyList.RemoveBody(pBody);
				//For some reason, PS3 seems to be trashing this linked list
				//BIZ_ASSERT(ValidActiveList());
			}
			pBody->m_GraphNode.ClearNodeFlags( C_GraphNode::E_Type_LightWakeUp | C_GraphNode::E_Type_Waking );
			pBody = pNextBody;
		}
	}

	SharedPhysics::C_Collider* C_Scene::CreateCollider(SharedPhysics::C_GraphNode* pNodeA,
		SharedPhysics::C_GraphNode* pNodeB)
	{
		return m_pCollisionManager->CreateColliderMesh(pNodeA, pNodeB);
	}

	SharedPhysics::C_MaterialPair* C_Scene::GetMaterialMatrix(U32& materialCount)
	{
		materialCount = m_pGraphManager->m_NumMaterials;
		return m_pGraphManager->m_pMaterialMatrix;
	}


	C_Body* C_Scene::GetBody(SharedPhysics::C_GraphNode* pNode) const
	{
		return pNode->m_pBody;
	}

	void C_Scene::RegisterCollisionCallback(C_CollisionCallback* pCallback)
	{ 
		m_pCollisionCallback = pCallback;
	}

	void C_Scene::RegisterEventCallback(C_EventCallback* pCallback)
	{
		m_pEventCallback = pCallback;
	}

	const U32 C_Scene::NumBodies() const
	{
		return m_Bodies.m_numElements;
	}

	C_Body* C_Scene::GetBody(const U32 index) const
	{
		return m_Bodies[index].m_pElement;
	}

	const U32 C_Scene::NumStaticBodies() const
	{
		return m_StaticBodies.m_numElements;
	}

	C_Body* C_Scene::GetStaticBody(const U32 index) const
	{
		return m_StaticBodies[index].m_pElement;
	}


	const U32 C_Scene::NumMeshBodies() const
	{
		return m_MeshCollisionBodies.m_numElements;
	}

	C_Body* C_Scene::GetMeshBody(const U32 index) const
	{
		return m_MeshCollisionBodies[index].m_pElement;
	}

	C_Body* C_Scene::GetStaticWorldBody() const
	{
		return m_pWorldBody;
	}

	SharedPhysics::C_MemoryIterator<C_BodyEvent> C_Scene::GetEventIterator()
	{
		return m_EventStream.GetIterator();
	}

	const U32 C_Scene::GetNumEvents() const
	{
		return m_EventStream.NumEvents();
	}

	
	Collision::C_AxisBoxMinMax C_Scene::GetWorldBounds()
	{
		/*Collision::C_AxisBoxMinMax box;
		box.SetInvalidExtents();
		if(m_pTree)
		{
			box = m_pTree->GetWorldBox();
		}
		return box;*/
		return m_WorldBounds;
	}

	SharedPhysics::C_Spring* C_Scene::CreateSpring()
	{
		return m_pSpringManager->GetNextFreeItem();
	}

	void C_Scene::GetLeftOverBodies(Btl::Vector<C_Body*>& bodies)
	{
		for(S32 a = 0; a < m_FluidMeshBodies.m_numElements; ++a)
		{
			bodies.PushBack(m_FluidMeshBodies[a].m_pElement);
		}

		for(S32 a = 0; a < m_FluidBodies.m_numElements; ++a)
		{
			bodies.PushBack(m_FluidBodies[a].m_pElement);
		}

		for(S32 a = 0; a < m_ClothBodies.m_numElements; ++a)
		{
			bodies.PushBack(m_ClothBodies[a].m_pElement);
		}

		for(S32 a = 0; a < m_DeformableBodies.m_numElements; ++a)
		{
			bodies.PushBack(m_DeformableBodies[a].m_pElement);
		}

		for(S32 a = 0; a < m_RopeBodies.m_numElements; ++a)
		{
			bodies.PushBack(m_RopeBodies[a].m_pElement);
		}

		for(S32 a = 0; a < m_Bodies.m_numElements; ++a)
		{
			bodies.PushBack(m_Bodies[a].m_pElement);
		}

		for(S32 a = 0; a < m_StaticBodies.m_numElements; ++a)
		{
			bodies.PushBack(m_StaticBodies[a].m_pElement);
		}

		for(S32 a = 0; a < m_MeshCollisionBodies.m_numElements; ++a)
		{
			bodies.PushBack(m_MeshCollisionBodies[a].m_pElement);
		}
	}

	SharedPhysics::C_Integrator& C_Scene::GetIntegrator()
	{
		return m_Integrator;
	}



#endif

}

