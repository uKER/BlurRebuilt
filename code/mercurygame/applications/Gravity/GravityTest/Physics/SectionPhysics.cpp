#include "stdafx.h"
#include "SectionPhysics.h"

#include <Render/Sky/SkyDome.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include <Render/Render/GridRender.h>
#include <Framework/Application/UserControls.h>
#include <Framework/Application/ApplicationBase.h>
#include <camera2.h>
#include <Input/GameInputMapping.h>
#if defined(PLATFORM_PC) || defined(PLATFORM_XENON)
#include <Input/Pad360.h>
#endif

using namespace Maths;

enum TestMode
{
	TRIANGLE_TRIANGLE,
	TEST_PHYSICS,
	TEST_LEVEL,
	TEST_CONVEXHULLGENERATION,
	TEST_CONE_TRIANGLE,
	TEST_CONE_SPHERE,
	TEST_CYLINDER_SPHERE,
	CONE_RAY,
	GJK_RAY_CAPSULE_TRIANGLE_CAST,
	GJK_RAY_OBB_TRIANGLE_CAST,
	GJK_RAY_CAPSULE_OBB_CAST,
	GJK_CYLINDER_CAPSULE,
	GJK_CONE_TRIANGLE,
	GJK_CONE_SPHERE,
	GJK_CONE_BOX,
	GJK_CONE_CYLINDER,
	GJK_CONE_CAPSULE,
	GJK_CONVEX_TRIANGLE,
	GJK_AABBs	,
	GJK_OBBs		,
	GJK_CAPSULE_CAPSULE,
	GJK_AABB_CAPSULE,
	GJK_CYLINDER_CYLINDER,
	GJK_MARGIN_CYLINDER_TRIANGLE,
	GJK_MARGIN,
	OBBS_INTERSECT,
	TRIANGLE_OBB,
	OBB_CAPSULE,
	OBB_SEGMENT,
	OBB_RAY,
	OBB_SPHERE,
	OBB_TRIANGLE_COLLIDE,
	SPHERE_TRIANGLE,
	SPHERE_RAY,
	TRIANGLE_SEGMENT,
	CLOSESTPT_TRIANGLE_SEGMENT,
	CAPSULE_RAY,
	CAPSULE_CAPSULE_COLLIDE,
	CAPSULE_TRINAGLE,
	CAPSULE_TRIANGLE_COLLIDE,
	CAPSULE_SPHERE,
	CAPSULE_SPHERE_COLLIDE,
	BUILDAABBTREE,
	AABBTREE_RAY,
	AABBTREE_SPHERE,
	AABBTREE_OBB,
	BOX_BOX_CONTACTS,
	SWEEP_AND_PRUNE,
	TEST_COMPOSITE_SHAPE,
	
	
	TEST_CYLINDER_TRIANGLE,
	TEST_CYLINDER_RAY,
	TEST_CONVEXHULL_RAY,
	SWEPT_RAY_TRIANGLE,
	SWEPT_SPHERE_SPHERE,
	SWEPT_SPHERE_OBB,
	SWEPT_SPHERE_CAPSULE,
	SWEPT_SPHERE_CYLINDER,
	
};

DEBUG_ENUM(TestMode)
	DE_OPTION(TRIANGLE_TRIANGLE, "TRIANGLE_TRIANGLE")
	DE_OPTION(TEST_PHYSICS, "Test Physics")
	DE_OPTION(TEST_LEVEL, "Test Level")
	DE_OPTION(TEST_CONVEXHULLGENERATION, "TEST_CONVEXHULLGENERATION")
	DE_OPTION(TEST_CONE_TRIANGLE, "TEST_CONE_TRIANGLE")
	DE_OPTION(TEST_CONE_SPHERE, "TEST_CONE_SPHERE")
	DE_OPTION(TEST_CYLINDER_SPHERE, "TEST_CYLINDER_SPHERE")
	DE_OPTION(GJK_RAY_CAPSULE_TRIANGLE_CAST, "GJK_RAY_CAPSULE_TRIANGLE_CAST")
	DE_OPTION(GJK_RAY_OBB_TRIANGLE_CAST, "GJK_RAY_OBB_TRIANGLE_CAST")
	DE_OPTION(GJK_RAY_CAPSULE_OBB_CAST, "GJK_RAY_CAPSULE_OBB_CAST")
	DE_OPTION(GJK_CYLINDER_CAPSULE, "GJK_CYLINDER_CAPSULE")
	DE_OPTION(CONE_RAY, "CONE_RAY")
	DE_OPTION(GJK_CONE_TRIANGLE, "GJK_CONE_TRIANGLE")
	DE_OPTION(GJK_CONE_SPHERE, "GJK_CONE_SPHERE")
	DE_OPTION(GJK_CONE_CAPSULE, "GJK_CONE_CAPSULE")
	DE_OPTION(GJK_CONE_CYLINDER, "GJK_CONE_CYLINDER")
	DE_OPTION(GJK_CONE_BOX, "GJK_CONE_BOX")
	DE_OPTION(GJK_CONVEX_TRIANGLE,"GJK_CONVEX_TRIANGLE")
	DE_OPTION(GJK_AABBs,	"GJK_AABBs")
	DE_OPTION(GJK_OBBs,		"GJK_OBBs")
	DE_OPTION(GJK_CAPSULE_CAPSULE,	"GJK_CAPSULE_CAPSULE")
	DE_OPTION(GJK_AABB_CAPSULE, "GJK_AABB_CAPSULE")
	DE_OPTION(GJK_CYLINDER_CYLINDER, "GJK_CYLINDER_CYLINDER")
	DE_OPTION(GJK_MARGIN_CYLINDER_TRIANGLE, "GJK_MARGIN_CYLINDER_TRIANGLE")
	DE_OPTION(GJK_MARGIN, "GJK MARGIN")
	DE_OPTION(OBBS_INTERSECT, "OBBS_INTERSECT")
	DE_OPTION(TRIANGLE_OBB, "TRIANGLE_OBB")
	DE_OPTION(OBB_CAPSULE, "OBB_CAPSULE")
	DE_OPTION(OBB_SEGMENT, "OBB_SEGMENT")
	DE_OPTION(OBB_RAY, "OBB_RAY")
	DE_OPTION(OBB_SPHERE, "OBB_SPHERE")
	DE_OPTION(OBB_TRIANGLE_COLLIDE, "OBB_Triangle_COLLIDE")
	DE_OPTION(SPHERE_TRIANGLE, "SPHERE_TRIANGLE")
	DE_OPTION(SPHERE_RAY, "SPHERE_RAY")
	DE_OPTION(TRIANGLE_SEGMENT, "IN_TRIANGLE_SEGMENT")
	DE_OPTION(CLOSESTPT_TRIANGLE_SEGMENT, "PT_TRIANGLE_SEGMENT")
	DE_OPTION(CAPSULE_RAY, "CAPSULE_RAY")
	DE_OPTION(CAPSULE_CAPSULE_COLLIDE, "CAPSULE_CAPSULE_COLLIDE")
	DE_OPTION(CAPSULE_TRINAGLE, "IN_CAPSULE_TRINAGLE")
	DE_OPTION(CAPSULE_TRIANGLE_COLLIDE, "PT_CAPSULE_TRIANGLE")
	DE_OPTION(CAPSULE_SPHERE, "CAPSULE_SPHERE")
	DE_OPTION(CAPSULE_SPHERE_COLLIDE, "CAPSULE_SPHERE_COLLIDE")
	DE_OPTION(BUILDAABBTREE, "BUILD_AABB_TREE")
	DE_OPTION(AABBTREE_RAY, "AABBTREE_RAY")
	DE_OPTION(AABBTREE_SPHERE, "AABBTREE_SPHERE")
	DE_OPTION(AABBTREE_OBB, "AABBTREE_OBB")
	DE_OPTION(BOX_BOX_CONTACTS, "Collide_box_box")
	DE_OPTION(SWEEP_AND_PRUNE, "SWEEP_AND_PRUNE")
	DE_OPTION(TEST_COMPOSITE_SHAPE, "COMPOSITE_SHAPE")
	DE_OPTION(TEST_CYLINDER_TRIANGLE, "TEST_CYLINDER_TRIANGLE")
	DE_OPTION(TEST_CYLINDER_RAY, "TEST_CYLINDER_RAY")
	DE_OPTION(TEST_CONVEXHULL_RAY, "TEST_CONVEXHULL_RAY")
	DE_OPTION(SWEPT_RAY_TRIANGLE,"SWEPT_RAY_TRIANGLE")
	DE_OPTION(SWEPT_SPHERE_SPHERE,"SWEPT_SPHERE_SPHERE")
	DE_OPTION(SWEPT_SPHERE_OBB,"SWEPT_SPHERE_OBB")
	DE_OPTION(SWEPT_SPHERE_CAPSULE,"SWEPT_SPHERE_CAPSULE")
	DE_OPTION(SWEPT_SPHERE_CYLINDER,"SWEPT_SPHERE_CYLINDER")
	
END_DEBUG_ENUM

Debug::Enum<TestMode> g_dbgTestMode("Test.GjkTestMode", TEST_PHYSICS);

C_SectionPhysics::C_SectionPhysics(const Util::C_String& name) : Framework::C_TestSection(name), pConvexShape(NULL)
{
	m_pSkyDome = new Render::Sky::C_SkyDome();

	// Map Physics Input buttons:
	//Framework::C_ApplicationBase::GetInstance()->SetInputButtonCount(E_PhysicsButton_Count);
	Framework::C_ApplicationBase::GetInstance()->SetupUserControls();

	Input::C_Device* pPad = Framework::C_ApplicationBase::GetInstance()->GetPad(0);
	Input::C_GameInput* pGameInput = Input::GetGameInput(0);
/* djh
#if defined(PLATFORM_PC) || defined(PLATFORM_XENON)
	pGameInput->SetButtonInput(E_PhysicsButton_A, new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_A));
	pGameInput->SetButtonInput(E_PhysicsButton_B, new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_B));
	pGameInput->SetButtonInput(E_PhysicsButton_X, new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_X));
	pGameInput->SetButtonInput(E_PhysicsButton_Y, new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_Y));
	pGameInput->SetButtonInput(E_PhysicsButton_Start, new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_Start));
#endif
#if defined(PLATFORM_PS3)
	pGameInput->SetButtonInput(E_PhysicsButton_A, new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_Cross));
	pGameInput->SetButtonInput(E_PhysicsButton_B, new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_Circle));
	pGameInput->SetButtonInput(E_PhysicsButton_X, new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_Square));
	pGameInput->SetButtonInput(E_PhysicsButton_Y, new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_Triangle));
	pGameInput->SetButtonInput(E_PhysicsButton_Start, new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_Start));
#endif
*/
	m_ElapsedTime = 0.f;
}

C_SectionPhysics::~C_SectionPhysics()
{
	SAFE_DELETE_PTR(m_pSkyDome);
	delete pConvexShape;
	pConvexShape = NULL;
}

void C_SectionPhysics::OnUpdate()
{
	Framework::C_TestSection::OnUpdate();

	switch(g_dbgTestMode.Value())
	{
	case(TEST_CONVEXHULLGENERATION):
		ConvexHullGenerationCodeTest();
		break;
	case(TEST_CONE_TRIANGLE):
		TestConeTriangle();
		break;
	case(TEST_CONE_SPHERE):
		TestConeSphere();
		break;
	case(TEST_CYLINDER_SPHERE):
		TestCylinderSphere();
		break;
	case(GJK_RAY_CAPSULE_TRIANGLE_CAST):
		GJKRayCastCapsuleTriangleTest();
		break;
	case(GJK_RAY_OBB_TRIANGLE_CAST):
		GJKRayCastOBBTriangleTest();
		break;
	case(GJK_RAY_CAPSULE_OBB_CAST):
		GJKRayCastCapsuleOBBTest();
		break;
	case(GJK_CYLINDER_CAPSULE):
		GJKMarginCylinderCapsule();
		break;
	case(CONE_RAY):
		TestConeRay();
		break;
	case(GJK_CONE_TRIANGLE):
		GJKConeTriangle();
		break;
	case (GJK_CONE_SPHERE):
		GJKConeSphere();
		break;
	case (GJK_CONE_CAPSULE):
		GJKConeCapsule();
		break;
	case (GJK_CONE_BOX):
		GJKConeBox();
		break;
	case (GJK_CONE_CYLINDER):
		GJKConeCylinder();
		break;
	case (GJK_CONVEX_TRIANGLE):
		GJKConvexTriangle();
		break;
	case (GJK_AABBs):
		GJKAABBsTest();
		break;
	case (GJK_OBBs):
		GJKOBBsTest();
		break;
	case (GJK_CAPSULE_CAPSULE):
		GJKCapsuleCapsule();
		break;
	case (GJK_AABB_CAPSULE):
		GJKAABBCapsule();
		break;
	case (GJK_CYLINDER_CYLINDER):
		GJKCylinderCyliner();
		break;
	case (GJK_MARGIN):
		GJKMarginOBB();
		break;
	case (OBBS_INTERSECT):
		OBBsIntersectionTest();
		break;
	case (TRIANGLE_OBB):
		TestTriangleOBB();
		break;
	case (OBB_CAPSULE):
		TestCollidedOBBCapsule();
		break;
	case (OBB_SEGMENT):
		TestOBBSegment();
		break;
	case (OBB_RAY):
		TestOBBRay();
		break;
	case (OBB_SPHERE):
		TestCollidedOBBSphere();
		break;
	case (OBB_TRIANGLE_COLLIDE):
		TestCollidedOBBTriangle();
		break;
	case (TRIANGLE_SEGMENT):
		TestIntersectSegmentTriangle();
		break;
	case (CLOSESTPT_TRIANGLE_SEGMENT):
		TestClosestPtSegmentTriangle();
		break;
	case (SPHERE_TRIANGLE):
		TestCollidedShpereTriangle();
		break;
	case (SPHERE_RAY):
		TestSphereRay();
		break;
	case(CAPSULE_RAY):
		TestCapsuleRay();
		break;
	case (CAPSULE_CAPSULE_COLLIDE):
		TestCollideCapsuleCapsule();
		break;
	case (CAPSULE_TRINAGLE):
		TestCapsuleTriangle();
		break;
	case (CAPSULE_SPHERE):
		TestCapsuleSphere();
		break;
	case (CAPSULE_SPHERE_COLLIDE):
		TestCollidedCapsuleSphere();
		break;
	case (CAPSULE_TRIANGLE_COLLIDE):
		TestCollidedCapsuleTriangle();
		break;
	case (BUILDAABBTREE):
		TestBuildAABBTree();
		break;
	case (AABBTREE_RAY):
		TestAABBTreeRay();
		break;
	case (AABBTREE_SPHERE):
		TestAABBTreeSphere();
		break;
	case (AABBTREE_OBB):
		TestAABBTreeOBB();
		break;
	case(BOX_BOX_CONTACTS):
		TestOBBOBB();
		break;
	case(SWEEP_AND_PRUNE):
		//SweepAndPruneTest();
		break;
	case(TEST_PHYSICS):
		TestPhysicsEngine(m_ElapsedTime);
		break;
	case(TEST_LEVEL):
		TestLevel();
		break;
	case(TRIANGLE_TRIANGLE):
		TestTriangleTriangle();
		break;
	case(GJK_MARGIN_CYLINDER_TRIANGLE):
		GJKMarginCylinderTriangle();
		break;
	case(TEST_CYLINDER_TRIANGLE):
		TestCylinderTriangle();
		break;
	case(TEST_CYLINDER_RAY):
		TestCylinderRay();
		break;
	case(TEST_CONVEXHULL_RAY):
		TestConvexHullRay();
		break;
	case(SWEPT_RAY_TRIANGLE):
		TestSweptRayTriangle();
		break;
	case (SWEPT_SPHERE_SPHERE):
		TestSweptSphereSphere();
		break;
	case (SWEPT_SPHERE_OBB):
		TestSweptSphereOBB();
		break;
	case (SWEPT_SPHERE_CAPSULE):
		TestSweptSphereCapsule();
		break;
	case (SWEPT_SPHERE_CYLINDER):
		TestSweptSphereCylinder();
		break;
	};
	
	m_ElapsedTime += 1.f / 60.f;
}

void C_SectionPhysics::OnRender3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_pSkyDome->Render();
	Render::DrawGrid();
	m_TestPhysics.OnRender();
}

void C_SectionPhysics::OnRender2DView()
{
	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);

	Util::C_String string = Util::C_String("Cam: ") + view.DebugFeed().InfoString();
	Display::FontRender()->Draw(string.Chars(), 64.f, 704.f, 20.f, 24.f, Maths::RGBA(255, 255, 255));
}
