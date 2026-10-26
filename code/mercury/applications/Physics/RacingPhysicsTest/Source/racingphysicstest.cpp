#include "stdafx.h"
#include "racingphysicstest.h"
#include "racingphysics/racingphysics.h"
#include "System\artery\artery.h"
#include "system\debug/log/debuglog.h"
#include "system\debug/log/subscribers/DebugViewSubscriber.h"
#include "camera2/CameraType.h"
#include "camera2/stock/ChaseCamera.h"
#include "camera2/stock/DebugFeed.h"

DEBUG_ENUM(Cam2::E_CameraType)
	DE_OPTION(0, "InGame")
	DE_OPTION(Cam2::E_Camera,         "FixedCamera")
	DE_OPTION(Cam2::E_FlyCamera,      "FlyCamera")
	DE_OPTION(Cam2::E_TvCamera,       "TvCamera")
	DE_OPTION(Cam2::E_ScriptCamera,   "ScriptCamera")
	DE_OPTION(Cam2::E_RelativeCamera, "RelativeCamera")
	DE_OPTION(Cam2::E_RelativeFlyCamera, "RelativeFlyCamera")
END_DEBUG_ENUM

static Debug::Enum<Cam2::E_CameraType> DebugType("Camera.DebugType", (Cam2::E_CameraType)0);

//D3DDevice* g_pd3dDevice;

RacingPhysicsTestApp RacingPhysicsTestApp::s_instance;

RacingPhysicsTestApp::~RacingPhysicsTestApp()
{
	delete m_pChaseCamera;
	m_pChaseCamera = 0;
	delete m_pFeed;
	m_pFeed = 0;
}

bool RacingPhysicsTestApp::Create()
{
	m_pDisplayManager = new Display::C_Manager();

	if(!m_pDisplayManager->Initialise())
		return false;
	
	//Display::C_VideoMode videoMode;
	//if(!m_pDisplayManager->GetVideoMode(&videoMode))
	//	return false;

	Display::C_DeviceParams deviceParams;

	deviceParams.m_LargeHeapSize = 1024*1024*400;
	deviceParams.m_SmallHeapSize = 1024*1024*5;
	deviceParams.m_DeviceMemorySize = 1024*1024*20;

	m_pDevice = new Display::C_Device;

	Display::Manager()->CreateDevice(m_pDevice, deviceParams);
	
	if (!m_pDevice)
		return false;

	m_pDevice->SetPresentInterval(Display::E_PresentInterval_One);
	
	/*const S32 windowIndex = m_pDevice->AddWindow(Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1.0f, 1.0f)));
	Display::C_Window* pWindow = m_pDevice->GetWindow(windowIndex);
	pWindow->Resize(
		deviceParams.m_WindowBufferWidth,
		deviceParams.m_WindowBufferHeight,
		deviceParams.m_WindowBufferMultisample,
		deviceParams.m_WindowBufferColourFormat,
		deviceParams.m_WindowBufferDepthStencilFormat);*/

	m_pDevice->AddView(Display::E_ViewType_2D | Display::E_ViewType_3D, 0);

	m_pBasicRender = new Display::C_BasicRender;
	m_pBasicRender->Create();

	void InitialiseDebugVariables();
	InitialiseDebugVariables();

	if (ArteryStartup(true, "RacingPhysics"))
	{
		Debug::NetworkStart();
		Debug::GetRootLogger().AddSubscriber( new Debug::DebugViewSubscriber() );
	}
	else
		assert(!"ArteryStart returned FALSE");

	InitialiseCamera();
	InitialiseVehicles();

	return true;
}

void RacingPhysicsTestApp::Destroy()
{
	delete m_pDevice;
	delete m_pDisplayManager;
	delete m_pBasicRender;

	XtShutdown();
}

void RacingPhysicsTestApp::Render()
{
	Device()->BeginScene();

	Display::C_ViewSetup viewSetup;
	assert(m_pFeed);
	m_pFeed->Live().ToViewSetup(viewSetup, 16.f / 9.f);
	Device()->GetView(0)->Setup(viewSetup, Device()->GetBackBuffer().GetWidth(), Device()->GetBackBuffer().GetHeight());
	Device()->SetViewIndex(0);

	Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil, Maths::RGBA(128, 128, 255, 255));

	RacingPhysics::InfinitePlaneSystem::Instance().GetPlane().DisplayDebug();

	RacingPhysics::VehicleManager::Instance().DisplayDebug();

	m_pFeed->DebugRender();

	Device()->SetViewIndex(-1);

	Device()->EndScene();

	Device()->Present();
}

Debug::Float g_peakTorqueScale("PerformanceModifiers.TorqueScale", 1.f);
Debug::Float g_forwardGripScale("PerformanceModifiers.ForwardGripScale", 1.f);
Debug::Float g_backwardGripScale("PerformanceModifiers.BackwardGripScale", 1.f);
Debug::Float g_rpmScale("PerformanceModifiers.RpmScale", 1.f);
Debug::Float g_transverseGripScale("PerformanceModifiers.TransverseGripScale", 1.f);

void RacingPhysicsTestApp::Update()
{
	Debug::NetworkUpdate();
	m_controller.ReadControls();
	UpdateCamera();

	static bool s_paused = false;

	if (m_controller.GetDigitalPressed(GamePad::Button_Start))
		s_paused = !s_paused;

	if (!s_paused)
	{
		for (int i = 0; i < RacingPhysics::VehicleManager::Instance().GetNumVehicles(); i++)
		{
			if (i % 2 == 1) continue;

			if (RacingPhysics::Motorbike* p = RacingPhysics::Motorbike::GetMotorbikePointer(&RacingPhysics::VehicleManager::Instance().GetVehicle(i)))
			{
				RacingPhysics::MotorbikeControls& controls = p->Controls();
				controls.Clear();
				controls.throttle = m_controller.GetAnalogue(GamePad::Button_RightTrigger);
				controls.fullBrakes = m_controller.GetAnalogue(GamePad::Button_LeftTrigger);
				controls.steer = m_controller.GetAxis(GamePad::Axis_LX);
				controls.lean = m_controller.GetAxis(GamePad::Axis_LY);
				controls.rearBrake = m_controller.GetDigitalHeld(GamePad::Button_A);
			}

			if (RacingPhysics::Car* p = RacingPhysics::Car::GetCarPointer(&RacingPhysics::VehicleManager::Instance().GetVehicle(i)))
			{
				RacingPhysics::CarControls& controls = p->Controls();
				controls.Clear();
				controls.throttle = m_controller.GetAnalogue(GamePad::Button_RightTrigger);
				controls.brake = m_controller.GetAnalogue(GamePad::Button_LeftTrigger);
				controls.steer = m_controller.GetAxis(GamePad::Axis_LX);
				controls.handbrake = m_controller.GetDigitalHeld(GamePad::Button_A);
				controls.gearUp = m_controller.GetDigitalPressed(GamePad::Button_B);
				controls.gearDown = m_controller.GetDigitalPressed(GamePad::Button_X);

				if (m_controller.GetDigitalPressed(GamePad::Button_RightShoulder))
					p->SetVelocity(p->GetVelocity() + Maths::Constants<RacingPhysics::Normal3R>::Y() * 10.f);

				p->PerformanceModifiers().peakTorqueScale = g_peakTorqueScale;
				p->PerformanceModifiers().forwardGripScale = g_forwardGripScale;
				p->PerformanceModifiers().backwardGripScale = g_backwardGripScale;
				p->PerformanceModifiers().rpmScale = g_rpmScale;
				p->PerformanceModifiers().transverseGripScale = g_transverseGripScale;
			}
		}

		RacingPhysics::Update();
		RacingPhysics::VehicleManager::Instance().UpdatePhysics();
		RacingPhysics::PhysicsTimer::Instance().UpdatePhysicsTimer();
	}
}

void RacingPhysicsTestApp::InitialiseCamera()
{
	// Camera2 stuff:
	m_pChaseCamera = new Cam2::C_ChaseCamera;
	if (m_pChaseCamera == 0)
		return;
		
	m_pChaseCamera->SetParam(9.7f, 2.6f, 0.92f);
	
	m_pFeed = new Cam2::C_DebugFeed(m_pChaseCamera);
}

void RacingPhysicsTestApp::InitialiseVehicles()
{
	RacingPhysics::Create();
	RacingPhysics::VehicleManager::Instance().Initialise();

	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < 1; j++)
		{
			//RacingPhysics::Motorbike& motorbike = RacingPhysics::VehicleManager::Instance().AddVehicle<RacingPhysics::Motorbike>();
			//RacingPhysics::VehicleInitialiseInfo info;
			//info.filename = "DefaultBike";
			//motorbike.Initialise(info);
			//motorbike.SetPosition(RacingPhysics::Point3R(j * 4.f, 0.3f, -i * 6.f));

			RacingPhysics::Car& car = RacingPhysics::VehicleManager::Instance().AddVehicle<RacingPhysics::Car>();
			RacingPhysics::VehicleInitialiseInfo info;
			info.filename = "DefaultCar";
			info.autoGears = true;
			car.Initialise(info);
			car.SetPosition(RacingPhysics::Point3R(j * 4.f, 10.3f, -i * 6.f));
		}
	}

	RacingPhysics::PhysicsTimer::Instance().InitialiseTimers();
}

void RacingPhysicsTestApp::UpdateCamera()
{
	// New camera stuff:
	assert(m_pChaseCamera);
	assert(m_pFeed);

	// Alternate between cameras:
	F32 shoulder = m_controller.GetAnalogue(GamePad::Button_LeftShoulder);
	bool press = shoulder > 0.5f;
	static bool debounce = false;
	if (press && !debounce)
	{
		static S32 list[] =	{0, Cam2::E_FlyCamera, Cam2::E_Camera};
		static S32 cycle = 0;
		
		cycle++;
		if (cycle >= sizeof(list) / sizeof(list[0]))
			cycle = 0;
			
		S32 choice = list[cycle];
		DebugType = (Cam2::E_CameraType)choice;
	}
	debounce = press;

	if (m_pFeed->DebugCameraType() != DebugType)
		m_pFeed->SetDebugCameraType(DebugType);

	float t = 1.f / 60.f;

	// Chase camera:
	Cam2::C_CamObj object;
	const RacingPhysics::Vehicle& vehicle = RacingPhysics::VehicleManager::Instance().GetVehicle(0);
	
	RacingPhysics::Axis axis = vehicle.GetAxis();
	axis.RotateLocalY180();
		
	object.position = RacingPhysics::ConvertHVec(axis.Position());
		
	RacingPhysics::Quaternion quat = RacingPhysics::Quaternion(axis);
	object.orient = Maths::C_Quaternion((float)quat.A().X(), (float)quat.A().Y(), (float)quat.A().Z(), quat.W());

	m_pChaseCamera->SetObject(object);
	m_pChaseCamera->Update(t);

	// Fly Camera:
	Cam2::CamInput input;
	input.axis[Cam2::E_CamAxis_FlyMoveX]   = S32(m_controller.GetAxis(GamePad::Axis_LX) * 32767.f);
	input.axis[Cam2::E_CamAxis_FlyMoveY]   = S32(m_controller.GetAxis(GamePad::Axis_LY) * 32767.f);
	input.axis[Cam2::E_CamAxis_FlyRotateX] = S32(m_controller.GetAxis(GamePad::Axis_RX) * 32767.f);
	input.axis[Cam2::E_CamAxis_FlyRotateY] = S32(m_controller.GetAxis(GamePad::Axis_RY) * 32767.f);
	input.button[Cam2::E_CamButton_FlySlow] = (U8)(m_controller.GetAnalogue(GamePad::Button_LeftTrigger) * 255.f);
	input.button[Cam2::E_CamButton_FlyFast] = (U8)(m_controller.GetAnalogue(GamePad::Button_RightTrigger) * 255.f);

	m_pFeed->SetInput(input);
	m_pFeed->SetObject(object);
	m_pFeed->Update(t);
}

void DisplayDirtyDiscError() {}

XT_APPLICATION
	XT_DEPENDS_ON(RacingPhysics_Surfaces)
	XT_DEPENDS_ON(RacingPhysics_DynamicsParameters)
	XT_DEPENDS_ON(RacingPhysics_MotorbikeDynamics)
	XT_DEPENDS_ON(RacingPhysics_CarDynamics)
XT_END_APPLICATION

