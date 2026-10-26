#ifndef TEST_SECTIONS_TEST_SECTION_LIVERY_H
#define TEST_SECTIONS_TEST_SECTION_LIVERY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace Render { namespace Sky { class C_SkyDome; }; };

namespace Vehicle
{

namespace Livery
{

class C_DebugVehicle
{
public:

	C_DebugVehicle();

	void Initialise();
	void Shutdown();
	void Update();
	void Render();

public:

	float						m_Width;
	float						m_Length;
	float						m_Height;

	Maths::HVec3				m_Position;

	Display::C_Texture2D		m_Texture_Left;
	Display::C_Texture2D		m_Texture_Right;
	Display::C_Texture2D		m_Texture_Front;
	Display::C_Texture2D		m_Texture_Rear;
	Display::C_Texture2D		m_Texture_Top;

}; // class C_DebugVehcile

}; // namespace Livery

}; // namespace Vehicle

namespace Framework
{

class C_TestSectionLivery : public C_TestSection
{
public:

	C_TestSectionLivery(const Util::C_String& name);
	~C_TestSectionLivery();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnUpdate();
	void OnPreRender();
	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender2DView();

protected:

	Render::Sky::C_SkyDome*		m_pSkyDome;

	Vehicle::Livery::C_DebugVehicle		m_DebugVehicle;
};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_LIVERY_H
