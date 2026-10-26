#ifndef NEWSCENE_H
#define NEWSCENE_H

#include "gravity/narrowphase/ResultSize.h"

#include "gravity/narrowphase/AxisBox.h"

#include "gravity/sharedphysics/AllocationManager.h"
#include "gravity/sdk/Body.h"
#include "gravity/SharedPhysics/SortedArray.h"
#include "gravity/SharedPhysics/Spring.h"
#include "gravity/sdk/RagdollController.h"
#include "gravity/sdk/FluidEvents.h"



#include "gravity/sharedphysics/SafeStack.h"
//#include "gravity/sdk/CollisionCallback.h"
#include "gravity/sharedphysics/Integrator.h"
#include "gravity/sharedphysics/Material.h"

#include <core/threads/spurwlock.h>
#include <system/scheduler/jobqueue.h>
#include "gravity/sharedphysics/spu_memory.h"
#include "gravity/sdk/BodyEvents.h"
#include "gravity/triggers/TriggerSystem.h"
#include <System/Debug/I_DebugRender.h>
#include "Btl/Containers/Vector.h"

namespace Solver2
{
	class C_ConstraintGraphManager;
	class C_MultiBody;
}

namespace Collision
{
	class C_LineCollisionCache;
	class C_CollisionManager;
	class C_Ray;
	class C_TransparentRay;
	class C_CollisionFilter;
	class C_Shape;
	class C_CompositeShape;
}

namespace SharedPhysics
{
	class C_Collider;
	class C_ContactStream;
	class C_ConstContactStreamPtr;
	//class C_MaterialPair;
}

namespace Gravity
{

class C_Object;
class C_Scene;
class C_Body;
class C_CharacterController;
class C_KinematicController;
class C_AnimatedShapeController;
class C_DistributionTree;
class C_ReferenceFrame;
class C_CollisionCallback;
class C_EventCallback;
class C_CollisionPhantom;
class C_WaterQuery;
class C_CollisionCallback;
class C_BodyRemoveCallback;

void DrawCompositeShape(DbUtil::I_DebugRender* pRender, const Collision::C_CompositeShape* pCompShape,
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter);

const U32 TaskBufferSize = 8;

const U32 TaskSize = 32;

template <U32 Size>
class C_TaskBuffer
{
public:
	void* m_pElements[Size];
};


struct ALIGN16 C_DeferredRayCommand
{
	Collision::C_Ray* m_pRay;
	C_Scene* m_pScene;
	Collision::C_LineCollisionCache* m_pCache;
	bool m_TraceStaticMeshOnly;
};

struct ALIGN16 C_DeferredTransparentRayCommand
{
	Collision::C_TransparentRay* m_pRay;
	C_Scene* m_pScene;
	bool m_TraceStaticMeshOnly;
};

struct ALIGN16 C_DeferredWaterCommand
{
	Gravity::C_WaterQuery* m_pQuery;
	C_Scene* m_pScene;
};

struct ALIGN16 C_DeferredCCDCommand
{
	C_DeferredCCDCommand* ppuAddress;
	Gravity::C_Body* m_pBody;
	C_Scene* m_pScene;
	SharedPhysics::C_GraphNode* m_pMeshNode;
	SharedPhysics::C_ContactStream* m_pStream;
	U8* m_pCurrentReadPoint;
};

typedef C_DeferredCCDCommand* C_DeferredCCDCommandPtr;

const U32 MaxRagdollControllers = 2;

struct ALIGN16 C_RagdollControllerCommand
{
	F32 m_Time;
	C_RagdollController* m_pController[MaxRagdollControllers];
	U32 m_NumControllers;

	C_RagdollControllerCommand() : m_NumControllers(0)
	{
	}
};

struct C_MeshDispatchTask
{
	Gravity::C_Scene* m_pScene;
};

//48-bytes - exactly the size of params allowed by the task scheduler!
const U32 MaxCollisionQuery = 9;
struct ALIGN16 C_CollisionQuery
{
	SharedPhysics::C_Collider* m_pColliders[MaxCollisionQuery];
	U32 m_NumPairs;
	SharedPhysics::C_ContactStream* m_pStream;
	Collision::C_CollisionManager* m_pManager;

	C_CollisionQuery() : m_pStream(NULL), m_pManager(NULL), m_NumPairs(0)
	{
	}

};

const U32 MaxPhantomCollisionQuery = 4;

struct ALIGN16 C_PhantomCollisionQuery
{
	Gravity::C_Object* m_pObjectA[MaxPhantomCollisionQuery];
	Gravity::C_Object* m_pObjectB[MaxPhantomCollisionQuery];
	SharedPhysics::C_ContactStream* m_pStream;
	Collision::C_CollisionManager* m_pManager;
	U32 m_NumPairs;

	C_PhantomCollisionQuery() : m_pStream(NULL), m_pManager(NULL), m_NumPairs(0)
	{
	}
};


const U32 MaxFluidQuery = 4;
struct ALIGN16 C_FluidQuery
{
	Gravity::C_Object* m_pObject[MaxFluidQuery];
	Gravity::C_Object* m_pFluidObject[MaxFluidQuery];
	Collision::C_CollisionManager* m_pManager;
	C_FluidEventBuffer* m_pFluidBuffer;
	U32 m_Size;
	
	C_FluidQuery() : m_pManager(NULL), m_Size(0)
	{
	}
};

struct NullBody
{
	SharedPhysics::C_GraphNode m_NullBody;
	SharedPhysics::C_RigidBody m_NullRigid;

	NullBody()
	{
		m_NullBody.ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active);
		m_NullBody.SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static);
		m_NullBody.m_pRigidBody = &m_NullRigid;
		m_NullBody.m_IslandIndex = 0;
	}
};

struct ALIGN16 C_SolverIsland
{
	SharedPhysics::C_GraphNode* m_pRootNode;
	SharedPhysics::C_StaticSPUMemoryAllocator* m_pMemoryAllocator;
	F32 m_TimeStep;
	U32 m_NumIterations;
	U32 m_JointIterationFrequency;
	//NullBody* m_pNullBody;
	C_Body* m_pWorldBody;

	C_SolverIsland() : m_pRootNode(NULL), m_pMemoryAllocator(NULL), m_TimeStep(0.f), m_NumIterations(0),
		m_JointIterationFrequency(0), m_pWorldBody(NULL)
	{
	}
};

const U32 MaxPostIntegrateSize = 32;
struct C_PostIntegrateTask
{
	Gravity::C_Body** m_pBodies;
	U32 m_NumBodies;
};

const U32 MaxAnimatedShapeUpdateSize = 32;
struct C_AnimatedShapeUpdateTask
{
	Gravity::C_AnimatedShapeController** m_pControllers;
	U32 m_NumControllers;
};

const U32 MaxKinematicUpdateSize = 32;
struct C_KinematicUpdateTask
{
	Gravity::C_KinematicController** m_pControllers;
	F32 m_Time;
	U32 m_NumControllers;
};

const U32 MaxIntegrationSize = 32;
struct C_IntegrateTask
{
	Gravity::C_Body** m_pBodies;
	F32 m_Time;
	U32 m_NumBodies;
};

const U32 ClothCollisionSize = 4;
struct ALIGN16 C_ClothCollisionQuery
{
	SharedPhysics::C_ContactStream* m_pStream;
	Collision::C_CollisionManager* m_pManager;
	Gravity::C_Object* m_pDynamicObject[ClothCollisionSize];
	C_Object* m_pClothObject[ClothCollisionSize];
	U32 m_Size;

	C_ClothCollisionQuery() : m_pStream(NULL), m_pManager(NULL), m_Size(0)
	{
	}
};

//Structure for putting broad phase on the SPU!
const U32 BroadPhaseTaskSize = 6;
struct BroadPhaseTask
{
	//Gravity::C_DistributionNode* m_Nodes[BroadPhaseTaskSize];
	Gravity::C_Body* m_pBodies[BroadPhaseTaskSize];
	SharedPhysics::C_ContactStream* m_pStream;
	SharedPhysics::C_ContactStream* m_pClothStream;
	Collision::C_CollisionManager* m_pManager;
	C_FluidEventBuffer* m_pFluidBuffer;
	Gravity::C_Scene* m_pScene;
	U32 m_TaskSize;
};



//void AddFillCacheTask(const S32 groupId, const S32 policyId, C_Scene* pScene, Collision::C_LineCollisionCache* pCache, Collision::C_Ray* pRay);
//void TestFillCache(Scheduler::Task* pTask);
void DeferredRayTrace(Scheduler::Task* pTask);
void DeferredTransparentRayTrace(Scheduler::Task* pTask);
void CollideMesh(Scheduler::Task* pTask);
void DispatchMeshCollision(Scheduler::Task* pTask);
void CollidePhantomMesh(Scheduler::Task* pTask);
void CollideGroundPlane(Scheduler::Task* pTask);
void CollideDynamic(Scheduler::Task* pTask);
void CollidePhantomDynamic(Scheduler::Task* pTask);
void Solve(Scheduler::Task* pTask);
void UpdateRagdollController(Scheduler::Task* pTask);
void Integrate(Scheduler::Task* pTask);
void DeferredWaterQuery(Scheduler::Task* pTask);
void DeferredContinuousCollisionDetection(Scheduler::Task* pTask);
void FluidCollision(Scheduler::Task* pTask);
void PostIntegrate(Scheduler::Task* pTask);
void AnimatedShapeControllerUpdate(Scheduler::Task* pTask);
void KinematicControllerUpdate(Scheduler::Task* pTask);
void IntegrateBodies(Scheduler::Task* pTask);
void DoBroadPhase(Scheduler::Task* pTask);
void DoWakeUpBroadPhase(Scheduler::Task* pTask);
void DoClothCollision(Scheduler::Task* pTask);

class C_Scene : public SharedPhysics::C_BlockItem<C_Scene>
{
	friend class C_Body;
	friend class C_PhysicsSDK;
	friend class C_KinematicController;
	friend class C_AnimatedShapeController;
	friend class C_RagdollController;
	friend class C_ImportSceneStaticInstance;
public:

	typedef SharedPhysics::C_PointerWrapper<C_Body> BodyPtr;
	typedef SharedPhysics::C_PointerWrapper<SharedPhysics::C_Constraint> ConstraintPtr;
	typedef SharedPhysics::C_PointerWrapper<C_CharacterController> ControllerPtr;
	typedef SharedPhysics::C_PointerWrapper<C_KinematicController> KinematicControllerPtr;
	typedef SharedPhysics::C_PointerWrapper<C_AnimatedShapeController> AnimatedShapeControllerPtr;
	typedef SharedPhysics::C_PointerWrapper<C_RagdollController> RagdollControllerPtr;
	typedef SharedPhysics::C_PointerWrapper<SharedPhysics::C_Spring> SpringPtr;
	typedef SharedPhysics::C_PointerWrapper<C_ReferenceFrame > ReferenceFramePtr;

	//SharedPhysics::SortedArray<SharedPhysics::C_Constraint*> m_ConstraintsArray;

	Collision::C_AxisBoxMinMax m_WorldBounds;
	SharedPhysics::C_SafeStack<true> m_Stack;

	Solver2::C_ConstraintGraphManager *m_pGraphManager;
	SharedPhysics::C_MaterialDebug m_MaterialDebug;
	//Solver2::C_MultiBody* m_pMultiBody;

private:
	SharedPhysics::C_AllocationManager<C_Body, 16, true>* m_pBodyManager;
	SharedPhysics::C_AllocationManager<SharedPhysics::C_Spring, 4, true>* m_pSpringManager;
	//SharedPhysics::C_AllocationManager<C_DeferredBodyCommand, 256, false> m_CommandManager;
	SharedPhysics::MemoryAllocator<C_DeferredBodyCommand, 32, false>* m_pCommandManager;
	SharedPhysics::MemoryAllocator<C_DeferredBodyCommand, 32, false>* m_pOtherCommandManager;
	SharedPhysics::MemoryAllocator<C_DeferredBodyCommand, 32, false>* m_pCommandManager1;
	SharedPhysics::MemoryAllocator<C_DeferredBodyCommand, 32, false>* m_pCommandManager2;
	SharedPhysics::MemoryAllocator<C_DeferredCCDCommand, 32, false>* m_pContinuousCommandList;
	SharedPhysics::MemoryAllocator<C_TaskBuffer<TaskSize>, TaskBufferSize, false>* m_pTaskBuffer;

	
	SharedPhysics::SortedArray<BodyPtr> m_FluidMeshBodies;
	SharedPhysics::SortedArray<BodyPtr> m_FluidBodies;
	SharedPhysics::SortedArray<BodyPtr> m_ClothBodies;
	SharedPhysics::SortedArray<BodyPtr> m_DeformableBodies;
	SharedPhysics::SortedArray<BodyPtr> m_RopeBodies;
	SharedPhysics::SortedArray<BodyPtr> m_Bodies;
	SharedPhysics::SortedArray<BodyPtr> m_StaticBodies;
	SharedPhysics::SortedArray<BodyPtr> m_MeshCollisionBodies;
	SharedPhysics::SortedArray<ControllerPtr> m_CharacterControllers;
	SharedPhysics::SortedArray<KinematicControllerPtr> m_KinematicControllers;
	SharedPhysics::SortedArray<AnimatedShapeControllerPtr> m_AnimatedShapeControllers;
	SharedPhysics::SortedArray<RagdollControllerPtr> m_RagdollControllers;
	SharedPhysics::SortedArray<ReferenceFramePtr> m_ReferenceFrames;

	C_KinematicController* m_pActiveControllers;
	C_AnimatedShapeController* m_pActiveAnimatedShapeControllers;
	C_RagdollController* m_pActiveRagdollControllers;

	Gravity::C_DistributionTree* m_pTree;
	C_FluidEventBuffer m_FluidEventBuffer;

	//SharedPhysics::SortedArray<ConstraintPtr> m_Constraints;
	SharedPhysics::SortedArray<SpringPtr> m_Springs;
	Collision::C_CollisionManager* m_pCollisionManager;
	SharedPhysics::C_StaticSPUMemoryAllocator m_SPUAllocator;
	//SharedPhysics::MemoryAllocator<C_CollisionQuery, 256, false>* m_pCollisionQueries;
	//SharedPhysics::MemoryAllocator<Gravity::C_ObjectPair, 128>* m_pContactPairs;
	SharedPhysics::MemoryAllocator<Gravity::C_ObjectPair, 128>* m_pClothPairs;
	//SharedPhysics::MemoryAllocator<Gravity::C_ObjectPair, 128>* m_pFluidPairs;

	C_TriggerSystem m_TriggerSystem;
	
	//SharedPhysics::MemoryAllocator<C_SolverIsland, 32>* m_pIslands;

	//C_DeferredBodyCommand* m_pFirstCommand;

	//C_DeferredClothBodyCommand* m_pClothCommand;
	
	C_CollisionCallback* m_pCollisionCallback;
	C_EventCallback* m_pEventCallback;
	SharedPhysics::C_Integrator m_Integrator;

	Scheduler::PolicyHandle m_Policy;
	Scheduler::GroupHandle m_Group;
	Scheduler::GroupHandle m_BroadPhaseGroup;
	//Scheduler::GroupHandle m_AuxiliaryGroup;
	//NullBody* m_pNullBody;
	C_Body* m_pWorldBody;
	C_EventStream m_EventStream;
	Maths::HF32 m_MinTimeStep;
	Maths::HF32 m_RemainingTime;

	//SharedPhysics::SortedArray<BodyPtr> m_ContinuousActiveBodyList;
	
	C_BodyList m_ContinuousBodyList; //Active object but need to do continuous collision detection, like grenades
	C_BodyList m_ActiveBodyList; //All Active objects which don't need to do continuous collision detection
	C_BodyList m_KinematicBodyList; //All kinematic objects
	C_BodyList m_WakingBodyList;//All waking up objects
	C_BodyList m_TouchedBodyList; // All sleeping or static objects which have been touched by an active or kinematic body
	SharedPhysics::SortedArray<BodyPtr> m_OverriddenBodyList;
	Gravity::C_Body* m_pGroundPlane;
	U32 m_NumActiveBodies;

	MT::SpuRwLock m_ReadWriteLock;
	MT::Lock m_BodyLock;
	//MT::Lock m_PhysicsLock;
	SharedPhysics::C_SynchronisedMemoryIterator<C_DeferredBodyCommand, 32, false> m_CommandIter;

	U32 m_NumIterations, m_JointIterationFrequency;
	U32 m_BodyIndex;
	void* m_pStaticUserData;
	

#ifndef PLATFORM_SPU
	C_Scene(SharedPhysics::C_MaterialPair* materialMatrix, const U32 numMaterials);
	
	~C_Scene();
#endif
public:

	void SetStaticUserData(void* pUserData);
	void* GetStaticUserData();
	void ResetBodyIndex();
	void SetNumSolverIterations(const U32 numSolverIterations);
	void SetJointIterationFrequency(const U32 jointIterationFrequency);
	U32 GetNumActiveBodies() const;
	void SendDeactivateMessage(C_Body* pBody);
	void SendActivateMessage(C_Body* pBody);

	SharedPhysics::C_MaterialPair* GetMaterialMatrix(U32& materialCount);

	Collision::C_AxisBoxMinMax GetWorldBounds();

	void GetCollisionPhantomIntersection(Gravity::C_CollisionPhantom* pPhantom, 
		SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack,
		Scheduler::PolicyHandle policy,	Scheduler::GroupHandle group);

	void GetCollisionPhantomIntersectionMesh(Gravity::C_CollisionPhantom* pPhantom, 
		SharedPhysics::C_ContactStream& stream, Scheduler::PolicyHandle policy,	Scheduler::GroupHandle group);

	void GetCollisionPhantomIntersectionMeshImmediate(Gravity::C_CollisionPhantom* pPhantom, 
		SharedPhysics::C_ContactStream& stream);

		
	void CompletePhantomCollisionQuery(SharedPhysics::C_ContactStream& stream, Scheduler::PolicyHandle policy, Scheduler::GroupHandle group);

	void SetMinTimeStep(const Maths::HF32 PCREF minTimeStep);

	void SetInternalTaskPolicy(Scheduler::PolicyHandle handle);

	void ResizeScene(Collision::C_AxisBoxMinMax PCREF worldBox, const Maths::HVec3 PCREF desiredSize, const Maths::HVec3 PCREF maxCellSzie, S32 maxDepth);

	SharedPhysics::C_ConstraintEdge* GetConstraintEdge();
	
	SharedPhysics::C_ConstraintTwin* GetConstraintTwin();

	SharedPhysics::C_ConstraintPoint* GetConstraintPoint();
	
	Gravity::C_DistributionTree* GetBroadPhase();

	SharedPhysics::C_Spring* CreateSpring();

	C_DeferredBodyCommand* GetCommand();

	SharedPhysics::C_Integrator& GetIntegrator();

	void SetGroundPlane(Gravity::C_Body* pGroundPlane);
	Gravity::C_Body* GetGroundPlane();

	void AddCommand(C_DeferredBodyCommand* pCommand);

	void CreateColliders();

	void ProcessCommands();
	
	C_Body* CreateBody();
	
	void AddBody(C_Body* pBody);

	void AddStaticBody(C_Body* pBody);

	void RemoveBody(C_Body* pBody, C_BodyRemoveCallback* pCallback = NULL);

	void ReleaseImportScene(C_ImportScene* pImportScene);

	void ReleaseBody(C_Body* pBody);

	void TemporarilyRemoveBody(C_Body* pBody);

	void RemoveStaticBody(C_Body* pBody);


	void AddClothBody(C_Body* pBody);

	void RemoveClothBody(C_Body* pBody);

	void AddDeformableBody(C_Body* pBody);

	void RemoveDeformableBody(C_Body* pBody);

	void AddRopeBody(C_Body* pBody);

	void RemoveRopeBody(C_Body* pBody);

	void AddFluidBody(C_Body* pBody);

	void RemoveFluidBody(C_Body* pBody);

	void AddMeshBody(C_Body* pBody);
	
	void RemoveMeshBody(C_Body* pBody);

	void AddFluidMeshBody(C_Body* pBody);

	void RemoveFluidMeshBody(C_Body* pBody);

	/*void AddContinuousBody(C_Body* pBody);

	void RemoveContinuousBody(C_Body* pBody);*/
	

	//void AddConstraint(SharedPhysics::C_Constraint* pConstraint);
	void AddDebugConstraint(SharedPhysics::C_Constraint* pConstraint);
	void RemoveConstraint(SharedPhysics::C_Constraint* pConstraint);
	const U32 NumConstraints() const;
	void ClearConstraints();
	SharedPhysics::C_Constraint* GetConstraint(const U32 index);


	void AddSpring(SharedPhysics::C_Spring* pSpring);
	void RemoveSpring(SharedPhysics::C_Spring* pSpring);
	const U32 NumSprings() const;
	void ClearSprings();
	SharedPhysics::C_Spring* GetSpring(const U32 index);

	

	void AddReferenceFrame(C_ReferenceFrame* pReferenceFrame);
	void RemoveReferenceFrame(C_ReferenceFrame* pReferenceFrame);
	const U32 NumReferenceFrames() const;
	void ClearReferenceFrame();
	C_ReferenceFrame* GetReferenceFrame(const U32 index);

	
	//Step the simulation forwards...

	void Simulate(const Maths::C_TimeS PCREF time);

	void WaitForDeferredContacts( const Maths::HF32 PCREF time );

	void GetContacts( const Maths::HF32 PCREF time );

	void GetMeshContacts();

	void DoContinuousCollisionDetection();

	bool DoContinuousCollisionDetection(C_Body* pBody, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_Collider* pCollider, SharedPhysics::C_GraphNode** pPpuGraphNode,
		SharedPhysics::C_ContactStream* pStream);

	void UpdateRagdollControllers(const Maths::HF32 PCREF time);

	//void GenerateSleepyContacts( U32 numPairs, C_Body* pNewBody );

	void WakeUpBodies( SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_ConstContactStreamPtr& clothStreamEnd, const Maths::HF32 PCREF time );

	void PutBodiesToSleep();

	C_Body* GetBody(SharedPhysics::C_GraphNode* pNode) const;

	void RegisterCollisionCallback(C_CollisionCallback* pCallback);

	void RegisterEventCallback(C_EventCallback* pCallback);

	const U32 NumBodies() const;

	C_Body* GetBody(const U32 index) const;

	const U32 NumStaticBodies() const;

	C_Body* GetStaticBody(const U32 index) const;

	const U32 NumMeshBodies() const;

	C_Body* GetMeshBody(const U32 index) const;

	C_Body* GetStaticWorldBody() const;
	
	void DoDeferredRayTrace(Collision::C_Ray* pRay, Collision::C_LineCollisionCache* pCache, Scheduler::GroupHandle group,
		Scheduler::PolicyHandle policy, bool traceStaticMeshOnly = false);

	void DoDeferredTransparentRayTrace(Collision::C_TransparentRay* pRay, Scheduler::GroupHandle group, 
		Scheduler::PolicyHandle policy, bool traceStaticMeshOnly = false);

	Scheduler::GroupHandle GetInternalGroupHandle();

	Scheduler::PolicyHandle GetInternalTaskPolicy();

	void DoDeferredWaterQuery(C_WaterQuery* pQuery, Scheduler::GroupHandle group,Scheduler::PolicyHandle policy);

	void DoRayTrace(Collision::C_Ray& ray, Collision::C_LineCollisionCache& cache, SharedPhysics::C_SafeStack<true>& m_Stack,
		bool bAcquireLock = true);
		
	void DoRayTraceMesh(Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& m_Stack,
		bool bAcquireLock = true);

	void DoRayTrace(Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& m_Stack, bool bAcquireLock = true);

	//void DoRayTrace(Collision::C_TransparentRay& ray, SharedPhysics::C_SafeStack<true>& m_Stack, bool bAcquireLock = true);

	void DoTransparentRayTrace(Collision::C_TransparentRay& ray, SharedPhysics::C_SafeStack<true>& m_Stack, bool bAcquireLock = true);

	void DoTransparentRayTraceMesh(Collision::C_TransparentRay& ray, SharedPhysics::C_SafeStack<true>& stack, bool bAcquireLock);
	
	void DoWaterQuery(C_WaterQuery* pQuery, SharedPhysics::C_SafeStack<true>& stack, bool bAcquireLock = true);

	void AddCharacterController(C_CharacterController* pController);
	void RemoveCharacterController(C_CharacterController* pController);

	void AddKinematicController(C_KinematicController* pController);
	void RemoveKinematicController(C_KinematicController* pController);

	void AddAnimatedShapeController(C_AnimatedShapeController* pController);
	void RemoveAnimatedShapeController(C_AnimatedShapeController* pController);

	void AddRagdollController(C_RagdollController* pController);
	void RemoveRagdollController(C_RagdollController* pController);


	U32 GetIntersectingBodies(const Collision::C_Sphere& sphere, Collision::C_CollisionFilter* pFilter,
		SharedPhysics::MemoryAllocator<C_Body*>& allocator, SharedPhysics::C_SafeStack<true>& stack);
	
	//Debug render function - passed an I_DebugRender to draw with and a C_CRAxisBox to clip
	//objects with (i.e. a visibility region)
	void DebugRenderScene(DbUtil::I_DebugRender* pRender, Collision::C_CRAxisBox& clipBox,
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter = NULL);

	void DrawTriggerLayer(DbUtil::I_DebugRender* pRender, C_TriggerLayer* pLayer, Collision::C_CRAxisBox& box, SharedPhysics::C_SafeStack<true>& stack, 
		Collision::C_CollisionFilter* pFilter);

	void FillCache(Collision::C_CRAxisBox& box, Collision::C_LineCollisionCache& pCache, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
		const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF maxCosAngle);

	SharedPhysics::C_MemoryIterator<C_BodyEvent> GetEventIterator();

	const U32 GetNumEvents() const;

	C_TriggerSystem* GetTriggerSystem();

	void GetLeftOverBodies(Btl::Vector<C_Body*>& bodies);

	void SetInternalGroupHandle(Scheduler::GroupHandle handle);

	void SetBroadPhaseGroupHandle(Scheduler::GroupHandle handle);

	SharedPhysics::C_Collider* CreateCollider(SharedPhysics::C_GraphNode* pNodeA, 
		SharedPhysics::C_GraphNode* pNodeB);


	private:
	void CopyClothCollisions(SharedPhysics::C_ConstContactStreamPtr& ptr);
	void UpdateControllers1(const Maths::HF32 PCREF dt);
	void UpdateControllers2(const Maths::HF32 PCREF dt);
	void UpdateShapes();
	void CreateFluidEnterExitEvents();
	void FinaliseRagdollControllers();
	void UpdateOverriddenBodies();
	void InsertBodyInWorld(C_Body* pBody);
	void RemoveBodyFromWorld(C_Body* pBody, C_BodyRemoveCallback* pCallback, void* pUserData);
	void RemoveBodyFromWorldNoDelete(C_Body* pBody);
	void InsertStaticBodyInWorld(C_Body* pBody);
	void RemoveStaticBodyFromWorld(C_Body* pBody);
	//void InsertContinuousBodyInWorld(C_Body* pBody);
	//void RemoveContinuousBodyFromWorld(C_Body* pBody);
	void InsertMeshBodyInWorld(C_Body* pBody);
	void RemoveMeshBodyFromWorld(C_Body* pBody);
	void InsertClothBodyInWorld(C_Body* pBody);
	void RemoveClothBodyFromWorld(C_Body* pBody);
	void InsertDeformableBodyInWorld(C_Body* pBody);
	void RemoveDeformableBodyFromWorld(C_Body* pBody);
	void InsertRopeBodyInWorld(C_Body* pBody);
	void RemoveRopeBodyFromWorld(C_Body* pBody);
	void InsertFluidBodyInWorld(C_Body* pBody);
	void RemoveFluidBodyFromWorld(C_Body* pBody);
	void InsertFluidMeshBodyInWorld(C_Body* pBody);
	void RemoveFluidMeshBodyFromWorld(C_Body* pBody);
	void GetContactsForWakingBody(C_Body* pBody, BroadPhaseTask& task, C_CollisionQuery& query, 
		const Maths::HF32 PCREF time);
	void GenerateCollisionTasks(C_ObjectWrapper* pList, C_Object& obj, C_CollisionQuery& query,
		C_DistributionNode* pNode, const U32& maxCount);
	void GenerateCollisionTasksKinematic(C_ObjectWrapper* pList, C_Object& obj, C_CollisionQuery& query,
		C_DistributionNode* pNode, const U32& maxCount);
	void GenerateCollisionTasksNoLock(C_ObjectWrapper* pList, C_Object& obj, C_CollisionQuery& query,
		C_DistributionNode* pNode, const U32& maxCount);
	void GenerateFluidTasks(C_ObjectWrapper* pList, C_Object& obj, C_FluidQuery& query,
		C_DistributionNode* pNode, const U32& maxCount);
	void GenerateCollisionTasksCloth(C_ObjectWrapper* pList, C_Object& obj, C_ClothCollisionQuery& query,
		C_DistributionNode* pNode);
	void GenerateCollisionTasksCloth2(C_ObjectWrapper* pList, C_Object& obj, C_ClothCollisionQuery& query,
		C_DistributionNode* pNode);
	void ApplyDeferredCommand(C_DeferredBodyCommand& command);
	void ResetSoftBodies();
	void UpdateSprings();
	void SimulateFluids(const Maths::HF32 PCREF dt);
	void Solve(const Maths::HF32 PCREF dt);
	void Integrate(const Maths::C_TimeS PCREF time);
	void UpdateSoftBodies(const Maths::HF32 PCREF dt, bool bFirstRun);
	void ClearConstraintList(Gravity::C_BodyList& list);
	void ClearConstraintListReset(Gravity::C_BodyList& list);

	void RenderObjectList(DbUtil::I_DebugRender* pRender, Collision::C_CRAxisBox& box,
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
		SharedPhysics::SortedArray<C_Scene::BodyPtr>& bodies);

	void RenderObjectLinkedList(DbUtil::I_DebugRender* pRender, Collision::C_CRAxisBox& box,
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
		Gravity::C_BodyList& list);

	void _Simulate(const Maths::C_TimeS PCREF time, bool& bFirstRun);

	U32 ActiveNodeComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, U32& clothCount, SharedPhysics::C_MemoryIterator<Gravity::C_ObjectPair, 128>& clothIter);
	U32 ActiveParentComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, U32& clothCount, SharedPhysics::C_MemoryIterator<Gravity::C_ObjectPair, 128>& clothIter);
	U32 ActiveChildComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, U32& clothCount, SharedPhysics::C_MemoryIterator<Gravity::C_ObjectPair, 128>& clothIter,
		SharedPhysics::C_SafeStack<true>& stack);
	U32 SleepingParentComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, U32& clothCount, SharedPhysics::C_MemoryIterator<Gravity::C_ObjectPair, 128>& clothIter);
	
	void WakeUpSleepingBodies(Collision::C_CRAxisBox& box, Collision::C_CollisionFilter* pFilter);


	//Equivalents of above that don't support cloth/rope.
	//Need to wait until we have cloth/rope parallel collision detection, then can add them into the 
	//below functions...
	public:
	void ActiveNodeComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount);
	void ActiveParentComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount);
	void ActiveChildComparison(C_DistributionNode* pNode, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, SharedPhysics::C_SafeStack<true>& stack);

	//Broad phase functions
	void ActiveObjectComparison(Gravity::C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, C_ClothCollisionQuery& clothQuery, SharedPhysics::C_SafeStack<true>& stack);
	void DebrisComparison(Gravity::C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, SharedPhysics::C_SafeStack<true>& stack);
	void KinematicObjectComparison(Gravity::C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, C_ClothCollisionQuery& clothQuery, SharedPhysics::C_SafeStack<true>& stack);
	void ClothComparison(Gravity::C_Object* pObj, C_ClothCollisionQuery& clothQuery, SharedPhysics::C_SafeStack<true>& stack);


	//Wake up broad phase functions
	void WakeUpComparison(Gravity::C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, C_ClothCollisionQuery& clothQuery, 
		SharedPhysics::C_SafeStack<true>& stack);
	void DebrisWakeUpComparison(Gravity::C_Object* pObj, C_CollisionQuery& query, const U32& dynamicCollisionCount,
		C_FluidQuery& fluidQuery, const U32& fluidCollisionCount, SharedPhysics::C_SafeStack<true>& stack);
	void WakeUpCloth(Gravity::C_Object* pObj, C_ClothCollisionQuery& clothQuery, SharedPhysics::C_SafeStack<true>& stack);	

	bool ContinuousCheckBoundingBox(Collision::C_Shape* pShapeA, Collision::C_Shape* pShapeB,
		const Maths::HVec3 PCREF transA, const Maths::HVec3 PCREF transB, Maths::HF32& lambda,
		Maths::HVec3& normal, Maths::HVec3& contactPoint);

	private:
	U32 CombinedPairGeneration(SharedPhysics::C_SafeStack<true>& stack, const Maths::HF32 PCREF time);
	U32 CompareObjectToList(SharedPhysics::C_MemoryIterator<C_ObjectPair, 128>& objectIter, Gravity::C_Object* pObject, 
		Gravity::C_ObjectWrapper* pList, Collision::C_AxisBoxMinMax& box, const U32& numContacts);

	bool ValidActiveList();
	

};

};

#endif
