#ifndef TEST_SECTION_PHYSICS_H
#define TEST_SECTION_PHYSICS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include <Framework/Application/UserControls.h>
#include "TestPhysics.h"
#include "gravity/treebuilder/AABBTree.h"
#include "gravity/exporter/convexhullgeneration.h"
#include "gravity/sdk/ShapeDescriptors.h"
#include "gravity/exporter/GravityExporter.h"
namespace Render { namespace Sky { class C_SkyDome; } }

namespace Render
{
namespace Debug
{
class C_DebugRenderSys;
}
};

class C_SectionPhysics : public Framework::C_TestSection
{
public:

	C_SectionPhysics(const Util::C_String& name);
	~C_SectionPhysics();

	virtual void OnUpdate();
	virtual void OnRender3DViewWithAA(const S32 viewIndex);
	virtual void OnRender2DView();


	void TestPhysicsEngine(F32 time);
	void TestCollideCapsuleCapsule();
	void TestCollidedShpereTriangle();
	void TestSphereRay();
	void TestBuildAABBTree();
	void TestAABBTreeRay();
	void BuildAABBTree();

	void TestAABBTreeSphere();
	void TestAABBTreeOBB();
	void TestCollidedCapsuleSphere();
	void TestCapsuleRay();
	void TestCapsuleSphere();
	void TestCapsuleTriangle();
	void TestTriangleOBB();
	void TestOBBSegment();
	void TestOBBRay();
	void OBBsIntersectionTest();
	void TestOBBOBB();
	void TestCollidedOBBTriangle();
	void TestCollidedOBBCapsule();
	void TestCollidedOBBSphere();
	void TestConvexHull();
	void TestIntersectSegmentTriangle();
	void TestClosestPtSegmentTriangle();
	void TestTriangleTriangle();
	void TestCollidedCapsuleTriangle();
	void TestSweptRayTriangle();
	void TestSweptSphereSphere();
	void TestSweptSphereOBB();
	void TestSweptSphereCapsule();
	void TestSweptSphereCylinder();
	void TestConvexHullRay();
	void GJKOBBsTest();
	void GJKCapsuleCapsule();
	void GJKAABBCapsule();
	void GJKConvexTriangle();
	void GJKCylinderCyliner();
	void GJKAABBsTest();
	void GJKConeSphere();
	void GJKConeCylinder();
	void GJKConeCapsule();
	void GJKConeTriangle();
	void GJKConeBox();

	void GJKMarginOBB();
	void GJKMarginCylinderCapsule();
	void GJKMarginCylinderTriangle();
	
	void GJKRayCastCapsuleTriangleTest();
	void GJKRayCastCapsuleOBBTest();
	void GJKRayCastOBBTriangleTest();

	void TestLevel();
	void TestCylinderTriangle();
	void TestCylinderRay();
	void TestCylinderSphere();
	void TestConeSphere();
	void TestConeRay();
	void TestConeTriangle();

	void ConvexHullGenerationCodeTest();

	void DrawAABBTreeNode(Baking::AABBTreeNode* node, Baking::AABBTree& tree, U32 depth, U32 maxDepth,
										const Collision::C_CRAxisBox& box);
	

private:
	Render::Sky::C_SkyDome*			m_pSkyDome;
	Render::Debug::C_DebugRenderSys* m_pDebugRenderSys;
	U64								m_iTimer;
	Samples::C_TestPhysics			m_TestPhysics;
	Collision::OptimisedAABBTree	m_optTree;//optimised tree
	F32 m_ElapsedTime;
	Gravity::C_ConvexHullMesh mesh; 
	Gravity::C_ConvexHullDesc pConvexHullDesc; 
	SharedPhysics::SortedArray<Gravity::C_ConverterTriangle > tris;
	Collision::C_Shape* pConvexShape;
};

//enum E_PhysicsButton
//{
//	E_PhysicsButton_A = Framework::E_Button_Count,
//	E_PhysicsButton_B,
//	E_PhysicsButton_X,
//	E_PhysicsButton_Y,
//	E_PhysicsButton_Start,
//	E_PhysicsButton_Crouch,
//
//	E_PhysicsButton_Count
//};

#endif // TEST_SECTION_PHYSICS_H
