#ifndef TEST_SECTION_PHYSICS_H
#define TEST_SECTION_PHYSICS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include <Framework/Application/UserControls.h>
#include "TestPhysics.h"
namespace Render { namespace Sky { class C_SkyDome; } }

namespace Render
{
namespace Debug
{
class C_DebugRenderSys;
}
};

class C_SectionPhysics : public Framework::C_TestSection
{
public:

	C_SectionPhysics(const Util::C_String& name);
	~C_SectionPhysics();

	virtual void OnUpdate();
	virtual void OnRender3DViewWithAA(const S32 viewIndex);
	virtual void OnRender2DView();

private:
	Render::Sky::C_SkyDome*			m_pSkyDome;
	Render::Debug::C_DebugRenderSys* m_pDebugRenderSys;
	U64								m_iTimer;
	Samples::C_TestPhysics			m_TestPhysics;

	void TestPhysicsEngine();
};

enum E_PhysicsButton
{
	E_PhysicsButton_ShootABox = Framework::E_Button_Count,
	E_PhysicsButton_ShootARagdoll,
	E_PhysicsButton_ModeSwitch,
	E_PhysicsButton_Jump,
	E_PhysicsButton_Crouch,

	E_PhysicsButton_Count
};

#endif // TEST_SECTION_PHYSICS_H
