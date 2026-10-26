#include "AftkRenderPch.h"

#include "camera/Camera.h"
#include "common/ViewContext.h"

using namespace AftkRender;

C_ViewContext::C_ViewContext()
: m_pSwapChain(NULL)
, m_pColourSurface(0)
, m_pDepthStencilSurface(0)
, m_ImplicitSwapChain(false)
, m_DepthStencilFormat(D3DFMT_UNKNOWN)
{
	m_View = AddView(C_Camera(), Maths::C_Rect(Maths::HVec2(0.f,0.f), Maths::HVec2(1.f,1.f)));
}

C_ViewContext::~C_ViewContext()
{
	Destroy();
	delete m_pColourSurface;
	delete m_pDepthStencilSurface;
	m_pColourSurface = 0;
	m_pDepthStencilSurface = 0;
}

bool C_ViewContext::Create()
{
	HRESULT hr = Display::Device()->m_pd3dDevice->GetSwapChain(0, &m_pSwapChain);
	assert(SUCCEEDED(hr) && "Failed to get the implicit swap chain");
	m_ImplicitSwapChain = true;

	//Display::Device()->SetPresentParams(m_PresentParams);
	m_View.camera.SetAspectRatio((F32)m_PresentParams.BackBufferWidth / (F32)m_PresentParams.BackBufferHeight);
	return true;
}

bool C_ViewContext::Create(const Display::C_DeviceParams &deviceParams)
{
	HRESULT hr;

	ZeroMemory(&m_PresentParams, sizeof(D3DPRESENT_PARAMETERS));
	m_PresentParams.BackBufferWidth				= deviceParams.m_Width;
	m_PresentParams.BackBufferHeight			= deviceParams.m_Height;
	m_PresentParams.BackBufferFormat			= Display::D3D9Format(deviceParams.m_ColourFormat);
	m_PresentParams.BackBufferCount				= 2;
	m_PresentParams.MultiSampleType				= Display::D3D9Multisample(deviceParams.m_Multisample);
	m_PresentParams.MultiSampleQuality			= 0;
	m_PresentParams.SwapEffect					= D3DSWAPEFFECT_DISCARD;
	m_PresentParams.hDeviceWindow				= deviceParams.m_hWnd;
	m_PresentParams.Windowed					= deviceParams.m_Windowed;
	m_PresentParams.EnableAutoDepthStencil		= TRUE;
	m_PresentParams.AutoDepthStencilFormat		= Display::D3D9Format(deviceParams.m_DepthStencilFormat);
	m_PresentParams.Flags						= 0;
	m_PresentParams.FullScreen_RefreshRateInHz	= 0;
	m_PresentParams.PresentationInterval		= D3DPRESENT_INTERVAL_ONE;

	m_DepthStencilFormat = Display::D3D9Format(deviceParams.m_DepthStencilFormat);
	hr = Display::Device()->m_pd3dDevice->CreateAdditionalSwapChain(&m_PresentParams, &m_pSwapChain);
	if (FAILED(hr))
		return false;

	m_pColourSurface = new Display::C_Surface();
/*	if (!Display::Device()->CreateColourSurface(
			deviceParams.m_Width, 
			deviceParams.m_Height, 
			Display::E_Usage_Colour, 
			deviceParams.m_ColourFormat, 
			deviceParams.m_Multisample, 
			0, 
			m_pColourSurface))
		return false;*/

	hr = m_pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_pColourSurface->m_pd3dSurface);
	if (SUCCEEDED(hr))
	{
		D3DSURFACE_DESC desc;
		hr = m_pColourSurface->m_pd3dSurface->GetDesc(&desc);
	}

	m_pDepthStencilSurface = new Display::C_Surface();
	if (!Display::Device()->CreateDepthStencilSurface(
			deviceParams.m_Width, 
			deviceParams.m_Height, 
			Display::E_Usage_DepthStencil, 
			deviceParams.m_DepthStencilFormat, 
			deviceParams.m_Multisample, 
			0, 
			m_pDepthStencilSurface))
		return false;

	m_ImplicitSwapChain = false;

	SetupView(m_View);
	m_View.camera.SetAspectRatio((F32)m_PresentParams.BackBufferWidth / (F32)m_PresentParams.BackBufferHeight);
	return true;
}

bool C_ViewContext::Rebuild()
{
	return Resize(m_PresentParams.BackBufferWidth, m_PresentParams.BackBufferHeight); 
}

bool C_ViewContext::Resize(U32 width, U32 height)
{
	m_PresentParams.BackBufferWidth = Maths::Max<U32>(width, 1);
	m_PresentParams.BackBufferHeight = Maths::Max<U32>(height, 1);

	if (!m_ImplicitSwapChain)
	{
		if (m_pSwapChain)
		{
			const ULONG refCount = m_pSwapChain->Release();
			assert(refCount == 0);

			Display::Device()->Destroy(m_pColourSurface);
			Display::Device()->Destroy(m_pDepthStencilSurface);
		}

		HRESULT hr = Display::Device()->m_pd3dDevice->CreateAdditionalSwapChain(&m_PresentParams, &m_pSwapChain);
		if (FAILED(hr))
			return false;
/*
		if (!Display::Device()->CreateColourSurface(
				m_PresentParams.BackBufferWidth, 
				m_PresentParams.BackBufferHeight, 
				Display::E_Usage_Colour, 
				m_PresentParams.BackBufferFormat, 
				m_PresentParams.MultiSampleType,
				0, 
				m_pColourSurface))
			return false;
*/
		hr = m_pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_pColourSurface->m_pd3dSurface);
		if (SUCCEEDED(hr))
		{
			D3DSURFACE_DESC desc;
			hr = m_pColourSurface->m_pd3dSurface->GetDesc(&desc);
		}

		if (!Display::Device()->CreateDepthStencilSurface(
				m_PresentParams.BackBufferWidth,
				m_PresentParams.BackBufferHeight,
				Display::E_Usage_DepthStencil,
				m_DepthStencilFormat,
				m_PresentParams.MultiSampleType,
				0,
				m_pDepthStencilSurface))
			return false;

	}
	m_View.camera.SetAspectRatio((F32)m_PresentParams.BackBufferWidth / (F32)m_PresentParams.BackBufferHeight);
	return true;
}

void C_ViewContext::Destroy()
{
	if (m_pSwapChain)
	{
		ULONG refCount = m_pSwapChain->Release();
		if (!m_ImplicitSwapChain)
			assert(refCount == 0);
	}

	if (m_pColourSurface)
		Display::Device()->Destroy(m_pColourSurface);

	if (m_pDepthStencilSurface)
		Display::Device()->Destroy(m_pDepthStencilSurface);

	m_pSwapChain = 0;
	m_ImplicitSwapChain = false;
}

void C_ViewContext::Update()
{
	UpdateTimer();
}


void C_ViewContext::BeginRender() const
{
	if (!m_ImplicitSwapChain)
	{
		assert(m_pSwapChain);
		Display::Device()->SetColourSurface(m_pColourSurface);
		Display::Device()->SetDepthStencilSurface(m_pDepthStencilSurface);
	}
	else
	{
		Display::Device()->SetColourDepthStencilSurface();
	}
	SetupView(m_View);
}

void C_ViewContext::EndRender(bool present) const
{
	if(Display::Device()->GetLost() || !present)
		return;

	if (!m_ImplicitSwapChain)
	{
		assert(m_pSwapChain);
		HRESULT hr = m_pSwapChain->Present(NULL, NULL, NULL, NULL, 0);
		//assert(SUCCEEDED(hr));
	}
	else
		Display::Device()->Present();
}

C_ViewContext::View& C_ViewContext::GetView()
{
	return m_View;
}

const C_ViewContext::View& C_ViewContext::GetView() const
{
	return m_View;
}

const D3DPRESENT_PARAMETERS& C_ViewContext::PresentParams() const
{
	return m_PresentParams;
}

void C_ViewContext::UpdateTimer()
{
	static Time::C_Time oldTime = Time::GetPerformanceCounter();
	const Time::C_Time newTime = Time::GetPerformanceCounter();
	m_Timer.frameTime = (F32)(newTime - oldTime) / (F32)SYS_USEC_PER_SEC;
	oldTime = newTime;
	
	// update frame rate 4 times per second
	m_Timer.cycle += m_Timer.frameTime;
	if(m_Timer.cycle > 0.25f)
	{
		m_Timer.frameRate = m_Timer.frameCount / m_Timer.cycle;
		m_Timer.frameCount = 0.f;
		m_Timer.cycle = 0.f;
	}
	else
		m_Timer.frameCount += 1.f;

	// clamp frame time
	if (m_Timer.frameTime > 1.f/30.f)
		m_Timer.frameTime = 1.f/30.f;

	// increment total elapsed time
	if(!m_Timer.paused)
		m_Timer.elapsedTime += m_Timer.frameTime;
}

C_ViewContext::View C_ViewContext::AddView(const C_Camera& camera, const Maths::C_Rect& windowCoords)
{
	C_ViewContext::View v;
	v.camera = camera;
	v.windowCoords = windowCoords;
	v.viewIndex = Display::Device()->AddView(Display::E_ViewType_2D | Display::E_ViewType_3D, 0);

	return v;
}

void C_ViewContext::SetupView(const C_ViewContext::View& view) const
{
	assert(view.viewIndex!=-1 && "View has not been added to the device");

	Display::C_View* pDisplayView = Display::Device()->GetView(view.viewIndex);
	assert(pDisplayView && "Failed to get view from the device");

	Maths::C_Rect rect(Maths::HVec2(0.f, 0.f), Maths::HVec2(m_PresentParams.BackBufferWidth, m_PresentParams.BackBufferHeight));

	pDisplayView->m_ViewSettings3D.m_ViewMatrixPrev	= pDisplayView->m_ViewSettings3D.m_ViewMatrix;
	pDisplayView->m_ViewSettings3D.m_ProjMatrixPrev	= pDisplayView->m_ViewSettings3D.m_ProjMatrix;

	pDisplayView->m_ViewSettings3D.m_CameraToWorldPrev	= pDisplayView->m_ViewSettings3D.m_CameraToWorld;
	pDisplayView->m_ViewSettings3D.m_CameraToDevicePrev	= pDisplayView->m_ViewSettings3D.m_CameraToDevice;
	pDisplayView->m_ViewSettings3D.m_WorldToCameraPrev	= pDisplayView->m_ViewSettings3D.m_WorldToCamera;
	pDisplayView->m_ViewSettings3D.m_WorldToDevicePrev	= pDisplayView->m_ViewSettings3D.m_WorldToDevice;

	Display::C_ViewSetup viewSetup = ConvertToViewSetup(view.windowCoords, view.camera);
	viewSetup.m_WindowCoords.SetBounds(	rect.Max() * Maths::HVec2(viewSetup.m_WindowCoords.Min().X(), viewSetup.m_WindowCoords.Min().Y()),
										rect.Max() * Maths::HVec2(viewSetup.m_WindowCoords.Max().X(), viewSetup.m_WindowCoords.Max().Y()));

	viewSetup.m_ViewSetup2D.m_ViewExtents.SetBounds(0,0,(F32)m_PresentParams.BackBufferWidth, (F32)m_PresentParams.BackBufferHeight);

	pDisplayView->Setup(viewSetup, m_PresentParams.BackBufferWidth, m_PresentParams.BackBufferHeight);

	// Reset the viewport - we don't want non-uniform scaling relative to a fixed 1024x768 device (C_ViewSetup::GetDeviceCoords())
	{
		Maths::C_Transform2d deviceTransformInv(Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(m_PresentParams.BackBufferWidth, m_PresentParams.BackBufferHeight)));
		deviceTransformInv.Invert();

		Maths::C_Transform2d windowTransform(viewSetup.m_WindowCoords);
		Maths::C_Transform2d viewportTransform(deviceTransformInv * windowTransform);

		pDisplayView->m_Viewport.m_X		= U32(0.5f + F32(m_PresentParams.BackBufferWidth) * viewportTransform.Offset().X());
		pDisplayView->m_Viewport.m_W		= U32(0.5f + F32(m_PresentParams.BackBufferWidth) * viewportTransform.Scale().X());
		pDisplayView->m_Viewport.m_Y		= U32(0.5f + F32(m_PresentParams.BackBufferHeight) * viewportTransform.Offset().Y());
		pDisplayView->m_Viewport.m_H		= U32(0.5f + F32(m_PresentParams.BackBufferHeight) * viewportTransform.Scale().Y());
		pDisplayView->m_Viewport.m_MinZ	= 0.0f;
		pDisplayView->m_Viewport.m_MaxZ	= 1.0f;
	}

	//set the view now instead of waiting for the next call to BeginScene() as we may need to render to multiple swapchains per frame.
	Display::Device()->SetView(pDisplayView, true);
}