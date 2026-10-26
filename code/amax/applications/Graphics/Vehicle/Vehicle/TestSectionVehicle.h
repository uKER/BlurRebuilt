#ifndef TEST_SECTION_VEHICLE_H
#define TEST_SECTION_VEHICLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>

namespace SceneGraph
{
	class C_SceneGraph;
	class C_BackFrontSorter;
	class C_View;
}

namespace Render { namespace Sky { class C_SkyDome; } }

class C_TestSectionVehicle : public Framework::C_TestSection
{
public:

	C_TestSectionVehicle(const Util::C_String& name);
	~C_TestSectionVehicle();

	bool CreateSceneGraph();
	bool OnCreate();
	void OnDestroy();

	void OnUpdate();

	void OnPreRender();

	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender2DView();

protected:

	bool								m_Initialised;

	Render::Sky::C_SkyDome*				m_pSkyDome;

	SceneGraph::C_SceneGraph*			m_pSceneGraph;
	SceneGraph::C_BackFrontSorter*	m_pSorter;
	SceneGraph::C_View*					m_pView;
};

#endif // TEST_SECTION_VEHICLE_H
