#ifndef CONSTRAINTGRAPHTEST_H
#define CONSTRAINTGRAPHTEST_H

#include <system/scheduler/jobqueue.h>

#ifndef USE_SOLVER_2
#include "gravity/solver/ConstraintGraphManager.h"
#include "gravity/solver/Constraint.h"
#else
#include "gravity/solver2/ConstraintGraphManager.h"
#include "gravity/solver2/Constraint.h"
//#include "gravity/solver2/Featherstone.h"
#endif
#include "gravity/sdk/Body.h"
#include "gravity/sdk/Scene.h"
#include "gravity/sdk/PhysicsSDK.h"
#include "gravity/sdk/InertiaTensorGenerator.h"
#include "gravity/sharedphysics/Material.h"
#include "gravity/sharedphysics/ContactStream.h"
#include "gravity/sharedphysics/ContactStreamIterator.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/linecollisioncache.h"
#include "gravity/importer/Importer.h"
#include "gravity/sdk/charactercontroller.h"
#include "gravity/sdk/RagdollController.h"
#include "samples/system/input.h"
#include "gravity/sdk/KinematicController.h"
#include "gravity/sdk/CollisionCallback.h"
#include "gravity/sdk/EventCallback.h"
#include "gravity/cloths/ParticleBasedFluid.h"



//#ifdef USE_SOLVER_2
//namespace Solver
//{
//	template <Serial serial>
//	typedef Solver2::C_SphericalConstraintDesc<Serial> C_SphericalConstraintDesc<Serial>;
//}
//#endif

namespace Samples
{
	class C_ContactDrawer : public Gravity::C_CollisionCallback
	{
	public:
		virtual void ProcessCollisions(SharedPhysics::C_ContactStreamIterator& iter);
	};

	class C_EventIterator : public Gravity::C_EventCallback
	{
	public:
		virtual void ProcessEvents(SharedPhysics::C_MemoryIterator<Gravity::C_BodyEvent>& eventIter, const U32 numEvents);
	};

	class TriggerEventCallback : public Gravity::C_TriggerEventCallback
	{
	public:
		SharedPhysics::SortedArray<SharedPhysics::C_PointerWrapper<Gravity::C_Trigger> > m_Triggers;
		virtual void ProcessTriggerEvents(SharedPhysics::C_MemoryIterator<Gravity::C_TriggerEvent>& iter, const U32 numEvents);

		void DrawTriggers(DbUtil::I_DebugRender* pRender);
	};

	class C_TriangleBuffer
	{
		Maths::HVec3* m_pVertexA;
		Maths::HVec3* m_pVertexB;
		Maths::HVec3* m_pVertexC;

	public:
		C_TriangleBuffer()
		{
		}
	};

	/*class C_SimpleCollisionFilter : public Collision::C_CollisionFilter
	{
	public:

		C_SimpleCollisionFilter() : Collision::C_CollisionFilter(0)
		{
		}

		virtual bool CanCollide(C_CollisionFilter* pOther) const
		{
			return true;
		}
		//Triangles are given individual collision flags, which can be checked against
		virtual bool CanCollide(U32 collisionFlags) const
		{
			return true;
		}
	};

	class C_PrismaticCollisionFilter : public Collision::C_CollisionFilter
	{
	public:
		C_PrismaticCollisionFilter() : Collision::C_CollisionFilter(0)
		{
		}

		virtual bool CanCollide(C_CollisionFilter* pOther) const
		{
			return false;
		}
		//Triangles are given individual collision flags, which can be checked against
		virtual bool CanCollide(U32 collisionFlags) const
		{
			return true;
		}
	};*/

#define MATERIAL_COUNT 64
#define CHARACTER_CONTROLLER 63
#define F_TRANSPARENT 0x80000000
#define PLATFORM  62

	const int NUM_GROUPS = 7;
	const int NUM_TASKS = 256;
	const int X_RANGE = 16;
	const int Y_RANGE = 16;

	class C_TestPhysics
	{
	public:
		Gravity::C_Scene* m_pScene;
		Collision::C_CollisionFilter m_Filter;
		//C_PrismaticCollisionFilter m_PrismFilter;
		Gravity::C_ExternalConstants m_ExternalConstants;
		SharedPhysics::C_MaterialPair m_Materials[MATERIAL_COUNT][MATERIAL_COUNT];
		C_ContactDrawer drawer;
		Util::C_Random random;
		//SharedPhysics::SortedArray<SharedPhysics::C_Constraint*> m_Constraints;
		Gravity::C_ImportScene* pImportScene;
		Gravity::C_ImportScene* chairScene;
		Gravity::C_ImportSceneStaticInstance* pStaticScene;
		Collision::C_CollisionFilter m_GroupFilter;
		C_EventIterator eventIter;

		SharedPhysics::SortedArray<Gravity::C_TriggerActor*> m_TriggerActors;
		SharedPhysics::SortedArray<Gravity::C_Trigger*> m_Triggers;

		bool m_UseController;


		Solver2::C_SphericalConstraint sphericalConstraint;
		Solver2::C_SphericalConstraint sphericalConstraint2;

		SharedPhysics::C_SafeStack<true> stack;
		Collision::C_LineCollisionCache m_Cache;
		Gravity::C_CharacterController m_Controller;
		Gravity::C_KinematicController m_KinematicController;
		Gravity::C_RagdollController m_RagdollController;

		Collision::C_ParticleBasedFluidSystem m_FluidSystem;
		
		//Gravity::C_Importer<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > import;
	
		
		Scheduler::GroupHandle Groups[NUM_GROUPS];
		Scheduler::PolicyHandle hPolicy, collisionPolicy;

		SharedPhysics::SortedArray<Gravity::C_Body*> m_DeleteBodies;

		Collision::C_Ray* m_pRays;

		Gravity::C_TriggerLayer* m_pLayer;
		Gravity::C_TriggerActor* m_pTriggerActor;

		TriggerEventCallback m_TriggerCallback;


		C_TestPhysics() : m_pScene(NULL), stack(4096), chairScene(NULL), pImportScene(NULL), pStaticScene(NULL),
			m_KinematicController(NULL), m_GroupFilter(0xffffffff, 0xffffffff, 1, 0, 0), m_Filter(0xffffffff, 0xffffffff)
		{
			m_pRays = new Collision::C_Ray[NUM_TASKS];
			pImportScene = NULL;
			pStaticScene = NULL;
			m_pLayer = NULL;
			m_pTriggerActor = NULL;

			InitialiseScheduler();
		}

		~C_TestPhysics()
		{
			delete m_pTriggerActor;

			if(pStaticScene)
				delete pStaticScene;

			if(pImportScene)
				delete pImportScene;

			if(chairScene)
				delete chairScene;

			if(m_pScene)
			{
				Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseScene(m_pScene);
			}
			delete [] m_pRays;


			RemoveTriggers();
			for(S32 a = 0; a < m_Triggers.m_numElements; ++a)
			{
				delete m_Triggers[a];	
			}
			m_Triggers.m_numElements = 0;


			ReleaseScheduler();
		}

		void RemoveTriggers();
		void InitialiseScheduler();
		void ReleaseScheduler();
		void TestInsertRemove();
		void TestSpatialQuery();
		void AddFillCacheTask(S32 groupId, S32 policyId);

		void SetUpMaterials()
		{
			for(U32 a = 0; a < MATERIAL_COUNT; ++a)
			{
				for(U32 b = 0; b < MATERIAL_COUNT; ++b)
				{
					m_Materials[a][b].m_Friction = 0.45f;//(g_StaticFriction[a][b] + g_DynamicFriction[a][b]) * 0.8f;
					m_Materials[a][b].m_RestitutionCoefficient = 0.1f;//g_Restitution[a][b];
				}
			}

			for(U32 a = 0; a < MATERIAL_COUNT; ++a)
			{
				m_Materials[CHARACTER_CONTROLLER][a].m_Friction = 0.f;
				m_Materials[CHARACTER_CONTROLLER][a].m_RestitutionCoefficient = 0.f;
				m_Materials[a][CHARACTER_CONTROLLER].m_Friction = 0.f;
				m_Materials[a][CHARACTER_CONTROLLER].m_RestitutionCoefficient = 0.f;

				m_Materials[PLATFORM][a].m_Friction = 100.f;
				m_Materials[PLATFORM][a].m_RestitutionCoefficient = 0.1f;
				m_Materials[a][PLATFORM].m_Friction = 100.f;
				m_Materials[a][PLATFORM].m_RestitutionCoefficient = 0.1f;
				
			}
		}

		//Add a box at a given position/orientation to the world

		/*Gravity::C_Body* AddCompositeShape(Maths::HVec3 PCREF position, Maths::HMat33 PCREF orientation, Maths::HVec3 PCREF extents,
			Collision::C_CollisionFilter* pFilter);*/

		Gravity::C_Body* AddCompositeShape(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF extents, 
			Collision::C_CollisionFilter* pFilter);


		/*Gravity::C_Body* AddBox(Maths::HVec3 PCREF position, Maths::HMat33 PCREF orientation, Maths::HVec3 PCREF extents, Maths::HF32 PCREF mass,
			Collision::C_CollisionFilter* pFilter);*/

		Gravity::C_Body* AddBox(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF extents, const Maths::HF32 PCREF mass, 
			Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddCone(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddCharacterController(Maths::HVec3 PCREF position, Maths::HMat33 PCREF orientation, Maths::HF32 PCREF height, Maths::HF32 PCREF radius, 
			Maths::HF32 PCREF rayHeight, Maths::HF32 PCREF rayRadius, Maths::HF32 PCREF mass,	Collision::C_CollisionFilter* pFilter);

		void AddGroundPlane(const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF distance,
		Collision::C_CollisionFilter* pFilter);

		//Gravity::C_Body* AddSphere(Maths::HVec3 PCREF position, const Maths::HF32 radius, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter); error C2719: 'radius': formal parameter with __declspec(align('16')) won't be aligned
		Gravity::C_Body* AddSphere(Maths::HVec3 PCREF position, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter);

		//Gravity::C_Body* AddCapsule(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass,
		//	Collision::C_CollisionFilter* pFilter); error C2719: 'height': formal parameter with __declspec(align('16')) won't be aligned
		Gravity::C_Body* AddCapsule(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass,
			Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddCylinder(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF Height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass,
			Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddConvexHullShape(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, Maths::HF32 PCREF mass,
			Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddChair(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position);

		void DoRayTrace(const Maths::HF32 PCREF dis);
		void DoTransparentRayTrace(const Maths::HF32 PCREF dis);
		void GetRayTraceResults();
		void SetUpBridge(const Maths::HVec3 PCREF startPoint, const Maths::C_Normal PCREF layDirection, const Maths::HF32 PCREF layGap, const Maths::HF32 PCREF slop, const Maths::HVec3 PCREF plankExtents, const U32 numPlanks,
		Collision::C_CollisionFilter* pFilter);

		void SetUpFLuid(const Maths::HVec3 PCREF height, Collision::C_CollisionFilter* pFilter);
	
		void SetUpCompositeShapes(const U32 numComposites, Collision::C_CollisionFilter* pFilter);
		void SetUpRandomBoxes(const U32 numBoxes, Collision::C_CollisionFilter* pFilter);
		void SetUpConvexHull(Collision::C_CollisionFilter* pFilter);
		void SetUpCapsule(Collision::C_CollisionFilter* pFilter);
		void SetUpSphere(Collision::C_CollisionFilter* pFilter);
		void AttachRopeToBody(Gravity::C_Body* pBody,Collision::C_CollisionFilter* pFilter,
			const Maths::HVec3 PCREF offsetPosition);

		void InitialiseTest(Collision::C_CollisionFilter* pFilter, const U32 demoMode);

		void DrawCompositeShape(const Collision::C_CompositeShape* composite);
		void DrawAnchorPoints();
		void AnimateAndDraw(const Maths::C_TimeS PCREF time);
		void UpdateRagdollConstraintWeight();
		void UpdateRagdollBodyWeight();
		void UpdateRagdollController(const Maths::HF32 PCREF time);
		void UpdateFeatherstoneRagdoll(const Maths::HF32 PCREF time);

		void SetUpRagdollArm(Collision::C_CollisionFilter* pFilter);
		void SetUpSphericalJointLimit(Collision::C_CollisionFilter* pFilter);
		void SetUpSphericalJoint(Collision::C_CollisionFilter* pFilter);
		void SetUpFixedDemo(Collision::C_CollisionFilter* pFilter);
		void SetUpTwoRotationalDemo(Collision::C_CollisionFilter* pFilter);
		void SetUpRopeDemo(Collision::C_CollisionFilter* pFilter);
		//void SetUpAngularLimitDemo(Collision::C_CollisionFilter* pFilter);
		void SetUpRevoluteJoint(Collision::C_CollisionFilter* pFilter);
		void SetUpRevoluteJointLimit(Collision::C_CollisionFilter* pFilter);
		void SetUpPrismaticJoint(Collision::C_CollisionFilter* pFilter);
		void SetUpPrismaticJointLimit(Collision::C_CollisionFilter* pFilter);
		void SetUpCar(Collision::C_CollisionFilter* pFilter);
		void SetUpDistanceConstraint(Collision::C_CollisionFilter* pFilter);
		void SetUpFrameOfReference(Collision::C_CollisionFilter* pFilter);
		void UpdateFrameOfReference(const Maths::HF32 PCREF time, const Maths::HF32 PCREF rot);

		//Returns a ptr to the torso!
		Gravity::C_Body* SetUpRagdoll(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position);
		void InitialiseRagdollConfig(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position);
		void InitiliseFeatherstoneRagdoll(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position);
		void InitialiseSerialLink(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position);
		void InitialiseFixedJointRagdoll(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position);
		void SetUpWall(const U32 height, const U32 width, Collision::C_CollisionFilter* pFilter);

		void InitialiseParticleBasedFluidSystem();
		void DrawParticleBaseFluidSystem();

		void SetUpTriggers();
		void SetUpCylinder(Collision::C_CollisionFilter* pFilter);
		void SetUpSpring(Collision::C_CollisionFilter* pFilter);
		
		
		void ShootACone(Collision::C_CollisionFilter* pFilter);
		void ShootABox(Collision::C_CollisionFilter* pFilter);
		void ShootAChair(Collision::C_CollisionFilter* pFilter);
		void ShootASphere(Collision::C_CollisionFilter* pFilter);
		void ShootACylinder(Collision::C_CollisionFilter* pFilter);
		void ShootACapsule(Collision::C_CollisionFilter* pFilter);
		void ShootAConvexHull(Collision::C_CollisionFilter* pFilter);

		void ShootARagdoll(Collision::C_CollisionFilter* pFilter);
		void OnRender();


		void AnchorPointDraw();


		void ProcessController(Samples::C_Input& input);
	};
};
#endif
