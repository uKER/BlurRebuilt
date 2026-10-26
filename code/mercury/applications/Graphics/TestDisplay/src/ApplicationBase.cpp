#include "StdAfx.h"
#include "ApplicationBase.h"
#include "CoreSystems.h"
#include "Sample.h"
#include <Display2/Utils/Utils.h>

C_ApplicationBaseParams::C_ApplicationBaseParams()
{
	m_BackBufferWidth	= 1280;
	m_BackBufferHeight	= 720;
}

C_ApplicationBase::C_ApplicationBase()
{
	m_pSample = NULL;
}

C_ApplicationBase::~C_ApplicationBase()
{
}

void C_ApplicationBase::SetSample(C_Sample* pSample)
{
	m_pSample = pSample;
}

bool C_ApplicationBase::Create()
{
	if(!m_InputManager.Create())
		return false;

	if(!m_DeviceManager.Initialise())
		return false;

	#if PLATFORM_PS3
	CellGcmConfig gcmConfig;
	cellGcmGetConfiguration(&gcmConfig);
	m_DeviceParams.m_VideoMemorySize = gcmConfig.localSize;
	#endif

	if(!m_DeviceManager.CreateDeviceIndirect(&m_Device, m_DeviceParams))
		return false;

	if(!m_DebugRender.m_FontRender.Create())
		return false;
	if(!m_DebugRender.m_FontRender.CreateVolatile())
		return false;

	if(!m_DebugRender.m_BasicRender.Create())
		return false;
	if(!m_DebugRender.m_BasicRender.CreateVolatile())
		return false;

	return true;
}

void C_ApplicationBase::Destroy()
{
	m_DebugRender.m_BasicRender.DestroyVolatile();
	m_DebugRender.m_BasicRender.Destroy();

	m_DebugRender.m_FontRender.DestroyVolatile();
	m_DebugRender.m_FontRender.Destroy();

	m_DeviceManager.DestroyDeviceIndirect(&m_Device);

	m_DeviceManager.Shutdown();

	m_InputManager.Destroy();
}

void C_ApplicationBase::Update()
{
	C_CoreSystems::Update();

	m_Timer.Update();

	m_InputManager.Update();

	Display2::RefreshShaders();

	if(m_pSample)
		m_pSample->Update();
}

void C_ApplicationBase::Render()
{
	if(!m_Device.BeginScene())
		return;

	m_Device.m_Display2.GetContext().m_pState->SetDefaultStates();
	m_Device.m_Display2.GetContextNoCheck().m_pState->SetDefaultStates();

	m_Device.SetColourDepthStencilSurface();
	m_Device.Clear(Display::E_Clear_Colour|Display::E_Clear_Depth, Maths::RGBA(0, 32, 0, 255));

	m_ViewSetup.m_ViewSetup3D.m_AspectRatio		= m_Camera.m_AspectRatio;
	m_ViewSetup.m_ViewSetup3D.m_FieldOfView		= m_Camera.m_FieldOfView;
	m_ViewSetup.m_ViewSetup3D.m_NearClip		= m_Camera.m_NearClip;
	m_ViewSetup.m_ViewSetup3D.m_FarClip			= m_Camera.m_FarClip;

	Display::MatrixLookAt(m_ViewSetup.m_ViewSetup3D.m_CameraToWorld, m_Camera.m_Pos, m_Camera.GetAt(), m_Camera.m_Up);
	
	m_Device.SetupView(&m_View, m_ViewSetup);
	m_Device.SetView(&m_View);

	if(m_pSample)
		m_pSample->Render();

	m_Device.SetDepthWriteEnable(Display::E_Bool_False);
	
	S32 size = 12;
	Maths::RGBA colour = Maths::RGBA(50, 50, 50, 255);

	for (S32 x = -size; x <= size; x++)
		m_DebugRender.m_BasicRender.DrawColoured3DLine((F32)x, 0.f, (F32)-size, (F32)x, 0.f, (F32)size, colour);

	for (S32 z = -size; z <= size; z++)
		m_DebugRender.m_BasicRender.DrawColoured3DLine((F32)-size, 0.f, (F32)z, (F32)size, 0.f, (F32)z, colour);

	m_Device.SetDepthWriteEnable(Display::E_Bool_True);

	m_DebugRender.Draw();

	static U32 frame = 0;
	static char temp[1024];
	sprintf_s(temp, sizeof(temp), "Frame %i", frame);
	m_DebugRender.m_FontRender.Draw(temp, 50.0f, 768-50.0f, 20.0f, 20.0f, Maths::RGBA(255, 255, 255, 255));
	frame++;

	m_Device.SetView(NULL);

	m_Device.EndScene();

	#if PLATFORM_PS3
	Display::RingBuffer()->EndFrame();
	#endif

	m_Device.Present();
}
