#include "stdafx.h"
#include "Render/Occlusion/Query.h"



namespace Render
{
namespace Occlusion
{



bool C_PercentageQuery::CreateVolatile()
{
	if (!m_Query.Create())
		return false;
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	if (!m_Query2.Create())
		return false;
#endif
	return true;
}

void C_PercentageQuery::DestroyVolatile()
{
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	m_Query2.Destroy();
#endif
	m_Query.Destroy();
}

F32 C_PercentageQuery::Visible()
{
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	U32 pixelsPassed = 0;
	U32 pixelsTotal = 0;
	Display::E_OcclusionQueryResult ePassedResult = m_Query.Visible(pixelsPassed);
	Display::E_OcclusionQueryResult eTotalResult = m_Query2.Visible(pixelsTotal);
	Display::E_OcclusionQueryResult eFailed = Display::E_OcclusionQueryResult_Failed;
	if (eFailed != ePassedResult && eFailed != eTotalResult)
		if (pixelsPassed + pixelsTotal > 0)
			return Maths::Clamp01( F32(pixelsPassed) / F32(pixelsTotal) );
#endif
	return 0.0f;
}

void C_PercentageQuery::SendFirstPass(const Maths::HMat43 &mWorld, const Maths::HVec3 &vScale)
{
	Display::Device()->SetDepthEnable(Display::E_Bool_True);

	m_Query.Begin();
	Display::OcclusionQueryRender()->Render(mWorld, vScale);
	m_Query.End();
}

void C_PercentageQuery::SendSecondPass(const Maths::HMat43 &mWorld, const Maths::HVec3 &vScale)
{
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	m_Query2.Begin();
	Display::OcclusionQueryRender()->Render(mWorld, vScale);
	m_Query2.End();
#endif
}



}; // namespace Occlusion
}; // namespace Render
