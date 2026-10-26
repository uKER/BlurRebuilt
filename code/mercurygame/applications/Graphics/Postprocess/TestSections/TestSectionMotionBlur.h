#ifndef TEST_SECTION_MOTION_BLUR_H
#define TEST_SECTION_MOTION_BLUR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSectionHDR.h>
#include <Render/Mesh/Mesh.h>
#include <Render/Sky/SkyDome.h>

namespace PostProcess
{
	class C_MotionBlurId;
	class C_MotionBlurDoFReduced;
};

struct C_MotionBlurBox
{
	Maths::HVec3			m_vPosition;
	Maths::HVec3			m_vLastPosition;
	Maths::HVec3			m_vVelocity;
};

class C_TestSectionMotionBlur : public Framework::C_TestSectionHDR
{
public:

	C_TestSectionMotionBlur(const Util::C_String& name);
	~C_TestSectionMotionBlur();

	bool OnCreate();
	void OnDestroy();

	void OnUpdate();

	void OnRenderHdr3DViewWithAA(const S32 viewIndex);
	void OnRenderHdr3DViewWithoutAA(const S32 viewIndex);

	void OnBeginPostEffects();
	void OnEndPostEffects();

private:
	
	Render::Mesh::C_Mesh					m_Mesh;
	Render::Sky::C_SkyDome					m_SkyDome;

	PostProcess::C_MotionBlurId*			m_pMotionBlurId;
	PostProcess::C_MotionBlurDoFReduced*	m_pMotionBlurDoFReduced;

	C_MotionBlurBox							m_Boxes[3];

};

#endif // TEST_SECTION_MOTION_BLUR_H
