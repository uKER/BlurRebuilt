#include "StdAfx.h"
#include "ConstraintGraphTest.h"


#include "gravity/SharedPhysics/AllocationManager.inl"
#include "gravity/sdk/ShapeDescriptors.h"
#include "gravity/sdk/ShapeDescriptors.inl"
#include "gravity/sdk/BodyHelper.h"
#include "gravity/sdk/BodyHelper.inl"
#include "gravity/SharedPhysics/SortedArray.inl"
#include "System/Misc/Random.h"
#include "gravity/broadphase/Object.h"
#include "gravity/broadphase/Object.inl"
#include "SectionPhysics.h"
#include "gravity/sdk/ConstraintDescriptors.h"
#include "Samples/System/View.h"
#include "Samples/System/Application.h"
#include "gravity/sdk/shapedescriptors.h"
//#include "gravity/narrowphase/optimisedaabbtree.inl"
#include "gravity/SharedPhysics/ContactStreamIterator.inl"
#include "gravity/importer/Importer.h"
#include "gravity/sdk/charactercontroller.h"
//#include "gravity/sdk/RagdollController.h"
#include "gravity/Cloths/HighFieldFluidCollision.h"

#include "gravity/SharedPhysics/Spring.h"
#include "system/misc/random.h"
#include "gravity/narrowphase/linecollisioncache.h"
#include "gravity/triggers/TriggerEvent.h"
#ifdef PLATFORM_XENON
#include <Samples/System/AtgInput.h>
#endif

//New comment!
//Newer comment!


namespace Samples
{

#ifndef PLATFORM_SPU
	Debug::Bool g_WallDemo("Test.WallDemo", true);
	Debug::Bool g_Ragdoll("Test.Ragdol", false);
	Debug::Bool g_dbDrawMesh("Test.DrawMesh", true);
	Debug::Bool g_dbDrawLocation("Test.DrawLocation", true);
	Debug::Bool g_dbResetRot("Test.NoRot", true);

	Debug::Bool bDrawContacts("Test.DrawContacts", false);
	Debug::Bool  g_dbNormaliseContacts	("Test.NormaliseContactMassRatios", false);
	Debug::Float g_dbMassRatioScale		("Test.MassRatioScale", 2.f, Debug::Range(1.f, 10.f));
	Debug::Float g_dbInfMassRatioScale	("Test.InfiniteMassRatioScale", 20.f, Debug::Range(1.f, 100.f));
	Debug::Bool bDrawBoundingBox("Test.DrawBoundingBoxes", false);
	Debug::Bool g_UseScheduler("Test.UseScheduler", true);
	Debug::Bool g_DrawRayResults("Test.DrawRayResults", false);
	Debug::Bool g_dbCullObjects("Test.CullObjects", true);
	Debug::Bool g_dbForceDraw("Test.ForceDraw", false);
	Debug::Bool g_dbDrawAnchorPoint("Test.DrawAnchorPoint", false);
	Debug::Bool g_dbDrawObjects("Test.DrawObjects", true);
	Debug::Bool g_dbDoRaytrace("Test.DoRaytraces", false);
	Debug::Bool g_dbDoTransparentRaytrace("Test.DoTransparentRaytraces", false);
	Debug::Bool g_dbTestParticleBasedFluidSystem("Test.ParticleBasedFluidSystem", false);

	Debug::Bool g_dbShootABox("Test.ShootABox", true);
	Debug::Bool g_dbShootAChair("Test.ShootAChair", false);
	Debug::Bool g_dbShootACone("Test.ShootACone", false);
	Debug::Bool g_dbShootASphere("Test.ShootASphere", false);
	Debug::Bool g_dbShootACylinder("Test.ShootACylinder", false);
	Debug::Bool g_dbShootACapsule("Test.ShootACapsule", false);
	Debug::Bool g_dbShootAConvexHull("Test.ShootAConvexHull", false);
	Debug::Bool g_dbInitialseFixedJointRagdoll("Test.FixedJointRagdoll", false);
	Debug::Bool g_dbCharacterControllerTeleport("Test.CharacterControllerTeleport", false);
	Debug::Bool g_dbUpdateConstraintAnchorPoint("Test.UpdateConstraintAnchorPoint", false);

	Debug::Float g_dbLinearSpringCoefficient("Test.LinearSpringCoefficient", 30.f, Debug::Range(0.f, 100.f));
	Debug::Float g_dbLinearDampingCoefficient("Test.LinearSpringDampingCoefficient", 10.f, Debug::Range(0.f, 100.f));
	Debug::Float g_dbAngularSpringCoefficient("Test.AngularSpringCoefficient", 5.f, Debug::Range(0.f, 100.f));
	Debug::Float g_dbAngularDampingCoefficient("Test.AngularSpringDampingCoefficient", 0.5f, Debug::Range(0.f, 100.f));


	Debug::Float g_dbVelocity("Test.Velocity", 15.f, Debug::Range(0.f, 20.f));
	
	//Debug::Float g_dbSpringCoefficient("Test.SpringCoefficient", 10.f, Debug::Range(0.f, 50.f));
	//Debug::Float g_dbDampingCoefficient("Test.SpringDampingCoefficient", 0.5f, Debug::Range(0.f, 20.f));

	enum E_DemoMode
	{	
		E_RopeDemo,
		E_MaxDemos,
		E_HingeJointLimitDemo,
		E_SphericalLimitDemo,
		E_RagdollArmDemos,
		E_SpringDemo,
		E_ParticleFluidDemo,
		E_TwoRotationalDemo,
		E_PrismaticJointLimitDemo,
		
		E_NullDemo,
		E_SphericalDemo,
		E_DistanceConstraintDemo,
		//E_AngularLimitDemo,
		E_ReferenceFrameDemo,
		E_CompositeDemo,
		E_CylinderDemo,
		E_SphereDemo,
		E_WallDemo,
		E_CapsuleDemo,
		E_ConvexHullDemo, 
		E_CarDemo,
		E_BridgeDemo,
		E_CatterpillarTracksDemo,
		
		E_HingeDemo,
		E_PrismaticDemo,
		E_FixedJointDemo,
		
		
		//E_MaxDemos
	};

	void C_ContactDrawer::ProcessCollisions(SharedPhysics::C_ContactStreamIterator& iter)
	{
		const Maths::HF32 massScale(g_dbMassRatioScale);
		const Maths::HF32 infMassScale(g_dbInfMassRatioScale);
		bool drawContacts(bDrawContacts);
		bool normaliseContacts(g_dbNormaliseContacts);
		while(iter.hasNextContact())
		{
			SharedPhysics::C_ContactStreamResult* result(iter.nextContact());


			//if(result->m_pNodeA->TestNodeFlags(Gravity::C_GraphNode::E_Type_Active) && 
			//	result->m_pNodeB->TestNodeFlags(Gravity::C_GraphNode::E_Type_Active))

			Gravity::C_Body* pBodyA(result->m_pNodeA->m_pBody);
			Gravity::C_Body* pBodyB(result->m_pNodeB->m_pBody);

			if(normaliseContacts)
			{
				iter.NormaliseContacts(massScale, infMassScale);
			}


			if(drawContacts)
			{
				while(iter.hasNextPatch())
				{
					SharedPhysics::C_ContactStreamPatch* patch(iter.nextPatch());
					const Maths::HVec3 norm(Maths::Load<Maths::HVec3>(&patch->m_Normal));
					while(iter.hasNextPoint())
					{
						SharedPhysics::C_ContactStreamPoint* point(iter.nextPoint());
						assert(point->m_PenetrationDepth < 5.f);
						Maths::HF32 penDepth(Maths::Load<Maths::HF32>(&point->m_PenetrationDepth));
						//Draw points of contact...
						const Maths::HVec3 contactPoint(Maths::Load<Maths::HVec3>(&point->m_ContactPoint));
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactPoint,
							0.01f, Maths::RGBA(255, 255, 0, 0));
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contactPoint, contactPoint + penDepth * norm, Maths::RGBA(0, 255, 0, 255));
					}
				}
			}
		}
	}

	void C_EventIterator::ProcessEvents(SharedPhysics::C_MemoryIterator<Gravity::C_BodyEvent>& eventIter, const U32 numEvents)
	{
		for(U32 a = 0; a < numEvents; ++a)
		{
			Gravity::C_BodyEvent* pEvent = eventIter.Get();
			//Do something with it...
		}
	}

	void TriggerEventCallback::ProcessTriggerEvents(SharedPhysics::C_MemoryIterator<Gravity::C_TriggerEvent>& iter, const U32 numEvents)
	{
		for(U32 a = 0; a < numEvents; ++a)
		{
			Gravity::C_TriggerEvent* pEvent = iter.Get();
			if(pEvent->GetEventType() == Gravity::C_TriggerEvent::E_Enter_Event)
			{
				Gravity::C_Trigger* pTrigger = pEvent->GetTrigger();

				SharedPhysics::C_PointerWrapper<Gravity::C_Trigger> wrapper(pTrigger);
				m_Triggers.InsertOverwrite(&wrapper);
			}
			else if (pEvent->GetEventType() == Gravity::C_TriggerEvent::E_Exit_Event)
			{
				Gravity::C_Trigger* pTrigger = pEvent->GetTrigger();
				SharedPhysics::C_PointerWrapper<Gravity::C_Trigger> wrapper(pTrigger);
				m_Triggers.Remove(&wrapper);
			}
		}
	}

	void TriggerEventCallback::DrawTriggers(DbUtil::I_DebugRender* pRender)
	{
		for(S32 a = 0; a < m_Triggers.m_numElements; ++a)
		{
			Gravity::C_Trigger* pTrigger = m_Triggers[a].m_pElement;

			if(pTrigger->m_pTriggerShape->m_ShapeType == Collision::E_Box_Shape)
			{
				Collision::C_BoxShape* pBox = (Collision::C_BoxShape*)pTrigger->m_pTriggerShape;
				pRender->Draw3dBox(-pBox->m_Box.e, pBox->m_Box.e, pBox->m_PosRot, Maths::RGBA(0, 255, 0));
			}
			else if (pTrigger->m_pTriggerShape->m_ShapeType == Collision::E_Sphere_Shape)
			{
				Collision::C_SphereShape* pSphere = (Collision::C_SphereShape*)pTrigger->m_pTriggerShape;
				pRender->Draw3dSphere(pSphere->m_Sphere.Centre(), pSphere->m_Sphere.Radius(), Maths::RGBA(0, 255, 0));
			}
		}
	}

	using namespace Gravity;
	Gravity::C_Body* C_TestPhysics::AddBox(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF extents, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = mass * 0.0001f;
		deactivator.m_SleepThresholdSq = mass * 0.00005f;
		C_Body* boxBody(m_pScene->CreateBody());
		boxBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_BoxDesc boxDesc;
		boxDesc.m_HalfExtents = extents;
		Maths::HF32 margin(Maths::Min(extents.X(), Maths::Min(extents.Y(), extents.Z())));
		margin *= 0.1f;
		boxDesc.m_Margin = margin;
		boxDesc.m_Epsilon = margin * 0.005f;
		Gravity::C_BodyHelper bodyHelper(&boxDesc);
		bodyHelper.CreateBody(mass, position, orientation, boxBody, pFilter, 0, 0);
		boxBody->SetDeactivator(deactivator);
		boxBody->Slop(F32(margin) * 0.01f);
		boxBody->Baumgarte(0.75f);
		
		m_pScene->AddBody(boxBody);
		return boxBody;
	}

	Gravity::C_Body* C_TestPhysics::AddCone(const Maths::HVec3 PCREF position,  const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = mass * 0.0001f;
		deactivator.m_SleepThresholdSq = mass * 0.00005f;
		C_Body* coneBody(m_pScene->CreateBody());
		coneBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_ConeDesc coneDesc;
		coneDesc.m_BaseRadius = radius;
		coneDesc.m_Height = height;
		Maths::HF32 margin(Maths::Min(radius, height) * 0.1f);
		coneDesc.m_Margin = margin;
		coneDesc.m_Epsilon = margin * 0.005f;
		Gravity::C_BodyHelper bodyHelper(&coneDesc);
		bodyHelper.CreateBody(mass, position, orientation, coneBody, pFilter, 0, 0);
		coneBody->SetDeactivator(deactivator);
		coneBody->Slop(F32(margin) * 0.01f);
		coneBody->Baumgarte(0.75f);
		
		m_pScene->AddBody(coneBody);
		return coneBody;
	}

	void C_TestPhysics::AddGroundPlane(const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF distance,
		Collision::C_CollisionFilter* pFilter)
	{
		Gravity::C_PlaneDesc desc;

		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.f;
		deactivator.m_SleepThresholdSq = 0.f;
		desc.m_Normal = normal;
		desc.m_Distance = distance;
		desc.m_PhysicsMaterialIndex = PLATFORM;
		desc.m_EffectMaterialIndex = PLATFORM;
		Gravity::C_Body* pBody(m_pScene->CreateBody());
		pBody->SetExternalConstants(&m_ExternalConstants);
		pBody->MakeStatic();
		Gravity::C_BodyHelper helper(&desc);
		Maths::HMat33 rotation;
		rotation.SetIdentity();
		helper.CreateBody(Maths::HF32(0.f), Maths::HVec3(0.f, 0.f, 0.f), rotation, pBody, pFilter, PLATFORM, PLATFORM);
		pBody->SetDeactivator(deactivator);
		//DebugBreak();
		m_pScene->SetGroundPlane(pBody);
	}


	Gravity::C_Body* C_TestPhysics::AddCharacterController(Maths::HVec3 PCREF position, Maths::HMat33 PCREF orientation, Maths::HF32 PCREF height, Maths::HF32 PCREF radius, 
			Maths::HF32 PCREF rayHeight, Maths::HF32 PCREF rayRadius, Maths::HF32 PCREF mass,	Collision::C_CollisionFilter* pFilter)
	{
		const Maths::HF32 zero;
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = -1.f;
		deactivator.m_SleepThresholdSq = -1.f;

		C_Body* controllerBody(m_pScene->CreateBody());
		controllerBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_CharacterControllerDesc controllerDesc;
		controllerDesc.m_CapsuleHeight = height * 0.5f;
		controllerDesc.m_CapsuleRadius = radius;
		controllerDesc.m_RayHeight = Maths::Max(rayHeight - rayRadius, zero);
		controllerDesc.m_RayRadius = rayRadius;
		controllerDesc.m_MaxCosAngle = Maths::HF32(0.5f); // cos 60
		controllerDesc.m_MinPen = rayHeight;
		controllerDesc.m_FeetMaterial = 0;
		controllerDesc.m_EffectMaterial = 0;
		Gravity::C_BodyHelper bodyHelper(&controllerDesc);
		bodyHelper.CreateBody(mass, position, orientation, controllerBody, pFilter, CHARACTER_CONTROLLER, 0);
		controllerBody->SetDeactivator(deactivator);
		controllerBody->Slop(0.00000001f);
		controllerBody->Baumgarte(0.7f);
		//controllerBody->MakeStatic();
		m_pScene->AddBody(controllerBody);
		return controllerBody;
	}

	//Gravity::C_Body* C_TestPhysics::AddSphere(Maths::HVec3 PCREF position, const Maths::HF32 radius, const Maths::HF32 PCREF mass, 
	//	Collision::C_CollisionFilter* pFilter) error C2719: 'radius': formal parameter with __declspec(align('16')) won't be aligned
	Gravity::C_Body* C_TestPhysics::AddSphere(Maths::HVec3 PCREF position, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass, 
		Collision::C_CollisionFilter* pFilter)
	{
		Maths::HMat33 orientation;
		orientation.SetIdentity();
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.05f;
		deactivator.m_SleepThresholdSq = 0.005f;
		C_Body* sphereBody(m_pScene->CreateBody());
		sphereBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_SphereDesc sphereDesc;
		sphereDesc.m_Radius = radius;
		sphereDesc.m_Epsilon = radius * 0.001f;
		Gravity::C_BodyHelper bodyHelper(&sphereDesc);
		bodyHelper.CreateBody(mass, position, orientation, sphereBody, pFilter, 0, 0);
		sphereBody->SetDeactivator(deactivator);
		m_pScene->AddBody(sphereBody);
		sphereBody->Slop(radius * 0.0001f);
		sphereBody->Baumgarte(0.8f);
		return sphereBody;
	}

	//Gravity::C_Body* C_TestPhysics::AddCapsule(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter *pFilter) error C2719: 'height': formal parameter with __declspec(align('16')) won't be aligned
	Gravity::C_Body* C_TestPhysics::AddCapsule(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter *pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.01f;
		deactivator.m_SleepThresholdSq = 0.001f;
		C_Body* capsuleBody(m_pScene->CreateBody());
		capsuleBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_CapsuleDesc capsuleDesc;
		capsuleDesc.m_Height = height;
		capsuleDesc.m_Radius=radius;//local space
		capsuleDesc.m_Epsilon = Maths::Max(height, radius) * 0.01f;
		Gravity::C_BodyHelper bodyHelper(&capsuleDesc);
		bodyHelper.CreateBody(mass, position, orientation, capsuleBody, pFilter, 0, 0);
		capsuleBody->SetDeactivator(deactivator);
		m_pScene->AddBody(capsuleBody);
		capsuleBody->Slop(0.00000005f);
		capsuleBody->Baumgarte(0.75f);
		return capsuleBody;
		
	}

	Gravity::C_Body* C_TestPhysics::AddCylinder(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF Height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.001f;
		deactivator.m_SleepThresholdSq = 0.0005f;
		C_Body* cylinderBody(m_pScene->CreateBody());
		cylinderBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_CylinderDesc cylinderDesc;
		//cylinderDesc.m_Center = position;
		cylinderDesc.m_Height=Height;
		cylinderDesc.m_Radius=radius;
		const Maths::HF32 margin(Maths::Min(radius, Height) * 0.2f);
		const Maths::HF32 epsilon(Maths::Max(radius, Height) * 0.001f);
		cylinderDesc.m_Margin = margin;
		cylinderDesc.m_Epsilon = epsilon;
		Gravity::C_BodyHelper bodyHelper(&cylinderDesc);
		bodyHelper.CreateBody(mass, position, orientation, cylinderBody, pFilter, 0, 0);
		cylinderBody->SetDeactivator(deactivator);
		m_pScene->AddBody(cylinderBody);
		cylinderBody->Slop(0.00000005f);
		cylinderBody->Baumgarte(0.8f);
		//cylinderBody->SetAngularDampingCoefficient(0.99f);
		return cylinderBody;
		
	}

	Gravity::C_Body* C_TestPhysics::AddCompositeShape(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF extents, Collision::C_CollisionFilter* pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.0005f;
		deactivator.m_SleepThresholdSq = 0.0001f;
		C_Body* boxBody(m_pScene->CreateBody());
		boxBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_CompositeDesc desc;
		Gravity::C_CompositeDesc desc2;
		Gravity::C_BoxDesc boxDescA;
		boxDescA.m_HalfExtents = Maths::HVec3(0.2f, 0.2f, 0.2f);
		boxDescA.m_Margin = 0.02f;
		boxDescA.m_Epsilon = 0.0005f;

		Gravity::C_SphereDesc sphereDescA;
		sphereDescA.m_Radius = 0.2f;
		sphereDescA.m_Epsilon = 0.0005f;

		Maths::HMat33 rot;
		rot.SetIdentity();
		const Maths::HVec3 trans;
		const Maths::HVec3 trans2(0.4f, 0.f, 0.f);
		const Maths::HF32 boxMass(0.5f);
		const Maths::HF32 sphereMass(1.f);
		desc.AddShape(&boxDescA, Maths::HMat43(rot, trans), boxMass, pFilter);
		desc.AddShape(&sphereDescA, Maths::HMat43(rot, trans2), sphereMass, pFilter);
		desc2.AddShape(&sphereDescA, Maths::HMat43(rot, -trans2), sphereMass, pFilter);
		desc2.AddShape(&boxDescA, Maths::HMat43(rot, trans), boxMass, pFilter);
		
		desc.AddShape(&desc2, Maths::HMat43(rot, -trans2), sphereMass+boxMass, pFilter);
		desc.Decompose();

		Gravity::C_BodyHelper bodyHelper(&desc);
		bodyHelper.CreateBody(2.5f, position, orientation, boxBody, pFilter, 1, 0);
		boxBody->SetDeactivator(deactivator);
		m_pScene->AddBody(boxBody);
		boxBody->Slop(0.000005f);
		boxBody->Baumgarte(0.8f);
		return boxBody;
	}

	Gravity::C_Body* C_TestPhysics::AddConvexHullShape(const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.0005f;
		deactivator.m_SleepThresholdSq = 0.0001f;

		C_Body* convexBody(m_pScene->CreateBody());

		convexBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_ConvexHullDesc desc(false);
		
		//TODO - enter in the points that make up the convex shape!!!!
		//Try a 10-point shape here...
		desc.AddPoint(Maths::HVec3(-0.2f, -0.2f, -0.2f));
		desc.AddPoint(Maths::HVec3(0.2f, -0.2f, -0.2f));
		desc.AddPoint(Maths::HVec3(-0.2f, 0.2f, -0.2f));
		desc.AddPoint(Maths::HVec3(0.2f, 0.2f, -0.2f));

		desc.AddPoint(Maths::HVec3(-0.2f, -0.2f, 0.2f));
		desc.AddPoint(Maths::HVec3(0.2f, -0.2f, 0.2f));
		desc.AddPoint(Maths::HVec3(-0.2f, 0.2f, 0.2f));
		desc.AddPoint(Maths::HVec3(0.2f, 0.2f, 0.2f));

		desc.AddPoint(Maths::HVec3(0.f, -0.4f, 0.f));
		desc.AddPoint(Maths::HVec3(0.f, 0.4f, 0.f));

		desc.AddTriangle(Gravity::C_TriangleIndices(0, 3, 1));
		desc.AddTriangle(Gravity::C_TriangleIndices(0, 2, 3));
		desc.AddTriangle(Gravity::C_TriangleIndices(4, 5, 7));
		desc.AddTriangle(Gravity::C_TriangleIndices(4, 7, 6));

		desc.AddTriangle(C_TriangleIndices(4, 2, 0));
		desc.AddTriangle(C_TriangleIndices(4, 6, 2));
		desc.AddTriangle(C_TriangleIndices(5, 3, 7));
		desc.AddTriangle(C_TriangleIndices(5, 1, 3));

		desc.AddTriangle(C_TriangleIndices(6, 7, 9));
		desc.AddTriangle(C_TriangleIndices(6, 9, 2));
		desc.AddTriangle(C_TriangleIndices(7, 3, 9));
		desc.AddTriangle(C_TriangleIndices(3, 2, 9));

		desc.AddTriangle(C_TriangleIndices(4, 8, 5));
		desc.AddTriangle(C_TriangleIndices(8, 1, 5));
		desc.AddTriangle(C_TriangleIndices(1, 8, 0));
		desc.AddTriangle(C_TriangleIndices(4, 0, 8));

		desc.m_Margin = 0.02f;
		desc.m_Epsilon = 0.0005f;

		Gravity::C_BodyHelper bodyHelper(&desc);
		bodyHelper.CreateBody(mass, position, orientation, convexBody, pFilter, 0, 0);
		convexBody->SetDeactivator(deactivator);
		m_pScene->AddBody(convexBody);
		convexBody->Slop(0.000005f);
		convexBody->Baumgarte(0.8f);
		return convexBody;
	}

	void C_TestPhysics::ShootABox(Collision::C_CollisionFilter* pFilter)
	{
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(camera.m_Eye);
		Maths::HVec3 dir((camera.m_At - camera.m_Eye));
		dir.Normalise();

		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(0.2f, 0.2f, 0.2f);
		
		//C_Body* body=AddSphere(position, 0.25f, 10.f, pFilter);

		C_Body* body=AddBox(position, orientation, extents, 3.f, pFilter);
		
		const Maths::HF32 vel(g_dbVelocity);
		body->LinearVelocity(dir * vel);
		body->AngularVelocity(Maths::HVec3(0.707f, 0.707f, 0.f));
		body->CalculateBodyVolume();
		//body->CalcualteSurfaceArea();
		body->SetBuoyancyCoefficient(25.f);
		body->Activate();

		const Maths::HVec3 offsetPosition(0.f, 0.f, -0.2f);

		AttachRopeToBody(body, pFilter, offsetPosition);

		
	}

	void C_TestPhysics::ShootACone(Collision::C_CollisionFilter* pFilter)
	{
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(camera.m_Eye);
		Maths::HVec3 dir((camera.m_At - camera.m_Eye));
		dir.Normalise();

		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(0.2f, 0.2f, 0.2f);
		
		//C_Body* body=AddSphere(position, 0.25f, 10.f, pFilter);

		C_Body* body=AddCone(position, orientation, 0.5f, 2.f, 3.f, pFilter);
		
		const Maths::HF32 vel(g_dbVelocity);
		body->LinearVelocity(dir * vel);
		body->AngularVelocity(Maths::HVec3(0.707f, 0.707f, 0.f));
		body->CalculateBodyVolume();
		//body->CalcualteSurfaceArea();
		body->SetBuoyancyCoefficient(20.f);
		
	}

	void C_TestPhysics::ShootAChair(Collision::C_CollisionFilter* pFilter)
	{
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(camera.m_Eye);
		Maths::HVec3 dir((camera.m_At - camera.m_Eye));
		dir.Normalise();

		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(0.1f, 0.1f, 0.1f);

		
		C_Body* body=AddChair(pFilter, position);
		
		const Maths::HF32 vel(g_dbVelocity);
		body->LinearVelocity(dir * vel);
		body->AngularVelocity(Maths::HVec3(0.707f, 0.707f, 0.f));
		body->CalculateBodyVolume();
		//body->CalcualteSurfaceArea();
		body->SetBuoyancyCoefficient(5.f);
	}

	void C_TestPhysics::ShootASphere(Collision::C_CollisionFilter* pFilter)
	{
		using namespace Maths;

		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		HVec3 position(camera.m_Eye);
		HVec3 dir((camera.m_At - camera.m_Eye));
		dir.Normalise();

		HMat33 rot;
		rot.SetIdentity();
		C_Quaternion orientation(rot);
		const HVec3 extents(0.1f, 0.1f, 0.1f);

	
		
		C_Body* body=AddSphere(position, 0.25f, HF32(1.f), pFilter);
		
		const Maths::HF32 vel(g_dbVelocity);
		body->LinearVelocity(dir * vel);
		body->AngularVelocity(Maths::HVec3(0.707f, 0.707f, 0.f));
		//C_Body* body=AddConvexHullShape(position, orientation, 80.f, pFilter);
		//body->MakeOneWayClothObject();
		body->CalculateBodyVolume();
		//body->CalcualteSurfaceArea();
		body->SetBuoyancyCoefficient(2.f);
	}

	void C_TestPhysics::ShootACylinder(Collision::C_CollisionFilter* pFilter)
	{
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(camera.m_Eye);
		Maths::HVec3 dir((camera.m_At - camera.m_Eye));
		dir.Normalise();

		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(0.1f, 0.1f, 0.1f);

		C_Body* body=AddCylinder(position, orientation, 0.25f, 0.25f, 1.f, pFilter);
	
		const Maths::HF32 vel(g_dbVelocity);
		body->LinearVelocity(dir * vel);
		body->AngularVelocity(Maths::HVec3(0.707f, 0.707f, 0.f));
		body->CalculateBodyVolume();
		//body->CalcualteSurfaceArea();
		body->SetBuoyancyCoefficient(3.f);

	}

	void C_TestPhysics::ShootACapsule(Collision::C_CollisionFilter* pFilter)
	{
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(camera.m_Eye);
		Maths::HVec3 dir((camera.m_At - camera.m_Eye));
		dir.Normalise();

		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(0.1f, 0.1f, 0.1f);


		C_Body* body=AddCapsule(position, orientation, 0.5f, 0.15f, 10.f, pFilter);
		
		const Maths::HF32 vel(g_dbVelocity);
		body->LinearVelocity(dir * vel);
		body->AngularVelocity(Maths::HVec3(0.707f, 0.707f, 0.f));
		body->SetBuoyancyCoefficient(5.f);
		body->CalculateBodyVolume();
		//body->CalcualteSurfaceArea();
	}

	void C_TestPhysics::ShootAConvexHull(Collision::C_CollisionFilter* pFilter)
	{
		using namespace Maths;
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		HVec3 position(camera.m_Eye);
		HVec3 dir((camera.m_At - camera.m_Eye));
		dir.Normalise();

		HMat33 rot;
		rot.SetIdentity();
		C_Quaternion orientation(rot);
		const HVec3 extents(0.1f, 0.1f, 0.1f);

		
		C_Body* body=AddConvexHullShape(position, orientation, HF32(1.f), pFilter);
		const Maths::HF32 vel(g_dbVelocity);
		body->LinearVelocity(dir * vel);
		body->AngularVelocity(Maths::HVec3(0.707f, 0.707f, 0.f));
		body->CalculateBodyVolume();
		//body->CalcualteSurfaceArea();
		body->SetBuoyancyCoefficient(5.f);
	}

	

	void C_TestPhysics::ShootARagdoll(Collision::C_CollisionFilter* pFilter)
	{
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(camera.m_Eye);
		Maths::HVec3 dir((camera.m_At - camera.m_Eye));
		dir.Normalise();

		C_Body* pRagdollBody(SetUpRagdoll(pFilter, position + Maths::HVec3(1.f, 0.f, 1.f)));
		//Maths::HVec3 force(dir * 20000.f);
		//pRagdollBody->ApplyForce(force, position);
		const Maths::HF32 vel(g_dbVelocity);
		pRagdollBody->LinearVelocity(dir * vel);


	}

	void C_TestPhysics::RemoveTriggers()
	{
		m_TriggerCallback.m_Triggers.m_numElements = 0;
		for(S32 a = 0; a < m_Triggers.m_numElements; ++a)
		{
			delete m_Triggers[a];	
		}
		m_Triggers.m_numElements = 0;

		for(S32 a = 0; a < m_TriggerActors.m_numElements; ++a)
		{
			delete m_TriggerActors[a];	
		}
		m_TriggerActors.m_numElements = 0;
	}

	void C_TestPhysics::SetUpTriggers()
	{
		RemoveTriggers();
		Gravity::C_TriggerSystem* pTriggerSystem = this->m_pScene->GetTriggerSystem();
		pTriggerSystem->DestroyLayers();
		Gravity::C_TriggerLayer* pLayer = pTriggerSystem->CreateLayer(m_pScene->m_WorldBounds.m_min,
			m_pScene->m_WorldBounds.m_max, Maths::HVec3(20.f, 20.f, 20.f), 10);

		pTriggerSystem->SetCallback(&m_TriggerCallback);

		m_pLayer = pLayer;


		Maths::HVec3 position(0.f, 0.f, 0.f);
		F32 rotY = 0.f;
		for(U32 a = 0; a < 4; ++a)
		{
			Gravity::C_Trigger* pTrigger = new Gravity::C_Trigger;
			/*Gravity::C_TriggerBox* pBox = new Gravity::C_TriggerBox;
			pTrigger->m_pTriggerShape = pBox;
			pBox->m_Box.e = Maths::HVec3(2.f, 2.f, 2.f);
			pBox->m_Filter = this->m_Filter;*/

			Gravity::C_BoxDesc boxDesc;
			boxDesc.m_HalfExtents = Maths::HVec3(2.f, 2.f, 2.f);
			pTrigger->m_pTriggerShape = boxDesc.CreateShape(&m_Filter);
			pLayer->AddTrigger(pTrigger);

			position = position + ((a %2 == 0) ? Maths::HVec3(-5.f, 0.f, 0.f) : Maths::HVec3(0.f, 0.f, 5.f));
			Maths::RotY rotation(rotY);
			rotY = rotY + 0.1f;
			if(rotY > 3.14 * 2.f)
				rotY = 0.f;
			Maths::HMat43 trans(Maths::HMat33(rotation), position);
			pLayer->SetTriggerMatrix(pTrigger, trans, stack);
		}
	}


	void C_TestPhysics::SetUpWall(const U32 height, const U32 width, Collision::C_CollisionFilter* pFilter)
	{
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(0.3f, 0.18f, 0.3f);
		//const Maths::HF32 dis(0.2f);
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HF32 two_plus_epsilon(two + Maths::Constants<Maths::HF32>::Quarter());
		
		C_Body* body;
	

		F32 x_base(13.f);
		F32 z(-0.3f);
		F32 y(-0.1f);
		for(U32 i=0; i<height; ++i)
		{
			y+=two * extents.Y();
			F32 x = (i%2)? zero:extents.X();
			x += x_base;
		
			for(U32 j=0; j<width; ++j)
			{
				const Maths::HVec3 pos(x, y, z);
				body=AddBox(pos, orientation, extents, 1.f, pFilter);
				//body->Deactivate();
				x += extents.X() * two_plus_epsilon;
			}
		}
	}

	void C_TestPhysics::SetUpCompositeShapes(const U32 numComposites, Collision::C_CollisionFilter* pFilter)
	{
		Maths::RotZ xrot(Maths::Constants<F32>::Pi() * Maths::Constants<F32>::Half());
		Maths::HMat33 rot;
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Two());
		//const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());
		F32 up = 1.f;
		for(U32 a = 0; a < 1; ++a)
		{
			//F32 fpos[3] = {random.SignedFRand(100.f), random.FRand(30.f, 60.f), random.SignedFRand(100.f)};
			F32 fpos[3] = {13.f, up, -0.3f};
			up += 0.75f;
			Maths::HVec3 pos(fpos[0], fpos[1], fpos[2]);
			Gravity::C_Body* pBody = AddCompositeShape(pos, orientation, extents, pFilter);
			//pBody->ApplyForce(Maths::HVec3 (140.f, 800.f, 75.f), pBody->Position() + Maths::HVec3(3.f, 4.f, 0.2f));
		}
	}

	
	void C_TestPhysics::SetUpFLuid(const Maths::HVec3 PCREF height, Collision::C_CollisionFilter* pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.001f;
		deactivator.m_SleepThresholdSq = 0.0005f;
		C_Body* fBody(m_pScene->CreateBody());
		fBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_FluidDesc fDesc;

		Gravity::C_BodyHelper fHelper(&fDesc);
		Maths::HF32 mass(0.f);
		Maths::HVec3 position(5.f, 2.f, 3.f);
		Maths::HMat33 orientation=Maths::HMat33(Maths::ZERO_MATRIX);
		fHelper.CreateBody(mass, position, orientation, fBody, pFilter, 0, 0);
		fBody->SetDeactivator(deactivator);
		m_pScene->AddFluidBody(fBody);
		//m_pScene->AddBody(cylinderBody);
	}

	Gravity::C_Body* C_TestPhysics::AddChair(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position)
	{
		Maths::HMat33 rot;
		rot.SetIdentity();

		Gravity::C_CompositeDesc desc;
		Gravity::C_BoxDesc leg;
		leg.m_HalfExtents = Maths::HVec3(0.05f, 0.25f, 0.05f);
		leg.m_Epsilon = 0.0001f;
		leg.m_Margin = 0.02f;
		leg.m_Mass = 1.f;

		Gravity::C_BoxDesc base;
		base.m_HalfExtents = Maths::HVec3(0.25f, 0.05f, 0.25f);
		base.m_Epsilon = 0.0001f;
		base.m_Margin = 0.02f;
		base.m_Mass = 2.f;

		Gravity::C_BoxDesc back;
		back.m_HalfExtents = Maths::HVec3(0.25f, 0.25f, 0.05f);
		back.m_Epsilon = 0.0001f;
		back.m_Margin = 0.02f;
		back.m_Mass = 2.f;

		Maths::HMat43 relative;
		relative.SetIdentity();

		desc.AddShape(&base, relative, base.m_Mass, pFilter);
		desc.AddShape(&leg, Maths::HMat43(rot, Maths::HVec3(-0.2f, -0.3f, -0.2f)), leg.m_Mass, pFilter); 
		desc.AddShape(&leg, Maths::HMat43(rot, Maths::HVec3(0.2f, -0.3f, -0.2f)), leg.m_Mass, pFilter);
		desc.AddShape(&leg, Maths::HMat43(rot, Maths::HVec3(-0.2f, -0.3f, 0.2f)), leg.m_Mass, pFilter);
		desc.AddShape(&leg, Maths::HMat43(rot, Maths::HVec3(0.2f, -0.3f, 0.2f)), leg.m_Mass, pFilter);
		desc.AddShape(&back, Maths::HMat43(rot, Maths::HVec3(0.f, 0.3f, 0.2f)), back.m_Mass, pFilter);

		desc.RemapShape();
		desc.SetRemapped();

		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.0005f;
		deactivator.m_SleepThresholdSq = 0.0001f;
		C_Body* boxBody(m_pScene->CreateBody());
		boxBody->SetExternalConstants(&m_ExternalConstants);

		Gravity::C_BodyHelper bodyHelper(&desc);
		bodyHelper.CreateBody(8.f, position, rot, boxBody, pFilter, 1, 0);
		boxBody->SetDeactivator(deactivator);
		m_pScene->AddBody(boxBody);
		boxBody->Slop(0.000005f);
		//boxBody->Baumgarte(0.8f);
		boxBody->SetBuoyancyCoefficient(10.f);
		//boxBody->SetAngularDampingCoefficient(Maths::HF32(0.99f));
		//boxBody->SetLinearDampingCoefficient(Maths::HF32(0.99f));
		return boxBody;
	}

	void C_TestPhysics::SetUpConvexHull(Collision::C_CollisionFilter* pFilter)
	{
		Maths::RotZ xrot(Maths::Constants<F32>::Pi() * Maths::Constants<F32>::Half());
		Maths::HMat33 rot;
		Maths::C_Quaternion orientation(rot);
		Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());
		F32 up = 2.f;
		for(U32 a = 0; a < 3; ++a)
		{
			//F32 fpos[3] = {random.SignedFRand(100.f), random.FRand(30.f, 60.f), random.SignedFRand(100.f)};
			F32 fpos[3] = {13.f, up, -0.3f};
			up += 0.5f;
			Maths::HVec3 pos(fpos[0], fpos[1], fpos[2]);
			Gravity::C_Body* pBody = AddConvexHullShape(pos, orientation, mass, pFilter);
		}
		//pBody->ApplyForce(Maths::HVec3 (500.f, 800.f, 75.f), pBody->Position() + Maths::HVec3(3.f, 4.f, 5.f));
	}

	void C_TestPhysics::SetUpSphere(Collision::C_CollisionFilter* pFilter)
	{
		//Maths::RotZ xrot(Maths::Constants<F32>::Pi() * Maths::Constants<F32>::Half());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());
		F32 up = 1.f;
		const Maths::HF32 radius(0.5f);
		for(U32 a = 0; a < 1; ++a)
		{
			//F32 fpos[3] = {random.SignedFRand(100.f), random.FRand(30.f, 60.f), random.SignedFRand(100.f)};
			F32 fpos[3] = {13.f, up, -0.3f};
			up += 0.5f;
			Maths::HVec3 pos(fpos[0], fpos[1], fpos[2]);
			Gravity::C_Body* pBody = AddSphere(pos, radius, const_cast<Maths::HF32&>(mass), pFilter);
		}
		//pBody->ApplyForce(Maths::HVec3 (500.f, 800.f, 75.f), pBody->Position() + Maths::HVec3(3.f, 4.f, 5.f));
	}

	void C_TestPhysics::SetUpCapsule(Collision::C_CollisionFilter* pFilter)
	{

		
		//Maths::RotZ xrot(Maths::Constants<F32>::Pi() * Maths::Constants<F32>::Half());
		Maths::HMat33 rot;
		Maths::C_Quaternion orientation(rot);
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());
		F32 up = 1.f;
		const Maths::HF32 height(0.5f);
		const Maths::HF32 radius(0.2f);
		for(U32 a = 0; a < 1; ++a)
		{
			//F32 fpos[3] = {random.SignedFRand(100.f), random.FRand(30.f, 60.f), random.SignedFRand(100.f)};
			F32 fpos[3] = {13.f, up, -0.3f};
			up += 0.5f;
			Maths::HVec3 pos(fpos[0], fpos[1], fpos[2]);
			Gravity::C_Body* pBody = AddCapsule(pos, orientation, height, radius, mass, pFilter);
		}
		//pBody->ApplyForce(Maths::HVec3 (500.f, 800.f, 75.f), pBody->Position() + Maths::HVec3(3.f, 4.f, 5.f));
	}

	void C_TestPhysics::SetUpCylinder(Collision::C_CollisionFilter* pFilter)
	{

		
		//Maths::RotZ xrot(Maths::Constants<F32>::Pi() * Maths::Constants<F32>::Half());
		Maths::HMat33 rot;
		Maths::C_Quaternion orientation(rot);
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());
		F32 up = 1.f;
		const Maths::HF32 height(0.5f);
		const Maths::HF32 radius(0.2f);
		for(U32 a = 0; a < 1; ++a)
		{
			//F32 fpos[3] = {random.SignedFRand(100.f), random.FRand(30.f, 60.f), random.SignedFRand(100.f)};
			F32 fpos[3] = {13.f, up, -0.3f};
			up += 0.5f;
			Maths::HVec3 pos(fpos[0], fpos[1], fpos[2]);
			Gravity::C_Body* pBody = AddCylinder(pos, orientation, height, radius, mass, pFilter);
		}
		//pBody->ApplyForce(Maths::HVec3 (500.f, 800.f, 75.f), pBody->Position() + Maths::HVec3(3.f, 4.f, 5.f));
	}

	

	void C_TestPhysics::SetUpRandomBoxes(const U32 numBoxes, Collision::C_CollisionFilter* pFilter)
	{
		
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Two());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());
		static F32 up = 10.f;
		for(U32 a = 0; a < numBoxes; ++a)
		{
			//F32 fpos[3] = {random.SignedFRand(100.f), random.FRand(30.f, 60.f), random.SignedFRand(100.f)};
			F32 fpos[3] = {30.f, up, 30.f};
			up += 4.f;
			Maths::HVec3 pos(fpos[0], fpos[1], fpos[2]);
			Gravity::C_Body* pBody = AddBox(pos, orientation, extents, mass, pFilter);
		}
	}

	void C_TestPhysics::SetUpBridge(const Maths::HVec3 PCREF startPoint, const Maths::C_Normal PCREF layDirection, const Maths::HF32 PCREF layGap, const Maths::HF32 PCREF slop, const Maths::HVec3 PCREF plankExtents, const U32 numPlanks,
		Collision::C_CollisionFilter* pFilter)
	{
		Maths::HVec3 position(startPoint);
		//First and last plank are static!
		Maths::HMat33 orientation;
		orientation.SetIdentity();
		Maths::HF32 mass(2.f);
		Gravity::C_Body* firstBody(AddBox(position, orientation, plankExtents, mass, pFilter));
		firstBody->MakeStatic();
		Gravity::SphericalJointDescriptor desc;
		//Gravity::RevoluteJointDescriptor< Sys::BinaryDeserialiser<Ut::BigEndianToNative > > desc;
		//desc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		//m_pScene->AddBody(firstBody);
		position += layDirection * layGap;
		Gravity::C_Body* nextBody = AddBox(position, orientation, plankExtents, mass, pFilter);
		desc.m_AnchorPoint1 = Maths::HVec3(-plankExtents.X() - slop, 0.f, -plankExtents.Z());
		desc.m_AnchorPoint2 = Maths::HVec3(plankExtents.X() + slop, 0.f, -plankExtents.Z());
		//F32 baumgarte(0.2f);
		SharedPhysics::C_Constraint* constraint = desc.CreateConstraint(*m_pScene->m_pGraphManager, nextBody, firstBody);
		//constraint->SetBaumgarte(baumgarte);
		desc.m_AnchorPoint1 = Maths::HVec3(-plankExtents.X() - slop, 0.f, plankExtents.Z());
		desc.m_AnchorPoint2 = Maths::HVec3(plankExtents.X() + slop, 0.f, plankExtents.Z());
		constraint = desc.CreateConstraint(*m_pScene->m_pGraphManager, nextBody, firstBody);
		//constraint->SetBaumgarte(baumgarte);
		firstBody = nextBody;

		bool bFlip = true;

		for(U32 a = 2; a < numPlanks; ++a)
		{
			position += layDirection * layGap;
			nextBody = AddBox(position, orientation, plankExtents, mass, pFilter);
			if(bFlip)
			{
				desc.m_AnchorPoint2 = Maths::HVec3(-plankExtents.X() - slop, 0.f, plankExtents.Z());
				desc.m_AnchorPoint1 = Maths::HVec3(plankExtents.X() + slop, 0.f, plankExtents.Z());
				constraint = desc.CreateConstraint(*m_pScene->m_pGraphManager, firstBody, nextBody);
				//constraint->SetBaumgarte(baumgarte);
				desc.m_AnchorPoint2 = Maths::HVec3(-plankExtents.X() - slop, 0.f, -plankExtents.Z());
				desc.m_AnchorPoint1 = Maths::HVec3(plankExtents.X() + slop, 0.f, -plankExtents.Z());
				constraint = desc.CreateConstraint(*m_pScene->m_pGraphManager, firstBody, nextBody);
				//constraint->SetBaumgarte(baumgarte);
			}
			else
			{
				desc.m_AnchorPoint2 = Maths::HVec3(-plankExtents.X() - slop, 0.f, -plankExtents.Z());
				desc.m_AnchorPoint1 = Maths::HVec3(plankExtents.X() + slop, 0.f, -plankExtents.Z());
				constraint = desc.CreateConstraint(*m_pScene->m_pGraphManager, firstBody, nextBody);
				//constraint->SetBaumgarte(baumgarte);
				desc.m_AnchorPoint2 = Maths::HVec3(-plankExtents.X() - slop, 0.f, plankExtents.Z());
				desc.m_AnchorPoint1 = Maths::HVec3(plankExtents.X() + slop, 0.f, plankExtents.Z());
				constraint = desc.CreateConstraint(*m_pScene->m_pGraphManager, firstBody, nextBody);
				//constraint->SetBaumgarte(baumgarte);
			}
			firstBody = nextBody;
			bFlip = !bFlip;
		}
		firstBody->MakeStatic();
	}

	void C_TestPhysics::SetUpSphericalJointLimit(Collision::C_CollisionFilter* pFilter)
	{
		
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientationA(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Maths::RotX rotX(30.f);
		Maths::HMat33 rotB(rotX);
		Maths::C_Quaternion orientationB(rotB);

		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(15.f, 2.5f, 0.f), orientationA, extents, mass, pFilter));
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(15.f, 2.f, 0.f), orientationA, extents, mass, pFilter));
		pBoxA->MakeStatic();
		//pBoxB->MakeStatic();

		pBoxA->GetShape()->m_Filter.m_GroupId = 1;
		pBoxB->GetShape()->m_Filter.m_GroupId = 1;
		pBoxA->GetShape()->m_Filter.m_GroupMask = 0;


		/*TwoRotationalProxyDescriptor desc;
		desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.3f, 0.f), Maths::HVec3(-0.25f, 0.3f, -0.25f));
		desc.DefineJointAxis(Maths::HVec3(0.f, -1.f, 0.f), Maths::HVec3(0.f, -1.f, 0.f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(constraint);*/



		/*SphericalJointLimitDescriptor descLimit;
		descLimit.DefineAnchorPoints(Maths::HVec3(0.f, -0.3f, 0.f), Maths::HVec3(-0.25f, 0.3f, -0.25f) );
		descLimit.DefineLimits(Maths::HVec3(-360.f, 0.f, -360.f), Maths::HVec3(360.f, 0.f, 360.f));
		SharedPhysics::C_Constraint* constraint = descLimit.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(constraint);*/


		SphericalJointLimitDescriptor descLimit;
		descLimit.DefineAnchorPoints(Maths::HVec3(0.0f, 0.5f, 0.0f), Maths::HVec3(-0.0f, -0.5f, -0.0f) );
		descLimit.DefineLimits(Maths::HVec3(0.f, -360.f, 0.f), Maths::HVec3(0.f, 360.f, 0.f));
		SharedPhysics::C_Constraint* constraint = descLimit.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		m_pScene->AddConstraint(constraint);

		/*pBoxA->MakeKinematic();
		this->m_KinematicController = Gravity::C_KinematicController(pBoxA);
		m_UseController = true;
		m_pScene->AddKinematicController(&m_KinematicController);*/
	}


	void C_TestPhysics::SetUpRagdollArm(Collision::C_CollisionFilter* pFilter)
	{
		
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientationA(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Half());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		const Maths::HF32 twoPi(Maths::Constants<Maths::HF32>::Pi() * Maths::Constants<Maths::HF32>::Two());
		Maths::RotX rotX((30.f/360.f)*twoPi);
		Maths::HMat33 rotMat(rotX);
		Maths::C_Quaternion orientationQuat(rotMat);

		Maths::HMat33 rotA(rotMat);
		Maths::HMat33 rotB(rot);
		Maths::HMat33 rotC(rot);
		Maths::HMat33 rotD(rot);


		

		Gravity::C_Body* pBoxD(AddBox(Maths::HVec3(15.f, 6.f, 3.3f), rotD, extents, mass, pFilter));
		Gravity::C_Body* pBoxC(AddBox(Maths::HVec3(15.f, 6.f, 2.2f), rotC, extents, mass, pFilter));
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(15.f, 6.f, 1.1f), rotB, extents, mass, pFilter));
		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(15.f, 6.f, 0.f), rotA, extents, mass, pFilter));
		
		
		
		pBoxA->MakeStatic();
		//pBoxB->MakeStatic();
		//pBoxC->MakeStatic();

		pBoxA->GetShape()->m_Filter.m_GroupId = 1;
		pBoxB->GetShape()->m_Filter.m_GroupId = 1;
		pBoxC->GetShape()->m_Filter.m_GroupId = 1;
		
		pBoxA->GetShape()->m_Filter.m_GroupMask = 5;
		pBoxA->GetShape()->m_Filter.m_GroupFlags = 1;

		pBoxB->GetShape()->m_Filter.m_GroupMask = 2;
		pBoxB->GetShape()->m_Filter.m_GroupFlags = 2;

		pBoxC->GetShape()->m_Filter.m_GroupMask = 5;
		pBoxC->GetShape()->m_Filter.m_GroupFlags = 4;

		
		SharedPhysics::C_Constraint* constraint;


		/*TwoRotationalProxyDescriptor desc;
		desc.DefineAnchorPoints(Maths::HVec3(0.0f, 0.0f, -1.0f), Maths::HVec3(-0.0f, -0.0f, 1.0f));
		desc.DefineJointAxis(rotB * Maths::HVec3(0.f, -1.f, 0.f), rotA * Maths::HVec3(0.f, -1.f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		m_pScene->AddConstraint(constraint);*/

		SphericalJointLimitDescriptor descLimit;
		descLimit.DefineAnchorPoints(Maths::HVec3(0.0f, 0.0f, -1.0f), Maths::HVec3(-0.0f, -0.0f, 1.0f) );
		descLimit.DefineLimits( Maths::HVec3(-40.f, -20.f, -15.f), Maths::HVec3(40.f, 20.f, 65.f));
		constraint = descLimit.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		m_pScene->AddConstraint(constraint);

		/*RevoluteJointLimitDescriptor revoluteLimit1;
		revoluteLimit1.DefineAnchorPoints(Maths::HVec3(0.0f, 0.0f, -0.55f), Maths::HVec3(-0.0f, -0.0f, 0.55f));
		revoluteLimit1.DefineJointAxis(rotB * Maths::HVec3(0.f, 1.f, 0.f), rotA * Maths::HVec3(0.f, 1.f, 0.f));
		revoluteLimit1.DefineJointLimit(Maths::HF32(0.f), Maths::HF32(150.f));
		constraint = revoluteLimit1.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		m_pScene->AddConstraint(constraint);*/

		//RevoluteJointDescriptor revoluteLimit1;
		//revoluteLimit1.DefineAnchorPoints(Maths::HVec3(0.0f, 0.0f, -0.55f), Maths::HVec3(-0.0f, -0.0f, 0.55f));
		//revoluteLimit1.DefineJointAxis(rotB * Maths::HVec3(0.f, 1.f, 0.f), rotA * Maths::HVec3(0.f, 1.f, 0.f));
		////revoluteLimit1.DefineJointLimit(Maths::HF32(0.f), Maths::HF32(150.f));
		//constraint = revoluteLimit1.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		//m_pScene->AddConstraint(constraint);

		RevoluteJointLimitDescriptor revoluteLimit;
		revoluteLimit.DefineAnchorPoints(Maths::HVec3(0.0f, 0.0f, 0.55f), Maths::HVec3(-0.0f, -0.0f, -0.55f));
		revoluteLimit.DefineJointAxis(rotB * Maths::HVec3(0.f, 1.f, 0.f), rotC*Maths::HVec3(0.f, 1.f, 0.f));
		revoluteLimit.DefineJointLimit(Maths::HF32(0.f), Maths::HF32(90.f));
		constraint = revoluteLimit.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxC);
		m_pScene->AddConstraint(constraint);

		//RevoluteJointDescriptor revoluteLimit;
		//revoluteLimit.DefineAnchorPoints(Maths::HVec3(0.0f, 0.0f, 0.55f), Maths::HVec3(-0.0f, -0.0f, -0.55f));
		//revoluteLimit.DefineJointAxis(rotB * Maths::HVec3(0.f, 1.f, 0.f), rotC * Maths::HVec3(0.f, 1.f, 0.f));
		////revoluteLimit.DefineJointLimit(Maths::HF32(0.f), Maths::HF32(150.f));
		//constraint = revoluteLimit.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxC);
		//m_pScene->AddConstraint(constraint);

		Gravity::FixedJointDescriptor fix;
		fix.DefineAnchorPoints(Maths::HVec3(0.0f, 0.0f, 0.55f), Maths::HVec3(-0.0f, -0.0f, -0.55f));
		constraint = fix.CreateConstraint(*m_pScene->m_pGraphManager, pBoxC, pBoxD);
		m_pScene->AddConstraint(constraint);

	}

	void C_TestPhysics::SetUpSphericalJoint(Collision::C_CollisionFilter* pFilter)
	{
		
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Maths::HVec3 positionA(15.f, 3.f, 0.f);
		Maths::HVec3 positionB(15.f, 2.f, 0.f);
		Gravity::C_Body* pBoxA(AddBox(positionA, orientation, extents, mass, pFilter));
		Gravity::C_Body* pBoxB(AddBox(positionB, orientation, extents, mass, pFilter));


		Maths::HVec3 worldAnchorPoint(15.f, 2.5f, 0.f);

		Maths::HVec3 anchorPointA = (worldAnchorPoint - positionA) * rot.InverseRotation();
		Maths::HVec3 anchorPointB = (worldAnchorPoint - positionB) * rot.InverseRotation();

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(worldAnchorPoint, 0.5f, Maths::RGBA(0, 255, 0));
		
		//pBoxA->MakeStatic();
		SphericalJointDescriptor desc;
		//desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.3f, 0.f), Maths::HVec3(-0.25f, 0.3f, -0.25f));
		desc.DefineAnchorPoints(anchorPointA, anchorPointB);
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);

		pBoxA->MakeStatic();
		//constraint->SetBaumgarte(0.2f);
		//constraint->SetScale(0.f);

		//m_Constraints.AddToEnd(&constraint);
//		m_pScene->m_ConstraintsArray(&constraint);
		m_pScene->AddConstraint(constraint);
		

		//Gravity::C_Body* pBoxC(AddBox(Maths::HVec3(15.f, 1.5f, 0.f), orientation, extents, mass, pFilter));
		//
		//desc.DefineAnchorPoints(Maths::HVec3(0.25f, -0.3f, 0.25f), Maths::HVec3(-0.25f, 0.3f, -0.25f) );
		//constraint = desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxC);
		//constraint->SetBaumgarte(0.2f);

		//m_pScene->AddConstraint(constraint);

		//pBoxA->MakeStatic();

	/*	pBoxA->MakeKinematic();
		this->m_KinematicController = Gravity::C_KinematicController(pBoxA);
		m_UseController = true;
		m_pScene->AddKinematicController(&m_KinematicController);*/
	}

	void C_TestPhysics::SetUpFixedDemo(Collision::C_CollisionFilter* pFilter)
	{
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(13.f, 1.f, 0.f), orientation, extents, mass, pFilter));
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(13.f, 1.f, 0.6f), orientation, extents, mass, pFilter));

		/*pBoxA->MakeStatic();
		pBoxB->MakeStatic();*/
		//Gravity::C_Body* pBoxC(AddBox(Maths::HVec3(13.f, 1.f, 2.f), orientation, extents, mass, pFilter));

		FixedJointDescriptor desc;
		desc.DefineAnchorPoints(Maths::HVec3(0.f, 0.0f, 0.3f), Maths::HVec3(0.f, 0.f, -0.3f));
		//desc.DefineAnchorPoints(Maths::HVec3(0.f, 0.0f, 0.5f), Maths::HVec3(0.f, 0.f, -0.5f));
		//desc.DefineAnchorPoints(Maths::HVec3(-0.0f, -0.25f, 0.25f), Maths::HVec3(0.f, 0.25f, 0.25f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(constraint);

		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(Maths::HVec3(13.f, 1.f, 0.f) + Maths::HVec3(0.f, 0.0f, 0.3f)*orientation, 0.5f, Maths::RGBA(255, 0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(Maths::HVec3(13.f, 1.f, 0.6f) + Maths::HVec3(0.f, 0.0f, -0.3f)*orientation, 0.5f, Maths::RGBA(0, 255, 0, 0));

		//SphericalJointDescriptor sDesc;
		////desc.DefineJointAxis(Maths::HVec3(0.f, 1.f, 0.f), Maths::HVec3(0.f, 1.f, 0.f));
		//sDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.0f, 0.5f), Maths::HVec3(0.f, 0.f, -0.5f));
		////desc.DefineAnchorPoints(Maths::HVec3(-0.0f, -0.25f, 0.25f), Maths::HVec3(0.f, 0.25f, 0.25f));
		//constraint=sDesc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxC);

		////m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);

		/*pBoxB->MakeKinematic();
		m_KinematicController = Gravity::C_KinematicController(pBoxB);
		m_UseController = true;
		m_pScene->AddKinematicController(&m_KinematicController);*/
	}

	void C_TestPhysics::SetUpRopeDemo(Collision::C_CollisionFilter* pFilter)
	{
		Gravity::C_RopeDesc ropeDesc;
		ropeDesc.m_RestLength = 0.08f;
		Maths::HF32 offset(0.06f);

		Maths::HVec3 position(0.f, 10.f, 0.f);

		Gravity::C_DescriptorParticle particle;
		particle.m_Position = position;
		particle.m_Mass = 0.f;
		ropeDesc.m_Vertices.AddToEnd(&particle);

		for(S32 a = 0; a < 50; ++a)
		{
			position.X(position.X() + offset);
			
			particle.m_Position = position;
			particle.m_Mass = 1.f;
			ropeDesc.m_Vertices.AddToEnd(&particle);
			
		}

		position.X(position.X() + offset);
		particle.m_Position = position;
		particle.m_Mass = 1.f;
		ropeDesc.m_Vertices.AddToEnd(&particle);

		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = -1.f;
		deactivator.m_SleepThresholdSq = -1.f;
		C_Body* ropeBody(m_pScene->CreateBody());
		
		Maths::HMat33 orientation;
		orientation.SetIdentity();

		Gravity::C_BodyHelper bodyHelper(&ropeDesc);
		bodyHelper.CreateBody(1.f, position, orientation, ropeBody, pFilter, 0, 0);
		ropeBody->SetDeactivator(deactivator);
		ropeBody->Baumgarte(0.75f);
		
		m_pScene->AddRopeBody(ropeBody);
		ropeBody->SetExternalConstants(&m_ExternalConstants);

		Collision::C_RopeShape* pRopeShape = (Collision::C_RopeShape*)ropeBody->GetShape();
		pRopeShape->m_Rope.m_pExternalConstants = &m_ExternalConstants;
		pRopeShape->m_Rope.SetDampingCoefficent(0.99f);
		ropeBody->Activate();

	}


	void C_TestPhysics::SetUpTwoRotationalDemo(Collision::C_CollisionFilter* pFilter)
	{

		using namespace Maths;

		HMat33 rot;
		rot.SetIdentity();
		C_Quaternion orientation(rot);
		const HVec3 extents(Constants<HF32>::Quarter(), Constants<HF32>::Quarter(), Constants<HF32>::Quarter());
		const HF32 mass(Constants<HF32>::Three());

		Gravity::C_Body* pBoxA(AddBox(HVec3(15.f, 2.5f, 0.f), orientation, extents, mass, pFilter));
		Gravity::C_Body* pBoxB(AddBox(HVec3(15.f, 2.f, 0.f), orientation, extents, mass, pFilter));

		//pBoxA->MakeStatic();
	//	pBoxB->MakeStatic();
		
		TwoRotationalProxyDescriptor desc;
		desc.DefineAnchorPoints(HVec3(0.f, -0.3f, 0.f), HVec3(-0.25f, 0.3f, -0.25f));
		desc.DefineJointAxis(HVec3(0.f, -1.f, 0.f), HVec3(0.f, -1.f, 0.f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(constraint);

		/*RevoluteJointDescriptor rDesc;
		rDesc.DefineAnchorPoints(HVec3(0.f, -0.3f, 0.f), HVec3(-0.25f, 0.3f, -0.25f));
		rDesc.DefineJointAxis(HVec3(0.f, -1.f, 0.f), HVec3(0.f, -1.f, 0.f));
		SharedPhysics::C_Constraint* rConstraint=rDesc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(rConstraint);*/

		/*DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(HVec3(15.f, 2.5f, 0.f) + HVec3(0.f, -0.3f, 0.f),HVec3(0.f, 0.f, 1.f), 2.f, 0.2f, RGBA(0, 0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(HVec3(15.f, 2.5f, 0.f) + HVec3(0.f, -0.3f, 0.f), 0.5f, RGBA(0, 255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(HVec3(15.f, 2.f, 0.f) + HVec3(-0.25f, 0.3f, -0.25f), 0.5f, RGBA(0, 255, 0, 0));*/
		//constraint->SetScale(0.f);

	}

	//void C_TestPhysics::SetUpAngularLimitDemo(Collision::C_CollisionFilter* pFilter)
	//{

	//	Maths::HMat33 rot;
	//	rot.SetIdentity();
	//	Maths::C_Quaternion orientation(rot);
	//	const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
	//	const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

	//	Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(13.f, 1.f, 0.f), orientation, Maths::HVec3(0.5f, 0.25f, 0.25f), mass, pFilter));
	//	Gravity::C_Body* pWorld(m_pScene->GetStaticWorldBody());

	//	//Gravity::C_Body* pBoxC(AddBox(Maths::HVec3(13.f, 1.f, 2.f), orientation, extents, mass, pFilter));

	//	//SphericalJointDescriptor desc;
	//	AngularLimitDescriptor desc;
	//	//desc.DefineJointAxis(Maths::HVec3(0.f, 1.f, 0.f), Maths::HVec3(0.f, 1.f, 0.f));
	//	desc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f));
	//	desc.DefineAngularLimits(Maths::HF32(-10.f), Maths::HF32(10.f));

	//	SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pWorld);
	//	
	//	m_pScene->AddConstraint(constraint);

	//}


	void C_TestPhysics::SetUpRevoluteJoint(Collision::C_CollisionFilter* pFilter)
	{
		
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(13.f, 1.f, 0.f), orientation, extents, mass*10.f, pFilter));
		//pBoxA->MakeStatic();
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(13.5f, 1.f, 0.f), orientation, extents, mass, pFilter));
		//Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(13.5f, 1.f, 0.f), orientation, extents, Maths::HF32(100.f), pFilter));
		//pBoxB->MakeStatic();
		
		RevoluteJointDescriptor desc;
		desc.DefineJointAxis(Maths::HVec3(0.f, 1.f, 0.f), Maths::HVec3(0.f, 1.f, 0.f));
		desc.DefineAnchorPoints(Maths::HVec3(0.25f, 0.0f, 0.25f), Maths::HVec3(-0.25f, 0.f, 0.25f));
		//desc.DefineAnchorPoints(Maths::HVec3(-0.0f, -0.25f, 0.25f), Maths::HVec3(0.f, 0.25f, 0.25f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(constraint);
		pBoxB->ApplyForce(Maths::HVec3(0, 0, 500.f), pBoxB->Position());


		/*pBoxA->MakeKinematic();
		m_KinematicController = Gravity::C_KinematicController(pBoxA);
		m_UseController = true;
		m_pScene->AddKinematicController(&m_KinematicController);*/
		

		/*Gravity::C_Body* pBoxC(AddBox(Maths::HVec3(0.f, 7.f, 0.f), orientation, extents, mass*3.f, pFilter));
		
		desc.DefineAnchorPoints(Maths::HVec3(0.5f, -0.65f, 0.5f), Maths::HVec3(-0.5f, 0.65f, -0.5f));
		desc.CreateConstraint(m_pScene->m_GraphManager, pBoxB, pBoxC);

		pBoxC->ApplyForce(Maths::HVec3(300.f, 0.f, 300.f), Maths::HVec3(0.f, 6.5f, 0.f));*/

	}

	void C_TestPhysics::SetUpRevoluteJointLimit(Collision::C_CollisionFilter* pFilter)
	{

		Maths::RotX tempRot(Maths::Constants<Maths::HF32>::Pi() * 0.4f);
		Maths::HMat33 rot(tempRot);
		//rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(13.f, 1.f, 0.f), orientation, extents, mass, pFilter));
		pBoxA->MakeStatic();
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(13.5f, 1.f, 0.f), orientation, extents, mass, pFilter));

		/*pBoxA->GetShape()->m_Filter.m_GroupId = 1;
		pBoxB->GetShape()->m_Filter.m_GroupId = 1;*/
		pBoxA->GetShape()->m_Filter.m_GroupMask = 0;

		//pBoxA->IBodyMatrix(pBoxA->IBodyMatrix() * 50.f);
		//pBoxB->MakeStatic();

		//pBoxA->IBodyMatrix(pBoxA->IBodyMatrix() * 50.f);

		//pBoxA->MakeKinematic();
		RevoluteJointLimitDescriptor desc;
		desc.DefineJointAxis(Maths::HVec3(0.f, 0.f, -1.f), Maths::HVec3(0.f, 0.f, -1.f));
		desc.DefineJointLimit(-0.f, 0.f);
		//desc.DefineJointLimit(-360.f, 360.f);
		desc.DefineAnchorPoints(Maths::HVec3(-0.25f, -0.26f, 0.25f), Maths::HVec3(0.25f, -0.26f, 0.25f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		//constraint->SetBaumgarte(20.f);
		m_pScene->AddConstraint(constraint);

		/*SphericalJointDescriptor sDesc;
		sDesc.DefineAnchorPoints(Maths::HVec3(-0.25f, -0.25f, 0.f), Maths::HVec3(0.25f, -0.25f, 0.f));
		SharedPhysics::C_Constraint* sConstraint=sDesc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		m_pScene->AddConstraint(sConstraint);*/

		/*sDesc.DefineAnchorPoints(Maths::HVec3(-0.25f, -0.25f, -0.25f), Maths::HVec3(0.25f, -0.25f, -0.25f));
		sConstraint=sDesc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		m_pScene->AddConstraint(sConstraint);*/


		
		
		
	/*	SphericalJointDescriptor sDesc;
		sDesc.DefineAnchorPoints(Maths::HVec3(-0.25f, -0.25f, 0.f), Maths::HVec3(0.25f, -0.25f, 0.f));
		constraint=sDesc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		m_pScene->AddConstraint(constraint);*/

		
		//DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(Maths::HVec3(13.f, 1.f, 0.f) + Maths::HVec3(0.25f, -0.25f, 0.f), 0.05f, Maths::RGBA(255, 0, 0));
		//DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(Maths::HVec3(13.5f, 1.f, 0.f)+ Maths::HVec3(-0.25f, -0.25f, 0.f), 0.5f, Maths::RGBA(0, 255, 0));

		
		/*m_KinematicController = Gravity::C_KinematicController(pBoxA);
		m_UseController = true;
		m_pScene->AddKinematicController(&m_KinematicController);
		pBoxB->ApplyForce(Maths::HVec3(0, 0, 500.f), pBoxA->Position());*/
		
	}

	void C_TestPhysics::SetUpPrismaticJoint(Collision::C_CollisionFilter* pFilter)
	{
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents1(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HVec3 extents2(Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(13.f, 1.5f, 0.f), orientation, extents1, mass, pFilter));
		
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(13.f, 1.f, 0.f), orientation, extents2, mass, pFilter));
		/*pBoxB->GetBody().m_ReciprocalMass = Maths::HF32(0);
		pBoxB->GetBody().m_IBody = Maths::Constants<Maths::HMat33>::Zero();
		pBoxB->GetBody().m_IBodyInverse = Maths::Constants<Maths::HMat33>::Zero();
		pBoxB->GetBody().m_InverseInertiaTensor = Maths::Constants<Maths::HMat33>::Zero();;
		pBoxB->GetBody().m_InertiaTensor = Maths::Constants<Maths::HMat33>::Zero();;*/
		//pBoxB->MakeStatic();
		
		
		//pBoxB->MakeStatic();
		/*Gravity::C_RigidBody& rBody = pBoxB->GetBody();
		rBody.m_IBody = rBody.m_IBodyInverse = rBody.m_InertiaTensor = rBody.m_InverseInertiaTensor = Maths::HMat33(Maths::ZERO_MATRIX);
		rBody.m_Mass = 1000.f;
		rBody.m_ReciprocalMass = 0.f;*/

		const Maths::HVec3 axis(1.f, 0.f, 0.f);
		
		PrismaticJointDescriptor desc;
		//desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.5f, 0.f), Maths::HVec3(0.f, 0.5f, 0.f));
		desc.DefineJointAxis(axis, axis);
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(constraint);


		FixedRotationalDescriptor fDesc;
		constraint = fDesc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(constraint);

		/*pBoxA->MakeKinematic();

		m_KinematicController = Gravity::C_KinematicController(pBoxA);
		m_UseController = true;
		m_pScene->AddKinematicController(&m_KinematicController);*/

		//pBoxB->MakeStatic();

		//m_Constraints.AddToEnd(&constraint);
		//pBoxB->ApplyForce(Maths::HVec3(0, 0, 500.f), pBoxB->Position());
		

	}

	void C_TestPhysics::SetUpPrismaticJointLimit(Collision::C_CollisionFilter* pFilter)
	{
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents1(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HVec3 extents2(Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(13.f, 1.5f, 0.f), orientation, extents1, mass, pFilter));
		//pBoxA->MakeStatic();
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(13.f, 1.f, 0.f), orientation, extents2, mass, pFilter));
		//pBoxB->MakeStatic();

		const Maths::HVec3 axis(1.f, 0.f, 0.f);
		const Maths::HF32 lowLimit(-5.5f);
		const Maths::HF32 highLimit(5.5f);
		PrismaticJointLimitDescriptor desc;
		//desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.5f, 0.f), Maths::HVec3(0.f, 0.5f, 0.f));
		desc.DefineJointLimit(-Maths::Constants<Maths::HF32>::Max(), Maths::Constants<Maths::HF32>::Max());
		desc.DefineJointAxis(axis, axis);
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(constraint);

		/*FixedRotationalDescriptor fDesc;
		constraint = fDesc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);
		m_pScene->AddConstraint(constraint);*/

	/*	pBoxA->MakeKinematic();
		m_KinematicController = Gravity::C_KinematicController(pBoxA);
		m_UseController = true;
		m_pScene->AddKinematicController(&m_KinematicController);*/
	}

	void C_TestPhysics::SetUpCar(Collision::C_CollisionFilter* pFilter)
	{
		const Maths::HF32 pi(Maths::Constants<Maths::HF32>::Pi());
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		Maths::HMat33 orientation;

		Maths::HVec3 CarBodyExtents(0.25f, 0.16f, 0.5f);
		Maths::HF32 CarBodyMass(10.f);
		Gravity::C_Body* pCarBody(AddBox(Maths::HVec3(13.f, 2.f, 0.f), orientation, CarBodyExtents , CarBodyMass, pFilter));
		//pCarBody->MakeStatic();
		//Add in the 4 wheel bodies!
		//Maths::RotZ rotz(pi * half);
		//orientation = Maths::HMat33(rotz);
		Maths::HVec3 wheel0OffSet(0.22f, -0.23f, 0.47f);
		Maths::HVec3 wheel1OffSet(-0.22f, -0.23f, 0.47f);
		Maths::HVec3 wheel2OffSet(0.22f, -0.23f, -0.47f);
		Maths::HVec3 wheel3OffSet(-0.22f, -0.23f, -0.47f);

		Maths::HVec3 wheel0Position(pCarBody->Position() + wheel0OffSet);
		Maths::HVec3 wheel1Position(pCarBody->Position() + wheel1OffSet);
		Maths::HVec3 wheel2Position(pCarBody->Position() + wheel2OffSet);
		Maths::HVec3 wheel3Position(pCarBody->Position() + wheel3OffSet);

		Gravity::C_Body* pWheel0(AddSphere(wheel0Position, Maths::HF32(0.05f), Maths::HF32(2.f), pFilter));
		//pWheel0->MakeStatic();
		Gravity::C_Body* pWheel1(AddSphere(wheel1Position,Maths::HF32(0.05f), Maths::HF32(2.f), pFilter));
		//pWheel1->MakeStatic();
		Gravity::C_Body* pWheel2(AddSphere(wheel2Position,Maths::HF32(0.05f), Maths::HF32(2.f), pFilter));
		//pWheel2->MakeStatic()
		Gravity::C_Body* pWheel3(AddSphere(wheel3Position,Maths::HF32(0.05f), Maths::HF32(2.f), pFilter));
		//pWheel3->MakeStatic();

		SharedPhysics::C_Constraint* constraint;
		Gravity::CarWheelDescriptor carWheelDesc;
		carWheelDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(0.f, 1.f, 0.f));
		carWheelDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.f, 0.f), Maths::HVec3(0.22, -0.25f, 0.47f));
		carWheelDesc.DefineOffsetAngle(pi * half);

		constraint = carWheelDesc.CreateConstraint(*m_pScene->m_pGraphManager, pWheel0, pCarBody);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		carWheelDesc.DefineJointAxis(Maths::HVec3(-1.f, 0.f, 0.f), Maths::HVec3(0.f, 1.f, 0.f));
		carWheelDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.f, 0.f), Maths::HVec3(-0.22, -0.25f, 0.47f));
		constraint = carWheelDesc.CreateConstraint(*m_pScene->m_pGraphManager, pWheel1, pCarBody);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);


		//Gravity::RevoluteJointDescriptor sphereDesc;
		Gravity::SphericalJointDescriptor sphereDesc;
		sphereDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.f, 0.f), Maths::HVec3(0.22, -0.25f, -0.47f));
		//sphereDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		constraint = sphereDesc.CreateConstraint(*m_pScene->m_pGraphManager, pWheel2, pCarBody);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		sphereDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.f, 0.f), Maths::HVec3(-0.22, -0.25f, -0.47f));
		constraint = sphereDesc.CreateConstraint(*m_pScene->m_pGraphManager, pWheel3, pCarBody);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);
	}


	void C_TestPhysics::SetUpSpring(Collision::C_CollisionFilter* pFilter)
	{
		using namespace Maths;

		HMat33 rot;
		rot.SetIdentity();
		
		Maths::RotY rotX(Maths::Constants<Maths::HF32>::Pi()/4.f);
		HMat33 rot1(rotX);
		const HVec3 extents(0.5f, 0.5f, 0.5f);
		const HF32 boxMass(Constants<HF32>::One());
		const HVec3 boxAPosition(13.5f, 1.f, 0.f);
		const HVec3 boxBPosition(13.5f, 3.f, 0.f);
		const C_Normal normal(HVec3(0.f, 1.f, 0.f));
		Gravity::C_Body* pBoxA (AddBox(boxAPosition, rot1, extents, boxMass, pFilter));
		Gravity::C_Body* pBoxB (AddBox(boxBPosition, rot, extents, boxMass, pFilter));
		pBoxB->MakeStatic();

		Gravity::SpringDescriptor desc;
		desc.DefineAnchorPoints(rot * HVec3(0.0f, 0.5f, 0.0f), rot * HVec3(0.0f, -0.5f, 0.0f));
		desc.DefineRestLength(1.f);
		desc.DefineSpringCoefficients(HF32(g_dbLinearSpringCoefficient),HF32(g_dbLinearDampingCoefficient),
			HF32(g_dbAngularSpringCoefficient), HF32(g_dbAngularDampingCoefficient));
		
		SharedPhysics::C_Spring* spring = desc.CreateSpring(m_pScene, pBoxA, pBoxB);
		

		m_pScene->AddSpring(spring);

	}

	void C_TestPhysics::SetUpDistanceConstraint(Collision::C_CollisionFilter* pFilter)
	{
		using namespace Maths;
	

		HMat33 rot;
		rot.SetIdentity();
		const HVec3 extents(0.25f, 0.25f, 0.25f);
		const HF32 boxMass(Constants<HF32>::One());
		const HVec3 boxAPosition(0.f, 0.25f, 0.f);
		const HVec3 boxBPosition(13.f, 1.0f, 0.f);
		const HVec3 anchorPoint1(-0.25f, 0.f, 0.f);
		//const HVec3 anchorPoint1(0.f, 0.f, 0.f);
		const HVec3 anchorPoint2(0.0f, 0.25f, 0.f);
		Gravity::C_Body* pBoxA (AddBox(boxAPosition, rot, extents, boxMass, pFilter));
		//Gravity::C_Body* pBoxB (AddBox(boxBPosition, rot, extents, boxMass, pFilter));
		Gravity::C_Body* pWorld(m_pScene->GetStaticWorldBody());

		//pBoxB->MakeStatic();
		//pBoxA->MakeStatic();

		DistanceConstraintDescriptor desc;
		desc.DefineDistanceLimit(HF32(2.f));
		desc.DefineAnchorPoints(anchorPoint1, anchorPoint2); 
		SharedPhysics::C_Constraint* constraint = desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pWorld);
		m_pScene->AddConstraint(constraint);

		
		
	}

	Gravity::C_Body* C_TestPhysics::SetUpRagdoll(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position)
	{
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		//const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Maths::HVec3 heapPosition(Maths::HVec3(0.f, 0.44f, 0.f) + position);

		//Gravity::C_Body* pHead(AddBox(Maths::HVec3(0.f, 1.5f, 0.f) + position, orientation, Maths::HVec3(0.5f, 0.5f, 0.5f), Maths::HF32(2.f), pFilter));
		Gravity::C_Body* pHead(AddSphere(heapPosition, Maths::HF32(0.1f), Maths::HF32(0.5f), pFilter));
		//pHead->MakeStatic();
		
		//Gravity::C_Body* pTorso(AddBox(Maths::HVec3(0.f, 0.f, 0.f) + position, orientation, Maths::HVec3(1.f, 1.f, 1.f), Maths::HF32(5.f), pFilter ));
		Gravity::C_Body* pTorso(AddCapsule(Maths::HVec3(0.f, 0.f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.2f), Maths::HF32(2.f), pFilter ));
		//pTorso->MakeStatic();

		Maths::RotZ rot_90(Maths::Constants<Maths::HF32>::Pi()*Maths::Constants<Maths::HF32>::Half());
		orientation = Maths::C_Quaternion(Maths::HMat33(rot_90));

		//Gravity::C_Body* pHip(AddBox(Maths::HVec3(0.f, -2.f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f,1.f), Maths::HF32(1.f), pFilter));
		Gravity::C_Body* pHip(AddCapsule(Maths::HVec3(0.f, -0.45f, 0.f) + position, orientation, Maths::HF32(0.1f), Maths::HF32(0.08f), Maths::HF32(2.f), pFilter));
		//pHip->MakeStatic();
		
		//Gravity::C_Body* pLeftArmTop(AddBox(Maths::HVec3(-2.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		
		Gravity::C_Body* pLeftArmTop(AddCapsule(Maths::HVec3(-0.41f, 0.2f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pLeftArmTop->MakeStatic();

		//Gravity::C_Body* pLeftArmBottom(AddBox(Maths::HVec3(-4.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pLeftArmBottom(AddCapsule(Maths::HVec3(-0.82f, 0.15f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pLeftArmBottom->MakeStatic();

		//Gravity::C_Body* pRightArmTop(AddBox(Maths::HVec3(2.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pRightArmTop(AddCapsule(Maths::HVec3(0.41f, 0.2f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pRightArmTop->MakeStatic();

		//Gravity::C_Body* pRightArmBottom(AddBox(Maths::HVec3(4.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pRightArmBottom(AddCapsule(Maths::HVec3(0.82f, 0.15f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pRightArmBottom->MakeStatic();

		orientation = Maths::C_Quaternion(rot);

		//Gravity::C_Body* pLeftLegTop(AddBox(Maths::HVec3(-0.5f, -4.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pLeftLegTop(AddCapsule(Maths::HVec3(-0.1f, -0.75f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pLeftLegTop->MakeStatic();

		//Gravity::C_Body* pLeftLegBottom(AddBox(Maths::HVec3(-0.5f, -6.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pLeftLegBottom(AddCapsule(Maths::HVec3(-0.1f, -1.15f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pLeftLegBottom->MakeStatic();

		//Gravity::C_Body* pRightLegTop(AddBox(Maths::HVec3(0.5f, -4.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pRightLegTop(AddCapsule(Maths::HVec3(0.1f, -0.75f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pRightLegTop->MakeStatic();

		//Gravity::C_Body* pRightLegBottom(AddBox(Maths::HVec3(0.5f, -6.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pRightLegBottom(AddCapsule(Maths::HVec3(0.1f, -1.15f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pRightLegBottom->MakeStatic();

		SphericalJointDescriptor desc;
		RevoluteJointDescriptor revDesc;

		desc.DefineAnchorPoints(Maths::HVec3(0.f, 0.32f, 0.f), Maths::HVec3(0.f, -0.12f, 0.f));
		//revDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHead);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		desc.DefineAnchorPoints(Maths::HVec3(-0.21f, 0.15f, 0.f), pLeftArmTop->Orientation() * Maths::HVec3(0.21f, 0.f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pLeftArmTop);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		revDesc.DefineAnchorPoints(pLeftArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f), pLeftArmBottom->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f));
		revDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftArmTop, pLeftArmBottom);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		desc.DefineAnchorPoints(Maths::HVec3(0.21f, 0.15f, 0.f), pRightArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pRightArmTop);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		revDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		revDesc.DefineAnchorPoints(pRightArmTop->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f), pRightArmBottom->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pRightArmTop, pRightArmBottom);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		revDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.31f, 0.f), pHip->Orientation()*Maths::HVec3(0.f, 0.085f, 0.f));
		revDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHip);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		desc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(-0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pLeftLegTop);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		revDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		revDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftLegTop, pLeftLegBottom);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		desc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pRightLegTop);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		revDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		revDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pRightLegTop, pRightLegBottom);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		return pTorso;

	}

	void C_TestPhysics::AnchorPointDraw()
	{
		const U32 size=m_pScene->NumConstraints();

		for(U32 i=0; i< size; ++i)
		{
			SharedPhysics::C_Constraint* constraint=m_pScene->GetConstraint(i);

			switch(constraint->m_ConstraintType)
			{
			case SharedPhysics::C_Constraint::E_Type_Fixed:
				{
#ifndef USE_SOLVER_2
					Solver::C_FixedRotationalConstraint* fix=(Solver::C_FixedRotationalConstraint*) constraint;
#else
					Solver2::C_FixedRotationalConstraint* fix=(Solver2::C_FixedRotationalConstraint*) constraint;
#endif
					SharedPhysics::C_GraphNode* pNodeA=fix->pNodeA;
					SharedPhysics::C_GraphNode* pNodeB=fix->pNodeB;
					SharedPhysics::C_RigidBody* pRigid1=pNodeA->m_pRigidBody;
					SharedPhysics::C_RigidBody* pRigid2=pNodeB->m_pRigidBody;
					/*const Maths::HVec3 point1(pRigid1->m_Position +  (Maths::HVec3(fix->m_AnchorPoint1) * pRigid1->m_OrientationMatrix));
					const Maths::HVec3 point2(pRigid2->m_Position +  (Maths::HVec3(fix->m_AnchorPoint2) * pRigid2->m_OrientationMatrix));


					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point1, 0.05f, Maths::RGBA(0, 255, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point2, 0.05f, Maths::RGBA(255, 0, 0, 255));*/
//					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(point1, Maths::HVec3(fix->m_JointAxis1) * pRigid1->m_OrientationMatrix, 0.5f, 0.025f, Maths::RGBA(0, 255, 0, 255));
//					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(point2, Maths::HVec3(fix->m_JointAxis2) * pRigid2->m_OrientationMatrix, 0.5f, 0.025f, Maths::RGBA(0, 255, 0, 255));

				}
				break;
			/*case SharedPhysics::C_Constraint::E_Type_Spherical:
				{
#ifndef USE_SOLVER_2
					Solver::C_SphericalConstraint* sphericalConstraint=(Solver::C_SphericalConstraint*) constraint;
#else
					Solver2::C_SphericalConstraint* sphericalConstraint=(Solver2::C_SphericalConstraint*) constraint;
#endif
					SharedPhysics::C_GraphNode* pNodeA=sphericalConstraint->pNodeA;
					SharedPhysics::C_GraphNode* pNodeB=sphericalConstraint->pNodeB;
					SharedPhysics::C_RigidBody* pRigid1=pNodeA->m_pRigidBody;
					SharedPhysics::C_RigidBody* pRigid2=pNodeB->m_pRigidBody;
					const Maths::HVec3 point1(pRigid1->m_Position +  (sphericalConstraint->m_AnchorPoint1 * pRigid1->m_OrientationMatrix));
					const Maths::HVec3 point2(pRigid2->m_Position +  (sphericalConstraint->m_AnchorPoint2 * pRigid2->m_OrientationMatrix));

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point1, 0.05f, Maths::RGBA(0, 255, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point2, 0.05f, Maths::RGBA(255, 0, 0, 255));

				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Revolute:
				{
#ifndef USE_SOLVER_2
					Solver::C_RevoluteConstraint* revoluteConstraint=(Solver::C_RevoluteConstraint*) constraint;
#else
					Solver2::C_RevoluteConstraint* revoluteConstraint=(Solver2::C_RevoluteConstraint*) constraint;
#endif
					SharedPhysics::C_GraphNode* pNodeA=revoluteConstraint->pNodeA;
					SharedPhysics::C_GraphNode* pNodeB=revoluteConstraint->pNodeB;
					SharedPhysics::C_RigidBody* pRigid1=pNodeA->m_pRigidBody;
					SharedPhysics::C_RigidBody* pRigid2=pNodeB->m_pRigidBody;
					const Maths::HVec3 point1(pRigid1->m_Position +  revoluteConstraint->m_AnchorPoint1 * pRigid1->m_OrientationMatrix);
					const Maths::HVec3 point2(pRigid2->m_Position +  revoluteConstraint->m_AnchorPoint2 * pRigid2->m_OrientationMatrix);

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point1, 0.05f, Maths::RGBA(0, 255, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point2, 0.05f, Maths::RGBA(255, 0, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(point1, revoluteConstraint->m_JointAxis1 * pRigid1->m_OrientationMatrix, 0.5f, 0.025f, Maths::RGBA(0, 255, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(point2, revoluteConstraint->m_JointAxis2 * pRigid2->m_OrientationMatrix, 0.5f, 0.025f, Maths::RGBA(0, 0, 255, 255));


				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Prismatic:
				{
#ifndef USE_SOLVER_2
					Solver::C_PrismaticConstraint* prismaticConstraint=(Solver::C_PrismaticConstraint*) constraint;
#else
					Solver2::C_PrismaticConstraint* prismaticConstraint=(Solver2::C_PrismaticConstraint*) constraint;
#endif
					SharedPhysics::C_GraphNode* pNodeA=prismaticConstraint->pNodeA;
					SharedPhysics::C_GraphNode* pNodeB=prismaticConstraint->pNodeB;
					SharedPhysics::C_RigidBody* pRigid1=pNodeA->m_pRigidBody;
					SharedPhysics::C_RigidBody* pRigid2=pNodeB->m_pRigidBody;
		

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(pRigid1->m_Position, 0.05f, Maths::RGBA(0, 255, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(pRigid2->m_Position, 0.05f, Maths::RGBA(255, 0, 0, 255));

					const Maths::HVec3 point1(pRigid1->m_Position + prismaticConstraint->m_JointAxis1 * pRigid1->m_OrientationMatrix);
					const Maths::HVec3 point2(pRigid2->m_Position + prismaticConstraint->m_JointAxis2 * pRigid2->m_OrientationMatrix);

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(pRigid1->m_Position, point1, Maths::RGBA(0, 255, 255, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(pRigid2->m_Position, point2, Maths::RGBA(0, 255, 255, 255));

				}
				break;
			case SharedPhysics::C_Constraint::E_Type_CarWheel:
				{
#ifndef USE_SOLVER_2
					Solver::C_CarWheelConstraint* carWheelConstraint = (Solver::C_CarWheelConstraint*)constraint;
#else
					Solver2::C_CarWheelConstraint* carWheelConstraint = (Solver2::C_CarWheelConstraint*)constraint;
#endif
					SharedPhysics::C_GraphNode* pNodeA = carWheelConstraint->pNodeA;
					SharedPhysics::C_GraphNode* pNodeB = carWheelConstraint->pNodeB;
					SharedPhysics::C_RigidBody* pRigid1=pNodeA->m_pRigidBody;
					SharedPhysics::C_RigidBody* pRigid2=pNodeB->m_pRigidBody;
					const Maths::HVec3 point1(pRigid1->m_Position + carWheelConstraint->m_AnchorPoint1 * pRigid1->m_OrientationMatrix);
					const Maths::HVec3 point2(pRigid2->m_Position + carWheelConstraint->m_AnchorPoint2 * pRigid2->m_OrientationMatrix);

					const Maths::HVec3 jointAxis1(carWheelConstraint->m_JointAxis1 * pRigid1->m_OrientationMatrix);
					const Maths::HVec3 jointAxis2(carWheelConstraint->m_JointAxis2 * pRigid2->m_OrientationMatrix);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point1, 0.05f, Maths::RGBA(0, 255, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point2, 0.05f, Maths::RGBA(255, 0, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point1, point1 + jointAxis1, Maths::RGBA(255, 255, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point2, point2 + jointAxis2, Maths::RGBA(255, 255, 0, 255));
					break;
				}
			case SharedPhysics::C_Constraint::E_Type_Angular:
				break;*/
			default: //Gravity::C_Constraint::E_Type_Contact:
				break;
			}
		}
		/*Gravity::C_GraphNode* pNodeA=sphericalConstraint.pNodeA;
		Gravity::C_GraphNode* pNodeB=sphericalConstraint.pNodeB;
		Gravity::C_RigidBody* pRigid1=pNodeA->m_pRigidBody;
		Gravity::C_RigidBody* pRigid2=pNodeB->m_pRigidBody;
		const Maths::HVec3 point1(pRigid1->m_Position +  sphericalConstraint.m_AnchorPoint1 * pRigid1->m_OrientationMatrix);
		const Maths::HVec3 point2(pRigid2->m_Position +  sphericalConstraint.m_AnchorPoint2 * pRigid2->m_OrientationMatrix);

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point1, 0.5f, Maths::RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point2, 0.5f, Maths::RGBA(255, 0, 0, 255));*/

	/*	Gravity::C_GraphNode* pNodeC=sphericalConstraint2.pNodeA;
		Gravity::C_GraphNode* pNodeD=sphericalConstraint2.pNodeB;
		Gravity::C_RigidBody* pRigid3=pNodeC->m_pRigidBody;
		Gravity::C_RigidBody* pRigid4=pNodeD->m_pRigidBody;
		const Maths::HVec3 point3(pRigid3->m_Position +  sphericalConstraint2.m_AnchorPoint1 * pRigid3->m_OrientationMatrix);
		const Maths::HVec3 point4(pRigid4->m_Position +  sphericalConstraint2.m_AnchorPoint2 * pRigid4->m_OrientationMatrix);

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point3, 0.5f, Maths::RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point4, 0.5f, Maths::RGBA(255, 0, 0, 255));*/

	}


	void C_TestPhysics::DoRayTrace(const Maths::HF32 PCREF dis)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(camera.m_Eye);
		Maths::C_Normal dir((camera.m_At - camera.m_Eye));

		const Maths::C_Normal up(camera.m_Up);
		const Maths::C_Normal right(up ^ dir);

		Collision::C_Ray ray;
		//const Maths::HVec3 end(position + dir * dis);
		for(S32 a = 0; a < 100; ++a)
		{
			ray.InitialiseRay(position, dir, dis, &m_Filter, 0.1f);
			//ray.InitialiseRay(position, end, &m_Filter);
			//ray.InitialiseRay(position, position + dir * dis, &m_Filter);
			//ray.SetDoubleSided(true);
			m_pScene->DoRayTrace(ray, m_pScene->m_Stack);
		}
		if(ray.m_result.m_pShape)
		{
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, Maths::RGBA(0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_end, 0.05f, Maths::RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(ray.m_result.m_contact, ray.m_result.m_normal, 0.2f, 0.05f, Maths::RGBA(255, 0, 0));
		}

		/*const Maths::HF32 negStartX(F32(X_RANGE/2));
		const Maths::HF32 negStartY(F32(Y_RANGE/2));
		const Maths::HF32 offset(5.f);

		Maths::HVec3 start(position);
		Maths::HVec3 end((position + dir * dis)  - (right * negStartX * offset + up * negStartY * offset));
		


		for(U32 a = 0; a < NUM_TASKS; ++a)
		{
			const U32 x = a%X_RANGE;
			const U32 y = a/X_RANGE;

			const Maths::HF32 x_Pos = Maths::HF32(F32(x)) * offset;
			const Maths::HF32 y_Pos = Maths::HF32(F32(y)) * offset;
			const Maths::HVec3 off = up * y_Pos + right * x_Pos;

			Collision::C_Ray& ray = m_pRays[a];
			ray.InitialiseRay(start, end+off, &m_Filter);
			ray.m_Radius = Maths::Constants<Maths::HF32>::Zero();
			

			if(g_UseScheduler)
				m_pScene->DoDeferredRayTrace(&ray, NULL, Groups[0], hPolicy);
			else
				m_pScene->DoRayTrace(ray, m_pScene->m_Stack);
		}*/
	}

	void C_TestPhysics::DoTransparentRayTrace(const Maths::HF32 PCREF dis)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(14.92f, 1.07f, 0.48f);//camera.m_Eye);
		Maths::C_Normal dir(-0.928f, -0.035f, -0.37f);//(camera.m_At - camera.m_Eye));

		const Maths::C_Normal up(camera.m_Up);
		const Maths::C_Normal right(up ^ dir);

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(position, dir, 10.f, 0.1f, Maths::RGBA(255, 0, 0));
		Collision::C_TransparentRay ray;
		//Collision::C_CollisionFilter solidFilter(0x7fffffff, 0x7fffffff);
		Collision::C_CollisionFilter solidFilter(0, 0);
		ray.InitialiseRay(position, dir, dis, &solidFilter, &m_Filter,  0.1f);
		//ray.InitialiseRay(position, position + dir * dis, &m_Filter);
		//ray.SetDoubleSided(true);
		//m_pScene->DoRayTrace(ray, m_pScene->m_Stack);

		m_pScene->DoTransparentRayTrace(ray, m_pScene->m_Stack);

		for(U32 i=0; i<ray.m_numLayers; ++i)
		{
			const Maths::HVec3 contact= ray.m_result[i].m_contact;
			const Maths::HVec3 normal = ray.m_result[i].m_normal;

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.05f, Maths::RGBA(0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contact, normal, 0.2f, 0.05f, Maths::RGBA(0, 255, 0));

		}

		if(ray.m_SolidResult.m_pShape)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_SolidResult.m_contact, 0.05f, Maths::RGBA(255,0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(ray.m_SolidResult.m_contact, ray.m_SolidResult.m_normal, 0.2f, 0.05f, Maths::RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_end, ray.m_start, Maths::RGBA(255, 255, 0));
		}

		/*Collision::C_Ray ray;
		ray.InitialiseRay(position, position + dir * dis,&m_Filter);

		m_pScene->DoRayTrace(ray, m_pScene->m_Stack);

		if(ray.GetResult().Hit())
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.GetResult().HitPoint(), 0.05f, Maths::RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(ray.GetResult().HitPoint(), ray.GetResult().Normal(), 0.2f, 0.05f, Maths::RGBA(255, 0, 0));
		}*/

	}

	void C_TestPhysics::GetRayTraceResults()
	{
		/*Scheduler::Flush(Groups[0]);

		if(g_DrawRayResults)
		{
			for(U32 a = 0; a < NUM_TASKS; ++a)
			{
				Collision::C_Ray& ray = m_pRays[a];
				if(ray.m_result.m_pShape)
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_result.m_contact, 0.05f, Maths::RGBA(255, 0, 0));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(ray.m_result.m_contact, ray.m_result.m_normal, 0.2f, 0.05f, Maths::RGBA(255, 0, 0));
				}
			}
		}*/
	}

	void C_TestPhysics::AttachRopeToBody(Gravity::C_Body* pBody, Collision::C_CollisionFilter* pFilter,
		const Maths::HVec3 PCREF offsetPosition)
	{
		Gravity::C_RopeDesc ropeDesc;
		ropeDesc.m_RestLength = 0.08f;
		Maths::HF32 offset(0.02f);

		Maths::HVec3 position(pBody->Position() - offsetPosition);

		Gravity::C_DescriptorParticle particle;
		particle.m_Position = position;
		particle.m_Mass = 0.f;
		ropeDesc.m_Vertices.AddToEnd(&particle);

		for(S32 a = 0; a < 18; ++a)
		{
			position.X(position.X() + offset);
			particle.m_Position = position;
			particle.m_Mass = 1.f;
			ropeDesc.m_Vertices.AddToEnd(&particle);
			
		}
		position.X(position.X() + offset);
		particle.m_Position = position;
		particle.m_Mass = 1.f;
		ropeDesc.m_Vertices.AddToEnd(&particle);

		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = -1.f;
		deactivator.m_SleepThresholdSq = -1.f;
		C_Body* ropeBody(m_pScene->CreateBody());
		
		Maths::HMat33 orientation;
		orientation.SetIdentity();

		Gravity::C_BodyHelper bodyHelper(&ropeDesc);
		bodyHelper.CreateBody(1.f, position, orientation, ropeBody, pFilter, 0, 0);
		ropeBody->SetDeactivator(deactivator);
		ropeBody->Baumgarte(0.75f);
		
		m_pScene->AddRopeBody(ropeBody);
		ropeBody->SetExternalConstants(&m_ExternalConstants);

		Collision::C_RopeShape* pRopeShape = (Collision::C_RopeShape*)ropeBody->GetShape();
		pRopeShape->m_Rope.m_pExternalConstants = &m_ExternalConstants;
		pRopeShape->m_Rope.SetDampingCoefficent(0.99f);
		pRopeShape->m_Rope.SetNumberJoints(1);
		pRopeShape->m_Rope.ConnectParticle(0, pBody, offsetPosition);
		ropeBody->Activate();
	}




	void C_TestPhysics::InitialiseTest(Collision::C_CollisionFilter* pFilter, const U32 demoMode)
	{
		//m_IMeshManager.Reset();
		m_UseController = false;
		if(pStaticScene)
		{
			delete pStaticScene;
		}
		
		if(pImportScene)
		{
			delete pImportScene;
		}

		if(chairScene)
			delete chairScene;

		if(m_pScene)
		{
			Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseScene(m_pScene);
		}
		Collision::C_AxisBoxMinMax worldBox;
		worldBox.m_max = Maths::HVec3(300.f, 50.f, 300.f);
		worldBox.m_min = Maths::HVec3(-300.f, -50.f, -300.f);
		const Maths::HVec3 desiredSize(10.f, 10.f, 10.f);
		S32 maxDepth = 6;
		SetUpMaterials();
		m_pScene = Gravity::C_PhysicsSDK::PhysicsSDK()->CreateScene(&m_Materials[0][0], MATERIAL_COUNT);
		m_ExternalConstants.m_Gravity = Maths::HVec3(0, -9.8f, 0);
		m_pScene->RegisterCollisionCallback(&drawer);
		m_pScene->RegisterEventCallback(&eventIter);
		m_pScene->SetInternalTaskPolicy(collisionPolicy);
		m_pScene->SetInternalGroupHandle(Groups[1]);
		m_pScene->RegisterRaytracingTaskPolicy(Groups[0]);

		//First thing - load in the level file for level collisions...
		const Maths::HF32 zero;
		const Maths::HVec3 vzero;
		Maths::HMat33 mat;
		mat.SetIdentity();
		Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "testLevel.dat" );
		//Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "Level.levelphysics" );



		m_pScene->ClearConstraints();

		pImportScene = new Gravity::C_ImportScene;
		pImportScene->Import(filePtr);
	//	pImportScene->GetDynamicShape(0);
	
		pStaticScene = new Gravity::C_ImportSceneStaticInstance(m_pScene, pImportScene, desiredSize);

		delete filePtr;

		

		//AddGroundPlane(Maths::C_Normal(0.f, 1.f, 0.f), Maths::HF32(10.f), &m_Filter);

		/*C_Body* pSphereBody = AddSphere(Maths::HVec3(16.f, 1.38f, -0.39f), 0.5f, 10.f, pFilter);
		pSphereBody->MakeStatic();*/


		//Load in model and constraints
		//filePtr=Sys::C_FileSystem::Instance().Open("testModel.dat");

		//pImportScene->Import(filePtr);


		//SharedPhysics::SortedArray<Gravity::C_ImportBodyWrapper> dynamicBodies;	

		//for(U32 i=0; i< pImportScene->m_DynamicShapes.m_numElements; ++i)
		//{
	
		//	Gravity::C_ImportShape* pImportShape=pImportScene->m_DynamicShapes[i];
		//	C_ShapeDesc* pDesc=pImportShape->GetShapeDesc();
		//	
		//	Gravity::C_Body* pBody(m_pScene->CreateBody());
		//	pBody->SetExternalConstants(&m_ExternalConstants);
		////	pBody->Baumgarte(0.9f);
		//	pBody->Slop(0.00000001f);
		//	
		//	Maths::HMat33 identity;
		//	identity.SetIdentity();

		//	Gravity::C_BodyHelper helper(pDesc);
		//	helper.CreateBody(pImportShape->GetShapeDesc()->GetMass(), pImportShape->GetImportPosition(), pImportShape->GetImportRotation(), pImportShape->GetImportIneriaTensor(), pBody, &m_Filter);
		//	
		//	SharedPhysics::C_Deactivator deactivator;
		//	deactivator.m_ActivateThresholdSq = 0.001f;
		//	deactivator.m_SleepThresholdSq = 0.0001f;
		//	deactivator.m_sSleepCountdown = 15;

		//	pBody->SetDeactivator(deactivator);
		//	
		//	m_pScene->AddBody(pBody);
		//	C_ImportBodyWrapper temp(pBody);
		//	dynamicBodies.Insert(&temp);
		//}

		//Gravity::C_Body tempBody;
		//for(S32 i = 0; i < pImportScene->m_ConstraintsDesc.m_numElements; ++i)
		//{
		//	Gravity::ConstraintDescriptors* desc = pImportScene->m_ConstraintsDesc[i];

		//	tempBody.SetBodyId(desc->m_PhysicsBodyAID);

		//	C_ImportBodyWrapper temp(&tempBody);
		//	C_ImportBodyWrapper* pBodyAWrapper(dynamicBodies.Find(&temp));
		//	tempBody.SetBodyId(desc->m_PhysicsBodyBID);
		//	C_ImportBodyWrapper* pBodyBWrapper(dynamicBodies.Find(&temp));

		//	if(pBodyAWrapper && pBodyBWrapper)
		//	{
		//		Gravity::C_Body* pBodyA(pBodyAWrapper->GetBody());
		//		Gravity::C_Body* pBodyB(pBodyBWrapper->GetBody());

		//		SharedPhysics::C_Constraint* pConstraint = desc->CreateConstraint(*m_pScene->m_pGraphManager, pBodyA, pBodyB);
		//		//pBodyA->MakeStatic();
		//		//pBodyB->MakeStatic();
		//		m_pScene->AddConstraint(pConstraint);
		//	}

		//}

		for(S32 i=0; i<pImportScene->m_ClothShapes.m_numElements; ++i)
		{
			Gravity::C_ImportShape* pImportShape=pImportScene->m_ClothShapes[i];
			C_ShapeDesc* pDesc=pImportShape->GetShapeDesc();
			
			Gravity::C_Body* pBody(m_pScene->CreateBody());
			pBody->SetExternalConstants(&m_ExternalConstants);
			pBody->Baumgarte(0.9f);
			pBody->Slop(0.00000001f);
			
			Maths::HMat33 identity;
			identity.SetIdentity();

			Gravity::C_BodyHelper helper(pDesc);
			helper.CreateBody(pImportShape->GetShapeDesc()->GetMass(), pImportShape->GetImportPosition(), pImportShape->GetImportRotation(), pImportShape->GetImportIneriaTensor(), pBody, &m_Filter);
			
			SharedPhysics::C_Deactivator deactivator;
			deactivator.m_ActivateThresholdSq = 0.f;
			deactivator.m_SleepThresholdSq = 0.0001f;
			deactivator.m_sSleepCountdown = 30;

			pBody->SetDeactivator(deactivator);
			Collision::C_ClothShape* pClothShape = (Collision::C_ClothShape*)pBody->GetShape();
			pClothShape->m_Cloth.SetDampingCoefficent(Maths::HF32(0.99f));

			pClothShape->m_Cloth.SetExternalConstants(&m_ExternalConstants);

			pClothShape->m_Cloth.SetStretchCoefficient(Maths::HF32(1.01f));
			pClothShape->m_Cloth.SetCompressCoefficient(Maths::HF32(0.99f));
			pClothShape->m_Cloth.SetThickness(Maths::HF32(0.10f));
			pClothShape->m_Cloth.SetCoefficientOfRestitution(Maths::HF32(0.05f));
			pClothShape->m_Cloth.m_BodyAngularDampingCoefficient = 0.97f;
			pClothShape->m_Cloth.m_BodyLinearDampingCoefficient = 0.97f;
			pClothShape->m_Cloth.SetFrictionCoefficient(0.15f);
			pClothShape->m_Cloth.SetStarch(1.f);
			pClothShape->m_Cloth.SetTearThreshold(1.3f);
			pClothShape->m_Cloth.m_NumIterations = 5;
		
			m_pScene->AddClothBody(pBody);
		}

		////Set Up Fluid
		//{
		//	
		//	Gravity::C_FluidDesc* pDesc=new Gravity::C_FluidDesc;
		//	pDesc->m_Mass = Maths::Constants<Maths::HF32>::Max();
	
		//	Gravity::C_Body* pFluidBody(m_pScene->CreateBody());

		//	const Maths::HVec3 position(3.f, 5.f, 4.f);
		//	Maths::HMat33 rotation;
		//	rotation.SetIdentity();
		//	const Maths::HMat33 inertia=rotation;
		//	Gravity::C_BodyHelper helper(pDesc);
		//	helper.CreateBody(position, rotation, pFluidBody, &m_Filter);
		//	
		//	SharedPhysics::C_Deactivator fluidDeactivator;
		//	fluidDeactivator.m_ActivateThresholdSq = -1.f;
		//	fluidDeactivator.m_SleepThresholdSq = 0.001f;
		//	fluidDeactivator.m_sSleepCountdown = 30;
		//

		//	pFluidBody->SetDeactivator(fluidDeactivator);

		//	Collision::C_FluidShape* pFluidShape = (Collision::C_FluidShape*)pFluidBody->GetShape();

		//	Collision::C_HFFCollisionManager collisionManager;

		//	pFluidShape->m_Fluid.SetCollisionManager(&collisionManager);
		//	pFluidShape->m_Fluid.SetGridDimension(30);
		//	pFluidShape->m_Fluid.SetWaveSpeed(Maths::HF32(5.f));
		//	pFluidShape->m_Fluid.SetWaveWidth(Maths::HF32(0.2f));
		//	pFluidShape->m_Fluid.SetDensity(Maths::HF32(30.f));

		//	const Maths::HF32 height(10.f);
		//	const Maths::HF32 ratio(0.5f);
		//	pFluidShape->m_Fluid.SetMaxSlop(height * ratio);

		//	pFluidShape->m_Fluid.Initilise(position, height);
		//	pFluidShape->m_Fluid.m_Velocity = Maths::HVec3(0.05f, 0.0f, 0.05f);
		//	pFluidShape->m_Fluid.m_LinearDrag = 4.0f;
		//	pFluidShape->m_Fluid.m_AngularDrag = 0.5f;
		//	pFluidShape->m_Fluid.m_MaxHeight = 10.1f;
		//	pFluidShape->m_Fluid.m_MaxBuoyancy = 20.f;
		//	pFluidBody->MakeStatic();
		//

		//	m_pScene->AddFluidBody(pFluidBody);


		//} 

		//InitialiseRagdollConfig(pFilter, Maths::HVec3(2.f, 2.f, 10.f));
		//InitiliseFeatherstoneRagdoll(pFilter, Maths::HVec3(2.f, 2.f, 10.f));
		//InitialiseSerialLink(pFilter, Maths::HVec3(2.f, 2.f, 10.f));
		/*if(g_dbInitialseFixedJointRagdoll)
			InitialiseFixedJointRagdoll(&m_Filter, Maths::HVec3(2.f, 2.f, 10.f));*/

		
		SetUpTriggers();

		Maths::HVec3 posOffset(4.f, 2.f, 0.f);


		//BuildChair(pFilter, Maths::HVec3(13.f, 1.f, -1.f));

			
			//if(demoMode == E_CharacterControllerDemo)
			{
				C_Body* pBody = AddCharacterController(Maths::HVec3(18.f, 2.5f, 0.f), Maths::HMat33(), 
					Maths::HF32(1.f), Maths::HF32(0.4f), Maths::HF32(0.6f), Maths::HF32(0.3f), Maths::HF32(60.f), pFilter);
				this->m_Controller = Gravity::C_CharacterController(pBody, Maths::HF32(10.f), Maths::HF32(500.f));
				m_pScene->AddCharacterController(&m_Controller);

				//Create a trigger actor....
				if(m_pTriggerActor)
					delete m_pTriggerActor;
				m_pTriggerActor = new C_TriggerActor;
				m_pTriggerActor->m_pShape = new Gravity::C_TriggerSphere(0.5f, &m_Filter);
				m_pLayer->AddTriggerActor(m_pTriggerActor);
				m_pLayer->SetTriggerActorMatrix(m_pTriggerActor, Maths::HMat43(pBody->Orientation(), pBody->Position()), stack);

				//Add character's pony tail...
				const Maths::HVec3 offsetPosition(0.f, 0.5f, -0.2f);
				AttachRopeToBody(pBody, &m_Filter, offsetPosition);


			}
			if(demoMode == E_ParticleFluidDemo)
			{
				if(g_dbTestParticleBasedFluidSystem)
					InitialiseParticleBasedFluidSystem();
			}
			if(demoMode == E_RagdollArmDemos)
			{
				SetUpRagdollArm(pFilter);
			}
			if(demoMode == E_RopeDemo)
			{
				SetUpRopeDemo(pFilter);
			}
			else if(demoMode == E_TwoRotationalDemo)
			{
				SetUpTwoRotationalDemo(pFilter);
			}
			else if(demoMode == E_ReferenceFrameDemo)
			{
				SetUpFrameOfReference(pFilter);
			}
			else if(demoMode == E_FixedJointDemo)
			{
				SetUpFixedDemo(pFilter);
			}
			else if(demoMode == E_DistanceConstraintDemo)
			{
				SetUpDistanceConstraint(pFilter);
			}
			else if(demoMode == E_SpringDemo)
			{
				SetUpSpring(pFilter);
				//SetUpSpringConstraint(pFilter);
			}
			else if(demoMode == E_WallDemo)
			{
				SetUpWall(8, 8, pFilter);
			}
			else if (demoMode == E_SphereDemo)
			{
				SetUpSphere(pFilter);
			}
			else if(demoMode == E_ConvexHullDemo)
			{
				SetUpConvexHull(pFilter);
			}
			else if (demoMode == E_CapsuleDemo)
			{
				SetUpCapsule(pFilter);
			}
			else if(demoMode == E_CylinderDemo)
			{
				SetUpCylinder(pFilter);
			}
			else if (demoMode == E_CompositeDemo)
			{
				SetUpCompositeShapes(3, pFilter);
			}
			else if(demoMode == E_SphericalLimitDemo)
			{
				SetUpSphericalJointLimit(pFilter);
			}
			else if (demoMode == E_SphericalDemo)
			{
				SetUpSphericalJoint(pFilter);
			}
			else if (demoMode == E_HingeJointLimitDemo)
			{
				SetUpRevoluteJointLimit(pFilter);
			}
			else if (demoMode == E_HingeDemo)
			{
				SetUpRevoluteJoint(pFilter);
			}
			else if(demoMode == E_PrismaticDemo)
			{
				SetUpPrismaticJoint(pFilter);
			}
			else if (demoMode == E_CarDemo)
			{
				SetUpCar(pFilter);
			}
			else if(demoMode == E_PrismaticJointLimitDemo)
			{
				SetUpPrismaticJointLimit(pFilter);
			}
			else if (demoMode == E_BridgeDemo)
			{
				SetUpBridge(Maths::HVec3(15.f, 4.4f, 0.6f), 
					Maths::C_Normal(1.f, 0.f, 0.f), Maths::HF32(0.7f),
					Maths::HF32(0.03f),
					Maths::HVec3(0.3f, 0.1f, 1.f), 26, pFilter);
			}
			else if (demoMode == E_CatterpillarTracksDemo)
			{
				SetUpBridge(Maths::HVec3(15.f, 4.4f, 0.6f), 
					Maths::C_Normal(1.f, 0.f, 0.f), Maths::HF32(0.58f), Maths::HF32(0.01f),
					Maths::HVec3(0.3f, 0.1f, 1.f), 35, pFilter);
			}
		/*	else if(demoMode == E_HingeJointLimitDemo)
			{
				this->SetUpRevoluteJointLimit(pFilter);
			}*/
		
		//delete filePtr;
		//delete chairFile;
		
	}

	void DrawMesh(const Collision::OptimisedAABBTree& tree, const Maths::HMat43 PCREF matrix, const Collision::C_CRAxisBox& untransformedBox,
		SharedPhysics::C_SafeStack<true>& stack, const Maths::RGBA colour)
	{
		////if(g_dbDrawMesh)
		//{
		//	Collision::C_CRAxisBox box;
		//	untransformedBox.Transform(matrix.InverseRotTrans(), box);
		//	//const U32 maxTriangles(2048);
		//	//U32 triangleDrawn = 0;
		//	//Grab the current camera position!
		//	const Maths::HF32 three(Maths::Constants<Maths::HF32>::Three());

		//	//const Maths::RGBA colours[7]=
		//	//{
		//	//	RGBA(55, 55, 55), RGBA(255, 255, 55), RGBA(255, 55, 255), RGBA(55, 255, 255),
		//	//		RGBA(128, 128, 0), RGBA(0, 128, 128), RGBA(128, 0, 128)
		//	//};

		//	//const Maths::RGBA colour(55, 55, 55);

		//	Collision::C_AxisBoxMinMax axisBox;

		//	//Traverse the triangle mesh...
		//	Collision::OptimisedAABBNode* pStackTop(tree.m_pNodes);
		//	stack.Push(&tree.m_pNodes[0], 0);
		//	S32 size(1);
		//	while(size--)
		//	{
		//		Collision::OptimisedAABBNode* pNode(pStackTop);
		//		pStackTop = stack.Pop<Collision::OptimisedAABBNode>(Maths::Max(size-1, 0));
		//		pNode->m_BoundingBox.LoadBox(axisBox, tree.m_Low, tree.m_High);
		//		if(g_dbForceDraw || axisBox.Intersects(box))
		//		{
		//			if(pNode->m_NumIndices > 0)
		//			{
		//				//Draw mesh!
		//				U32 processed = 0;
		//				
		//				const Collision::C_TriangleHeader* header(tree.m_Triangles.GetHeader(pNode->m_StartIndex));
		//				U32 offset = pNode->m_StartIndex + sizeof(Collision::C_TriangleHeader);
		//				//while(processed < pNode->m_NumIndices)
		//				
		//				U32 headerIndex = 0;
		//				for(U32 p = 0; p < pNode->m_NumIndices; ++p, ++headerIndex)
		//				{
		//					if(header->m_NumTriangles <= headerIndex)
		//					{
		//						header = tree.m_Triangles.GetHeader(offset);
		//						offset += sizeof(Collision::C_TriangleHeader);
		//						headerIndex = 0;
		//					}
		//					const Collision::AABBTreeTriangle* tri(tree.m_Triangles.GetTriangles(offset));
		//					offset += sizeof(Collision::AABBTreeTriangle);
		//					const Maths::HVec3 vA(Maths::Load<Maths::HVec3>(&tree.m_pVertices[tri->m_IndexA]));
		//					const Maths::HVec3 vB(Maths::Load<Maths::HVec3>(&tree.m_pVertices[tri->m_IndexB]));
		//					const Maths::HVec3 vC(Maths::Load<Maths::HVec3>(&tree.m_pVertices[tri->m_IndexC]));
		//					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleLit(vA*matrix,vB*matrix, vC*matrix, colour);
		//					//const Maths::HVec3 norm = Maths::Load<Maths::HVec3>(&tree.m_pNormals[tri->m_NormalIndex]);
		//					//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow((vA + vB + vC)/Maths::Constants<Maths::HF32>::Three(),
		//					//	norm, 0.3f, 0.05f, Maths::RGBA(255, 0, 0));
		//				}
		//			}
		//			else
		//			{
		//				pStackTop = pNode + 1;
		//				stack.Push(pNode + pNode->m_RightChildOffset, size++);
		//				stack.Push(pNode + 1, size++);
		//				
		//			}
		//			//if(triangleDrawn > maxTriangles)
		//			//	goto Simulate;
		//		}
		//	}
		//}
	}

		void C_TestPhysics::DrawCompositeShape(const Collision::C_CompositeShape* pCompShape)
	{
		using namespace Maths;

		Collision::C_CRAxisBox axisBox;
		axisBox.extents = Constants<Maths::HVec3>::Max();
		static bool subComp=false;
		const HVec3 position=pCompShape->Position();
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(position, 0.5f, RGBA(255, 0, 0, 0));
		for(U32 i = 0; i < pCompShape->m_NumShapes; ++i)
		{
			switch(pCompShape->m_pShapes[i].m_pShape->GetShape())
			{
			case Collision::E_InstancedTriangleMesh_Shape:
				{
					Collision::C_InstancedTriangleMeshShape* pMeshShape((Collision::C_InstancedTriangleMeshShape*)pCompShape->m_pShapes[i].m_pShape);
					const HMat43 transform(pMeshShape->Orientation(), pMeshShape->Position());
					const RGBA colour(55, 55, 55);
					DrawMesh(*pMeshShape->m_AABBTree.m_pTree, transform, axisBox, this->m_pScene->m_Stack, colour);
					break;
				}
			case Collision::E_Box_Shape:
				{
			
				const Collision::C_BoxShape* const __restrict pBoxShape((Collision::C_BoxShape*)pCompShape->m_pShapes[i].m_pShape);
 
				HMat43 absPosRot = Maths::HMat43(pBoxShape->m_Box.u, pBoxShape->m_Box.c);
				HVec3 minV(-pBoxShape->m_Box.e);
				HVec3 maxV(pBoxShape->m_Box.e);
				if(!subComp)
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBoxLit(minV, maxV, absPosRot, RGBA(0, 255, 0, 255));
				else
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBoxLit(minV, maxV, absPosRot, RGBA(255, 255, 0, 255));

				break;
				}
			case Collision::E_Sphere_Shape:
				{
					//Must be a sphere
					const Collision::C_SphereShape* const __restrict pSphereShape((Collision::C_SphereShape*)pCompShape->m_pShapes[i].m_pShape);
					const HVec3 center=pSphereShape->m_Sphere.Centre();
					
					if(!subComp)
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(center, pSphereShape->m_Sphere.Radius(), Maths::RGBA(0, 255, 0));
					else
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(center, pSphereShape->m_Sphere.Radius(), Maths::RGBA(255, 255, 0));

					break;
				}
			case Collision::E_Composite_Shape:
				{
					//const Collision::C_CompositeShape* const __restrict pCompShape((Collision::C_CompositeShape*)pCompShape->m_pShapes[i].m_pShape);
					const Collision::C_CompositeShape* pCompShape2((Collision::C_CompositeShape*)pCompShape->m_pShapes[i].m_pShape);
					subComp = true;
					DrawCompositeShape(pCompShape2);
					subComp = false;
					break;
				}
			default:
				{
					break;
				}
			}
		}
	}

	void C_TestPhysics::DrawAnchorPoints()
	{
		/*for(S32 a = 0; a < this->m_Constraints.m_numElements; ++a)
		{
			SharedPhysics::C_Constraint* constraint = m_Constraints[a];
			switch(constraint->m_ConstraintType)
			{
			case E_Type_Spherical:
				{
					Solver2::C_SphericalConstraint* spherical = (Solver2::C_SphericalConstraint*)constraint;
					SharedPhysics::C_GraphNode* nodeA = spherical->pNodeA;
					SharedPhysics::C_GraphNode* nodeB = spherical->pNodeB;
					const Maths::HVec3 localAnchorA = spherical->m_AnchorPoint1;
					const Maths::HVec3 localAnchorB = spherical->m_AnchorPoint2;

					const Maths::HMat43 transA(nodeA->m_pRigidBody->m_OrientationMatrix, nodeA->m_pRigidBody->m_Position);
					const Maths::HMat43 transB(nodeB->m_pRigidBody->m_OrientationMatrix, nodeB->m_pRigidBody->m_Position);

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(localAnchorA * transA, 0.5f, Maths::RGBA(0, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(localAnchorB * transB, 0.5f, Maths::RGBA(255, 0, 255));
					break;
				}
			case E_Type_Revolute:
				{
					break;
				}
			};
		}*/
	}

	void C_TestPhysics::AnimateAndDraw(const Maths::C_TimeS PCREF time)
	{
		PERF_EVENT("Gravity.AnimateAndDraw");
		using namespace Maths;

		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		HVec3 position(camera.m_Eye);

		DbUtil::DefaultDebugRenderSys()->Deferred()->SetLightDirection(Maths::HVec3(0.707f, 0.707f, 0.f));
		Collision::C_CRAxisBox _box, box;
		C_Normal dir(camera.m_At - position);
		_box.centre = Maths::Constants<Maths::HVec3>::Zero();
		_box.extents = Maths::HVec3(8.f, 8.f, 5.f);

		C_Normal yAxis(camera.m_Up);
		
		const C_Normal nx( yAxis ^ dir );
		HMat33 _mat(nx, yAxis, dir);
		_box.Transform(Maths::HMat43(_mat, position + dir * Maths::HF32(5.f)), box);

		//Get the level mesh and draw it first...

		if(g_dbDrawLocation)
		{
			char data[256];
			sprintf(data, "(%2.2f, %2.2f, %2.2f)", F32(position.X()), F32(position.Y()), F32(position.Z()));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(150, 50), data);
		}


//Simulate:

		//UpdateRagdollController(time.Get());
		UpdateRagdollBodyWeight();
		UpdateRagdollConstraintWeight();
		
		UpdateRagdollController(time.Get());

		m_pLayer->SetTriggerActorMatrix(m_pTriggerActor, Maths::HMat43(m_Controller.GetBody()->Orientation(), m_Controller.GetBody()->Position()), stack);
		//m_pScene->SetMinTimeStep(1.f/120.f);

		m_pScene->Simulate(time);
		
		if(g_dbTestParticleBasedFluidSystem)
		{
			m_FluidSystem.TimeStep(time.Get());
			DrawParticleBaseFluidSystem();
		}
			
		if(m_UseController)
		{
			static HF32 rot=0.f;
			if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
				rot=0.f;
			rot+=0.002f;
			
			if(g_dbResetRot)
			{
				rot=0.f;
			}

			UpdateFrameOfReference(time.Get(), rot);
		}

		
		if(g_dbCharacterControllerTeleport)
		{
			HF32 dist(2.1f);
			HVec3 direction(0.0, 0.f, 1.f);
			HF32 distTravelled = m_Controller.TeleportWithCollision(m_Controller.Position() + dist*direction,
				Maths::C_Quaternion());
		}

		if(g_dbUpdateConstraintAnchorPoint)
		{
			if(m_pScene->NumConstraints() > 0)
			{
				SharedPhysics::C_Constraint* pConstraint = m_pScene->GetConstraint(0);
				if(pConstraint->m_ConstraintType == SharedPhysics::C_Constraint::E_Type_Spherical)
				{
					static F32 y = 2.5f;
					static F32 offset = 0.04f;
					Solver2::C_SphericalConstraint* pSphere = (Solver2::C_SphericalConstraint*)pConstraint;
					SharedPhysics::C_RigidBody* pRigidBodyA = pConstraint->pNodeA->m_pRigidBody;
					SharedPhysics::C_RigidBody* pRigidBodyB = pConstraint->pNodeB->m_pRigidBody;
				
					HVec3 worldAnchorPoint = pRigidBodyB->m_Position + pSphere->m_AnchorPoint2 * pRigidBodyB->m_OrientationMatrix;

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(worldAnchorPoint, 0.25f, RGBA(0, 0, 255));
					
					//HF32 y = worldAnchorPoint.Y();

					if(y < HF32(3.f) && y >= HF32(0.5f))
					{
						y += offset;
					}
					else
					{
						offset = -offset;
						y += offset;
					}

				/*	HVec3 newWorldAnchorPoint(worldAnchorPoint.X(), y, worldAnchorPoint.Z()); 

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(newWorldAnchorPoint, 0.5f, RGBA(0, 255, 0));

					pSphere->m_AnchorPoint1 = (newWorldAnchorPoint - pRigidBodyA->m_Position)* pRigidBodyA->m_OrientationMatrix.InverseRotation();*/
				
					//pSphere->m_AnchorPoint2 = (newWorldAnchorPoint - pRigidBodyB->m_Position)* pRigidBodyB->m_OrientationMatrix.InverseRotation();
				}
			}
			
		}

		//DebugBreak();
		this->AddFillCacheTask(Groups[0], hPolicy);
		
	//	UpdateFeatherstoneRagdoll(time.Get());
		
		
	}

	void C_TestPhysics::OnRender()
	{
		using namespace Maths;
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		HVec3 position(camera.m_Eye);

		DbUtil::DefaultDebugRenderSys()->Deferred()->SetLightDirection(Maths::HVec3(0.707f, 0.707f, 0.f));
		Collision::C_CRAxisBox _box, box;
		C_Normal dir(camera.m_At - position);
		_box.centre = Maths::Constants<Maths::HVec3>::Zero();
		_box.extents = Maths::HVec3(8.f, 8.f, 5.f);

		C_Normal yAxis(camera.m_Up);
		
		const C_Normal nx( yAxis ^ dir );
		HMat33 _mat(nx, yAxis, dir);
		_box.Transform(Maths::HMat43(_mat, position + dir * Maths::HF32(5.f)), box);

		m_pScene->DebugRenderScene(DbUtil::DefaultDebugRenderSys()->Primitive(), box, m_pScene->m_Stack);
		m_TriggerCallback.DrawTriggers(DbUtil::DefaultDebugRenderSys()->Primitive());
		AnchorPointDraw();
	}

	
	void C_TestPhysics::ProcessController(Samples::C_Input& input)
	{
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(camera.m_Eye);
		Maths::HVec3 dir((camera.m_At - camera.m_Eye));
		dir.Y(Maths::Constants<Maths::HF32>::Zero());
		dir.Normalise();

		Maths::HVec3 direction;

		Maths::HVec3 y(0.f, 1.f, 0.f);
		Maths::HVec3 x(y ^ dir);

		F32 thumbX = input.AxisValue(Samples::E_Axis_LX);
		F32 thumbY = input.AxisValue(Samples::E_Axis_LY);

		direction += dir * thumbY;
		direction += x * thumbX;


		const Maths::HF32 targetSpeed(3.f);
		m_Controller.SetTargetVelocity(direction * targetSpeed, Maths::Constants<Maths::HVec3>::Zero());
	}

	void SetCamera(Samples::E_CameraType type)
	{
		C_Application* g_App=Samples::GetApp();
		const Samples::C_View* view(g_App->GetView(0));
		Samples::C_Camera& camera(view->m_Camera);
		camera.SetType(type);
		camera.m_ChaseDistance = Maths::HF32(4.f);
	}

	void C_TestPhysics::InitialiseScheduler()
	{
		Scheduler::Policy piMed[] = { 7, 7, 7, 7, 7, 7, 7, 7 };
		static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);
		hPolicy = _hPolicy;

		Scheduler::Policy piHigh[] = {10, 10, 10, 10, 10, 10, 10, 10 };
		static Scheduler::PolicyHandle _highPolicy = Scheduler::CreatePolicy(piHigh);
		collisionPolicy = _highPolicy;
		

		Scheduler::Init();
		Scheduler::Start();

		Scheduler::SetGroupPoolSize(NUM_GROUPS);
		for (int i = 0; i < NUM_GROUPS; i++)
			Groups[i] = Scheduler::CreateGroup();

	}

	void C_TestPhysics::AddFillCacheTask(const S32 groupId, const S32 policyId)
	{
		/*Collision::C_LineCollisionCache* pCache = new Collision::C_LineCollisionCache(128);
		Collision::C_Ray* pRay = new Collision::C_Ray;
		pRay->InitialiseRay(Maths::HVec3(10.f, 10.f, 10.f), Maths::HVec3(-10.f, -10.f, -10.f), &m_Filter);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(pRay->m_start, pRay->m_end, Maths::RGBA(255, 0, 0));
		Gravity::AddFillCacheTask(groupId, policyId, m_pScene, pCache, pRay);
		pCache->DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred());
		if(pRay->m_result.Hit())
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(pRay->m_result.HitPoint(), 0.2f, Maths::RGBA(0, 0, 255));
		}
		
		delete pCache;
		delete pRay;*/
	}
	
	void C_TestPhysics::ReleaseScheduler()
	{
		Scheduler::Shutdown();
	}


	void C_SectionPhysics::TestPhysicsEngine(F32 time)
	{
		static bool bPaused = false;
		PERF_EVENT("Gravity.TestPhysicsEngine");
		const F32 minPressFreq = 0.2f;
		static U32 demoMode = E_RopeDemo;
		static bool initialised = false;
		if(!initialised)
		{
			initialised = true;
			m_TestPhysics.InitialiseTest(&m_TestPhysics.m_Filter, demoMode);
			SetCamera(Samples::E_CameraType_Chase);
		}

		if(m_input.IsButtonDn(E_Button_Start))
		{
			bPaused = !bPaused;
		}

		if(m_input.IsButtonDn(E_Button_B))
		{
			static F32 pressTime = 0.f;
			if((time - pressTime) > minPressFreq || bPaused)
			{
				demoMode = (demoMode + 1) %E_MaxDemos;
				pressTime = time;
			}


			m_TestPhysics.InitialiseTest(&m_TestPhysics.m_Filter, demoMode);
		}

		if(bPaused)
		{
			if(m_input.IsButtonDn(E_Button_A))
			{
				m_TestPhysics.AnimateAndDraw(Maths::C_TimeS(0.01f));
			}
		}
		else
		{
			if(m_input.IsButtonDn(E_Button_A))// && demoMode != E_CompositeDemo)
			{
				static F32 pressTime = 0.f;
				if((time - pressTime) > minPressFreq)
				{
					if(g_dbShootABox)
						m_TestPhysics.ShootABox(&m_TestPhysics.m_Filter);
					if(g_dbShootASphere)
						m_TestPhysics.ShootASphere(&m_TestPhysics.m_Filter);
					if(g_dbShootACylinder)
						m_TestPhysics.ShootACylinder(&m_TestPhysics.m_Filter);
					if(g_dbShootACapsule)
						m_TestPhysics.ShootACapsule(&m_TestPhysics.m_Filter);
					if(g_dbShootAChair)
						m_TestPhysics.ShootAChair(&m_TestPhysics.m_Filter);
					if(g_dbShootACone)
						m_TestPhysics.ShootACone(&m_TestPhysics.m_Filter);
					if(g_dbShootAConvexHull)
						m_TestPhysics.ShootAConvexHull(&m_TestPhysics.m_Filter);
					pressTime = time;
				}
			}
			if(m_input.IsButtonDn(E_Button_Y))
			{
				static F32 pressTime = 0.f;
				if((time - pressTime) > minPressFreq)
				{
					m_TestPhysics.ShootARagdoll(&m_TestPhysics.m_Filter);
					pressTime = time;
				}
			}

			static bool bCharacterController = true;

			if(bCharacterController)
			{
				m_TestPhysics.ProcessController(m_input);
			}
			if(m_input.IsButtonDn(E_Button_X))
			{
				static F32 pressTime = 0.f;
				if((time - pressTime) > minPressFreq)
				{
					if(bCharacterController == false)
					{
						SetCamera(Samples::E_CameraType_Chase);
					}
					else
					{
						SetCamera(Samples::E_CameraType_Fly);
					}
					bCharacterController = !bCharacterController;
					pressTime = time;
				}
			}

			if(m_input.IsButtonDn(E_Button_LtShoulder))
			{
				static F32 pressTime = 0.f;
				if((time - pressTime) > minPressFreq)
				{
					m_TestPhysics.m_Controller.Jump(Maths::HF32(5.f));
					pressTime = time;
				}
			}

			if(m_input.IsButtonDn(E_Button_RtShoulder))
			{
				//Crouch!!!
			}


			//Now put random wind gusts into the scene



			bool bDoRaytrace(g_dbDoRaytrace);
			bool bDoTransparentRaytrace(g_dbDoTransparentRaytrace);

			if(bDoRaytrace)
				m_TestPhysics.DoRayTrace(Maths::HF32(100.f));

			if(bDoTransparentRaytrace)
				m_TestPhysics.DoTransparentRayTrace(Maths::HF32(100.f));

			m_TestPhysics.AnimateAndDraw(Maths::C_TimeS(0.01f));//1.f/60.f));
			//if(demoMode == E_CharacterControllerDemo)
			if(bCharacterController)
			{
				Maths::HVec3 pos(m_TestPhysics.m_Controller.Position());
				C_Application* g_App=Samples::GetApp();
				const Samples::C_View* view(g_App->GetView(0));
				Samples::C_Camera& camera(view->m_Camera);
				//camera.m_ChaseDistance = Maths::Constants<Maths::HF32>::One();
				camera.m_At = pos;
			}
			if(bDoRaytrace)
				m_TestPhysics.GetRayTraceResults();
		}
	}
#endif
};
