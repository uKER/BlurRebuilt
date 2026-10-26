#include <nexus_app/stdafx.h>
#include <nexus_app/NexusSystem.h>
#include <nexus_app/DisplayManager.h>
#include <nexus_app/ViewportParent.h>

using namespace Nexus;

C_ViewportDisplay::C_ViewportDisplay()
{
	m_pCamera = 0;
	m_pOverlay2D = 0;
	m_isCreated = false;
	m_pParentCWnd = 0;
	m_pSwapChain = 0;
	m_pDepthSurf = 0;
	m_pRenderSurf =0;
}

C_ViewportDisplay::~C_ViewportDisplay()
{
	Destroy();
}

bool C_ViewportDisplay::Create(CWnd* pParentCWnd)
{
	if (!pParentCWnd)
	{
		assert(pParentCWnd);
		return false;
	}
	m_pParentCWnd = pParentCWnd;

	m_pOverlay2D = new C_Overlay2D(*this);
	if (!m_pOverlay2D)
		return false;

	m_pCamera = 
		new NexusRender::TargetCamera(g_pViewportParent->GetDisplayManagerPtr()->GetD3DWindowPtr());
	if (!m_pCamera)
		return false;

	PostResetDevice();

	m_isCreated = true;
	return true;
}

void C_ViewportDisplay::Destroy()
{
	if (m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = 0;
	}

	if (m_pOverlay2D)
	{
		delete m_pOverlay2D;
		m_pOverlay2D = 0;
	}

	PreResetDevice();

	m_isCreated = false;
}

bool C_ViewportDisplay::Draw()
{
	IDirect3DDevice9* pD3DDevice = 
		g_pViewportParent->GetDisplayManagerPtr()->GetD3DWindowPtr()->DevicePtr();

	HRESULT hr = pD3DDevice->TestCooperativeLevel();
	if ((hr == D3DERR_DEVICELOST) || (hr == D3DERR_DEVICENOTRESET)) 
	{ 
		Tu::Status resetStatus = g_pViewportParent->GetDisplayManagerPtr()->ResetDevice();
		if (!resetStatus)
		{
			//Return upon critical failure.
			return resetStatus;
		}
	} 
	else if(FAILED(hr)) 
		//Try again next time
		return true;

	if (!m_pSwapChain)
		return true;


	pD3DDevice->SetRenderTarget(0, m_pRenderSurf);
	pD3DDevice->SetDepthStencilSurface(m_pDepthSurf);

	RECT rect;
	m_pParentCWnd->GetClientRect(&rect);
	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = rect.right - rect.left;
	vp.Height = rect.bottom - rect.top;
	vp.MinZ = 0;
	vp.MaxZ = 1;
	pD3DDevice->SetViewport(&vp);

	pD3DDevice->Clear(
		0, 
		NULL, 
		D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
		0x87878787, 
		1.0f, 
		0);
	pD3DDevice->BeginScene();

	if (g_pNexusSystem->GetCorePtr()->TryLock())
	{
		if (
			m_pCamera &&
			g_pNexusSystem->GetCorePtr()->IsSceneLoaded() &&
			g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())
		{
			m_pCamera->OnUpdateDevice();
			g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->SetCameraPtr(m_pCamera);

			if (!g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnDraw())
			{
				g_pNexusSystem->GetCorePtr()->Unlock();
				return false;
			}

			if (!m_pOverlay2D->Draw())
			{
				g_pNexusSystem->GetCorePtr()->Unlock();
				return false;
			}
		}
		g_pNexusSystem->GetCorePtr()->Unlock();
	}

	pD3DDevice->EndScene();
	m_pSwapChain->Present(NULL, NULL, NULL, NULL, NULL);
	return true;
}

void C_ViewportDisplay::CentreCamera()
{
	if (m_pCamera == 0)
		return;

	m_pCamera->SetOrigin(Maths::HVec3(0,0,0));
	m_pCamera->SetRotation(0,0,0);
}

void C_ViewportDisplay::OnCameraZoom()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())
		g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnCameraZoom();
}
void C_ViewportDisplay::FrameAll()
{
	if (!m_pCamera || !g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())
		return;
	
	m_pCamera->Frame(
		g_pNexusSystem->GetCorePtr()->GetAftkScenePtr()->GetEvaluatedBounds(
			&g_pNexusSystem->GetCorePtr()->GetBoundsFilter()));

	if (!g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())
		g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnCameraZoom();
}

void C_ViewportDisplay::FrameSelected()
{
	if (!m_pCamera || !g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())
		return;

	if (g_pNexusSystem->GetCorePtr()->GetNumSelected() == 0)
	{
		FrameAll();
		return;
	}

	Aftk::C_Bounds bounds;

	Nexus::AftkSceneNodeMap& selected = g_pNexusSystem->GetCorePtr()->GetSelected();
	for (
		Nexus::AftkSceneNodeMap::Iterator it = selected.Begin();
		it != selected.End(); 
		++it)
		bounds = bounds + it->GetKey()->GetWorldBounds();

	m_pCamera->Frame(bounds);

	g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnCameraZoom();
}

void C_ViewportDisplay::Frame(const Aftk::C_Bounds& bounds)
{
	if (!m_pCamera || !g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())
		return;
	
	m_pCamera->Frame(bounds);
	g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnCameraZoom();
}

NexusRender::Camera* C_ViewportDisplay::GetCameraPtr()
{
	return m_pCamera;
}

bool C_ViewportDisplay::IsCreated() const
{
	return m_isCreated;
}

void C_ViewportDisplay::Resize(U32 width, U32 height)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())
	{
		PreResetDevice();
		//Make sure the renderer correctly handles the D3D device being reset
		
		g_pViewportParent->GetDisplayManagerPtr()->GetD3DWindowPtr()->Resize(width, height);
		//Make sure the renderer correctly handles the D3D device being reset
		
		PostResetDevice();

		if (m_pCamera != 0)
			m_pCamera->OnResize();
	}
}

void C_ViewportDisplay::PreResetDevice()
{
	if (!g_pViewportParent)
		return;
	IDirect3DDevice9* pD3DDevice = 
		g_pViewportParent->GetDisplayManagerPtr()->GetD3DWindowPtr()->DevicePtr();
	if (!pD3DDevice)
		return;

	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = 0;

		m_pDepthSurf->Release();
		m_pDepthSurf = 0;

		m_pRenderSurf->Release();
		m_pRenderSurf = 0;
	}

	g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->PreResetDevice();
}

void C_ViewportDisplay::PostResetDevice()
{
	if (!g_pViewportParent)
		return;
	IDirect3DDevice9* pD3DDevice = 
		g_pViewportParent->GetDisplayManagerPtr()->GetD3DWindowPtr()->DevicePtr();
	if (!pD3DDevice)
		return;

	g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->PostResetDevice(0);

	RECT rect;
	m_pParentCWnd->GetClientRect(&rect);

	D3DPRESENT_PARAMETERS d3dPresentParams;
	ZeroMemory(&d3dPresentParams, sizeof(D3DPRESENT_PARAMETERS));
	d3dPresentParams.Windowed				= true;
	d3dPresentParams.SwapEffect				= D3DSWAPEFFECT_COPY;
	d3dPresentParams.hDeviceWindow			= m_pParentCWnd->m_hWnd;
	d3dPresentParams.EnableAutoDepthStencil = true;
	d3dPresentParams.AutoDepthStencilFormat = D3DFMT_D24S8;

	d3dPresentParams.BackBufferWidth		= rect.right - rect.left;
	d3dPresentParams.BackBufferHeight		= rect.bottom - rect.top;
	d3dPresentParams.BackBufferFormat		= D3DFMT_A8R8G8B8;
	d3dPresentParams.MultiSampleType		= D3DMULTISAMPLE_NONE;
	d3dPresentParams.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;
	pD3DDevice->CreateAdditionalSwapChain(&d3dPresentParams, &m_pSwapChain);
	if (!m_pSwapChain)
	{
		//Under what circumstances can this happen? I've seen at least one case where this has occurred.
		return;
	}

	m_pSwapChain->GetBackBuffer(
		0,
		D3DBACKBUFFER_TYPE_MONO,
		&m_pRenderSurf);

	pD3DDevice->CreateDepthStencilSurface(
		d3dPresentParams.BackBufferWidth,
		d3dPresentParams.BackBufferHeight,
		D3DFMT_D24X8, 
		D3DMULTISAMPLE_NONE, 
		0,
		false,
		&m_pDepthSurf, 
		0);

	if (m_pCamera)
		m_pCamera->OnResize();
}
