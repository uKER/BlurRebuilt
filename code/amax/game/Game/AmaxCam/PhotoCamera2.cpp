
#include "stdafx.h"
#include "PhotoCamera2.h"

#include <Framework/Application/ApplicationBase.h>
#include <gravity/narrowphase/Shapes.h>
#include <gravity/sdk/CollisionPhantom.h>
#include <gravity/sdk/ShapeDescriptors.h>
#include <gravity/sharedphysics/ContactStream.h>
#include <Gravity/SharedPhysics/ContactStreamIterator.h>
#include <Render/State/RenderState.h>
#include <system/debug/I_DebugRender.h>
#include <System/Scheduler/jobqueue.h>

#include "CameraSystem.h"
#include "Game/GameControls/GameControls.h"
#include "Game/GameWorld/GameWorld.h"
#include "Helper.h"
#include "PhysicsChecks.h"
#include "SchedulerConfig.h"
#include "ui/Game/UILua.h"
#include "VehicleFeed.h"

namespace Render
{
	const Display::C_Texture2D* GetGlobalDepthStencilBuffer(const bool useAAIfAvailable = false);
}

namespace AmaxCam
{

using namespace Game::Actors;
using namespace Maths;

static Debug::Float g_MoveSpeed("Camera2.PhotoCamera2.MoveSpeed", 12.0f, Debug::Range(0.f, 20.f));
static Debug::Float g_VerticalSpeed("Camera2.PhotoCamera2.VerticalSpeed", 4.0f, Debug::Range(0.f, 10.f));
static Debug::Float g_HeightLimit("Camera2.PhotoCamera2.HeightLimit", 4.0f, Debug::Range(0.f, 20.f));
static Debug::Bool g_RenderContacts("Camera2.PhotoCamera2.RenderContacts", false);
static Debug::Int g_SolverCount("Camera2.PhotoCamera2.SolverCount", 30, Debug::Range(0, 100));
static Debug::Float g_Baumgarte("Camera2.PhotoCamera2.Baumgarte", 6.f, Debug::Range(0.f, 5.f));
static Debug::Float g_Slop("Camera2.PhotoCamera2.Slop", 0.01f, Debug::Range(0.f, 0.5f));
static Debug::Float g_FovMin("Camera2.PhotoCamera2.FovMin", 30.f, Debug::Range( 1.f,  60.f));
static Debug::Float g_FovMax("Camera2.PhotoCamera2.FovMax", 80.f, Debug::Range(60.f, 170.f));
static Debug::Float g_RangeRadius("Camera2.PhotoCamera2.RangeRadius", 10.f, Debug::Range(0.f, 100.f));
static Debug::Bool g_ShowRange("Camera2.PhotoCamera2.ShowRange", false);
static Debug::Int g_TickCount("Camera2.PhotoCamera2.TickCount", 2, Debug::Range(1, 8));
static Debug::Float g_CameraRadius("Camera2.PhotoCamera2.CameraRadius", 0.5f, Debug::Range(0.f, 1.f));

C_PhotoCamera2::C_PhotoCamera2()
{
	SetName("Photo2");
	m_PadIndex = 0;
	m_GameInput = 0;
	m_ControlsEnabled = true;
	m_InvertY = m_RightStick = false;
	m_SchedulerGroup = -1;
	m_Scene = 0;
	m_PhysicsChecks = 0;
	m_Radius = 0.f;
	m_Shape = 0;
	m_Phantom = 0;
	m_StackData = 0;
	m_ContactStream = 0;
	m_GroundHeight = 0.f;
}

C_PhotoCamera2::~C_PhotoCamera2()
{
	Exit();
}

void C_PhotoCamera2::Init()
{
	Exit();
	ReadProfileSettings();

	m_SchedulerGroup = Scheduler::CreateGroup();
	m_SchedulerPolicy = Game::C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy();

	m_Scene = GetGameWorld()->GetPhysicsScene();
	m_PhysicsChecks = new C_PhysicsChecks(m_Scene, 0);

	m_Radius = g_CameraRadius;
	Gravity::C_SphereDesc sphereDesc;
	sphereDesc.m_Radius = m_Radius;

	Collision::C_CollisionFilter filter;
	m_Shape = sphereDesc.CreateShape(&filter);
	m_Phantom = new Gravity::C_CollisionPhantom(m_Shape);
	
	m_StackData = new U8[1024 * sizeof(void*)];
	m_ContactStream = new SharedPhysics::C_ContactStream(16 * 1024);
}

void C_PhotoCamera2::CopyFromVehicleFeed(const C_VehicleFeed* vehicleFeed)
{
	// Copy the pad index from the vehicle feed:
	if (!vehicleFeed)
		return;
	
	S32 pad = vehicleFeed->PadIndex();
	// If pad is out of range, just use pad 0:
	if (pad < 0 || pad > 3)	
		pad = 0;
	SetPadIndex(pad);

	// Use the chase camera position as a starting point:
	const Cam2::C_Camera* camera = vehicleFeed->ChaseCamera();
	if (camera)
	{
		CopyFrom(*camera);
		m_RangeCentre = camera->Object().position;
	}
	
	F32 groundBelow = GroundBelow(Position(), 0);
	if (groundBelow < 0.f)
	{
		// There is no ground below so we must use the position the last time the car was on the ground:
		HVec3 lastGround = vehicleFeed->LastKnownSafeMatrix().AW();
		SetPosition(lastGround + HVec3(0.f, 0.8f, 0.f));
		if (camera)
			LookAt(camera->Object().position);
		
		m_RangeCentre = lastGround;
	}

	// Set motion blur vector:
	HVec3 veloc = vehicleFeed->VehicleVelocity();
	m_MotionBlur = veloc * 1.f / 30.f;
}

void C_PhotoCamera2::Exit()
{
	SAFE_DELETE_PTR(m_ContactStream)
	SAFE_DELETE_PTR(m_StackData)
	SAFE_DELETE_PTR(m_Phantom)
	SAFE_DELETE_PTR(m_Shape)
	SAFE_DELETE_PTR(m_PhysicsChecks)
	m_Scene = 0;
	m_SchedulerPolicy = 0;

	if (m_SchedulerGroup >= 0)
		Scheduler::DestroyGroup(m_SchedulerGroup);
	
	m_SchedulerGroup = -1;
}

static C_Quaternion LevelUpQuat(const C_Quaternion& orient)
{
	HMat33 mat(orient);
	F32 axy = mat.AX().Y();
	F32 ayy = mat.AY().Y();
	F32 roll = atan2(-axy, ayy);

	return orient * C_Quaternion(0.f, 0.f, sin(roll * -0.5f), cos(roll * -0.5f));
}

void C_PhotoCamera2::OnUpdate()
{
	PERF_EVENT("PhotoCamera2.OnUpdate");

	GetGameInput();
	GetControls();
	AutoFocusUpdate();
	
	// Let's fix the timestep so that the physics is always predictable:
	F32 period = 1.f / 30.f;
	S32 ticks = g_TickCount;

	for (S32 i = 0; i < ticks; i++)
		TimeStep(period / (F32)ticks);
}

void C_PhotoCamera2::TimeStep(F32 period)
{
	// Rotate camera:
	F32 fov = Fov();
	F32 rota = m_Input.yaw   * fov * period; // Rotate less if we are zoomed in
	F32 rotb = m_Input.pitch * fov * period;
	F32 rotc = m_Input.roll * period;

	C_Quaternion orient = Orient();
	orient = orient * C_Quaternion(0.f, 0.f, sin(rotc), cos(rotc));
	orient = orient * C_Quaternion(sin(rotb), 0.f, 0.f, cos(rotb));
	orient = C_Quaternion(0.f, sin(-rota), 0.f, cos(-rota)) * orient;
	orient.Normalise();

	if (m_Input.rollReset)
		orient = LevelUpQuat(orient);

	SetOrient(orient);

	// Zoom in and out:
	F32 zoom = m_Input.zoom * period;
	fov -= zoom * fov;

	if (m_Input.zoomReset)
		fov = DegToRad(60.f);

	fov = Max(fov, DegToRad((F32)g_FovMin));
	fov = Min(fov, DegToRad((F32)g_FovMax));
	SetFov(fov);

	// Move camera:
	HVec3 move = m_Input.move * HMat33(Orient());
	m_Velocity = move * HVec3((F32)g_MoveSpeed, 0.f, (F32)g_MoveSpeed); // Planar movement
	m_Velocity += HVec3(0.f, m_Input.up * g_VerticalSpeed, 0.f); // vertical movement
	
	UpdatePhantom();
	HeightConstraint();
	RangeConstraint();
	RenderContacts();
	ProcessContacts();
	
	// Integrate adjusted velocity:
	HVec3 newPos = Position() + m_Velocity * period;

	// Only move if there is ground below the new point:
	if (GroundBelow(newPos, 0) >= 0.f)
		SetPosition(newPos);
}

void C_PhotoCamera2::SetMotionVector(const HVec3& vector)
{
	SetPrevPosition(Position() - vector);
}

// Get the game input device:
void C_PhotoCamera2::GetGameInput()
{
	m_GameInput = 0;

	if (m_ControlsEnabled && m_PadIndex >= 0 && m_PadIndex < 4 && !CameraSystem().PadUsedForDebug(m_PadIndex))
		m_GameInput = Input::GetGameInput(m_PadIndex);
}

// Get the movement input values:
void C_PhotoCamera2::GetControls()
{
	// Start with blank input:
	m_Input = C_Input();

	if (!m_GameInput)
		return;

#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (0x0040ef50) reads the photo axes, which the keyboard reaches too
	F32 x1 = m_GameInput->GetAxisSignedValue(Game::E_Axis_PhotoMoveX);
	F32 y1 = m_GameInput->GetAxisSignedValue(Game::E_Axis_PhotoMoveZ);
	F32 x2 = m_GameInput->GetAxisSignedValue(Game::E_Axis_PhotoRotateX);
	F32 y2 = m_GameInput->GetAxisSignedValue(Game::E_Axis_PhotoRotateY);
#else
	F32 x1 = m_GameInput->GetAxisSignedValue(Framework::E_Axis_FlyMoveX);
	F32 y1 = m_GameInput->GetAxisSignedValue(Framework::E_Axis_FlyMoveZ);
	F32 x2 = m_GameInput->GetAxisSignedValue(Framework::E_Axis_FlyRotateX);
	F32 y2 = m_GameInput->GetAxisSignedValue(Framework::E_Axis_FlyRotateY);
#endif
	bool zoom = m_GameInput->GetButtonPressed(Game::E_Button_PhotoZoom);
	bool roll = m_GameInput->GetButtonPressed(Game::E_Button_PhotoRoll);

	if (roll)
	{
		m_Input.roll = x1;
	}
	else if (zoom)
	{
		m_Input.zoom = y2;
	}
	else if (m_RightStick)
	{
		m_Input.move = HVec3(x2, 0.f, y2); // Right stick to move
		m_Input.yaw = x1;
		m_Input.pitch = y1;
	}
	else
	{
		m_Input.move = HVec3(x1, 0.f, y1); // Left stick to move
		m_Input.yaw = x2;
		m_Input.pitch = y2;
	}
	
	if (m_InvertY)
		m_Input.pitch = -m_Input.pitch;

	m_Input.rollReset = m_GameInput->GetButtonPressed(Game::E_Button_PhotoRollReset);
	m_Input.zoomReset = m_GameInput->GetButtonPressed(Game::E_Button_PhotoZoomReset);
	m_Input.up = m_GameInput->GetAxisSignedValue(Game::E_Axis_PhotoMoveUp);
	
	// Saturate the up/down axis so that vertical movement moves better on PS3
	if (m_Input.up >  0.1f)
		m_Input.up =  1.0f;
	if (m_Input.up < -0.1f)
		m_Input.up = -1.0f;
}

void C_PhotoCamera2::UpdatePhantom()
{
	PERF_EVENT("PhotoCamera2.UpdatePhantom");

	if (!m_Shape || !m_Phantom || !m_Scene)
		return;

	m_Radius = g_CameraRadius;
	Collision::C_SphereShape* sphere = (Collision::C_SphereShape*)m_Phantom->GetShape();
	if (sphere)
	{
		sphere->m_Sphere.Radius(m_Radius);
		sphere->m_LocalBoundingBox.SetExtents(HVec3(m_Radius));
	}

	// Update the phantom with the new position:
	HMat43 trans;
	trans.SetIdentity();
	trans.AW(Position());
	m_Phantom->Transform(trans);

	// Get intersections of the phantom:
	SharedPhysics::C_SafeStack<true> safeStack(m_StackData, 1024);
	m_Scene->GetCollisionPhantomIntersection(m_Phantom, *m_ContactStream, safeStack, m_SchedulerPolicy, m_SchedulerGroup);
	m_Scene->CompletePhantomCollisionQuery(*m_ContactStream, m_SchedulerPolicy, m_SchedulerGroup);
	
	GetContacts();

	m_ContactStream->Reset();
}

void C_PhotoCamera2::GetContacts()
{
	PERF_EVENT("PhotoCamera2.GetContacts");
	m_Contacts.Clear();

	// Copy the contacts into our array:
	SharedPhysics::C_ContactStreamIterator iter(m_ContactStream, 0, 0.f);
	while (iter.hasNextContact())
	{
		const SharedPhysics::C_ContactStreamResult* contact = iter.nextContact();

		while (iter.hasNextPatch())
		{
			const SharedPhysics::C_ContactStreamPatch* patch = iter.nextPatch();

			while (iter.hasNextPoint())
			{
				const SharedPhysics::C_ContactStreamPoint* point = iter.nextPoint();

				C_Contact contact;
				contact.point = Load<HVec3>(&point->m_ContactPoint);
				contact.normal = -Load<HVec3>(&patch->m_Normal);
				contact.penetration = point->m_PenetrationDepth; 
				m_Contacts.PushBack(contact);
			}
		}
	}
}

void C_PhotoCamera2::HeightConstraint()
{
	// Raytrace down to find the height of the track:
	HVec3 pos = Position();
	HVec3 normal;
	F32 groundBelow = GroundBelow(pos, &normal);
	if (groundBelow >= 0.f)
	{
		// Update ground height if there is ground below:
		m_GroundHeight = pos.Y() - groundBelow;
		m_GroundNormal = normal;
	}

	HVec3 limit = pos;
	limit.Y(m_GroundHeight + g_HeightLimit);

	F32 pene = pos.Y() + m_Radius - limit.Y();
	
	if (pene >= 0.f)
	{
		C_Contact contact;
		contact.point = limit;
		contact.normal = -m_GroundNormal;
		contact.penetration = pene;
		m_Contacts.PushBack(contact);
	}
}

void C_PhotoCamera2::RangeConstraint()
{
	// Constrain to a cylinder:
	HVec3 pos = Position();
	HVec3 rel = pos - m_RangeCentre;
	rel.Y(0.f);
	
	F32 dist = Max(1e-5f, rel.Length());
	F32 pene = dist - g_RangeRadius + m_Radius;

	if (pene >= 0.f)
	{
		rel.Normalise();
		
		C_Contact contact;
		contact.point = m_RangeCentre + rel * (dist + m_Radius);
		contact.point.Y(pos.Y());
		
		contact.normal = -rel;
		contact.penetration = pene;
		m_Contacts.PushBack(contact);
	}

	if (g_ShowRange)
	{
		DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
		for (S32 i = 0; i < 16; i++)
		{
			F32 angStart = (F32) i      * Pi / 8.f;
			F32 angEnd   = (F32)(i + 1) * Pi / 8.f;
			
			HVec3 radialStart(cos(angStart), 0.f, sin(angStart));
			HVec3 radialEnd  (cos(angEnd  ), 0.f, sin(angEnd  ));
			
			HVec3 spoke1 = m_RangeCentre + radialStart * g_RangeRadius;
			HVec3 spoke2 = m_RangeCentre + radialEnd   * g_RangeRadius;

			debugRender->Draw3dLine(spoke1, spoke2, RGBA(0xffffffff));
		}
	}
}

void C_PhotoCamera2::RenderContacts()
{
	if (!g_RenderContacts)
		return;

	DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
	for (S32 i = 0; i < m_Contacts.Size(); i++)
	{
		const C_Contact& contact = m_Contacts[i];

		debugRender->Draw3dCross(contact.point, 0.1f, RGBA(0xffff0000));
		debugRender->Draw3dArrow(contact.point, contact.normal, 0.2f, 0.05f, RGBA(0xff00ff00));
	}
}

void C_PhotoCamera2::ProcessContacts()
{
	PERF_EVENT("PhotoCamera2.ProcessContacts");

	HVec3 projectedVelocity = m_Velocity;
	HVec3 constraintImpulse;

	for (S32 n = 0; n < g_SolverCount; n++)
	{
		for (S32 i = 0; i < m_Contacts.Size(); i++)
		{
			C_Contact& contact = m_Contacts[i];

			// We want the velocity in the normal direction to be slightly positive
			F32 pene = contact.penetration - g_Slop;
			
			F32 error = (-projectedVelocity | contact.normal) + pene * g_Baumgarte;
			
			contact.lambda += error;
			contact.lambda = Maths::Max(contact.lambda, 0.f);

			constraintImpulse = HVec3();
			for (S32 k = 0; k < m_Contacts.Size(); k++)
			{
				constraintImpulse += m_Contacts[k].lambda * m_Contacts[k].normal;
			}

			projectedVelocity = m_Velocity + constraintImpulse;
		}
	}
		
	m_Velocity += constraintImpulse;
}

void C_PhotoCamera2::ReadProfileSettings()
{
}

void C_PhotoCamera2::AutoFocusUpdate()
{
	if (m_GameInput && m_GameInput->GetButtonPressed(Game::E_Button_ChangeView))
		AutoFocus();
}

void C_PhotoCamera2::AutoFocus()
{
	// Get depth texture:
	const PostProcess::I_HighDynamicRange* hdr = Render::C_RenderState::Instance().HDR();
	const Display::C_Texture2D* depth = hdr ? hdr->GetDepthBufferCentreTexture() : 0;
	if (!depth)
		return;

	U8* data = hdr->GetDepthBufferCentreTexture()->GetAddress();
	if (!data)
		return;

	U32 pixel = *(U32*)data; // ddddddss : depth, stencil

	F32 depthVal = 0.f;
#ifdef PLATFORM_PS3
	// Convert 24-bit (/ 8-bit stencil) depth buffer value into 32-bit float:
	depthVal = (F32)(pixel & 0x00ffffff) / (F32)0x1000000;
#else
	depthVal = *(F32 *)&pixel;
#endif

	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	const Display::C_ViewSettings3D& viewSettings = view.m_View.GetViewSettings3D();
	
	F32 nearClip = viewSettings.GetNearClip();
	F32 farClip = viewSettings.GetFarClip();
	F32 inear = 1.f / nearClip;
	F32 ifar  = 1.f / farClip;
	F32 ival = ifar + depthVal * (inear - ifar);
	
	SetIdepth(ival);
	// Call LUA to set the Aperture value if it's 0:
	Lua::Call(UI::g_pUILua->GetState(), "PhotoMode_OnAutoFocus");
}

S32 C_PhotoCamera2::CalcAddr(S32 x, S32 y, S32 pitch)
{
	return y * pitch + (x << 2);
}

// Return the distance of ground below a point or -1 if there isn't any
F32 C_PhotoCamera2::GroundBelow(HVec3 position, HVec3* normal)
{
	F32 traceBelow = g_HeightLimit + 2.f; // Ground limit plus a bit extra to be sure
	HVec3 down(0.f, -traceBelow, 0.f);
	
	F32 trace = 1.f;
	if (m_PhysicsChecks)
		trace = m_PhysicsChecks->RayTraceNormal(position, position + down, 0.f, normal, true);
	
	if (trace >= 1.f)
		return -1.f;
	else
		return trace * traceBelow;
}

} // End of namespace
