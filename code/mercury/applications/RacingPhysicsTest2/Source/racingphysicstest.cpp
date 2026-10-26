#include "stdafx.h"
#include "racingphysicstest.h"
#include "racingphysics/racingphysics.h"
#include "racingphysics/debug/draw.h"
#include "racingphysics/debug/colour.h"
#include "System\artery\artery.h"
#include "system\debug/log/debuglog.h"
#include "system\debug/log/subscribers/DebugViewSubscriber.h"
#include "gravity/sdk/PhysicsSDK.h"
#include "gravity/sdk/Body.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sdk/bodyhelper.h"
#include "gravity/sdk/shapedescriptors.inl"
#include "gravity/narrowphase/optimisedaabbtree.h"
#include "gravity/narrowphase/optimisedaabbtree.inl"
#include "gravity/sharedphysics/sortedarray.inl"
#include "gravity/sdk/bodyhelper.inl"
#include "gravity/importer/importer.h"
#include <system\debug\I_DebugRender.h>
#include "gravity/narrowphase/shapes.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/narrowphase/primitive.h"
#include "gravity/sharedphysics/contactstreamiterator.h"
#include "gravity/sharedphysics/contactstreamiterator.inl"
#include "render/debug/debugrender.h"
#include "render/debug/immediatedebugrender.h"
#include <camera2.h>
#include <camera2/stock/ChaseCamera2.h>
#include "gravity/narrowphase/optimisedaabbtree.h"
#include "racingphysics/vehicles/vehiclemanager.h"
#include <System/misc/random.h>
#pragma warning (push, 3)
#include <system/debug/I_DebugRender.h>
#pragma warning (pop)

DEBUG_ENUM(Cam2::E_CameraType)
	DE_OPTION(0,							"InGame")
	DE_OPTION(Cam2::E_FixedCamera,			"FixedCamera")
	DE_OPTION(Cam2::E_FlyCamera,			"FlyCamera")
	DE_OPTION(Cam2::E_TvCamera,				"TvCamera")
	DE_OPTION(Cam2::E_ScriptCamera,			"ScriptCamera")
	DE_OPTION(Cam2::E_RelativeCamera,		"RelativeCamera")
	DE_OPTION(Cam2::E_RelativeFlyCamera,	"RelativeFlyCamera")
	DE_OPTION(Cam2::E_TvFeed,				"TvFeed")
END_DEBUG_ENUM

static Debug::Enum<Cam2::E_CameraType> DebugType("Camera.DebugType", (Cam2::E_CameraType)0);
static S32 LastType = 0;

class C_PhysicsChecks : public Cam2::I_Physics
{
public:
	virtual F32 RayTrace(const Maths::HVec3& start, const Maths::HVec3& end, F32 radius) const
	{
		F32 length = (end - start).Length();
		if (length < 1e-6f)
			return 1.f;

		Maths::HVec3 direction = (end - start) / length;

		F32 further = length ;
		if (radius > 0.f)
			further *= 1.5f; // Ray trace 50% further to get an accurate capsule trace
		
		Collision::C_Ray ray;
		ray.InitialiseRay(start, Maths::C_Normal(direction), further, RacingPhysics::VehicleManager::Instance().GetWheelGroundFilter(), radius);
		
		Gravity::C_Scene* pScene = RacingPhysics::VehicleManager::Instance().GravityScene();
		pScene->DoRayTrace(ray, pScene->m_Stack);
	
		F32 frac = 1.f;
		if (ray.m_result.m_pShape)
		{
			frac = (F32)ray.m_result.m_distance / length;
			if (frac > 1.f)
				frac = 1.f;
		}

		return frac;
	}
};

static C_PhysicsChecks s_PhysicsChecks;

RacingPhysicsTestApp RacingPhysicsTestApp::s_instance;

RacingPhysicsTestApp::RacingPhysicsTestApp() : m_pScene(NULL), stack(16000), m_GroundFilter(0, 3, 0, 0, 0), m_BoxFilter(2, 2, 0, 0, 0), m_pFontSystem(NULL)
{
	m_pChaseCamera = 0;
	m_debugUsesInput = 0;
	m_camGroup = 0;
	m_camSel = 0;
	m_pCoreSystems = 0;
}


RacingPhysicsTestApp::~RacingPhysicsTestApp()
{
	delete m_pDebugRenderSys;
	delete m_pChaseCamera;

	delete pStaticScene;
	delete pImportScene;
	delete m_pCoreSystems;
	m_pChaseCamera = 0;
}

void RacingPhysicsTestApp::InitialiseScheduler()
{
	Scheduler::Policy piMed[] = { 7, 7, 7, 7, 7, 7, 7, 7 };
	static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);
	hPolicy = _hPolicy;

	Scheduler::Policy piHigh[] = {10, 10, 10, 10, 10, 10, 10, 10 };
	static Scheduler::PolicyHandle _highPolicy = Scheduler::CreatePolicy(piHigh);
	collisionPolicy = _highPolicy;
	

	Scheduler::Init();
	Scheduler::Start();

	Scheduler::SetGroupPoolSize(NUM_GROUPS+2);
	for (int i = 0; i < NUM_GROUPS; i++)
		Groups[i] = Scheduler::CreateGroup();
}

void RacingPhysicsTestApp::KillScheduler()
{
	Scheduler::Shutdown();
}

bool RacingPhysicsTestApp::Create()
{
#ifdef PLATFORM_XENON
	Sys::C_FileSystem::Instance().SetWorkingDirectory("GAME:");
#endif

	m_pDisplayManager = new Display::C_Manager();

	if(!m_pDisplayManager->Initialise())
		return false;

	Display::C_DeviceParams deviceParams;

#if defined(PLATFORM_PS3)
	deviceParams.m_SystemMemorySize	= 1024*1024*45;
	deviceParams.m_MainMemorySize = 1024*1024*10;
	deviceParams.m_DeviceMemorySize	= 1024*1024*50;
	deviceParams.m_UniqueBackBuffer	= true;
#elif defined(PLATFORM_XENON)
	deviceParams.m_LargeHeapSize = 1024*1024*300;
	deviceParams.m_SmallHeapSize = 1024*1024*5;
	deviceParams.m_DeviceMemorySize	= 1024*1024*20;
#endif

	m_pDevice = new Display::C_Device;

	Display::Manager()->CreateDevice(m_pDevice, deviceParams);
	
	if (!m_pDevice)
		return false;

	m_pDevice->SetPresentInterval(Display::E_PresentInterval_One);

	m_pDevice->AddView(Display::E_ViewType_2D | Display::E_ViewType_3D, 0);

	m_pBasicRender = new Display::C_BasicRender;
	m_pBasicRender->Create();

	void InitialiseDebugVariables();
	InitialiseDebugVariables();

	m_pCoreSystems = new C_CORESYSTEMS();

	// TODO: Initialise cross-platform core sytems params here.

	Framework::C_CoreSystemsParams coreSystemsParams;
	m_pCoreSystems->Initialise(coreSystemsParams);

	m_pFontSystem = new Render::Font::C_FontSystem();
	Render::Font::FontSystem(m_pFontSystem);

	m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
	DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	//Render::Debug::C_ImmediateDebugRender* immediate = (Render::Debug::C_ImmediateDebugRender*)m_pDebugRenderSys->Immediate();
	//immediate->Create();
	m_pDevice->CreateRenderers();
	m_pDevice->CreateRenderersVolatile();
	InitialiseScheduler();
	InitialiseGravity();
	RacingPhysics::VehicleManager::Instance().SetGravityScene(m_pScene);
	RacingPhysics::VehicleManager::Instance().SetGravityConstants(&m_ExternalConstants);
	InitialiseCamera();
	InitialiseVehicles();

	return true;
}

void RacingPhysicsTestApp::Destroy()
{
	//Render::Debug::C_ImmediateDebugRender* immediate = (Render::Debug::C_ImmediateDebugRender*)m_pDebugRenderSys->Immediate();
	//immediate->Destroy();
	m_pDevice->DestroyRenderers();
	m_pDevice->DestroyRenderersVolatile();
	delete m_pDevice;
	delete m_pDisplayManager;
	delete m_pBasicRender;
	delete m_pFontSystem;

	Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseScene(m_pScene);
	KillScheduler();
	XtShutdown();
}

/*Gravity::C_Body* RacingPhysicsTestApp::AddBox(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF extents, const Maths::HF32 PCREF mass, Collision::C_CollisionFilter* pFilter)
{
	SharedPhysics::C_Deactivator deactivator;
	deactivator.m_ActivateThresholdSq = mass * 0.01f;
	deactivator.m_SleepThresholdSq = mass * 0.005f;
	Gravity::C_Body* boxBody(m_pScene->CreateBody());
	boxBody->SetExternalConstants(&m_ExternalConstants);
	Gravity::C_BoxDesc boxDesc;
	boxDesc.m_HalfExtents = extents;
	Maths::HF32 margin(Maths::Max(extents.X(), Maths::Min(extents.Y(), extents.Z())));
	margin *= 0.3f;
	boxDesc.m_Margin = margin;
	boxDesc.m_Epsilon = margin * 0.0005f;
	Gravity::C_BodyHelper bodyHelper(&boxDesc);
	bodyHelper.CreateBody(mass, position, orientation, boxBody, pFilter, RacingPhysics::BoxMaterial);
	boxBody->SetDeactivator(deactivator);
	boxBody->Slop(F32(margin) * 0.01f);
	boxBody->Baumgarte(0.75f);
	m_pScene->AddBody(boxBody);
	return boxBody;
}

Gravity::C_Body* RacingPhysicsTestApp::AddCar(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, Collision::C_CollisionFilter* pFilter)
{
	Gravity::C_Body* m_pGravityCarBody = m_pScene->CreateBody();
	
	SharedPhysics::C_Deactivator deactivator;
	const Maths::HF32 topBoxMass(900.f);
	const Maths::HF32 bottomBoxMass(3000.f);
	const Maths::HF32 mass(topBoxMass + bottomBoxMass);// + sphereMass);
	const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
	deactivator.m_ActivateThresholdSq =0.01f;
	deactivator.m_SleepThresholdSq = 0.01f;
	m_pGravityCarBody->SetExternalConstants(&m_ExternalConstants);
	m_pGravityCarBody->SetDeactivator(deactivator);
	

	Gravity::C_BoxDesc boxDesc;
	boxDesc.m_HalfExtents = Maths::HVec3(1.1f, 0.4f, 1.8f);
	Gravity::C_BoxDesc topBoxDesc;
	topBoxDesc.m_HalfExtents = Maths::HVec3(1.1f, 0.3f, 0.9f);

	Maths::HF32 margin(0.5f);
	margin *= 0.2f;
	boxDesc.m_Margin = margin;
	boxDesc.m_Epsilon = margin * 0.005f;
	topBoxDesc.m_Margin = margin;
	topBoxDesc.m_Epsilon = margin * 0.005f;

	Gravity::C_CompositeDesc compDesc;
	Maths::HMat33 rot;
	rot.SetIdentity();

	const Maths::HF32 CofMY((((boxDesc.m_HalfExtents.Y()* bottomBoxMass) + (topBoxDesc.m_HalfExtents.Y() * topBoxMass)))/Maths::HF32(mass));
	const Maths::HF32 midY = (boxDesc.m_HalfExtents.Y() + topBoxDesc.m_HalfExtents.Y());
	const Maths::HF32 centreYOffset = 0.f;
	const Maths::HF32 centreOfBox(boxDesc.m_HalfExtents.Y() - CofMY);
	const Maths::HF32 centreOfTopBox(centreOfBox + boxDesc.m_HalfExtents.Y() + topBoxDesc.m_HalfExtents.Y());

	compDesc.AddShape(&boxDesc, Maths::HMat43(rot, Maths::HVec3(0.f, centreOfBox, 0.f)), bottomBoxMass, pFilter);
	compDesc.AddShape(&topBoxDesc, Maths::HMat43(rot, Maths::HVec3(0.f, centreOfTopBox, 0.f)), topBoxMass, pFilter);
	compDesc.RemapShape();
	compDesc.SetRemapped();
	Gravity::C_BodyHelper bodyHelper(&compDesc);
	bodyHelper.CreateBody(mass, position, orientation, m_pGravityCarBody, pFilter, RacingPhysics::BoxMaterial);
	m_pGravityCarBody->Slop(F32(margin) * 0.0001f);
	m_pGravityCarBody->Baumgarte(0.7f);
	m_pScene->AddBody(m_pGravityCarBody);
	return m_pGravityCarBody;
}

void RacingPhysicsTestApp::ShootABox(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position, const Maths::HVec3 PCREF vLook)
{
	Maths::HMat33 rot;
	rot.SetIdentity();
	Maths::C_Quaternion orientation(rot);
	const Maths::HVec3 extents(0.65f, 0.85f, 0.65f);
	Gravity::C_Body* body = AddBox(position, orientation, extents, 1200.f, pFilter);
	body->LinearVelocity(vLook * 15.f);
	body->AngularVelocity(Maths::HVec3(0.707f, 0.707f, 0.f));
}

void RacingPhysicsTestApp::ShootACar(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position, const Maths::HVec3 PCREF vLook)
{
	Maths::HMat33 rot;
	rot.SetIdentity();
	Maths::C_Quaternion orientation(rot);
	const Maths::HVec3 extents(0.85f, 0.85f, 0.85f);
	Gravity::C_Body* body = AddCar(position, orientation, pFilter);
	body->LinearVelocity(vLook * 40.f);
	body->AngularVelocity(Maths::HVec3(0.577f, 0.577f, 0.577f) * 10.f);
}*/

void RacingPhysicsTestApp::InitialiseGravity()
{
	m_ExternalConstants.m_Gravity = Maths::HVec3(0.f, -9.8f, 0.f);
	Collision::C_CollisionFilter* pFilter = &m_GroundFilter;
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HVec3 vzero(zero);
	Maths::HMat33 mat;
	mat.SetIdentity();
	for(U32 a = 0; a < MATERIAL_COUNT; ++a)
	{
		for(U32 b = 0; b < MATERIAL_COUNT; ++b)
		{
			if((a == RacingPhysics::CarBodyMaterial && b == RacingPhysics::BoxMaterial) || 
				(a == RacingPhysics::BoxMaterial && b == RacingPhysics::CarBodyMaterial))
			{
				m_Materials[a][b].m_Friction = 0.55f;//(g_StaticFriction[a][b] + g_DynamicFriction[a][b]) * 0.8f;
				m_Materials[a][b].m_RestitutionCoefficient = 0.05f;//g_Restitution[a][b];
			}
			if((b == RacingPhysics::BoxMaterial) || 
				(a == RacingPhysics::BoxMaterial))
			{
				m_Materials[a][b].m_Friction = 0.55f;//(g_StaticFriction[a][b] + g_DynamicFriction[a][b]) * 0.8f;
				m_Materials[a][b].m_RestitutionCoefficient = 0.1f;//g_Restitution[a][b];
			}
			else
			{
				m_Materials[a][b].m_Friction = 0.55f;//(g_StaticFriction[a][b] + g_DynamicFriction[a][b]) * 0.8f;
				m_Materials[a][b].m_RestitutionCoefficient = 0.02f;//g_Restitution[a][b];
			}
		}
	}
	//m_Materials[0].m_Friction = 0.8f;
	//m_Materials[0].m_RestitutionCoefficient = 0.1f;
	m_pScene = Gravity::C_PhysicsSDK::PhysicsSDK()->CreateScene(&m_Materials[0][0], MATERIAL_COUNT);
	m_pScene->SetInternalGroupHandle(Groups[1]);
	m_pScene->SetInternalTaskPolicy(collisionPolicy);

	Collision::C_AxisBoxMinMax worldBox;
	worldBox.m_min = Maths::HVec3(-100.f, -100.f, -100.f);
	worldBox.m_max = Maths::HVec3(100.f, 100.f, 100.f);
	Maths::HVec3 desiredSize(5.f, 5.f, 5.f);
	const U32 maxDepth(6);

	//Load in an environment
	Gravity::C_Body* pTriMeshBody = m_pScene->CreateBody();
	Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "test.dat" );
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> archive(*filePtr);


	pImportScene = new Gravity::C_ImportScene;
	pImportScene->Import(filePtr);
	pStaticScene = new Gravity::C_ImportSceneStaticInstance(m_pScene, pImportScene);

	Util::C_Random random;

	for(U32 i=0; i< pImportScene->m_DynamicShapes.m_numElements; ++i)
	{

		Gravity::C_ImportShape* pImportShape=pImportScene->m_DynamicShapes[i];
		Gravity::C_ShapeDesc* pDesc=pImportShape->GetShapeDesc();
		pDesc->RemapShape();
		pDesc->SetRemapped();
		
		Gravity::C_Body* pBody(m_pScene->CreateBody());
		pBody->SetExternalConstants(&m_ExternalConstants);
		pBody->Baumgarte(0.7f);
		pBody->Slop(0.00000001f);
		
		Maths::HMat33 identity;
		identity.SetIdentity();

		Gravity::C_BodyHelper helper(pDesc);
		helper.CreateBody(pImportShape->GetShapeDesc()->GetMass(), pImportShape->GetImportPosition(), pImportShape->GetImportRotation(), pImportShape->GetImportIneriaTensor(), pBody, &this->m_BoxFilter);
		
		SharedPhysics::C_Deactivator deactivator;
		if(pDesc->m_Type == Collision::E_Box_Shape)
		{
			deactivator.m_ActivateThresholdSq = 0.1f;
			deactivator.m_SleepThresholdSq = 0.01f;
			deactivator.m_sSleepCountdown = 15;
			pBody->SetMass(random.FRand(50.f, 150.f));
		}
		else
		{
			deactivator.m_ActivateThresholdSq = 1.f;
			deactivator.m_SleepThresholdSq = 2.f;
			deactivator.m_sSleepCountdown = 15;
			pBody->SetMass(4000.f);
		}

		pBody->SetDeactivator(deactivator);
		
		m_pScene->AddBody(pBody);
	
	}

	Gravity::C_PlaneDesc planeDesc;
	planeDesc.m_Normal = Maths::Constants<Maths::C_Normal>::Y();
	planeDesc.m_Distance = 0.f;

	Collision::C_CollisionFilter filter;
	//Collision::C_Shape* pPlane = planeDesc.CreateShape(&filter);
	Gravity::C_Body* pPlaneBody = m_pScene->CreateBody();
	Gravity::C_BodyHelper helper(&planeDesc);
	Maths::HMat33 identity;
	identity.SetIdentity();

	helper.CreateBody(0.f, Maths::HVec3(0.f, 0.f, 0.f), identity, pPlaneBody, &filter, 0, 0);
	SharedPhysics::C_Deactivator deactivator;
	pPlaneBody->SetDeactivator(deactivator);

	m_pScene->SetGroundPlane(pPlaneBody);
	

	delete filePtr;
}

void RacingPhysicsTestApp::Render()
{
	PERF_EVENT("RacingPhysicsTestApp.Render");

	assert(m_pChaseCamera);

	Device()->BeginScene();

	Display::C_ViewSetup viewSetup;
	m_DebugFeed.Live().ToViewSetup(viewSetup, 16.f / 9.f);
	viewSetup.m_ViewSetup3D.m_FarClip = 1000.f;

	Device()->GetView(0)->Setup(viewSetup, Device()->GetBackBuffer().GetWidth(), Device()->GetBackBuffer().GetHeight());
	Device()->SetViewIndex(0);

	Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil, Maths::RGBA(128, 128, 255, 255));

	RenderGravity();
	
	

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();

	Display::Device()->ResetStates();
	Display::Device()->DefaultStates();
	Display::Device()->SetVertexShader(NULL);
	Display::Device()->SetPixelShader(NULL);
	DbUtil::DefaultDebugRenderSys()->RenderPrimitive();
	DbUtil::DefaultDebugRenderSys()->PrimitiveEndFrame();

	RacingPhysics::VehicleManager::Instance().DisplayDebug();

	Display::Device()->ResetStates();
	Display::Device()->DefaultStates();
	Display::Device()->SetVertexShader(NULL);
	Display::Device()->SetPixelShader(NULL);


	m_pChaseCamera->DebugRender();
	m_DebugFeed.Live().DebugRender();


	Device()->SetViewIndex(-1);

	Device()->EndScene();

	Device()->Present();
}

void RacingPhysicsTestApp::RenderGravity()
{
	DbUtil::DefaultDebugRenderSys()->Deferred()->SetDepthEnable(true);
	DbUtil::DefaultDebugRenderSys()->Deferred()->SetLightDirection(Maths::HVec3(0.707f, -0.707f, 0.f));
	const Maths::HVec3 camPos(this->m_DebugFeed.Live().Position());
	const Maths::HMat43 rot(m_DebugFeed.Live().Mat43());
	const Maths::HF32 viewDistance(200.f);

	Collision::C_CRAxisBox box;
	box.centre = camPos;
	box.extents = Maths::HVec3(100.f, 50.f, 100.f);

	m_pScene->DebugRenderScene(DbUtil::DefaultDebugRenderSys()->Primitive(), box, m_pScene->m_Stack);

}

Debug::Float g_peakTorqueScale("PerformanceModifiers.TorqueScale", 1.f);
Debug::Float g_forwardGripScale("PerformanceModifiers.ForwardGripScale", 1.f);
Debug::Float g_backwardGripScale("PerformanceModifiers.BackwardGripScale", 1.f);
Debug::Float g_rpmScale("PerformanceModifiers.RpmScale", 1.f);
Debug::Float g_transverseGripScale("PerformanceModifiers.TransverseGripScale", 1.f);

namespace RacingPhysics
{
	extern Debug::Float g_timeStepScale;
}

void RacingPhysicsTestApp::Update()
{
	PERF_EVENT("RacingPhysicsTestApp.Update");

	Debug::NetworkUpdate();
	m_controller.ReadControls();

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
				
				if (m_debugUsesInput)
					controls.Clear();
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

				if (m_controller.GetDigitalHeld(GamePad::Button_Back)
					&& m_controller.GetDigitalPressed(GamePad::Button_A))
				{
					Maths::HVec3 c = m_DebugFeed.Live().Position();
					p->SetPosition(RacingPhysics::Point3R(c.X(), c.Y(), c.Z()));
					RacingPhysics::Axis axis;
					axis.SetPosition(RacingPhysics::Point3R(c.X(), c.Y(), c.Z()));
					p->SetAxis(axis);
					p->SetVelocity(Maths::Constants<RacingPhysics::Vector3R>::Null());
					p->SetAngularVelocity(Maths::Constants<RacingPhysics::Vector3R>::Null());
					p->CopyToGravity();
					if (m_pChaseCamera)
						m_pChaseCamera->Reset();
				}

				p->PerformanceModifiers().peakTorqueScale = g_peakTorqueScale;
				p->PerformanceModifiers().forwardGripScale = g_forwardGripScale;
				p->PerformanceModifiers().backwardGripScale = g_backwardGripScale;
				p->PerformanceModifiers().rpmScale = g_rpmScale;
				p->PerformanceModifiers().transverseGripScale = g_transverseGripScale;

				if (m_debugUsesInput)
					controls.Clear();

				//p->CopyToGravity();
			}
		}

		RacingPhysics::UpdateXt();
		//RacingPhysics::VehicleManager::Instance().UpdatePhysics();
		RacingPhysics::PhysicsTimer::Instance().UpdatePhysicsTimer();

		/*if (m_controller.GetDigitalPressed(GamePad::Button_Y))
		{
			Maths::HMat43 mat = m_DebugFeed.Live().Mat43();
			Maths::HVec3 pos = mat.AW() + mat.AY() * -0.2f;
			Maths::HVec3 dir = mat.AZ();
			ShootABox(&m_BoxFilter, pos, dir);
		}

		if (m_controller.GetDigitalPressed(GamePad::Button_B))
		{
			Maths::HMat43 mat = m_DebugFeed.Live().Mat43();
			Maths::HVec3 pos = mat.AW() + mat.AY() * -0.2f;
			Maths::HVec3 dir = mat.AZ();
			ShootACar(&m_BoxFilter, pos, dir);
		}*/

		m_pScene->Simulate(RacingPhysics::PhysicsTimer::Instance().PhysicsUpdateTime() * RacingPhysics::g_timeStepScale);
	}

	UpdateCamera();
}

void RacingPhysicsTestApp::InitialiseCamera()
{
	// Camera2 stuff:
	m_pChaseCamera = new Cam2::C_ChaseCamera2;
	if (m_pChaseCamera == 0)
		return;

	m_pChaseCamera->SetPhysics(&s_PhysicsChecks);

	m_DebugFeed.ShowFeed(m_pChaseCamera);

	// Build camera list:
	m_camList.Clear();
	m_camList.PushBack();
	m_camList.Back().PushBack(0);
	m_camList.PushBack();
	m_camList.Back().PushBack(Cam2::E_FlyCamera);
	m_camList.Back().PushBack(Cam2::E_FixedCamera);
	m_camList.Back().PushBack(Cam2::E_RelativeFlyCamera);
	m_camList.Back().PushBack(Cam2::E_RelativeCamera);
	m_camList.PushBack();
	m_camList.Back().PushBack(Cam2::E_TvCamera);
}

void RacingPhysicsTestApp::CreateGravityCar(RacingPhysics::Car& car, const char* fileName, const Maths::HF32 mass, const Maths::HVec3 PCREF cOfMOffset)
{
	Gravity::C_Scene* pScene =  RacingPhysics::VehicleManager::Instance().GravityScene();
	Gravity::C_Body* pBody = pScene->CreateBody();
	Collision::C_CollisionFilter* pFilter = const_cast<Collision::C_CollisionFilter*>(RacingPhysics::VehicleManager::Instance().GetCarBodyFilter());
	Maths::HMat33 orientation;

	//// Rotate matrix 180 degrees about y.
	orientation.SetIdentity();
	//orientation.AX(-orientation.AX());
	//orientation.AZ(-orientation.AZ());

	Sys::I_BinaryStream *file = Sys::C_FileSystem::Instance().Open( fileName );
	Gravity::C_ImportScene* pImportScene = new Gravity::C_ImportScene;
	pImportScene->Import(file);

	Gravity::C_CompositeDesc compDesc;
	for(S32 a = 0; a< pImportScene->m_DynamicShapes.m_numElements; ++a)
	{
		Gravity::C_ImportShape* pImportShape=pImportScene->m_DynamicShapes[0];
		Gravity::C_ShapeDesc* pDesc=pImportShape->GetShapeDesc();
		compDesc.AddShape(pDesc, Maths::HMat43(pImportShape->GetImportRotation(),
			pImportShape->GetImportPosition()), pDesc->GetMass(), pFilter);
	}

	//pDesc->ApplyStaticRotation(orientation);

	//Maths::RotY rotY(Maths::Constants<Maths::HF32>::Pi());
	//Maths::HMat33 staticRotation(rotY);
	//compDesc.ApplyStaticRotation(staticRotation);
	//compDesc.RemapShape();
	
	pBody->SetExternalConstants(RacingPhysics::VehicleManager::Instance().GetGravityConstants());
	pBody->Baumgarte(0.8f);
	pBody->Slop(0.00000001f);
	
	Maths::HMat33 identity;
	identity.SetIdentity();

	Gravity::C_BodyHelper helper(&compDesc);

	//const Maths::HMat33 transRot(staticRotation.Transposed());
	//const Maths::HMat33 inertiaTensor( transRot * pImportShape->GetImportIneriaTensor() * staticRotation );
	//const Maths::HMat33 inertiaTensor( pImportShape->GetImportIneriaTensor());

	helper.CreateBody(mass, Maths::HVec3(0.f, 0.f, 0.f), orientation, compDesc.InertiaTensor(mass), pBody, pFilter);
	//pBody->SetMass(mass);
	

	SharedPhysics::C_Deactivator deactivator;
	deactivator.m_ActivateThresholdSq = -1.f;
	deactivator.m_SleepThresholdSq = -1.f;
	deactivator.m_sSleepCountdown = 15;

	pBody->SetDeactivator(deactivator);
	pScene->AddBody(pBody);
	car.SetGravityBody(pBody, Maths::HVec3(0.f, -car.Wheel(0).Radius(), 0.f));
	delete file;
}

void RacingPhysicsTestApp::InitialiseVehicles()
{
	RacingPhysics::CreateXt();
	RacingPhysics::VehicleManager::Instance().Initialise();

	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < 1; j++)
		{
			RacingPhysics::Car& car = RacingPhysics::VehicleManager::Instance().AddVehicle<RacingPhysics::Car>();
			RacingPhysics::VehicleInitialiseInfo info;
			info.filename = "DefaultCar";
			info.autoGears = true;
			car.Initialise(info);
			CreateGravityCar(car, "car.dat", 1200.f, Maths::HVec3(0.f, 0.45f, -0.1f));
			car.SetPosition(RacingPhysics::Point3R(-1302.f + i * 2, 6.f, -257.f + j * 3));
			car.CopyToGravity();
		}
	}

	RacingPhysics::PhysicsTimer::Instance().InitialiseTimers();
}

void RacingPhysicsTestApp::UpdateCamera()
{
	CyclesCameras();
	
	// New camera stuff:
	assert(m_pChaseCamera);

	float t = 1.f / 60.f;

	// Chase camera:
	Cam2::C_CamObj object;
	const RacingPhysics::Vehicle& vehicle = RacingPhysics::VehicleManager::Instance().GetVehicle(0);
	
	RacingPhysics::Axis axis = vehicle.GetAxis();
	//axis.RotateLocalY180();
		
	object.position = RacingPhysics::ConvertHVec(vehicle.GetCofG());
		
	RacingPhysics::Quaternion quat = RacingPhysics::Quaternion(axis);
	object.orient = Maths::C_Quaternion((float)quat.A().X(), (float)quat.A().Y(), (float)quat.A().Z(), quat.W());

	m_pChaseCamera->SetObject(object);

	// Set the look around angle:
	F32 angle = 0.f;
	if (!m_debugUsesInput)
	{
		F32 lookx = m_controller.GetAxis(GamePad::Axis_RX);
		F32 looky = m_controller.GetAxis(GamePad::Axis_RY);
		F32 mag = lookx * lookx + looky * looky;
		if (mag >= 0.2f * 0.2f)
			angle = atan2(lookx, looky);
	}
	m_pChaseCamera->SetLookAround(angle);
	m_pChaseCamera->Update(t);

	// Controls for Debug cameras:
	Cam2::CamInput input;

	F32 stickx = m_controller.GetAxis(GamePad::Axis_LX);
	F32 sticky = m_controller.GetAxis(GamePad::Axis_LY);
	if (m_controller.GetDigitalHeld(GamePad::Button_X))
	{
		input.val[Cam2::E_Input_FlyRoll] = stickx;
		
		if (m_controller.GetDigitalHeld(GamePad::Button_DPadDown))
			input.val[Cam2::E_Input_FlyZoom] = -1.f;
		if (m_controller.GetDigitalHeld(GamePad::Button_DPadUp))
			input.val[Cam2::E_Input_FlyZoom] = 1.f;
	}
	else
	{
		input.val[Cam2::E_Input_FlyMoveX] = stickx;
		input.val[Cam2::E_Input_FlyMoveZ] = sticky;
		input.val[Cam2::E_Input_FlyRotateX] = m_controller.GetAxis(GamePad::Axis_RX);
		input.val[Cam2::E_Input_FlyRotateY] = m_controller.GetAxis(GamePad::Axis_RY);
	}

	input.val[Cam2::E_Input_FlySlow] = m_controller.GetAnalogue(GamePad::Button_LeftTrigger);
	input.val[Cam2::E_Input_FlyFast] = m_controller.GetAnalogue(GamePad::Button_RightTrigger);

	m_DebugFeed.SetInput(input);
	m_DebugFeed.SetObject(object);
	m_DebugFeed.Update(t);
}

void RacingPhysicsTestApp::CyclesCameras()
{
	if (m_camList.Size() <= 0)
		return;

	// Alternate between cameras:
	bool back = m_controller.GetDigitalHeld(GamePad::Button_Back);
	bool leftShoulder = m_controller.GetDigitalPressed(GamePad::Button_LeftShoulder);

	if (back && leftShoulder)
	{
		m_camGroup++; // Change camera group
		m_camSel = 0;
	}

	if (!back && leftShoulder)
		m_camSel++; // Change camera selection

	// Wrap around group:
	if (m_camGroup >= m_camList.Size())
		m_camGroup = 0;

	Ut::Vector<S32>& group = m_camList[m_camGroup];
	if (group.Size() <= 0)
		return;
	
	// Wrap around group:
	if (m_camSel >= group.Size())
		m_camSel = 0;
	
	if (leftShoulder)
		DebugType = (Cam2::E_CameraType)group[m_camSel];

	if (LastType != DebugType)
	{
		switch (DebugType)
		{
			default:
				m_DebugFeed.SetDebugType(DebugType);
			break;
			case 0:
				m_DebugFeed.ShowFeed(m_pChaseCamera);
			break;
		}

		LastType = DebugType;
	}

	m_debugUsesInput = m_DebugFeed.UsesInput();
}

void DisplayDirtyDiscError() {}

XT_APPLICATION
	XT_DEPENDS_ON(RacingPhysics_Surfaces)
	XT_DEPENDS_ON(RacingPhysics_DynamicsParameters)
	XT_DEPENDS_ON(RacingPhysics_MotorbikeDynamics)
	XT_DEPENDS_ON(RacingPhysics_CarDynamics)
XT_END_APPLICATION

