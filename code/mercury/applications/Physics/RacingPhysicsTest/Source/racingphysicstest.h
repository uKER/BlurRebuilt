#ifndef RP_TEST_APP_H
#define RP_TEST_APP_H
#ifdef _MSC_VER
#pragma once
#endif

#include "gamepad.h"
#include "display/render/basicrender.h"

namespace Cam2
{
	class C_ChaseCamera;
	class C_DebugFeed;
}

class RacingPhysicsTestApp
{
public:
	static RacingPhysicsTestApp& Instance() {return s_instance;}

	bool Create();
	void Destroy();

	void Render();
	void Update();

	Display::C_Device* Device() {return m_pDevice;}

private:
	RacingPhysicsTestApp()
	{
		m_pFeed = 0;
		m_pChaseCamera = 0;
	}
	~RacingPhysicsTestApp();
	static RacingPhysicsTestApp s_instance;

	void InitialiseCamera();
	void UpdateCamera();

	void InitialiseVehicles();
	void UpdateVehicles();

	Display::C_Manager*	m_pDisplayManager;
	Display::C_Device* m_pDevice;
	Display::C_View m_gameView;
	Display::C_BasicRender* m_pBasicRender;
	GamePad m_controller;
	
	Cam2::C_DebugFeed* m_pFeed;
	Cam2::C_ChaseCamera* m_pChaseCamera;
};

#endif