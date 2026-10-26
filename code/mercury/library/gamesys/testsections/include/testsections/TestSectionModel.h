#ifndef TEST_SECTIONS_TEST_SECTION_MODEL_H
#define TEST_SECTIONS_TEST_SECTION_MODEL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "RenderingData/RenderingData.h"
#include "Renderingdata/Scene.h"
#include "Renderingdata/Target.h"
#include "Render/Sky/SkyDome.h"
#include "TestSections/TestSectionHDR.h"

namespace Framework
{

class C_TestSectionModel : public C_TestSectionHDR
{
public:

	C_TestSectionModel(const Util::C_String& name);
	~C_TestSectionModel();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroy();
	void OnDestroyVolatile();

	void OnUpdate();
	void OnRenderHdr3DViewWithAA(const S32 viewIndex);
	void OnRenderHdr3DViewWithoutAA(const S32 viewIndex);

protected:

	Display::C_TextureCube			m_TextureCube;
	Render::Sky::C_SkyDome			m_SkyDome;
	RenderingData::Target::Scene	m_Scene;

};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_MODEL_H
