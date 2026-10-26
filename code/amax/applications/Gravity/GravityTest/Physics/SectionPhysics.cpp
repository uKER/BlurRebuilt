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
	TEST_PHYSICS,
	GJK_AABBs	,
	GJK_OBBs		,
	GJK_CAPSULE_CAPSULE,
	GJK_AABB_CAPSULE,
	GJK_CYLINDER_CYLINDER,
	GJK_MARGIN_CYLINDER_TRIANGLE,
	GJK_MARGIN,
	OBBS_INTERSECT,
	TRIANGLE_AABB,
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
	SEGMENT_SEGMENT,
	CLOSESTPT_SEGMENT_SEGMENT,
	CAPSULE_SEGMENT,
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
	BUILDKDTREE,
	BOX_BOX_CONTACTS,
	SWEEP_AND_PRUNE,
	KDTREE_RAY,
	TEST_COMPOSITE_SHAPE,
	TRIANGLE_TRIANGLE,
	
	TEST_HEAPSORT,
	TEST_CYLINDER_TRIANGLE,
	TEST_CYLINDER_RAY,
	TEST_CONVEXHULL_RAY,
	SWEPT_RAY_TRIANGLE,
	SWEPT_SPHERE_SPHERE,
	SWEPT_SPHERE_OBB,
	SWEPT_SPHERE_CAPSULE,
	SWEPT_SPHERE_CYLINDER,
	XENO_BOX_BOX
};

DEBUG_ENUM(TestMode)
	DE_OPTION(TEST_PHYSICS, "Test Physics")
	DE_OPTION(GJK_AABBs,	"GJK_AABBs")
	DE_OPTION(GJK_OBBs,		"GJK_OBBs")
	DE_OPTION(GJK_CAPSULE_CAPSULE,	"GJK_CAPSULE_CAPSULE")
	DE_OPTION(GJK_AABB_CAPSULE, "GJK_AABB_CAPSULE")
	DE_OPTION(GJK_CYLINDER_CYLINDER, "GJK_CYLINDER_CYLINDER")
	DE_OPTION(GJK_MARGIN_CYLINDER_TRIANGLE, "GJK_MARGIN_CYLINDER_TRIANGLE")
	DE_OPTION(GJK_MARGIN, "GJK MARGIN")
	DE_OPTION(OBBS_INTERSECT, "OBBS_INTERSECT")
	DE_OPTION(TRIANGLE_AABB, "TRIANGLE_AABB")
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
	DE_OPTION(SEGMENT_SEGMENT, "SEGMENT_SEGMENT")
	DE_OPTION(CLOSESTPT_SEGMENT_SEGMENT, "PT_SEGMENT_SEGMENT")
	DE_OPTION(CAPSULE_SEGMENT, "CAPSULE_SEGMENT")
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
	DE_OPTION(BUILDKDTREE, "BUILD_KD_TREE")
	DE_OPTION(BOX_BOX_CONTACTS, "Collide_box_box")
	DE_OPTION(SWEEP_AND_PRUNE, "SWEEP_AND_PRUNE")
	DE_OPTION(KDTREE_RAY, "KDTREE_RAY")
	DE_OPTION(TEST_COMPOSITE_SHAPE, "COMPOSITE_SHAPE")
	DE_OPTION(TRIANGLE_TRIANGLE, "TRIANGLE_TRIANGLE")
	DE_OPTION(TEST_HEAPSORT, "TEST_HEAPSORT")
	DE_OPTION(TEST_CYLINDER_TRIANGLE, "TEST_CYLINDER_TRIANGLE")
	DE_OPTION(TEST_CYLINDER_RAY, "TEST_CYLINDER_RAY")
	DE_OPTION(TEST_CONVEXHULL_RAY, "TEST_CONVEXHULL_RAY")
	DE_OPTION(SWEPT_RAY_TRIANGLE,"SWEPT_RAY_TRIANGLE")
	DE_OPTION(SWEPT_SPHERE_SPHERE,"SWEPT_SPHERE_SPHERE")
	DE_OPTION(SWEPT_SPHERE_OBB,"SWEPT_SPHERE_OBB")
	DE_OPTION(SWEPT_SPHERE_CAPSULE,"SWEPT_SPHERE_CAPSULE")
	DE_OPTION(SWEPT_SPHERE_CYLINDER,"SWEPT_SPHERE_CYLINDER")
	DE_OPTION(XENO_BOX_BOX, "XENO_BOX_BOX")
END_DEBUG_ENUM

Debug::Enum<TestMode> g_dbgTestMode("Test.GjkTestMode", TEST_PHYSICS);

C_SectionPhysics::C_SectionPhysics(const Util::C_String& name) : Framework::C_TestSection(name)
{
	m_pSkyDome = new Render::Sky::C_SkyDome();

	// Map Physics Input buttons:
	Framework::C_ApplicationBase::GetInstance()->SetInputButtonCount(E_PhysicsButton_Count);
	Framework::C_ApplicationBase::GetInstance()->SetupUserControls();

	Input::C_Device* pPad = Framework::C_ApplicationBase::GetInstance()->GetPad(0);
	Input::C_GameInput* pGameInput = Input::GetGameInput(0);

#if defined(PLATFORM_PC) || defined(PLATFORM_XENON)
	pGameInput->SetButtonInput(E_PhysicsButton_ShootABox,     new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_A));
	pGameInput->SetButtonInput(E_PhysicsButton_ShootARagdoll, new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_Y));
	pGameInput->SetButtonInput(E_PhysicsButton_ModeSwitch,    new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_B));
	pGameInput->SetButtonInput(E_PhysicsButton_Jump,          new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_X));
#endif
}

C_SectionPhysics::~C_SectionPhysics()
{
	SAFE_DELETE_PTR(m_pSkyDome);
}

void C_SectionPhysics::OnUpdate()
{
	U64 iTime = Time::GetPerformanceCounter() - m_iTimer;

	Util::C_String str;
	str.Printf("time %d", iTime);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(HVec2(30.f,65.f), str);

	m_iTimer = Time::GetPerformanceCounter();

	switch(g_dbgTestMode.Value())
	{
	case(TEST_PHYSICS):
		TestPhysicsEngine();
		break;
	};
}

void C_SectionPhysics::OnRender3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_pSkyDome->Render();
	Render::DrawGrid();
}

void C_SectionPhysics::OnRender2DView()
{
	Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);

	Util::C_String string = Util::C_String("Cam: ") + view.DebugFeed().InfoString();
	Display::FontRender()->Draw(string.Chars(), 64.f, 704.f, 20.f, 24.f, Maths::RGBA(255, 255, 255));
}
