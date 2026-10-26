#ifndef CONSTRAINTGRAPHTEST_H
#define CONSTRAINTGRAPHTEST_H

#include "gravity/solver/ConstraintGraphManager.h"
#include "gravity/sdk/Body.h"
#include "gravity/sdk/Scene.h"
#include "gravity/sdk/PhysicsSDK.h"
#include "gravity/sdk/InertiaTensorGenerator.h"
#include "gravity/sharedphysics/Material.h"
#include "gravity/sharedphysics/ContactStream.h"
#include "gravity/sharedphysics/ContactStreamIterator.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/linecollisioncache.h"
#include "gravity/importer/InstanceMeshResourceManager.h"
#include "gravity/sdk/charactercontroller.h"

namespace Cam2
{
	class C_Camera;
	class C_CharacterCamera;
}
namespace Framework { class C_View; }
namespace Input { class C_GameInput; }

namespace Samples
{

	class C_ContactDrawer : public Gravity::C_CollisionCallback
	{
	public:
		virtual void ProcessCollisions(SharedPhysics::C_ContactStream& stream);
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
		SharedPhysics::SortedArray<SharedPhysics::C_Constraint*> m_Constraints;

		Solver::C_SphericalConstraint sphericalConstraint;
		Solver::C_SphericalConstraint sphericalConstraint2;
		SharedPhysics::C_SafeStack<true> stack;
		Collision::C_LineCollisionCache m_Cache;
		Gravity::C_CharacterController m_Controller;
		Gravity::C_InstanceMeshResourceManager m_IMeshManager;

		
		Scheduler::GroupHandle Groups[NUM_GROUPS];
		Scheduler::PolicyHandle hPolicy, collisionPolicy;

		Collision::C_Ray* m_pRays;


		C_TestPhysics() : m_pScene(NULL), stack(4096)
		{
			m_pRays = new Collision::C_Ray[NUM_TASKS];

			InitialiseScheduler();
			m_Camera = 0;
			m_CameraCycle = 0;
		}

		~C_TestPhysics()
		{
			if(m_pScene)
			{
				Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseScene(m_pScene);
			}
			delete [] m_pRays;

			ReleaseScheduler();
		}

		void InitialiseScheduler();
		void ReleaseScheduler();

		void SetUpMaterials()
		{
			F32 g_Restitution[ MATERIAL_COUNT ][ MATERIAL_COUNT ] = 
			{
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.50f,      0.50f,      0.50f,      0.50f,      0.50f,      0.50f,      0.50f,      0.50f,      0.50f,      0.50f,      0.50f,      0.50f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.50f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f },
				{   0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f,      0.01f }
			};

			F32 g_StaticFriction[ MATERIAL_COUNT ][ MATERIAL_COUNT ] = 
			{
				{   0.40f,   0.35f,   0.30f,   0.25f,   0.20f,   0.15f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.35f,   0.35f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.30f,   0.40f,   0.30f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.25f,   0.40f,   0.40f,   0.25f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.20f,   0.40f,   0.40f,   0.40f,   0.20f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.15f,   0.40f,   0.40f,   0.40f,   0.40f,   0.15f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.40f,   0.00f },
				{   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f }
			};

			F32 g_DynamicFriction[ MATERIAL_COUNT ][ MATERIAL_COUNT ] = 
			{
				{   0.30f,   0.25f,   0.20f,   0.15f,   0.10f,   0.05f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.25f,   0.25f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.20f,   0.30f,   0.20f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.15f,   0.30f,   0.30f,   0.15f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.10f,   0.30f,   0.30f,   0.30f,   0.10f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.05f,   0.30f,   0.30f,   0.30f,   0.30f,   0.05f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.30f,   0.00f },
				{   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f,   0.00f }
			};

			for(U32 a = 0; a < MATERIAL_COUNT; ++a)
			{
				for(U32 b = 0; b < MATERIAL_COUNT; ++b)
				{
					m_Materials[a][b].m_Friction = 0.65f;//(g_StaticFriction[a][b] + g_DynamicFriction[a][b]) * 0.8f;
					m_Materials[a][b].m_RestitutionCoefficient = 0.1f;//g_Restitution[a][b];
				}
			}

			for(U32 a = 0; a < MATERIAL_COUNT; ++a)
			{
				m_Materials[CHARACTER_CONTROLLER][a].m_Friction = 0.f;
				m_Materials[CHARACTER_CONTROLLER][a].m_RestitutionCoefficient = 0.f;
				m_Materials[a][CHARACTER_CONTROLLER].m_Friction = 0.f;
				m_Materials[a][CHARACTER_CONTROLLER].m_RestitutionCoefficient = 0.f;
			}
		}

		//Add a box at a given position/orientation to the world

		Gravity::C_Body* AddCompositeShape(Maths::HVec3 PCREF position, Maths::HMat33 PCREF orientation, Maths::HVec3 PCREF extents,
			Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddBox(Maths::HVec3 PCREF position, Maths::HMat33 PCREF orientation, Maths::HVec3 PCREF extents, Maths::HF32 PCREF mass,
			Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddCharacterController(Maths::HVec3 PCREF position, Maths::HMat33 PCREF orientation, Maths::HF32 PCREF height, Maths::HF32 PCREF radius, 
			Maths::HF32 PCREF rayHeight, Maths::HF32 PCREF rayRadius, Maths::HF32 PCREF mass,	Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddSphere(Maths::HVec3 PCREF position, const Maths::HF32 radius, Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddCapsule(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass,
			Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddCylinder(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF Height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass,
			Collision::C_CollisionFilter* pFilter);

		Gravity::C_Body* AddConvexHullShape(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, Maths::HF32 PCREF mass,
			Collision::C_CollisionFilter* pFilter);

		void DoRayTrace(const Maths::HF32 PCREF dis);
		void GetRayTraceResults();
		void SetUpBridge(const Maths::HVec3 PCREF startPoint, const Maths::C_Normal PCREF layDirection, const Maths::HF32 PCREF layGap, const Maths::HF32 PCREF slop, const Maths::HVec3 PCREF plankExtents, const U32 numPlanks,
		Gravity::C_CollisionFilter* pFilter);

		void SetUpCompositeShapes(const U32 numComposites, Collision::C_CollisionFilter* pFilter);
		void SetUpRandomBoxes(const U32 numBoxes, Collision::C_CollisionFilter* pFilter);
		void SetUpConvexHull(Collision::C_CollisionFilter* pFilter);
		void SetUpCapsule(Collision::C_CollisionFilter* pFilter);
		void SetUpSphere(Collision::C_CollisionFilter* pFilter);

		void InitialiseTest(Collision::C_CollisionFilter* pFilter, const U32 demoMode);

		void DrawCompositeShape(const Collision::C_CompositeShape* composite);
		void AnimateAndDraw(const Maths::C_TimeS PCREF time);

		void SetUpSphericalJoint(Collision::C_CollisionFilter* pFilter);
		void SetUpRevoluteJoint(Collision::C_CollisionFilter* pFilter);
		void SetUpRevoluteJointLimit(Collision::C_CollisionFilter* pFilter);
		//Returns a ptr to the torso!
		Gravity::C_Body* SetUpRagdoll(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position);
		void SetUpWall(const U32 height, const U32 width, Collision::C_CollisionFilter* pFilter);
		void SetUpCylinder(Collision::C_CollisionFilter* pFilter);
		void SetUpPrismaticJoint(Collision::C_CollisionFilter* pFilter);
		void SetUpPrismaticJointLimit(Collision::C_CollisionFilter* pFilter);
		void SetUpCar(Collision::C_CollisionFilter* pFilter);
		
		void ShootABox(Collision::C_CollisionFilter* pFilter);
		void ShootARagdoll(Collision::C_CollisionFilter* pFilter);

		void AnchorPointDraw();

		void ProcessController(Input::C_GameInput& input);
		Framework::C_View& View();
		Cam2::C_Camera& LiveCamera();
		Cam2::C_CharacterCamera* m_Camera;
		S32 m_CameraCycle;
	};
};
#endif
