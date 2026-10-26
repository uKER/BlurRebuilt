#include "StdAfx.h"
#include "SectionSkyDome.h"
#include "Render/Render/GridRender.h"
#include <System\Debug\I_DebugRender.h>
#ifdef PLATFORM_PS3
#include "Render/Debug/ImmediateDebugRenderPS3.h"
#endif
 
#include "Render/Debug/ImmediateDebugRender.h"
#include "Render/Debug/DebugRender.h"
#include "Gravity/NarrowPhase/GJK.h"
#include "Gravity/NarrowPhase/PrimitiveTest.h"

using namespace Maths;

namespace Samples
{

Debug::LogManager* logManager;

Render::Debug::C_DebugRenderSys *g_pDebugRenderSys;

//Debug::Logger &g_MichelleLog = Debug::GetLogger("michelle");

C_SectionSkyDome::C_SectionSkyDome(const Util::C_String& name) : C_Section(name)
{
	g_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;

	DbUtil::I_DebugRenderSysPtr debugRenderSysPtr = Render::Debug::DebugRenderSys();
 
    DbUtil::DefaultDebugRenderSys( debugRenderSysPtr );
}

C_SectionSkyDome::~C_SectionSkyDome()
{
	delete g_pDebugRenderSys;
	delete logManager;
}

void C_SectionSkyDome::OnUpdate(const Samples::C_Input &input, const Samples::C_Timer &timer)
{
	
	/*HVec3 p(1.f, -3.f, 2.5f);
	HVec3 q(1.f, -3.f, -2.f);
	HVec3 w(6.f, 2.f, 4.f);
	HVec3 a(2.f,3.f, -7.f);
	HVec3 b(8.f, 5.f, 4.f);
	HVec3 c( 2.f, 3.f, 8.f );*/
	/*const HF32 zero(Maths::ZERO_INIT);
	static HF32 angle(Maths::ZERO_INIT);
	const HF32 incr(0.001f);
	const HF32 max(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two());
	angle += incr;

	angle = Select(angle.GT(max), zero, angle);
	RotX rotx(angle);
	HMat44 mat(rotx);
	a=a*mat;
	b=b*mat;
	c=c*mat;
	*/
	
	//HVec3 d( -2.f, 3.f, 2.f);
	//p=NewAlgorithms::ClosestPtPointSegment(c, a, b);
	//HVec3 c1, c2;
	//HF32 s=ClosestPtSegmentTriangle(p, q, a, b, c, c1, c2);
	//HF32 s=TomasIntersectTriangleTriangle(a, b, c, p, q, w);

	/*ConvexHull conA(3);
	conA[0]=a;
	conA[1]=b;
	conA[2]=c;

	ConvexHull conB(3);
	conB[0]=p;
	conB[1]=q;
	conB[2]=w;

	F32 dis=0.f;
	HF32 s=GJK(conA, conB, dis);*/
	//c=ClosestPtPointSegment(p, a, b);

	
	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, c, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(p, q, w, Maths::RGBA(0, 0, 255));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, c, b, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(p, w, q, Maths::RGBA(0, 0, 255));*/
	
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, c, b, Maths::RGBA(255, 0, 0));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(p, w, Maths::RGBA(255, 0, 0));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(c, p, Maths::RGBA(64, 255, 0));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(p, 0.05f, Maths::RGBA(64, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(w, 0.05f, Maths::RGBA(64, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(c2, 0.05f, Maths::RGBA(0, 0, 255));
	//LOGINFO(g_MichelleLog, "%f", s);

}

void C_SectionSkyDome::OnRender(const C_Input& input, const C_Timer& timer)
{
	GPU_EVENT(0xFFFFFFFF, "RenderSectionSkyDome");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	m_SkyDome.Render();

	Render::DrawGrid();

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();

	Util::C_String str;
	str.Printf("Sky Dome Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
