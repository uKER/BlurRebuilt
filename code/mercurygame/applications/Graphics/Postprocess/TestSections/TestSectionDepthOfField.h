#ifndef TEST_SECTION_DEPTH_OF_FIELD_H
#define TEST_SECTION_DEPTH_OF_FIELD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "Effect/Postprocess/DepthOfField/Classify.h"
#include "Effect/Postprocess/DepthOfField/FgBg.h"
#include "Effect/Postprocess/DepthOfField/Simple.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Sky/SkyDome.h"

class C_TestSectionDepthOfField : public Framework::C_TestSection
{
public:

	C_TestSectionDepthOfField(const Util::C_String& name);
	~C_TestSectionDepthOfField();

	bool OnCreate();
	void OnDestroy();

	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender3DViewWithoutAA(const S32 viewIndex);

private:
	
	Render::Mesh::C_Mesh					m_Mesh;
	Render::Sky::C_SkyDome					m_SkyDome;
	PostProcess::DepthOfField::C_Classify	m_DepthOfFieldClassify;
	PostProcess::DepthOfField::C_FgBg		m_DepthOfFieldFgBg;
	PostProcess::DepthOfField::C_Simple		m_DepthOfFieldSimple;

	Display::C_Texture2D					m_ColourTexture;
	Display::C_Texture2D					m_DepthTexture;

};

#endif // TEST_SECTION_DEPTH_OF_FIELD_H
