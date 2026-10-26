#include "StdAfx.h"
#include "TestPhysics.h"
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
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/narrowphase/optimisedaabbtree.inl"
#include "gravity/SharedPhysics/ContactStreamIterator.inl"
#include "gravity/importer/Importer.h"
#include "gravity/sdk/charactercontroller.h"
#include <Framework/Application/ApplicationBase.h>
#include <Framework/Application/UserControls.h>
#include <camera2.h>
#include <camera2/stock/CharacterCamera.h>
#include <Input/Device.h>

namespace Samples
{
	Debug::Bool g_WallDemo("Test.WallDemo", true);
	Debug::Bool g_Ragdoll("Test.Ragdol", false);
	Debug::Bool g_dbDrawMesh("Test.DrawMesh", true);
	Debug::Bool g_dbDrawLocation("Test.DrawLocation", true);

	Debug::Bool bDrawContacts("Test.DrawContacts", false);
	Debug::Bool bDrawBoundingBox("Test.DrawBoundingBoxes", false);
	Debug::Bool g_UseScheduler("Test.UseScheduler", true);
	Debug::Bool g_DrawRayResults("Test.DrawRayResults", false);
	Debug::Bool g_dbCullObjects("Test.CullObjects", true);

	enum E_DemoMode
	{		
		E_WallDemo,
		E_SphereDemo,
		E_CapsuleDemo,
		E_CylinderDemo,
		E_ConvexHullDemo, 
		E_CompositeDemo,
		E_SphericalDemo,
		E_HingeDemo,
		E_PrismaticDemo,
		E_PrismaticJointLimitDemo,
		E_CarDemo,
		E_BridgeDemo,
		E_CatterpillarTracksDemo,
		E_MaxDemos,
		E_HingeJointLimitDemo
	};

	void C_ContactDrawer::ProcessCollisions(SharedPhysics::C_ContactStream& stream)
	{
		if(bDrawContacts)
		{
			SharedPhysics::C_ContactStreamIterator iter(&stream);
			while(iter.hasNextContact())
			{
				SharedPhysics::C_ContactStreamResult* result(iter.nextContact());
				//if(result->m_pNodeA->TestNodeFlags(Gravity::C_GraphNode::E_Type_Active) && 
				//	result->m_pNodeB->TestNodeFlags(Gravity::C_GraphNode::E_Type_Active))

				Gravity::C_Body* pBodyA((Gravity::C_Body*)result->m_pNodeA->m_pUserData);
				Gravity::C_Body* pBodyB((Gravity::C_Body*)result->m_pNodeB->m_pUserData);
				//if(result->m_pNodeA->TestNodeFlags(Gravity::C_GraphNode::E_Type_Active) && 
			//		result->m_pNodeB->TestNodeFlags(Gravity::C_GraphNode::E_Type_Active))*/

				//if(pBodyA->GetShape()->GetShape() == Collision::E_Box_Shape && 
				//	pBodyB->GetShape()->GetShape() == Collision::E_Box_Shape)
				{
					while(iter.hasNextPatch())
					{
						SharedPhysics::C_ContactStreamPatch* patch(iter.nextPatch());
						const Maths::HVec3 norm(Maths::Load<Maths::HVec3>(&patch->m_Normal));
						while(iter.hasNextPoint())
						{
							SharedPhysics::C_ContactStreamPoint* point(iter.nextPoint());
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
	}

	using namespace Gravity;
	Gravity::C_Body* C_TestPhysics::AddBox(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF extents, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = mass * 0.001f;
		deactivator.m_SleepThresholdSq = mass * 0.0005f;
		C_Body* boxBody(m_pScene->CreateBody());
		boxBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_BoxDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > boxDesc;
		boxDesc.m_HalfExtents = extents;
		Maths::HF32 margin(Maths::Min(extents.X(), Maths::Min(extents.Y(), extents.Z())));
		margin *= 0.2f;
		boxDesc.m_Margin = margin;
		boxDesc.m_Epsilon = margin * 0.005f;
		Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > bodyHelper(&boxDesc);
		bodyHelper.CreateBody(mass, position, orientation, boxBody, pFilter, 0);
		boxBody->SetDeactivator(deactivator);
		boxBody->Slop(F32(margin) * 0.001f);
		boxBody->Baumgarte(0.75f);
		
		m_pScene->AddBody(boxBody);
		return boxBody;
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
		Gravity::C_CharacterControllerDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > controllerDesc;
		controllerDesc.m_CapsuleHeight = height;
		controllerDesc.m_CapsuleRadius = radius;
		controllerDesc.m_RayHeight = Maths::Max(rayHeight - rayRadius, zero);
		controllerDesc.m_RayRadius = rayRadius;
		controllerDesc.m_MaxCosAngle = Maths::HF32(0.5f); // cos 60
		controllerDesc.m_MinPen = rayHeight;
		controllerDesc.m_FeetMaterial = 0;
		Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > bodyHelper(&controllerDesc);
		bodyHelper.CreateBody(mass, position, orientation, controllerBody, pFilter, CHARACTER_CONTROLLER);
		controllerBody->SetDeactivator(deactivator);
		controllerBody->Slop(0.00000001f);
		controllerBody->Baumgarte(0.6f);
		//controllerBody->MakeStatic();
		m_pScene->AddBody(controllerBody);
		return controllerBody;
	}

	Gravity::C_Body* C_TestPhysics::AddSphere(Maths::HVec3 PCREF position, const Maths::HF32 radius, Maths::HF32 PCREF mass, 
		Collision::C_CollisionFilter* pFilter)
	{
		Maths::HMat33 orientation;
		orientation.SetIdentity();
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.05f;
		deactivator.m_SleepThresholdSq = 0.005f;
		C_Body* sphereBody(m_pScene->CreateBody());
		sphereBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_SphereDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > sphereDesc;
		sphereDesc.m_Radius = radius;
		sphereDesc.m_Epsilon = radius * 0.001f;
		Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > bodyHelper(&sphereDesc);
		bodyHelper.CreateBody(mass, position, orientation, sphereBody, pFilter, 0);
		sphereBody->SetDeactivator(deactivator);
		m_pScene->AddBody(sphereBody);
		sphereBody->Slop(radius * 0.0001f);
		sphereBody->Baumgarte(0.8f);
		return sphereBody;
	}

	Gravity::C_Body* C_TestPhysics::AddCapsule(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter *pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.01f;
		deactivator.m_SleepThresholdSq = 0.001f;
		C_Body* capsuleBody(m_pScene->CreateBody());
		capsuleBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_CapsuleDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > capsuleDesc;
		capsuleDesc.m_Height = height;
		capsuleDesc.m_Radius=radius;//local space
		capsuleDesc.m_Epsilon = Maths::Max(height, radius) * 0.005f;
		Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > bodyHelper(&capsuleDesc);
		bodyHelper.CreateBody(mass, position, orientation, capsuleBody, pFilter, 0);
		capsuleBody->SetDeactivator(deactivator);
		m_pScene->AddBody(capsuleBody);
		capsuleBody->Slop(0.00000005f);
		capsuleBody->Baumgarte(0.95f);
		return capsuleBody;
		
	}

	Gravity::C_Body* C_TestPhysics::AddCylinder(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF Height, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.01f;
		deactivator.m_SleepThresholdSq = 0.005f;
		C_Body* cylinderBody(m_pScene->CreateBody());
		cylinderBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_CylinderDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > cylinderDesc;
		//cylinderDesc.m_Center = position;
		cylinderDesc.m_Height=Height;
		cylinderDesc.m_Radius=radius;
		const Maths::HF32 margin(Maths::Min(radius, Height) * 0.2f);
		const Maths::HF32 epsilon(Maths::Max(radius, Height) * 0.001f);
		cylinderDesc.m_Margin = 0.f; //margin;
		cylinderDesc.m_Epsilon = epsilon;
		Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > bodyHelper(&cylinderDesc);
		bodyHelper.CreateBody(mass, position, orientation, cylinderBody, pFilter, 0);
		cylinderBody->SetDeactivator(deactivator);
		m_pScene->AddBody(cylinderBody);
		cylinderBody->Slop(0.00000005f);
		cylinderBody->Baumgarte(0.8f);
		return cylinderBody;
		
	}

	Gravity::C_Body* C_TestPhysics::AddCompositeShape(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF extents, Collision::C_CollisionFilter* pFilter)
	{
		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.005f;
		deactivator.m_SleepThresholdSq = 0.001f;
		C_Body* boxBody(m_pScene->CreateBody());
		boxBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_CompositeDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > desc;
		Gravity::C_CompositeDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > desc2;
		Gravity::C_BoxDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > boxDescA;
		boxDescA.m_HalfExtents = Maths::HVec3(0.2f, 0.2f, 0.2f);
		boxDescA.m_Margin = 0.02f;
		boxDescA.m_Epsilon = 0.0005f;

		Gravity::C_SphereDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > sphereDescA;
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

		Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > bodyHelper(&desc);
		bodyHelper.CreateBody(2.5f, position, orientation, boxBody, pFilter, 1);
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
		deactivator.m_ActivateThresholdSq = 0.005f;
		deactivator.m_SleepThresholdSq = 0.001f;

		C_Body* convexBody(m_pScene->CreateBody());

		convexBody->SetExternalConstants(&m_ExternalConstants);
		Gravity::C_ConvexHullDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > desc;
		
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

		desc.m_Margin = 0.002f;
		desc.m_Epsilon = 0.0005f;

		Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > bodyHelper(&desc);
		bodyHelper.CreateBody(mass, position, orientation, convexBody, pFilter, 0);
		convexBody->SetDeactivator(deactivator);
		m_pScene->AddBody(convexBody);
		convexBody->Slop(0.000005f);
		convexBody->Baumgarte(0.8f);
		return convexBody;
	}

	void C_TestPhysics::ShootABox(Collision::C_CollisionFilter* pFilter)
	{
		Cam2::C_Camera& cam = LiveCamera();

		Maths::HVec3 position(cam.Position());
		Maths::HVec3 dir(cam.Mat43().AZ());
		dir.Normalise();

		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(0.15f, 0.15f, 0.15f);

		C_Body* body=AddBox(position, orientation, extents, 1.f, pFilter);
		//Maths::HVec3 force(dir * 2000.f);
		//force.Z(-force.Z());
		body->LinearVelocity(dir * 15.f);
		body->AngularVelocity(Maths::HVec3(0.707f, 0.707f, 0.f));

	}

	void C_TestPhysics::ShootARagdoll(Collision::C_CollisionFilter* pFilter)
	{
		Cam2::C_Camera& cam = LiveCamera();

		Maths::HVec3 position(cam.Position());
		Maths::HVec3 dir(cam.Mat43().AZ());
		dir.Normalise();

		C_Body* pRagdollBody(SetUpRagdoll(pFilter, position + Maths::HVec3(1.f, 0.f, 1.f)));
		//Maths::HVec3 force(dir * 20000.f);
		//pRagdollBody->ApplyForce(force, position);
		pRagdollBody->LinearVelocity(dir * 20.f);
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

	void C_TestPhysics::SetUpConvexHull(Collision::C_CollisionFilter* pFilter)
	{
		Maths::RotZ xrot(Maths::Constants<F32>::Pi() * Maths::Constants<F32>::Half());
		Maths::HMat33 rot;
		Maths::C_Quaternion orientation(rot);
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());
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
			Gravity::C_Body* pBody = AddSphere(pos, radius, mass, pFilter);
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
		Gravity::C_CollisionFilter* pFilter)
	{
		Maths::HVec3 position(startPoint);
		//First and last plank are static!
		Maths::HMat33 orientation;
		orientation.SetIdentity();
		Maths::HF32 mass(2.f);
		Gravity::C_Body* firstBody(AddBox(position, orientation, plankExtents, mass, pFilter));
		firstBody->MakeStatic();
		Gravity::SphericalJointDescriptor< Sys::BinaryDeserialiser<Ut::BigEndianToNative > > desc;
		m_pScene->AddBody(firstBody);
		position += layDirection * layGap;
		Gravity::C_Body* nextBody = AddBox(position, orientation, plankExtents, mass, pFilter);
		desc.m_AnchorPoint1 = Maths::HVec3(-plankExtents.X() - slop, 0.f, -plankExtents.Z());
		desc.m_AnchorPoint2 = Maths::HVec3(plankExtents.X() + slop, 0.f, -plankExtents.Z());
		desc.CreateConstraint(*m_pScene->m_pGraphManager, nextBody, firstBody);
		desc.m_AnchorPoint1 = Maths::HVec3(-plankExtents.X() - slop, 0.f, plankExtents.Z());
		desc.m_AnchorPoint2 = Maths::HVec3(plankExtents.X() + slop, 0.f, plankExtents.Z());
		desc.CreateConstraint(*m_pScene->m_pGraphManager, nextBody, firstBody);
		firstBody = nextBody;

		for(U32 a = 2; a < numPlanks; ++a)
		{
			position += layDirection * layGap;
			nextBody = AddBox(position, orientation, plankExtents, mass, pFilter);
			desc.m_AnchorPoint2 = Maths::HVec3(-plankExtents.X() - slop, 0.f, -plankExtents.Z());
			desc.m_AnchorPoint1 = Maths::HVec3(plankExtents.X() + slop, 0.f, -plankExtents.Z());
			desc.CreateConstraint(*m_pScene->m_pGraphManager, firstBody, nextBody);
			desc.m_AnchorPoint2 = Maths::HVec3(-plankExtents.X() - slop, 0.f, plankExtents.Z());
			desc.m_AnchorPoint1 = Maths::HVec3(plankExtents.X() + slop, 0.f, plankExtents.Z());
			desc.CreateConstraint(*m_pScene->m_pGraphManager, firstBody, nextBody);
			firstBody = nextBody;
		}
		firstBody->MakeStatic();
	}

	void C_TestPhysics::SetUpSphericalJoint(Collision::C_CollisionFilter* pFilter)
	{
		
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(15.f, 2.5f, 0.f), orientation, extents, mass, pFilter));
		pBoxA->MakeStatic();
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(15.f, 2.f, 0.f), orientation, extents, mass, pFilter));
		
		SphericalJointDescriptor< Sys::BinaryDeserialiser<Ut::BigEndianToNative > > desc;
		desc.DefineAnchorPoints(Maths::HVec3(-0.25f, 0.3f, -0.25f), Maths::HVec3(0.f, -0.3f, 0.f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);
		

		Gravity::C_Body* pBoxC(AddBox(Maths::HVec3(15.f, 1.5f, 0.f), orientation, extents, mass, pFilter));
		
		desc.DefineAnchorPoints(Maths::HVec3(0.25f, -0.3f, 0.25f), Maths::HVec3(-0.25f, 0.3f, -0.25f) );
		desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxC);

		//pBoxC->ApplyForce(Maths::HVec3(300.f, 0.f, 300.f), Maths::HVec3(0.f, 6.5f, 0.f));

	//	Gravity::C_ConstraintEdge* pEdge(m_pScene->GetConstraintEdge());
	//	Gravity::C_ConstraintTwin* pTwin(m_pScene->GetConstraintTwin());
	//	/*Gravity::C_ConstraintEdge* pEdge2(m_pScene->GetConstraintEdge());
	//	Gravity::C_ConstraintTwin* pTwin2(m_pScene->GetConstraintTwin());*/
	//	Gravity::C_ConstraintPoint* pPoint(m_pScene->GetConstraintPoint());
	//	//Gravity::C_ConstraintPoint* pPoint2(m_pScene->GetConstraintPoint());
	//	pEdge->AddConstraintPoint(pPoint);
	//	//pEdge2->AddConstraintPoint(pPoint2);
	//	
	//	sphericalConstraint.m_AnchorPoint1 = Maths::HVec3(0.f, 1.f, 0.f);
	//	sphericalConstraint.m_AnchorPoint2 = Maths::HVec3(0.f, -1.f, 0.f);
	//	sphericalConstraint.pNodeA = pBoxB->GetGraphNode();
	//	sphericalConstraint.pNodeB = pBoxA->GetGraphNode();

	//	//sphericalConstraint2.m_AnchorPoint1 = Maths::HVec3(0.f, -1.f, 0.f);
	//	//sphericalConstraint2.m_AnchorPoint2 = Maths::HVec3(0.f, 1.f, 0.f);
	//	//sphericalConstraint2.pNodeA = pBoxB->GetGraphNode();
	//	//sphericalConstraint2.pNodeB = pBoxC->GetGraphNode();
	//	//
	//	pPoint->m_pConstraint = &sphericalConstraint;
	//	//pPoint2->m_pConstraint = &sphericalConstraint2;

	//	Gravity::LinkPersistent(pBoxB->GetGraphNode(), pBoxA->GetGraphNode(), pEdge, pTwin);
	//	//Gravity::LinkPersistent(pBoxB->GetGraphNode(), pBoxC->GetGraphNode(), pEdge2, pTwin2);
	//	pBoxB->ApplyForce(Maths::HVec3(300.f, 0.f, 300.f), Maths::HVec3(0.f, 6.5f, 0.f));
	}


	void C_TestPhysics::SetUpRevoluteJoint(Collision::C_CollisionFilter* pFilter)
	{
		
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(13.f, 1.f, 0.f), orientation, extents, mass, pFilter));
		//pBoxA->MakeStatic();
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(13.5f, 1.f, 0.f), orientation, extents, mass, pFilter));
		//pBoxB->MakeStatic();
		
		RevoluteJointDescriptor< Sys::BinaryDeserialiser<Ut::BigEndianToNative > > desc;
		desc.DefineJointAxis(Maths::HVec3(0.f, 0.1f, 0.f), Maths::HVec3(0.f, 1.f, 0.f));
		desc.DefineAnchorPoints(Maths::HVec3(-0.25f, -0.f, -0.25f), Maths::HVec3(0.25f, 0.f, 0.25f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);

		m_Constraints.AddToEnd(&constraint);
		pBoxB->ApplyForce(Maths::HVec3(0, 0, 500.f), pBoxB->Position());
		

		/*Gravity::C_Body* pBoxC(AddBox(Maths::HVec3(0.f, 7.f, 0.f), orientation, extents, mass*3.f, pFilter));
		
		desc.DefineAnchorPoints(Maths::HVec3(0.5f, -0.65f, 0.5f), Maths::HVec3(-0.5f, 0.65f, -0.5f));
		desc.CreateConstraint(m_pScene->m_GraphManager, pBoxB, pBoxC);

		pBoxC->ApplyForce(Maths::HVec3(300.f, 0.f, 300.f), Maths::HVec3(0.f, 6.5f, 0.f));*/

	}

	void C_TestPhysics::SetUpRevoluteJointLimit(Collision::C_CollisionFilter* pFilter)
	{
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		Gravity::C_Body* pBoxA(AddBox(Maths::HVec3(13.f, 1.f, 0.f), orientation, extents, mass, pFilter));
		//pBoxA->MakeStatic();
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(13.5f, 1.f, 0.f), orientation, extents, mass, pFilter));
		//pBoxB->MakeStatic();
		
		RevoluteJointLimitDescriptor< Sys::BinaryDeserialiser<Ut::BigEndianToNative > > desc;
		desc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		desc.DefineAnchorPoints(Maths::HVec3(-0.25f, -0.25f, 0.f), Maths::HVec3(0.25f, -0.25f, 0.f));
		desc.DefineJointLimit(Maths::HF32(1.57f), Maths::HF32(0.25f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxB, pBoxA);

		m_Constraints.AddToEnd(&constraint);
		pBoxB->ApplyForce(Maths::HVec3(0, 0, 500.f), pBoxB->Position());
		
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
		//pBoxA->MakeStatic();
		Gravity::C_Body* pBoxB(AddBox(Maths::HVec3(13.f, 1.f, 0.f), orientation, extents2, mass, pFilter));
		//pBoxB->MakeStatic();
		/*Gravity::C_RigidBody& rBody = pBoxB->GetBody();
		rBody.m_IBody = rBody.m_IBodyInverse = rBody.m_InertiaTensor = rBody.m_InverseInertiaTensor = Maths::HMat33(Maths::ZERO_MATRIX);
		rBody.m_Mass = 1000.f;
		rBody.m_ReciprocalMass = 0.f;*/

		const Maths::HVec3 axis(1.f, 0.f, 0.f);
		
		PrismaticJointDescriptor<Sys::BinaryDeserialiser< Ut::BigEndianToNative > > desc;
		//desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.5f, 0.f), Maths::HVec3(0.f, 0.5f, 0.f));
		desc.DefineJointAxis(axis, axis);
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);

		m_Constraints.AddToEnd(&constraint);
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
		const Maths::HF32 lowLimit(-0.5f);
		const Maths::HF32 highLimit(1.f);
		PrismaticJointLimitDescriptor<Sys::BinaryDeserialiser< Ut::BigEndianToNative > > desc;
		//desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.5f, 0.f), Maths::HVec3(0.f, 0.5f, 0.f));
		desc.DefineJointLimit(lowLimit, highLimit);
		desc.DefineJointAxis(axis, axis);
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pBoxA, pBoxB);

		m_Constraints.AddToEnd(&constraint);
	}

	void C_TestPhysics::SetUpCar(Collision::C_CollisionFilter* pFilter)
	{
		const Maths::HF32 pi(Maths::Constants<Maths::HF32>::Pi());
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		Maths::HMat33 orientation;
		Gravity::C_Body* pCarBody(AddBox(Maths::HVec3(13.f, 2.f, 0.f), orientation, Maths::HVec3(0.25f, 0.16f, 0.5f), Maths::HF32(10.f), pFilter));
		//pCarBody->MakeStatic();
		//Add in the 4 wheel bodies!
		//Maths::RotZ rotz(pi * half);
		//orientation = Maths::HMat33(rotz);
		Gravity::C_Body* pWheel0(AddSphere(pCarBody->Position() + Maths::HVec3(0.22f, -0.23f, 0.47f), Maths::HF32(0.05f), Maths::HF32(2.f), pFilter));
		//pWheel0->MakeStatic();
		Gravity::C_Body* pWheel1(AddSphere(pCarBody->Position() + Maths::HVec3(-0.22f, -0.23f, 0.47f),Maths::HF32(0.05f), Maths::HF32(2.f), pFilter));
		//pWheel1->MakeStatic();
		Gravity::C_Body* pWheel2(AddSphere(pCarBody->Position() + Maths::HVec3(0.22f, -0.23f, -0.47f),Maths::HF32(0.05f), Maths::HF32(2.f), pFilter));
		//pWheel2->MakeStatic();
		Gravity::C_Body* pWheel3(AddSphere(pCarBody->Position() + Maths::HVec3(-0.22f, -0.23f, -0.47f),Maths::HF32(0.05f), Maths::HF32(2.f), pFilter));
		//pWheel3->MakeStatic();

		SharedPhysics::C_Constraint* constraint;
		Gravity::CarWheelDescriptor<Sys::BinaryDeserialiser< Ut::BigEndianToNative > > carWheelDesc;
		carWheelDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(0.f, 1.f, 0.f));
		carWheelDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.f, 0.f), Maths::HVec3(0.22, -0.25f, 0.47f));
		carWheelDesc.DefineOffsetAngle(pi * half);

		constraint = carWheelDesc.CreateConstraint(*m_pScene->m_pGraphManager, pWheel0, pCarBody);
		m_Constraints.AddToEnd(&constraint);

		carWheelDesc.DefineJointAxis(Maths::HVec3(-1.f, 0.f, 0.f), Maths::HVec3(0.f, 1.f, 0.f));
		carWheelDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.f, 0.f), Maths::HVec3(-0.22, -0.25f, 0.47f));
		constraint = carWheelDesc.CreateConstraint(*m_pScene->m_pGraphManager, pWheel1, pCarBody);
		m_Constraints.AddToEnd(&constraint);


		//Gravity::RevoluteJointDescriptor sphereDesc;
		Gravity::SphericalJointDescriptor<Sys::BinaryDeserialiser< Ut::BigEndianToNative > > sphereDesc;
		sphereDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.f, 0.f), Maths::HVec3(0.22, -0.25f, -0.47f));
		//sphereDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		constraint = sphereDesc.CreateConstraint(*m_pScene->m_pGraphManager, pWheel2, pCarBody);
		m_Constraints.AddToEnd(&constraint);

		sphereDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.f, 0.f), Maths::HVec3(-0.22, -0.25f, -0.47f));
		constraint = sphereDesc.CreateConstraint(*m_pScene->m_pGraphManager, pWheel3, pCarBody);
		m_Constraints.AddToEnd(&constraint);
	}

	Gravity::C_Body* C_TestPhysics::SetUpRagdoll(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position)
	{
		Maths::HMat33 rot;
		rot.SetIdentity();
		Maths::C_Quaternion orientation(rot);
		//const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half());
		const Maths::HF32 mass(Maths::Constants<Maths::HF32>::Three());

		//Gravity::C_Body* pHead(AddBox(Maths::HVec3(0.f, 1.5f, 0.f) + position, orientation, Maths::HVec3(0.5f, 0.5f, 0.5f), Maths::HF32(2.f), pFilter));
		Gravity::C_Body* pHead(AddSphere(Maths::HVec3(0.f, 0.44f, 0.f) + position, Maths::HF32(0.1f), Maths::HF32(0.5f), pFilter));
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
		
		Gravity::C_Body* pLeftArmTop(AddCapsule(Maths::HVec3(-0.41f, 0.15f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pLeftArmTop->MakeStatic();

		//Gravity::C_Body* pLeftArmBottom(AddBox(Maths::HVec3(-4.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pLeftArmBottom(AddCapsule(Maths::HVec3(-0.82f, 0.15f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//pLeftArmBottom->MakeStatic();

		//Gravity::C_Body* pRightArmTop(AddBox(Maths::HVec3(2.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pRightArmTop(AddCapsule(Maths::HVec3(0.41f, 0.15f, 0.f) + position, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
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

		SphericalJointDescriptor<Sys::BinaryDeserialiser< Ut::BigEndianToNative > > desc;
		RevoluteJointDescriptor<Sys::BinaryDeserialiser< Ut::BigEndianToNative > > revDesc;

		desc.DefineAnchorPoints(Maths::HVec3(0.f, 0.32f, 0.f), Maths::HVec3(0.f, -0.12f, 0.f));
		//desc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHead);
		m_Constraints.AddToEnd(&constraint);

		desc.DefineAnchorPoints(Maths::HVec3(-0.21f, 0.15f, 0.f), pLeftArmTop->Orientation() * Maths::HVec3(0.21f, 0.f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pLeftArmTop);
		m_Constraints.AddToEnd(&constraint);

		revDesc.DefineAnchorPoints(pLeftArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f), pLeftArmBottom->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f));
		revDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftArmTop, pLeftArmBottom);
		m_Constraints.AddToEnd(&constraint);

		desc.DefineAnchorPoints(Maths::HVec3(0.21f, 0.15f, 0.f), pRightArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pRightArmTop);
		m_Constraints.AddToEnd(&constraint);

		revDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		revDesc.DefineAnchorPoints(pRightArmTop->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f), pRightArmBottom->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pRightArmTop, pRightArmBottom);
		m_Constraints.AddToEnd(&constraint);

		revDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.31f, 0.f), pHip->Orientation()*Maths::HVec3(0.f, 0.085f, 0.f));
		revDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHip);
		m_Constraints.AddToEnd(&constraint);

		desc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(-0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pLeftLegTop);
		m_Constraints.AddToEnd(&constraint);

		revDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		revDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftLegTop, pLeftLegBottom);
		m_Constraints.AddToEnd(&constraint);

		desc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pRightLegTop);
		m_Constraints.AddToEnd(&constraint);

		revDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		revDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		constraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pRightLegTop, pRightLegBottom);
		m_Constraints.AddToEnd(&constraint);

		return pTorso;

	}

	void C_TestPhysics::AnchorPointDraw()
	{
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			SharedPhysics::C_Constraint* constraint=m_Constraints[i];

			switch(constraint->m_ConstraintType)
			{
			case SharedPhysics::C_Constraint::E_Type_Fixed:
				break;
			case SharedPhysics::C_Constraint::E_Type_Spherical:
				{
					Solver::C_SphericalConstraint* sphericalConstraint=(Solver::C_SphericalConstraint*) constraint;
					SharedPhysics::C_GraphNode* pNodeA=sphericalConstraint->pNodeA;
					SharedPhysics::C_GraphNode* pNodeB=sphericalConstraint->pNodeB;
					SharedPhysics::C_RigidBody* pRigid1=pNodeA->m_pRigidBody;
					SharedPhysics::C_RigidBody* pRigid2=pNodeB->m_pRigidBody;
					const Maths::HVec3 point1(pRigid1->m_Position +  sphericalConstraint->m_AnchorPoint1 * pRigid1->m_OrientationMatrix);
					const Maths::HVec3 point2(pRigid2->m_Position +  sphericalConstraint->m_AnchorPoint2 * pRigid2->m_OrientationMatrix);

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point1, 0.05f, Maths::RGBA(0, 255, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point2, 0.05f, Maths::RGBA(255, 0, 0, 255));

				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Revolute:
				{
					Solver::C_RevoluteConstraint* revoluteConstraint=(Solver::C_RevoluteConstraint*) constraint;
					SharedPhysics::C_GraphNode* pNodeA=revoluteConstraint->pNodeA;
					SharedPhysics::C_GraphNode* pNodeB=revoluteConstraint->pNodeB;
					SharedPhysics::C_RigidBody* pRigid1=pNodeA->m_pRigidBody;
					SharedPhysics::C_RigidBody* pRigid2=pNodeB->m_pRigidBody;
					const Maths::HVec3 point1(pRigid1->m_Position +  revoluteConstraint->m_AnchorPoint1 * pRigid1->m_OrientationMatrix);
					const Maths::HVec3 point2(pRigid2->m_Position +  revoluteConstraint->m_AnchorPoint2 * pRigid2->m_OrientationMatrix);

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point1, 0.05f, Maths::RGBA(0, 255, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point2, 0.05f, Maths::RGBA(255, 0, 0, 255));


				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Prismatic:
				{
					Solver::C_PrismaticConstraint* prismaticConstraint=(Solver::C_PrismaticConstraint*) constraint;
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
					Solver::C_CarWheelConstraint* carWheelConstraint = (Solver::C_CarWheelConstraint*)constraint;
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
				break;
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
		/*const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		C_Application* g_App=GetApp();
		const C_View* view = g_App->GetView(0);
		const C_Camera& camera=view->m_Camera;
		Maths::HVec3 position(camera.m_Eye);
		Maths::C_Normal dir((camera.m_At - camera.m_Eye));

		const Maths::C_Normal up(camera.m_Up);
		const Maths::C_Normal right(up ^ dir);

		const Maths::HF32 negStartX(F32(X_RANGE/2));
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

	void C_TestPhysics::InitialiseTest(Collision::C_CollisionFilter* pFilter, const U32 demoMode)
	{
		m_IMeshManager.Reset();
		if(m_pScene)
		{
			Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseScene(m_pScene);
		}
		Collision::C_AxisBoxMinMax PCREF worldBox;
		worldBox.m_max = Maths::HVec3(100.f, 50.f, 100.f);
		worldBox.m_min = Maths::HVec3(-100.f, -40.f, -100.f);
		const Maths::HVec3 PCREF desiredSize(20.f, 20.f, 20.f);
		S32 maxDepth = 6;
		SetUpMaterials();
		m_pScene = Gravity::C_PhysicsSDK::PhysicsSDK()->CreateScene(&m_Materials[0][0], MATERIAL_COUNT);
		m_ExternalConstants.m_Gravity = Maths::HVec3(0, -9.8f, 0);
		m_pScene->RegisterCollisionCallback(&drawer);
		m_pScene->SetInternalTaskPolicy(collisionPolicy);
		m_pScene->SetInternalGroupHandle(Groups[1]);

		//First thing - load in the level file for level collisions...
		const Maths::HF32 zero;
		const Maths::HVec3 vzero;
		Maths::HMat33 mat;
		mat.SetIdentity();
		Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "PhysicsLevel.dat" );
		Sys::BinaryDeserialiser<Ut::BigEndianToNative> archive(*filePtr);


		Gravity::Importer<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > import;
		U32 m_Type;
		archive >> m_Type;
		const Maths::HF32 base(0.5f);
		const Maths::HVec3 p(13, 2, -5);
		U32 count=0;
		while(m_Type!=0xffffffff) // the end of primitives and mesh
		{
			Gravity::C_ShapeDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> >* sDesc=import.Import(archive, m_Type);
			//
			switch(sDesc->m_Type)
			{
			case Collision::E_TriangleMesh_Shape:
				{
				Gravity::C_Body* pBody(m_pScene->CreateBody());
				Gravity::C_TriangleMeshDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> >* triDes=(Gravity::C_TriangleMeshDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> >*)sDesc;
				Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > triMeshHelper(triDes);
				triMeshHelper.CreateBody(zero, vzero, mat, pBody, archive, pFilter, 0);
				//pBody->MakeStatic();
				m_pScene->AddMeshBodyToWorld(pBody);
				pBody->Slop(Maths::Constants<F32>::Max());
				pBody->Baumgarte(Maths::Constants<F32>::Max());

				worldBox.m_max = Maths::Max(worldBox.m_max, pBody->GetShape()->m_LocalBoundingBox.centre + pBody->GetShape()->m_LocalBoundingBox.extents);
				worldBox.m_min = Maths::Min(worldBox.m_min, pBody->GetShape()->m_LocalBoundingBox.centre - pBody->GetShape()->m_LocalBoundingBox.extents);

				m_pScene->ResizeScene(worldBox, desiredSize, maxDepth);
				break;
				}
			case Collision::E_TriangleMeshBase_Shape:
				{
					S32 m_Id;
					archive >> m_Id;
					archive >> m_Type;
					Gravity::C_TriangleMeshBaseDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> >* triDes=(Gravity::C_TriangleMeshBaseDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> >*)sDesc;
					//Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative>> triMeshHelper(triDes);
					//triMeshHelper.CreateBody(zero, vzero, mat, pBody, archive, pFilter, 0);
					Gravity::C_TriangleMeshShape* tri=(Gravity::C_TriangleMeshShape*)triDes->CreateShape(archive, pFilter);
					m_IMeshManager.Add(tri, m_Id);

					//worldBox.m_max = Maths::Max(worldBox.m_max, pTriMeshBody->GetShape()->m_LocalBoundingBox.centre + pTriMeshBody->GetShape()->m_LocalBoundingBox.extents);
					//worldBox.m_min = Maths::Min(worldBox.m_min, pTriMeshBody->GetShape()->m_LocalBoundingBox.centre - pTriMeshBody->GetShape()->m_LocalBoundingBox.extents);

					//m_pScene->ResizeScene(worldBox, desiredSize, maxDepth);
					
				break;
				}
			case Collision::E_InstancedTriangleMesh_Shape:
				{
					S32 m_UniqueId;
					S32 m_BaseId;

					Gravity::C_Body* pBody(m_pScene->CreateBody());
					Maths::HMat43 m_Transform;
					archive >> m_BaseId;	// the base instance id
					archive >> m_UniqueId; // the unique id assign to instance node by the exporter
					archive >> m_Transform;
					const Maths::HMat33 rotation(m_Transform.Rotation());
					Gravity::C_TriangleMeshShape* tri=m_IMeshManager.Find(m_BaseId);
					assert(tri!=NULL);
					Gravity::C_InstancedTriangleMeshDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> >* iTriDes=(Gravity::C_InstancedTriangleMeshDesc<Sys::BinaryDeserialiser<Ut::BigEndianToNative> >*)sDesc;
					iTriDes->m_pShape = tri;
					iTriDes->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Static;
					Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > triMeshHelper(iTriDes);
					triMeshHelper.CreateBody(zero, m_Transform.Translation(),m_Transform.Rotation(), pBody, pFilter, 0);
					pBody->Slop(Maths::Constants<F32>::Max());

					Collision::C_CRAxisBox box;
					pBody->GetShape()->m_LocalBoundingBox.Transform(m_Transform, box);
					worldBox.m_max = Maths::Max(worldBox.m_max, box.centre + box.extents);
					worldBox.m_min = Maths::Min(worldBox.m_min, box.centre - box.extents);

					m_pScene->ResizeScene(worldBox, desiredSize, maxDepth);
					//pBody->MakeStatic();
					//pBody->MakeInstancedCollidableMesh();
					//pBody->MakeStaticMesh();

					m_pScene->AddBody(pBody);
					break;
				}
			case Collision::E_Composite_Shape:
				{
				Gravity::C_Body* pBody(m_pScene->CreateBody());
				Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > helper(sDesc);
				const Maths::HF32 mass(1.f);
#if 1
				helper.CreateBody(mass,pBody, archive, &m_Filter, 0);
#else
				const Maths::HF32 x(p.X() + count*base);
				const Maths::HF32 y(p.Y() + count*base);
				const Maths::HF32 z(p.Z() + count*base);
				count++;

				const Maths::HVec3 position(x,y,z);
				sphereHelper.CreateBody(position,radius, mat, pBody, archive, &m_Filter, 0);
#endif
				
				//pBody->MakeStatic();
				pBody->SetExternalConstants(&m_ExternalConstants);
				pBody->Baumgarte(0.7f);
				m_pScene->AddBody(pBody);
				}

				break;
			default:
				{
					/*
						create the objects at the spot of a3d file pass to us
					*/
					Gravity::C_Body* pBody(m_pScene->CreateBody());
					Gravity::C_BodyHelper<Sys::BinaryDeserialiser<Ut::BigEndianToNative> > helper(sDesc);
					const Maths::HF32 mass(1.f);
#if 1
					helper.CreateBody(mass,pBody, archive, &m_Filter, 0);
#else
					const Maths::HF32 x(p.X() + count*base);
					const Maths::HF32 y(p.Y() + count*base);
					const Maths::HF32 z(p.Z() + count*base);
					count++;

					const Maths::HVec3 position(x,y,z);
					sphereHelper.CreateBody(position,radius, mat, pBody, archive, &m_Filter, 0);
#endif
					//pBody->MakeStatic();
					pBody->SetExternalConstants(&m_ExternalConstants);
					pBody->Baumgarte(0.7f);
					m_pScene->AddBody(pBody);

				}
				
			}

			archive >> m_Type;
		
		}



			m_Constraints.m_numElements = 0;

			{
				C_Body* pBody = AddCharacterController(Maths::HVec3(18.f, 2.5f, 0.f), Maths::HMat33(), 
					Maths::HF32(0.5f), Maths::HF32(0.3f), Maths::HF32(0.4f), Maths::HF32(0.2f), Maths::HF32(2.f), pFilter);
				this->m_Controller = Gravity::C_CharacterController(pBody, Maths::HF32(10.f), Maths::HF32(500.f));
				m_pScene->AddCharacterController(&m_Controller);
			}
			if(demoMode == E_WallDemo)
			{
				SetUpWall(8, 8, pFilter);
			}
			else if(demoMode == E_ConvexHullDemo)
			{
				SetUpConvexHull(pFilter);
			}
			else if (demoMode == E_CompositeDemo)
			{
				SetUpCompositeShapes(3, pFilter);
			}
			else if (demoMode == E_HingeDemo)
			{
				SetUpRevoluteJoint(pFilter);
			}
			else if (demoMode == E_SphericalDemo)
			{
				SetUpSphericalJoint(pFilter);
			}
			else if(demoMode == E_PrismaticDemo)
			{
				SetUpPrismaticJoint(pFilter);
			}
			else if (demoMode == E_CapsuleDemo)
			{
				SetUpCapsule(pFilter);
			}
			else if(demoMode == E_CylinderDemo)
			{
				SetUpCylinder(pFilter);
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
					Maths::C_Normal(1.f, 0.f, 0.f), Maths::HF32(0.42f), Maths::HF32(0.01f),
					Maths::HVec3(0.3f, 0.1f, 1.f), 45, pFilter);
			}
		
		delete filePtr;

		if (m_Camera)
			delete m_Camera;
		
		m_Camera = new Cam2::C_CharacterCamera;
		m_CameraCycle = 0;
		View().DebugFeed().ShowFeed(m_Camera);
	}

	void C_TestPhysics::DrawCompositeShape(const Collision::C_CompositeShape* pCompShape)
	{
		static bool subComp=false;
		for(U32 i = 0; i < pCompShape->m_NumShapes; ++i)
		{
			switch(pCompShape->m_pShapes[i].m_pShape->GetShape())
			{
			case Collision::E_Box_Shape:
				{
			
				const Collision::C_BoxShape* const __restrict pBoxShape((Collision::C_BoxShape*)pCompShape->m_pShapes[i].m_pShape);

				Maths::HMat43 absPosRot = Maths::HMat43(pBoxShape->m_Box.u, pBoxShape->m_Box.c);
				Maths::HVec3 minV(-pBoxShape->m_Box.e);
				Maths::HVec3 maxV(pBoxShape->m_Box.e);
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
					const Maths::HVec3 center=pSphereShape->m_Sphere.Centre();
					
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

	void DrawMesh(const Collision::OptimisedAABBTree& tree, const Maths::HMat43 PCREF matrix, const Collision::C_CRAxisBox& untransformedBox,
		SharedPhysics::C_SafeStack<true>& stack)
	{
		//if(g_dbDrawMesh)
		{
			Collision::C_CRAxisBox box;
			untransformedBox.Transform(matrix.InverseRotTrans(), box);
			//const U32 maxTriangles(2048);
			//U32 triangleDrawn = 0;
			//Grab the current camera position!
			const Maths::HF32 three(Maths::Constants<Maths::HF32>::Three());

			//const Maths::RGBA colours[7]=
			//{
			//	RGBA(55, 55, 55), RGBA(255, 255, 55), RGBA(255, 55, 255), RGBA(55, 255, 255),
			//		RGBA(128, 128, 0), RGBA(0, 128, 128), RGBA(128, 0, 128)
			//};

			const Maths::RGBA colour(55, 55, 55);

			Collision::C_AxisBoxMinMax axisBox;

			//Traverse the triangle mesh...
			Collision::OptimisedAABBNode* pStackTop(tree.m_pNodes);
			stack.Push(&tree.m_pNodes[0], 0);
			S32 size(1);
			while(size--)
			{
				Collision::OptimisedAABBNode* pNode(pStackTop);
				pStackTop = stack.Pop<OptimisedAABBNode>(Maths::Max(size-1, 0));
				pNode->m_BoundingBox.LoadBox(axisBox, tree.m_Low, tree.m_High);
				if(axisBox.Intersects(box))
				{
					if(pNode->m_NumIndices > 0)
					{
						//Draw mesh!
						U32 processed = 0;
						
						const Collision::C_TriangleHeader* header(tree.m_Triangles.GetHeader(pNode->m_StartIndex));
						U32 offset = pNode->m_StartIndex + sizeof(Collision::C_TriangleHeader);
						//while(processed < pNode->m_NumIndices)
						
						U32 headerIndex = 0;
						for(U32 p = 0; p < pNode->m_NumIndices; ++p, ++headerIndex)
						{
							if(header->m_NumTriangles <= headerIndex)
							{
								header = tree.m_Triangles.GetHeader(offset);
								offset += sizeof(Collision::C_TriangleHeader);
								headerIndex = 0;
							}
							const Collision::AABBTreeTriangle* tri(tree.m_Triangles.GetTriangles(offset));
							offset += sizeof(Collision::AABBTreeTriangle);
							const Maths::HVec3 vA(Maths::Load<Maths::HVec3>(&tree.m_pVertices[tri->m_IndexA]));
							const Maths::HVec3 vB(Maths::Load<Maths::HVec3>(&tree.m_pVertices[tri->m_IndexB]));
							const Maths::HVec3 vC(Maths::Load<Maths::HVec3>(&tree.m_pVertices[tri->m_IndexC]));
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleLit(vA*matrix,vB*matrix, vC*matrix, colour);
							//const Maths::HVec3 norm = Maths::Load<Maths::HVec3>(&tree.m_pNormals[tri->m_NormalIndex]);
							//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow((vA + vB + vC)/Maths::Constants<Maths::HF32>::Three(),
							//	norm, 0.3f, 0.05f, Maths::RGBA(255, 0, 0));
						}
					}
					else
					{
						pStackTop = pNode + 1;
						stack.Push(pNode + pNode->m_RightChildOffset, size++);
						stack.Push(pNode + 1, size++);
						
					}
					//if(triangleDrawn > maxTriangles)
					//	goto Simulate;
				}
			}
		}
	}

	void C_TestPhysics::AnimateAndDraw(const Maths::C_TimeS PCREF time)
	{
		Framework::C_View& view = View();
		Cam2::C_Camera& camera = view.DebugFeed().Live();
		Maths::HVec3 position(camera.Position());
		
		DbUtil::DefaultDebugRenderSys()->Deferred()->SetLightDirection(Maths::HVec3(0.707f, 0.707f, 0.f));
		Collision::C_CRAxisBox _box, box;
		Maths::C_Normal dir(camera.Mat43().AZ());
		_box.centre = Maths::Constants<Maths::HVec3>::Zero();
		_box.extents = Maths::HVec3(8.f, 8.f, 5.f);

		Maths::C_Normal yAxis(camera.Mat43().AY());
		
		const C_Normal nx( yAxis ^ dir );
		Maths::HMat33 _mat(nx, yAxis, dir);
		_box.Transform(Maths::HMat43(_mat, camera.Position() + dir * Maths::HF32(5.f)), box);

		//Get the level mesh and draw it first...

		if(g_dbDrawLocation)
		{
			char data[256];
			sprintf(data, "(%2.2f, %2.2f, %2.2f)", F32(position.X()), F32(position.Y()), F32(position.Z()));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(150, 50), data);
		}

		if(g_dbDrawMesh)
		{
			Maths::HMat43 matrix;
			matrix.SetIdentity();
			//const U32 maxTriangles(2048);
			//U32 triangleDrawn = 0;
			//Grab the current camera position!
			const Maths::HF32 three(Maths::Constants<Maths::HF32>::Three());
			const U32 numMeshes(m_pScene->NumMeshBodies());

			//const Maths::RGBA colours[7]=
			//{
			//	RGBA(55, 55, 55), RGBA(255, 255, 55), RGBA(255, 55, 255), RGBA(55, 255, 255),
			//		RGBA(128, 128, 0), RGBA(0, 128, 128), RGBA(128, 0, 128)
			//};

			const Maths::RGBA colour(55, 55, 55);

			Collision::C_AxisBoxMinMax axisBox;
			for(U32 a = 0; a < numMeshes; ++a)
			{
				//Draw mesh!
				const Gravity::C_Body* pMeshBody(m_pScene->GetMeshBody(a));
				const Collision::C_TriangleMeshShape* pTriMeshShape((const Collision::C_TriangleMeshShape*)pMeshBody->GetShape());
				const Collision::OptimisedAABBTree & tree(pTriMeshShape->m_AABBTree);

				DrawMesh(tree, matrix, box, stack);
			}
		}
		
//Simulate:
		m_pScene->Simulate(time);
		const U32 numBodies(m_pScene->NumBodies());

		Maths::HMat43 identity;
		//char msg[128];
		for(U32 a = 0; a < numBodies; ++a)
		{
			const Gravity::C_Body* const __restrict pBody(m_pScene->GetBody(a));
			const C_Object* const __restrict pCollisionObject(pBody->GetObject());
			if(g_dbCullObjects && !pCollisionObject->m_TransformedBox.Intersects(box))
				continue;
			//Draw bounding box...
			if(bDrawBoundingBox)
			{
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(pCollisionObject->m_TransformedBox.centre - pCollisionObject->m_TransformedBox.extents,
					pCollisionObject->m_TransformedBox.centre + pCollisionObject->m_TransformedBox.extents, identity, Maths::RGBA(255, 255, 0));
			}
			if(pBody->GetShape()->GetShape() == Collision::E_Box_Shape)
			{
				const Collision::C_BoxShape* const __restrict pBoxShape((Collision::C_BoxShape*)pBody->GetShape());
				//Draw the box around the body!

				const Maths::HVec3 min(-pBoxShape->m_Box.e);
				const Maths::HVec3 max(pBoxShape->m_Box.e);
				//const Maths::HMat33 orientation(pBoxShape->Orientation());
				//const Maths::HVec3 position(pBoxShape->Position());
							
				/*if(pBody->IsActive())
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBoxLit(min, max, pBoxShape->m_PosRot, Maths::RGBA(0, 255, 0));
				}
				else*/
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBoxLit(min, max, pBoxShape->m_PosRot, Maths::RGBA(255, 0, 0));
				}
			}
			else if(pBody->GetShape()->GetShape() == Collision::E_Composite_Shape)
			{
				//Draw composite here...
				const Collision::C_CompositeShape* const __restrict pCompShape((Collision::C_CompositeShape*)pBody->GetShape());
				DrawCompositeShape(pCompShape);
			}
			else if (pBody->GetShape()->GetShape() == Collision::E_ConvexHull_Shape)
			{
				//We are going to hard-code in our 10-point convex hull here...
				C_ConvexHullShape* pShape((C_ConvexHullShape*)pBody->GetShape());
				U32 addy = (U32)alloca(sizeof(Maths::HVec3)*pShape->m_Hull.m_NumVertices + 16);
				addy = (addy + 15)& 0xfffffff0;
				Maths::HVec3* verts((Maths::HVec3*)addy);
				const Maths::HMat33 orientation(pShape->m_Hull.m_Orientation);
				const Maths::HVec3 position(pShape->m_Hull.m_Position);
				const Maths::RGBA colour(0, 0, 255);
				for(U32 b = 0; b < pShape->m_Hull.m_NumVertices; ++b)
				{
					verts[b] = position + (Maths::Load<Maths::HVec3>(&pShape->m_Hull.m_pVertices[b]) * orientation);
				}

				//Loop through all the convex triangles and draw them!

				for(U32 b = 0; b < pShape->m_Hull.m_NumTriangles; ++b)
				{
					Collision::C_ConvexTriangle& tri(pShape->m_Hull.m_pTriangles[b]);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleLit(verts[tri.m_VertexA],
						verts[tri.m_VertexB], verts[tri.m_VertexC], colour);	
				}

			}
			else if (pBody->GetShape()->GetShape() == Collision::E_Capsule_Shape)
			{
				Collision::C_CapsuleShape* pShape((C_CapsuleShape*)pBody->GetShape());

				const U32 numSegments = 2;
				const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
				const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
				const Maths::HVec3 d(pShape->m_Capsule.End() - pShape->m_Capsule.Start());
				const Maths::HF32 length(d.Length());
				const Maths::HVec3 unitD(d/length);
				const Maths::HF32 stepSize(Maths::Constants<Maths::HF32>::Half());
				Maths::HF32 ratio;
				for(int i=0; i<numSegments; i++)
				{
					HVec3 e(pShape->m_Capsule.Start()+d*ratio);
					//HVec3 e2(pShape->m_Cylinder.m_Center-pShape->m_Cylinder.m_CenterAxis*ratio*pShape->m_Cylinder.m_halfHeight);
					//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e1, pShape->m_Cylinder.m_CenterAxis, pShape->m_Cylinder.m_Radius, RGBA(255, 0, 0));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, unitD, pShape->m_Capsule.Radius(), RGBA(255, 0, 0));
					ratio += stepSize;
				}
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(pShape->m_Capsule.Start(), pShape->m_Capsule.Radius(), Maths::RGBA(255, 0, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(pShape->m_Capsule.End(), pShape->m_Capsule.Radius(), Maths::RGBA(255, 0, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pShape->m_Capsule.Start(), pShape->m_PosRot.Rotation().AX(), 0.1f, 0.1f, Maths::RGBA(255, 0, 0));
			}
			else if (pBody->GetShape()->GetShape() == Collision::E_Cylinder_Shape)
			{
				Collision::C_CylinderShape* pShape((C_CylinderShape*)pBody->GetShape());

				const U32 numSegments = 10;
				const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
				const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
				const Maths::HF32 stepSize(0.1f);
				Maths::HF32 ratio;
				for(int i=0; i<10; i++)
				{
					HVec3 e1(pShape->m_Cylinder.m_Center+pShape->m_Cylinder.m_CenterAxis*ratio*pShape->m_Cylinder.m_halfHeight);
					HVec3 e2(pShape->m_Cylinder.m_Center-pShape->m_Cylinder.m_CenterAxis*ratio*pShape->m_Cylinder.m_halfHeight);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e1, pShape->m_Cylinder.m_CenterAxis, pShape->m_Cylinder.m_Radius, RGBA(255, 0, 0));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e2, pShape->m_Cylinder.m_CenterAxis, pShape->m_Cylinder.m_Radius, RGBA(255, 0, 0));
					ratio += stepSize;
				}
			}
			else if (pBody->GetShape()->GetShape() == Collision::E_Sphere_Shape)
			{
				Collision::C_SphereShape* pShape((C_SphereShape*)pBody->GetShape());
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(pShape->m_Sphere.Centre(), pShape->m_Sphere.Radius(), Maths::RGBA(255, 0, 0));
			}
			else if (pBody->GetShape()->GetShape() == Collision::E_InstancedTriangleMesh_Shape)
			{
				Collision::C_InstancedTriangleMeshShape* pTriShape = (Collision::C_InstancedTriangleMeshShape*)pBody->GetShape();
				Collision::C_InstancedAABBTree& tree = pTriShape->m_AABBTree;

				DrawMesh(*tree.m_pTree, tree.m_PosRot, box, stack);
			}
			else if (pBody->GetShape()->GetShape() == Collision::E_CharacterController_Shape)
			{
				Collision::C_CharacterControllerShape* pShape = (Collision::C_CharacterControllerShape*)pBody->GetShape();
				Collision::C_Capsule& capsule = pShape->m_Controller.m_EnclosingCapsule;

				Maths::RGBA colour(0, 255, 0);
/*
				if(!pBody->IsActive())
				{
					colour = Maths::RGBA(255, 0, 0);
				}*/

				const U32 numSegments = 2;
				const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
				const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
				const Maths::HVec3 d(capsule.End() - capsule.Start());
				const Maths::HF32 length(d.Length());
				const Maths::HVec3 unitD(d/length);
				const Maths::HF32 stepSize(Maths::Constants<Maths::HF32>::Half());
				Maths::HF32 ratio;
				for(int i=0; i<numSegments; i++)
				{
					HVec3 e(capsule.Start()+d*ratio);
					//HVec3 e2(pShape->m_Cylinder.m_Center-pShape->m_Cylinder.m_CenterAxis*ratio*pShape->m_Cylinder.m_halfHeight);
					//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e1, pShape->m_Cylinder.m_CenterAxis, pShape->m_Cylinder.m_Radius, RGBA(255, 0, 0));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, unitD, capsule.Radius(), colour);
					ratio += stepSize;
				}
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.Start(), capsule.Radius(), colour);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.End(), capsule.Radius(), colour);
				//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(pShape->m_Controller.m_Ray.m_start, pShape->m_Controller.m_Ray.m_end, colour);
			}

			//const Gravity::C_GraphNode * pNode(pBody->GetGraphNode());
			//if(pNode->m_IslandIndex >= 0)
			//{
			//	sprintf(msg, "%i", pNode->m_IslandIndex);
			//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(pBody->Position(), Util::C_String(msg));
			//}
		}
		//AnchorPointDraw();

		assert(m_Camera);

		Cam2::C_CamObj object;
		object.position = m_Controller.Position();
		m_Camera->SetObject(object);
		m_Camera->Update(1.f / 60.f);
		m_Camera->DebugRender(true);

		// Update camera controls:
		view.DebugFeed().SetInput( Framework::GetFlyCameraControls(0) );

		// Changing camera type:
		if (Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_ChangeCamera))
		{
			static S32 list[] =	{0, Cam2::E_FlyCamera, Cam2::E_Camera};

			m_CameraCycle++;
			if (m_CameraCycle >= sizeof(list) / sizeof(list[0]))
				m_CameraCycle = 0;

			S32 type = list[m_CameraCycle];
			
			if (type == 0)
				view.DebugFeed().ShowFeed(m_Camera);
			else
				view.DebugFeed().SetDebugType(type);
		}
	}

	void C_TestPhysics::ProcessController(Input::C_GameInput& input)
	{
		Cam2::C_Camera& cam = LiveCamera();

		Maths::HVec3 position(cam.Position());
		Maths::HVec3 dir(cam.Mat43().AZ());
		dir.Y(Maths::Constants<Maths::HF32>::Zero());
		dir.Normalise();

		Maths::HVec3 direction;

		Maths::HVec3 y(0.f, 1.f, 0.f);
		Maths::HVec3 x(y ^ dir);

		F32 thumbX = input.GetAxisSignedValue(Framework::E_Axis_FlyMoveX);
		F32 thumbY = input.GetAxisSignedValue(Framework::E_Axis_FlyMoveY);

		direction += dir * thumbY;
		direction += x * thumbX;

		const Maths::HF32 targetSpeed(5.f);
		m_Controller.SetTargetVelocity(direction * targetSpeed);

		if (input.GetButtonPressedDebounced(E_PhysicsButton_Jump))
			m_Controller.Jump(Maths::HF32(5.f));

		// Move camera:
		F32 lookX = input.GetAxisSignedValue(Framework::E_Axis_FlyRotateX);
		F32 lookY = input.GetAxisSignedValue(Framework::E_Axis_FlyRotateY);
		m_Camera->SetLookAround(lookX * 1.5f, lookY * 1.5f);
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
	
	void C_TestPhysics::ReleaseScheduler()
	{
		Scheduler::Shutdown();
	}

	// Shortcut to gameview:
	Framework::C_View& C_TestPhysics::View()
	{
		return Framework::C_ApplicationBase::GetInstance()->Get3DView(0);
	}
	
	// Shortcut to live camera:
	Cam2::C_Camera& C_TestPhysics::LiveCamera()
	{
		return View().DebugFeed().Live();
	}
}

void C_SectionPhysics::TestPhysicsEngine()
{
	static U32 demoMode = Samples::E_WallDemo;
	static bool initialised = false;
	if(!initialised)
	{
		initialised = true;
		m_TestPhysics.InitialiseTest(&m_TestPhysics.m_Filter, demoMode);
	}

	Input::C_GameInput* gameInput = Input::GetGameInput(0);

	if (gameInput->GetButtonPressedDebounced(E_PhysicsButton_ShootABox))
		m_TestPhysics.ShootABox(&m_TestPhysics.m_Filter);

	if (gameInput->GetButtonPressedDebounced(E_PhysicsButton_ShootARagdoll))
		m_TestPhysics.ShootARagdoll(&m_TestPhysics.m_Filter);

	static bool bCharacterController = true;

	if(bCharacterController && !m_TestPhysics.View().DebugFeed().UsesInput())
		m_TestPhysics.ProcessController(*gameInput);

	if (gameInput->GetButtonPressedDebounced(E_PhysicsButton_ModeSwitch))
	{
		demoMode = (demoMode + 1) % Samples::E_MaxDemos;

		m_TestPhysics.InitialiseTest(&m_TestPhysics.m_Filter, demoMode);
	}

	m_TestPhysics.DoRayTrace(Maths::HF32(100.f));
	m_TestPhysics.AnimateAndDraw(Maths::C_TimeS(1.f / 60.f));
	//if(demoMode == E_CharacterControllerDemo)
	if(bCharacterController)
	{
		Maths::HVec3 pos(m_TestPhysics.m_Controller.Position());
	}
	m_TestPhysics.GetRayTraceResults();
}
