#ifndef RP_TEST_APP_H
#define RP_TEST_APP_H
#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Application/Application.h>
#include "Framework/Application/CoreSystems.h"

#include "gamepad.h"
#include "display/render/basicrender.h"
#include "gravity/sdk/Scene.h"
#include "system/debug/i_debugrender.h"
#include "racingphysics/vehicles/vehiclemanager.h"
#include <camera2/stock/DebugFeed.h>
#include "gravity/importer/importer.h"

#include "renderingdata/renderingdata.h"
#include "system/serialise/c_filesystem.h"
#include "system/serialise/c_fileblock.h"
#include "renderingdata/archive.h"
#include "render/sky/skydome.h"
#include "render/font/font.h"

namespace Collision
{
	class OptimisedAABBTree;
};

namespace Cam2
{
	class C_ChaseCamera2;
}

namespace RacingPhysics
{
	class Car;
}

const int NUM_GROUPS = 7;
const int MATERIAL_COUNT = 64;

class RacingPhysicsTestApp
{
public:
	static RacingPhysicsTestApp& Instance() {return s_instance;}

	bool Create();
	void Destroy();

	void Render();
	void Update();

	Display::C_Device* Device() {return m_pDevice;}

	void ShootABox(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position, const Maths::HVec3 PCREF vLook);
	void ShootACar(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position, const Maths::HVec3 PCREF vLook);
	void CreateGravityCar(RacingPhysics::Car& car, const char* fileName, const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF cOfMOffset);

private:
	RacingPhysicsTestApp();

	~RacingPhysicsTestApp();
	static RacingPhysicsTestApp s_instance;

	void InitialiseCamera();
	void UpdateCamera();
	void CyclesCameras();

	void InitialiseScheduler();
	void KillScheduler();
	void InitialiseVehicles();
	void UpdateVehicles();
	void InitialiseGravity();
	void RenderGravity();

	Gravity::C_Body* AddBox(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF extents, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter);
	Gravity::C_Body* AddCar(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, Collision::C_CollisionFilter* pFilter);

	Framework::C_CoreSystems* m_pCoreSystems;
	Display::C_Manager*	m_pDisplayManager;
	Display::C_Device* m_pDevice;
	Display::C_View m_gameView;
	Display::C_BasicRender* m_pBasicRender;
	GamePad m_controller;
	Gravity::C_Scene* m_pScene;
	SharedPhysics::C_MaterialPair m_Materials[MATERIAL_COUNT][MATERIAL_COUNT];
	SharedPhysics::C_SafeStack<true> stack;
	Gravity::C_ExternalConstants m_ExternalConstants;
	Collision::C_CollisionFilter m_GroundFilter;
	Collision::C_CollisionFilter m_BoxFilter;
	Cam2::C_DebugFeed m_DebugFeed;

	DbUtil::I_DebugRenderSys* m_pDebugRenderSys;
	Cam2::C_ChaseCamera2* m_pChaseCamera;
	bool m_debugUsesInput;
	Ut::Vector< Ut::Vector< S32 > > m_camList;
	S32 m_camGroup, m_camSel;

	Scheduler::GroupHandle Groups[NUM_GROUPS];
	Scheduler::PolicyHandle hPolicy, collisionPolicy;

	Gravity::C_ImportScene* pImportScene;
	Gravity::C_ImportSceneStaticInstance* pStaticScene;

	Render::Font::C_FontSystem* m_pFontSystem;
};

#endif
