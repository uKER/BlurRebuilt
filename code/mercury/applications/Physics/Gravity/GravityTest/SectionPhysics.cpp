#include "StdAfx.h"
#include "SectionPhysics.h"
//#include "GameFileSystem.h"
#include <Render/Render/GridRender.h>
#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
//#include <Render/Shaders/World/psWorld.h>
//#include <Render/Shaders/World/vsWorld.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include <System/Serialise/TextSerialise.h>

#include "gravity/sharedphysics/MemoryAllocator.inl"
//#include "./CollideModel.h"
#include "gravity/narrowphase/PrimitiveTest.h"
//#include "gravity/narrowphase/PrimitiveTest.inl"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/GJK.inl"
#include "gravity/sharedphysics/BinaryHeap.h"
#include "gravity/sharedphysics/BinaryHeap.inl"
//#include "gravity/narrowphase/ISA_GJK.inl"
#include "gravity/treebuilder/AABBTree.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/treebuilder/AABBTree.inl"
#include "gravity/narrowphase/Shape.h"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/sharedphysics/AllocationManager.inl"
#include "gravity/narrowphase/OBBOBB.h"
//#include "OBBSegment.inl"
#include "gravity/narrowphase/OBBOBB.inl"
#include "system/misc/random.h"
#include "render/font/font.h"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>

using namespace Collision;
using namespace Baking;
//------------------------------------ 

// -----------------------
// Colide Render Debug
// -----------------------

//Debug::Bool g_DrawTriangles("Test.DrawTriangles", true);

enum E_CollideRenderMode
{	
	E_CollideRenderMode_Off = 0,
	E_CollideRenderMode_Solid,
	E_CollideRenderMode_Wireframe,
	E_CollideRenderMode_Both,	
	E_CollideRenderMode_Count
};

DEBUG_ENUM(E_CollideRenderMode)
	DE_OPTION(E_CollideRenderMode_Off,			"Off")
	DE_OPTION(E_CollideRenderMode_Solid,		"Solid")
	DE_OPTION(E_CollideRenderMode_Wireframe,	"Wireframe")
	DE_OPTION(E_CollideRenderMode_Both,			"Both")
END_DEBUG_ENUM

Debug::Bool                      g_CollideRenderShowLevel         ( "Collide.ShowLevel",            true );

/*void SetupCollideRenderInfo(Physics::C_CollideRenderInfo& collideRenderInfo)
{

	collideRenderInfo.m_MaterialFlagsSolid = -1;
	collideRenderInfo.m_MaterialFlagsWireframe = 0;

	if(g_CollideRenderShowLevel)
	{
		collideRenderInfo.m_RenderLevel = true;
	}
	else
	{
		collideRenderInfo.m_RenderLevel = false;
	}
}*/

//-------------------------

using namespace Maths;

enum TestMode
{
	TEST_PHYSICS,
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
	XENO_BOX_BOX,
};

DEBUG_ENUM(TestMode)
	DE_OPTION(TEST_PHYSICS, "Test Physics")
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


namespace Samples
{

Debug::Float g_RaytraceCapsuleRadius("Test.Raytrace.CapsuleRadius", 0.0f);
Debug::Float g_RaytraceRayLength("Test.Raytrace.RayLength", 100.0f);
Debug::Bool g_RaytraceTraceStatic("Test.Raytrace.TraceStatic", true);
Debug::Bool g_RaytraceTraceDynamic("Test.Raytrace.TraceDynamic", false);
Debug::Bool g_RaytraceShowLines("Test.Raytrace.ShowLines", true);
Debug::Logger &g_MichelleLog = Debug::GetLogger("michelle");
//Debug::Bool g_TestBoxBox("Test.BoxBox", true);

static bool TestPhysics = false;
//Physics::C_CollideModel g_CollideModel;

C_SectionPhysics::C_SectionPhysics(const Util::C_String& name) : C_Section(name)
{
	//C_GameFileSystem::Instance().Initialise(Sys::C_FileSystem::Instance());
	//C_GameFileSystem::Instance().InitialisePAKs();

	// create debug renderer
	m_pFontSystem = new Render::Font::C_FontSystem;
	Render::Font::FontSystem(m_pFontSystem);
	m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;

	DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);

	m_bDebounceSpace = true;
	m_bDoMovement = true;
}

C_SectionPhysics::~C_SectionPhysics()
{
	delete m_pDebugRenderSys;
	delete m_pFontSystem;
	m_pDebugRenderSys = NULL;
}

void C_SectionPhysics::CollideDebugRender()
{
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	//m_Level.m_pPhysicsWorld->DbgRender( DbUtil::DefaultDebugRenderSys()->Immediate(), viewSettings3D.m_WorldToCamera );

	//Physics::C_CollideRenderInfo collideRenderInfo;
	//SetupCollideRenderInfo(collideRenderInfo);

	//m_Level.m_pPhysicsWorld->CollideRender( DbUtil::DefaultDebugRenderSys()->Immediate(), viewSettings3D.m_WorldToCamera, collideRenderInfo );
}


bool C_SectionPhysics::OnCreate()
{
#ifdef PLATFORM_XENON
	// get main thread real handle and store in MT::Thread TLS
	//HANDLE h = GetCurrentThread();
	//DuplicateHandle(h,h,h,&m_hMainThread,0,FALSE,0);
	//MT::Thread::SetCurrentThreadHandle( m_hMainThread );
#endif

	//if (!m_Level.CreateLevel())
	//	return false;
	
	//if (!m_Level.LoadLevel("Levels\\", "Surfaces"))
//	if (!m_Level.LoadLevel("Levels\\", "Physics"))
//	{
//		LOGERROR(g_Log, "Failed to load the level\n");
///		return false;
//	}

//	g_CollideModel.LoadFromFile("Objects\\GE_LT_Computer_4.xep");

	return true;
}

bool C_SectionPhysics::OnCreateVolatile()
{
	return true;
}

void C_SectionPhysics::OnDestroyVolatile()
{
}

void C_SectionPhysics::OnDestroy()
{
#ifdef PLATFORM_XENON
	CloseHandle(m_hMainThread);
#endif
}

void C_SectionPhysics::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	PhysicsTestUpdate(input,timer);
}

void C_SectionPhysics::OnPreRender(const C_Input& input, const C_Timer& timer)
{
}

void C_SectionPhysics::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionLevel");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 160, 0, 255));

	m_SkyDome.Render();

//	m_Level.RenderLevel();

	Util::C_String str;
	str.Printf("Physics : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
	if(TestPhysics)
		m_TestPhysics.OnRender();

	m_pDebugRenderSys->RenderDeferred();
	m_pDebugRenderSys->RenderPrimitive();
	DbUtil::DefaultDebugRenderSys()->RenderPrimitive();

	CollideDebugRender();
	DbUtil::DefaultDebugRenderSys()->PrimitiveEndFrame();
}


void TriangleAABBTest()
{
	//AABB box(8);
	/*AABB box;
	box.c=HVec3(3.f, 5.f, -2.f);
	box.radius=HVec3(2.f, 1.f, 4.f);

	const HVec3 c(box.c);
	const HVec3 r(box.radius);

	const HVec3 a(4.f,6.f, -7.f);
	const HVec3 b(8.f, 8.f, -4.f);
	const HVec3 d(-3.f, 4.f, -8.f );


	HF32 s1;
	HF32 s2;
	//HF32 s=IntersectAABBTriangle(box, a, d, b);
	//s=IntersectAABBTriangle2(box, a, d, b);
	for(int i=0; i<1000; ++i)
	{
		s1=IntersectAABBTriangle(box, a, d, b);
	}
	
	
	
	C_AxisBox boxb(box.c - box.radius, box.c + box.radius);
	for(int i=0; i<1000; ++i)
	{
		s2 = Physics::IntersectBoxTriangle(boxb, a, d, b);
	}
	
	
	
	
	//s1=IntersectAABBTriangle(box, a, d, b);
	//s2 = IntersectBoxTriangle(boxb, a, d, b);
	//assert(s1.EQ(s2));

	
	HMat33 matA(Maths::UNDEFINED);
	matA.SetIdentity();

	HMat43 trans(matA, box.c);

	HVec3 minA(-box.radius);
	HVec3 maxA(box.radius);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, trans, RGBA(0, 0, 255));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, d,  Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, d, b,  Maths::RGBA(255, 0, 0));
	
	LOGINFO(g_MichelleLog, "%f", (F32)s1);*/
}

void C_SectionPhysics::GJKMargin()
{
	static bool bFirstFrame = true;

	static C_OBB boxA;
	static C_OBB boxB;
	if(bFirstFrame)
	{
		boxA.c=HVec3(-1.f, 5.f, -2.f);
		//boxA.c=HVec3(0.f, 0.f, 0.f);
		boxA.u.SetIdentity();
		boxA.e=HVec3(0.1f,0.2f, 0.1f);
		//boxA.e=HVec3(1.f, 1.f, 1.f);
		boxA.Margin(0.0f);
		boxA.Epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		//
		//if(!bFirstFrame)
		{
			boxA.WarmStart(boxA.c);
		}

		
		boxB.c=HVec3(-1.05f, 5.f, -2.f);
		//boxB.c=HVec3(1.f, 0.f, 0.f);
		boxB.u.SetIdentity();
		boxB.e=HVec3(0.1f, 0.2f, 0.1f);
		boxB.Epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		//boxB.e=HVec3(1.f, 1.f, 1.f);
		//boxB[0]=boxB.c;
		boxB.Margin(0.0f);
		//if(!bFirstFrame)
		{
			boxB.WarmStart(boxB.c);
			//bFirstFrame = true;
		}
		bFirstFrame = false;
	}

	boxA.WarmStart(boxA.c);
	boxB.WarmStart(boxB.c);


	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}
	
	const Maths::RotZ rotya(rot);
	HMat33 matra(rotya);
	const Maths::RotX rotxa(rot);
	HMat33 m(rotxa);
	boxA.u=matra*m;

	/*const Maths::RotY rotyb(rot);
	HMat33 matrb(rotyb);
	boxB.u=matrb;
	*/

	const HVec3 zero(Maths::Constants<HVec3>::Zero());
	HF32 penetration;
	//HVec3 closestA(Maths::UNDEFINED);
	//HVec3 closestB(Maths::UNDEFINED);
	/*HVec3 contact(Maths::UNDEFINED);
	HVec3 normal(Maths::UNDEFINED);*/
	C_CollisionResult<1, 32> result;
	

	bool s;
	EPA epa;
	bool fail=false; //GJK or EPA doesn't fail
	for(int i=0; i< 1; ++i)
	{
		C_ContactPatch<32>& patch(result.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		Maths::HF32 penetration;
		s=GJKCPairMargin<C_OBB, C_OBB>(boxA, boxB, point.m_ContactPoint, patch.m_Normal.m_V, penetration, epa);
		point.m_PenetrationDepth = penetration;
		if(s)
		{
			//s = Collision::CollideBoxBox(boxA, boxB, result, 0);

			const Maths::HVec3 q0(epa.aBuf[0]-epa.bBuf[0]);
			const Maths::HVec3 q1(epa.aBuf[1]-epa.bBuf[1]);
			const Maths::HVec3 q2(epa.aBuf[2]-epa.bBuf[2]);
			const Maths::HVec3 q3(epa.aBuf[3]-epa.bBuf[3]);
			const Maths::HVec3 q4(epa.aBuf[4]-epa.bBuf[4]);
			const Maths::HVec3 q5(epa.aBuf[5]-epa.bBuf[5]);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q0, RGBA(255, 0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q1, RGBA(255, 0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q2, RGBA(255, 0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q5, RGBA(255, 0, 0, 255));


			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q3, q0, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q3, q1, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q3, q2, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q3, q5, RGBA(255, 255, 0, 255));

			/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q0, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q1, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q2, RGBA(0, 255, 0, 255));
			*/

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(q0, q1, q2, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(q2, q1, q0, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q0, Util::C_String("0"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q1, Util::C_String("1"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q2, Util::C_String("2"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q3, Util::C_String("3"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q4, Util::C_String("4"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q5, Util::C_String("5"));


			const Maths::HVec3 c023((q0+q2+q3)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t0(q2-q3);
			const Maths::HVec3 t1(q0-q3);

			Maths::C_Normal n0(t0^t1);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c023, n0, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c213((q1+q3+q2)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t2(q1-q3);
			const Maths::HVec3 t3(q2-q3);

			Maths::C_Normal n2(t2^t3);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c213, n2, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c042((q0+q2+q4)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t4(q4-q2);
			const Maths::HVec3 t5(q0-q2);

			Maths::C_Normal n3(t4^t5);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c042, n3, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c241((q1+q4+q2)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t6(q4-q1);
			const Maths::HVec3 t7(q2-q1);

			Maths::C_Normal n4(t6^t7);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c241, n4, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c503((q5+q0+q3)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t8(q0-q3);
			const Maths::HVec3 t9(q5-q3);

			Maths::C_Normal n5(t8^t9);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c503, n5, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c153((q5+q3+q1)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t10(q5-q3);
			const Maths::HVec3 t11(q1-q3);

			Maths::C_Normal n6(t10^t11);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c153, n6, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c540((q5+q0+q4)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t12(q4-q0);
			const Maths::HVec3 t13(q5-q0);

			Maths::C_Normal n7(t12^t13);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c540, n7, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c145((q5+q1+q4)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t14(q4-q5);
			const Maths::HVec3 t15(q1-q5);

			Maths::C_Normal n8(t14^t15);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c145, n8, 0.05f, 0.02f, RGBA(0, 255, 0, 255));


			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(q0, q1, q5, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(q5, q1, q0, RGBA(0, 255, 0, 255));
			
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(q3, 0.05f, RGBA(0, 0, 255, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(q4, 0.05f, RGBA(0, 255, 255, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(q5, 0.05f, RGBA(255, 0, 255, 255));


			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(HVec3(0.f, 0.f, 0.f), 0.05f, RGBA(0, 0, 0, 255));

		}
	}

	//HF32 s=GJKCPair(boxA, boxB, dis, closestA, closestB);
	HMat43 matA(boxA.u, boxA.c);
	HVec3 minA(-boxA.e);
	HVec3 maxA(boxA.e);

	HMat43 matB(boxB.u, boxB.c);
	HVec3 minB(-boxB.e);
	HVec3 maxB(boxB.e);


	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(0, 255, 0));
	

	if(s)
	{
		C_ContactPatch<32>& patch(result.m_ContactPatches[0]);
		for(U32 a = 0; a < patch.m_NumContactPoints; ++a)
		{
			C_ContactPoint& point(patch.m_ContactPoints[a]);
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minBm, maxBm, matB, Maths::RGBA(0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point.m_ContactPoint, point.m_ContactPoint + patch.m_Normal * point.m_PenetrationDepth, Maths::RGBA(255, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.05f, Maths::RGBA(64, 0, 255));
		}
	}
	/*else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(0, 0, 255));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minBm, maxBm, matB, Maths::RGBA(0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestA + normal * dis, Maths::RGBA(255, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.05f, Maths::RGBA(64, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.05f, Maths::RGBA(64, 0, 255));
	}*/

	

}


/*void C_SectionPhysics::GJKMargin()
{
	OBB boxA;
	boxA.c=HVec3(-1.f, 5.f, -2.f);
	boxA.u=HMat33(UNDEFINED);
	boxA.u.SetIdentity();
	boxA.e=HVec3(1.f, 2.f, 3.f);
	boxA.Margin(0.3f);
	boxA.WarmStart(boxA.c);

	OBB boxB;
	boxB.c=HVec3(-2.f, 4.f, 3.f);
	boxB.u=HMat33(UNDEFINED);
	boxB.u.SetIdentity();
	boxB.e=HVec3(2.f, 1.f, 2.f);
	//boxB[0]=boxB.c;
	boxB.Margin(0.3f);
	boxB.WarmStart(boxB.c);

	OBB boxAm;
	boxAm.c=boxA.c;
	boxAm.u=boxA.u;
	const HVec3 marginA(boxA.Margin());
	//boxAm.e=HVec3(boxA.e.X()+marginA, boxA.e.Y()+marginA, boxA.e.Z()+marginA);
	boxAm.e=boxA.e+marginA;

	OBB boxBm;
	boxBm.c=boxB.c;
	boxBm.u=boxB.u;
	const HVec3 marginB(boxB.Margin());
	boxBm.e=boxB.e + marginB;


	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}
	
	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	boxA.u=matra;

	boxAm.u=boxA.u;

	const HVec3 zero(Maths::Constants<HVec3>::Zero());
	HF32 dis;
	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	HVec3 closest(Maths::UNDEFINED);
	

	int s;
	EPA epa;
	//for(int i=0; i< 1000; ++i)
	{
		s=GJKCPairMargin<OBB, OBB>(boxA, boxB, dis, closest, closestA, closestB, epa);
	}

	//HF32 s=GJKCPair(boxA, boxB, dis, closestA, closestB);
	HMat43 matA(boxA.u, boxA.c);
	HVec3 minA(-boxA.e);
	HVec3 maxA(boxA.e);

	HMat43 matB(boxB.u, boxB.c);
	HVec3 minB(-boxB.e);
	HVec3 maxB(boxB.e);


	HVec3 minAm(-boxAm.e);
	HVec3 maxAm(boxAm.e);

	HVec3 minBm(-boxBm.e);
	HVec3 maxBm(boxBm.e);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(255, 0, 0));
	

	if(s== E_DEEP_INTERSECTION)
	{
		
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(0, 255, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minBm, maxBm, matB, Maths::RGBA(0, 255, 0));

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, Maths::RGBA(255, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.05f, Maths::RGBA(64, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.05f, Maths::RGBA(64, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closest, 0.5f, Maths::RGBA(64, 0, 255));
	}
	else if(s==E_MARGIN_INTERSECTION)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minAm, maxAm, matA, Maths::RGBA(255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minBm, maxBm, matB, Maths::RGBA(0, 255, 0));
		C_Normal dir(closestA-closestB);
		HVec3 closest(closestB+dir* boxB.Margin());
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, Maths::RGBA(255, 255, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.05f, Maths::RGBA(64, 0, 255));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.05f, Maths::RGBA(64, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closest, 0.05f, Maths::RGBA(64, 0, 255));
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(0, 0, 255));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minBm, maxBm, matB, Maths::RGBA(0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, Maths::RGBA(255, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.05f, Maths::RGBA(64, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.05f, Maths::RGBA(64, 0, 255));
	}

	

}*/

void C_SectionPhysics::TestHeapsort()
{
	SharedPhysics::BinaryHeapStorage<float> heap;
	Util::C_Random rand;
	Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "testheap.txt", Sys::E_FileAccess_Write );
	Sys::TextSerialiser serial(*filePtr);

	rand.Seed(32);

	float temp[10];

	for(U32 i=0; i<10; i++)
	{
		temp[i]=rand.FRand();
		heap.insert(&temp[i]);
		serial << temp[i] <<"\n";
	}

	delete filePtr;

	Sys::I_BinaryStream *filePtr2 = Sys::C_FileSystem::Instance().Open( "testheap1.txt", Sys::E_FileAccess_Write );
	Sys::TextSerialiser serial2(*filePtr);

	for(U32 i=0; i<heap.size(); ++i)
	{
		float temp=*(heap.value(i));
		serial2 << temp<<"\n";
	}

	delete filePtr2;




	float temp2[10];
	U32 count=0;
	while(!heap.isEmpty())
	{
		float* temp;
		heap.deleteTop(&temp);
		if(count < 10)
		{
			temp2[count]=rand.FRand();
			heap.insert(&temp2[count++]);
		}
	}

	


}




void TestAABBSegment()
{
	C_AABB box;
	box.c=HVec3(3.f, 5.f, -2.f);
	box.radius=HVec3(2.f, 1.f, 4.f);

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;
	rot+=0.02f;

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	box.c=box.c*matra;
	
	

	const HVec3 a(1.f, 5.f, -7.f);
	const HVec3 b(8.f, 6.f, -2.f);
	const HVec3 half(Maths::Constants<HVec3>::Half());
	HVec3 mid((a+b)*half);
	
	HF32 s1;
	HF32 s2;
	//HF32 s=IntersectAABBTriangle(box, a, d, b);
	//s=IntersectAABBTriangle2(box, a, d, b);
	for(int i=0; i<1000; ++i)
	{
		s1=IntersectAABBSegment(box, a, b);
	}
	
	HMat33 matA(Maths::UNDEFINED);
	matA.SetIdentity();

	HMat43 trans(matA, box.c);

	HVec3 minA(-box.radius);
	HVec3 maxA(box.radius);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, trans, RGBA(0, 0, 255));

	if(s1)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, Maths::RGBA(255, 0, 0));
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, Maths::RGBA(0, 255, 0));
	}
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(b, 0.05f, Maths::RGBA(0, 255, 0));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mid, 0.05f, Maths::RGBA(0, 0, 255));

	
	LOGINFO(g_MichelleLog, "%f", (F32)s1);

}


void TestCapsuleSegment()
{
	Collision::C_Capsule cap;
	cap.m_Start=HVec3(2.f, 3.f,-4.f);
	cap.m_End=HVec3(-2.f, 5.f, 1.f);
	cap.m_Radius=2.f;

	const HVec3 mid((cap.m_Start+cap.m_End)*0.5f);

	HVec3 a(4.f, 6.f, -5.5f);
	HVec3 b(-3.f, -5.f, 1.f);

	F32 s=IntersectCapsuleSegment(cap, a, b);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(cap.m_Start, cap.m_End, RGBA(0, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(cap.m_Start, 0.5f, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(cap.m_End, 0.5f, RGBA(0, 64, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(cap.m_Start, cap.m_Radius, RGBA(0,255,0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(cap.m_End, cap.m_Radius, RGBA(0,255,0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(mid, cap.m_Radius, RGBA(0,255,0));

	LOGINFO(g_MichelleLog, "%f", s);

}

void TestSegmentSegment()
{

	/*HVec3 p(3.f, 4.f, -2.f);
	HVec3 q(-6.f, 3.f, 2.f);
	HVec3 a(4.f, 6.f, -5.5f);
	HVec3 b(-3.f, -5.f, 1.f);

	F32 s=IntersectSegmentSegment(p,q, a, b);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(p, q, RGBA(0, 0, 255));
	
	LOGINFO(g_MichelleLog, "%f", s);*/

}

void TestClosestPtSegmentSegment()
{

	HVec3 p(3.f, 4.f, -2.f);
	HVec3 q(-6.f, 3.f, 2.f);

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;
	rot+=0.02f;

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	p=p*matra;
	q=q*matra;

	
	const HVec3 a(4.f, 6.f, -5.5f);
	const HVec3 b(-3.f, -5.f, 1.f);

	HVec3 closestA(UNDEFINED);
	HVec3 closestB(UNDEFINED);
	HVec3 closestC(UNDEFINED);
	HVec3 closestD(UNDEFINED);

	for(int i=0; i<1000; ++i)
	{
		ClosestPtSegmentSegment(p,q, a, b, closestA, closestB);
	}

	/*for(int i=0; i<1000; ++i)
	{
		CEClosestPtSegmentSegment(p,q, a, b, closestC, closestD);
	}
	*/

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(p, q, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, RGBA(0, 255, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestC, closestD, RGBA(255, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.5f, RGBA(0, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.5f, RGBA(255, 0, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestC, 0.5f, RGBA(0, 255, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestD, 0.5f, RGBA(0, 255, 0, 255));
	
	//LOGINFO(g_MichelleLog, "%f", s);

}



void C_SectionPhysics::TestCapsuleTriangle()
{
	Collision::C_Capsule capsule;
	capsule.m_Start=HVec3(3.f, 4.f, -2.f);
	capsule.m_End=HVec3(-6.f, 3.f, 2.f);
	capsule.m_Radius=2.f;

	HVec3 na=capsule.m_End-capsule.m_Start;

	HVec3 a(4.f,6.f, -7.f);
	HVec3 b(8.f, 2.f, 4.f);
	HVec3 c(-3.f, 4.f, -8.f );

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	a=a*matra;
	b=b*matra;
	c=c*matra;
	

	F32 s;
	for(int i=0; i<1000; ++i)
	{
		s=IntersectCapsuleTriangle(capsule, a, b, c);
	}

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 0));
	}
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(255, 0, 0));
	

	if(s)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, c, RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, c, b, RGBA(0, 255, 0));
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, c, RGBA(255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, c, b, RGBA(255, 0, 0));

	}


}


void C_SectionPhysics::TestCapsuleSphere()
{
	Collision::C_Capsule capsule;
	capsule.m_Start=HVec3(3.f, 4.f, -2.f);
	capsule.m_End=HVec3(-6.f, 3.f, 2.f);
	capsule.m_Radius=2.f;

	HVec3 na=capsule.m_End-capsule.m_Start;

	Collision::C_Sphere sphere;
	sphere.Centre(HVec3(2.f, 3.f, -7.f));
	sphere.Radius(Maths::HF32(3.f));
	

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	sphere.Centre(sphere.Centre()*matra);
	

	F32 s;
	for(int i=0; i<1000; ++i)
	{
		s=IntersectCapsuleSphere(capsule, sphere);
	}

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 0));
	}
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(255, 0, 0));
	

	if(s)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(255, 0, 0));
	} 
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(0, 255, 0));
	}


}


void C_SectionPhysics::OBBOBBCollisionTest()
{
	C_CollisionResult<4, 32> result;
	C_OBB obbA, obbB;
	//CollideBoxBox(obbA, obbB, result);
}


int compare(int& a, int& b)
{
	//return (a>b)-(a<b);
	return (a<b)-(a>b);
	
}

/// some code that allows us to shoot physics objects, rays etc. out of the camera
void C_SectionPhysics::PhysicsTestUpdate(const C_Input& input, const C_Timer& timer)
{
	m_input=input;
	U64 iTime = Time::GetPerformanceCounter() - m_iTimer;

	Util::C_String str;
	str.Printf("time %d", iTime);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(HVec2(30.f,65.f), str);

	// interpret some buttons that allow us to fire more rays
#ifdef PLATFORM_PC
	if (input.IsKeyDn(' '))
#else
	if (input.IsButtonDn(E_Button_B))
#endif
	{
		if (!m_bDebounceSpace)
		{
/*
			// shoot an object from the camera
			Maths::HVec3 vAt(
				Display::Device()->GetView()->GetViewSettings3D().m_ViewPosition.x,
				Display::Device()->GetView()->GetViewSettings3D().m_ViewPosition.y,
				Display::Device()->GetView()->GetViewSettings3D().m_ViewPosition.z);

			// position = vAt
			Display::C_Matrix mtx = Display::Device()->GetView()->GetViewSettings3D().m_ViewMatrix;
			Maths::HMat33 rm;
			rm.AX(HVec3(mtx._11, mtx._12, mtx._13));
			rm.AY(HVec3(mtx._21, mtx._22, mtx._23));
			rm.AZ(HVec3(mtx._31, mtx._32, mtx._33));
			rm.Transpose();
			// rotation = (C_Quaternion)rm;
*/
			//m_bDoMovement = !m_bDoMovement;
			m_bDebounceSpace = true;
		}
	}
	else
		m_bDebounceSpace = false;

	m_iTimer = Time::GetPerformanceCounter();

	TestPhysics = false;
	switch(g_dbgTestMode.Value())
	{
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
		GJKMargin();
		break;
	case (OBBS_INTERSECT):
		OBBsIntersectionTest();
		break;
	case (TRIANGLE_AABB):
		TriangleAABBTest();
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
		C_SectionPhysics::TestIntersectSegmentTriangle();
		break;
	case (CLOSESTPT_TRIANGLE_SEGMENT):
		C_SectionPhysics::TestClosestPtSegmentTriangle();
		break;
	case (SPHERE_TRIANGLE):
		TestCollidedShpereTriangle();
		break;
	case (SPHERE_RAY):
		TestSphereRay();
		break;
	case (SEGMENT_SEGMENT):
		TestSegmentSegment();
		break;
	case (CLOSESTPT_SEGMENT_SEGMENT):
		TestClosestPtSegmentSegment();
		break;
	case (CAPSULE_SEGMENT):
		TestCapsuleSegment();
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
	case(BUILDKDTREE):
		TestBuildKDTree();
		break;
	case(BOX_BOX_CONTACTS):
		TestOBBOBB();
		break;
	case(SWEEP_AND_PRUNE):
		//SweepAndPruneTest();
		break;
	case(KDTREE_RAY):
		TestKDTreeRay();
		break;
	case(TEST_PHYSICS):
		{	TestPhysics = true;
			TestPhysicsEngine(timer.GetElapsedTime());
		}
		break;
	case(TRIANGLE_TRIANGLE):
		TestTriangleTriangle();
		break;
	case(GJK_MARGIN_CYLINDER_TRIANGLE):
		GJKMarginCylinderTriangle();
		break;
	case(TEST_HEAPSORT):
		this->TestHeapsort();
		break;
	case(TEST_CYLINDER_TRIANGLE):
		this->TestCylinderTriangle();
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
	case (XENO_BOX_BOX):
		TestXenoCollideBoxBox();
		break;
	};
	
	
}

}; // namespace Samples

