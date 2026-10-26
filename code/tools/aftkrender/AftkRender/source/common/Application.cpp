#include "AftkRenderPch.h"
#include "System/Time/Time.h"
#include "common/Application.h"
#include "AftkRender.h"

using namespace AftkRender;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

C_Application::C_Application()
: m_pDisplayDevice(NULL)
{
	Display::C_ManagerBase::CreateErrors();
	m_pDisplayManager = new Display::C_Manager();
}

C_Application::~C_Application()
{
	Destroy();
}

bool C_Application::Create(U32 width, U32 height)
{
	assert(m_pDisplayManager);
	if (!m_pDisplayManager->Initialise())
		return false;

	m_pDisplayDevice = new Display::C_Device();
	assert(m_pDisplayDevice && "Failed to create Display::C_Device object");

	m_DeviceParams.m_Width = width;
	m_DeviceParams.m_Height = height;

	if (!m_pDisplayManager->CreateDevice(m_pDisplayDevice, m_DeviceParams))
	{
		SAFE_DELETE_PTR(m_pDisplayDevice);
		return false;
	}

	if (!m_pDisplayDevice->CreateRenderersVolatile())
		return false;

	if (!m_pDisplayDevice->CreateRenderers())
		return false;

	//Create a view context for the default swap chain
	C_ViewContext *pViewContext = new C_ViewContext();
	if (!pViewContext)
		return false;

	if (!pViewContext->Create())
		return false;

	m_ViewContextArray.PushBack(pViewContext);

	CreateAftkRenderVertexDeclarations();
	return true;
}

bool C_Application::Resize(U32 width, U32 height, U32 context)
{
	if (context == 0)
	{
		bool success = true;

		//We're resizing the device buffers
		m_pDisplayDevice->DestroyVolatile();
		m_pDisplayDevice->DestroyRenderersVolatile();

		for (S32 i=1; i<m_ViewContextArray.Size(); ++i)
			m_ViewContextArray[i]->Destroy();

		m_DeviceParams.m_Width = Maths::Max<U32>(width, 1);
		m_DeviceParams.m_Height = Maths::Max<U32>(height, 1);

		success &= m_pDisplayDevice->CreateVolatile(m_DeviceParams);
		if (!success) return success;
		success &= m_pDisplayDevice->CreateRenderersVolatile();
		if (!success) return success;

		success &= m_ViewContextArray[0]->Resize(m_DeviceParams.m_Width, m_DeviceParams.m_Height);
		for (S32 i=1; i<m_ViewContextArray.Size(); ++i)
			success &= m_ViewContextArray[i]->Rebuild();

		return success;
	}
	else
	{
		//just recreate the swap chain
		return m_ViewContextArray[context]->Resize(width, height);
	}

	return false;
}

bool C_Application::Resize(U32 width, U32 height, const C_ViewContext *pContext)
{
	for (S32 i=0; i<m_ViewContextArray.Size(); ++i)
		if (pContext == m_ViewContextArray[i])
			return Resize(width, height, i);

	return false;
}

void C_Application::Destroy()
{
	if (!m_pDisplayDevice)
		return;

	C_Renderer::GetTextureManager().Clear();
	C_Renderer::GetShaderManager().Clear();

	for (S32 i=0; i<m_ViewContextArray.Size(); ++i)
		delete m_ViewContextArray[i];

	m_ViewContextArray.Clear();

	if (m_pDisplayDevice)
	{
		m_pDisplayDevice->DestroyRenderersVolatile();
		m_pDisplayDevice->DestroyRenderers();
		
		if (m_pDisplayDevice->IsCreated())
			m_pDisplayManager->DestroyDevice(m_pDisplayDevice);

		m_pDisplayDevice = 0;
	}

	m_pDisplayManager->Shutdown();
	SAFE_DELETE_PTR(m_pDisplayManager);
}

void C_Application::Update()
{
	for (S32 i=0; i<m_ViewContextArray.Size(); ++i)
		m_ViewContextArray[i]->Update();
}

C_ViewContext* C_Application::AddViewContext(const Display::C_DeviceParams& params)
{
	C_ViewContext *pViewContext = new C_ViewContext();
	if (!pViewContext)
	{
		delete pViewContext;
		return NULL;
	}

	if (!pViewContext->Create(params))
	{
		delete pViewContext;
		return NULL;
	}

	m_ViewContextArray.PushBack(pViewContext);
	//assert(m_pDisplayDevice->GetD3DDevice()->GetNumberOfSwapChains() == m_ViewContextArray.Size());
	return pViewContext;
}

C_ViewContext* C_Application::GetViewContext(U32 index)
{
	if (index < (U32)m_ViewContextArray.Size())
		return m_ViewContextArray[index];
	else
		return 0;
}

const C_ViewContext* C_Application::GetViewContext(U32 index) const
{
	if (index < (U32)m_ViewContextArray.Size())
		return m_ViewContextArray[index];
	else
		return 0;

}

C_ViewContext* C_Application::GetViewContextFromHWND(HWND hwnd)
{
	if (hwnd == m_DeviceParams.m_hWnd)
		return m_ViewContextArray[0];

	for (S32 i=1; i<m_ViewContextArray.Size(); ++i)
		if (m_ViewContextArray[i]->PresentParams().hDeviceWindow == hwnd)
			return m_ViewContextArray[i];

	return 0;
}

const C_ViewContext* C_Application::GetViewContextFromHWND(HWND hwnd) const
{
	if (hwnd == m_DeviceParams.m_hWnd)
		return m_ViewContextArray[0];

	for (S32 i=1; i<m_ViewContextArray.Size(); ++i)

		if (m_ViewContextArray[i]->PresentParams().hDeviceWindow == hwnd)
			return m_ViewContextArray[i];

	return 0;
}

void C_Application::DestroyViewContext(AftkRender::C_ViewContext *pContext)
{
	//1st view context is reserved
	for (S32 i=1; i<m_ViewContextArray.Size(); ++i)
		if (m_ViewContextArray[i] == pContext)
		{
			delete pContext;
			m_ViewContextArray.EraseNonOrdered(i);
		}
}

bool C_Application::BeginRendering(const C_ViewContext *pContext)
{
	if (!CheckDeviceState())
		return false;

	if (m_pDisplayDevice->InScene())
		return false;

	for (S32 i=0; i<m_ViewContextArray.Size(); ++i)
	{
		if(m_ViewContextArray[i] == pContext)
		{
			m_pDisplayDevice->SetView(NULL, false);
			
			if (!m_pDisplayDevice->BeginScene())
				return false;

			m_ViewContextArray[i]->BeginRender();
		}
	}

	return true;
}

void C_Application::EndRendering(const C_ViewContext *pContext, bool present)
{
	if(!CheckDeviceState())
		return;

	if (!m_pDisplayDevice->InScene())
		return;

	for (S32 i=0; i<m_ViewContextArray.Size(); ++i)
	{
		if(m_ViewContextArray[i] == pContext)
		{
			m_pDisplayDevice->SetView(NULL, false);
			m_pDisplayDevice->EndScene();
			m_ViewContextArray[i]->EndRender(present);
		}
	}
}

bool C_Application::CheckDeviceState()
{
	if (m_pDisplayDevice->GetLost())
	{
		//destroy swapchains
		for (S32 i=0; i<m_ViewContextArray.Size(); ++i)
			m_ViewContextArray[i]->Destroy();

		//reset d3d
		m_pDisplayDevice->DestroyVolatile();
		m_pDisplayDevice->DestroyRenderersVolatile();
		if (!m_pDisplayDevice->CreateVolatile(m_DeviceParams))
		{
			LOGDEBUG(g_Log, "Failed to recreate D3D volatile data\n");
			return false;
		}
		if (!m_pDisplayDevice->CreateRenderersVolatile())
		{
			LOGDEBUG(g_Log, "Failed to recreate volatile renderer data\n");
			return false;
		}

		//recreate swapchains
		m_ViewContextArray[0]->Create();
		for (S32 i=1; i<m_ViewContextArray.Size(); ++i)
			m_ViewContextArray[i]->Rebuild();
	}
	return true;
}