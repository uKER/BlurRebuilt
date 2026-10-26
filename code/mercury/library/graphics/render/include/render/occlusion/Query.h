#ifndef RENDER_OCCLUSION_QUERY_H
#define RENDER_OCCLUSION_QUERY_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Render
{
namespace Occlusion
{



class C_PercentageQuery : public Ut::ListElementDouble<C_PercentageQuery>
{
public:

	C_PercentageQuery() {}

	bool CreateVolatile();
	void DestroyVolatile();

	F32 Visible();

	void SendFirstPass(const Maths::HMat43 &mWorld, const Maths::HVec3 &vScale);
	void SendSecondPass(const Maths::HMat43 &mWorld, const Maths::HVec3 &vScale);

private:

	Display::C_OcclusionQuery	m_Query;
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	Display::C_OcclusionQuery	m_Query2;
#endif

};



}; // namespace Occlusion
}; // namespace Render



#endif // RENDER_OCCLUSION_QUERY_H
