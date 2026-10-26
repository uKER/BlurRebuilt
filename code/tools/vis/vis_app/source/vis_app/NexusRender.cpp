#include <vis_app/stdafx.h>

//Nexus includes
#include <vis_app/NexusSystem.h>
#include <vis_app/NexusRender.h>

#include "../../resource.h"
#include "..\..\include\vis_app\NexusProgressDialog.h"

#include "..\..\include\vis_app\mainfrm.h"

using namespace Nexus;

NexusRender::NexusRender()
{
	m_isCreated = false;
	m_pISceneRenderer = 0;
	m_pNexusSystem = 0;
	m_pCamera = 0;
	m_globalShadingMode = NEXUS_SHADING_TYPE_SMOOTH;
	memset(&m_timerFrequency, 0, sizeof(LARGE_INTEGER));
	m_startTime.QuadPart = 0;
	m_animationPaused = true;

	g_pNexusRender = this;
}

NexusRender::~NexusRender()
{
}

Tu::Status NexusRender::Create(CView* pView, NexusSystem* pNexusSystem)
{
	if (m_isCreated) 
	{
		return Tu::Status::SUCCESS;
	}
	m_pView = pView;

	if (!QueryPerformanceFrequency(&m_timerFrequency))
	{
		return Tu::Status(Tu::Status::FAILURE, "Could not query performance counter.");
	}

	Tu::Status status;
	m_d3dWindow.AttachToWindow(0, m_pView->m_hWnd);
	m_pNexusSystem = pNexusSystem;
	
	// set default renderer (even when we haven't loaded a scene yet)
	// (we're doing this to make the grid appear)
	SRD::SR_D3D_TEXTURE_MANAGER.Create(m_d3dWindow.D3DDevicePtr());
//	SRD::SR_D3D_SHADER_MANAGER.Create(m_d3dWindow.D3DDevicePtr());

	SRD::GSceneRenderer* pGSR = new SRD::GSceneRenderer();
	//Connect the renderer with the rendering settings stored within this class.
	pGSR->SetSettingsPtr(&m_settings);

	//Attach output window
	//[HACK] OUTPUT WINDOW DISABLED
	//pGSR->SetOutputNotificationCallback(g_pMainFrame->OutputLogText);

	m_pISceneRenderer = (SRD::GSceneRenderer*)pGSR;
	if (!m_pISceneRenderer->Create(
		&m_d3dWindow, 
		pNexusSystem->GetScenePtr(),
		&pNexusSystem->GetSceneBounds()))
	{
		return Tu::Status::FAILURE;
	}

	m_pCamera = new SRD::TargetCamera(&m_d3dWindow);

	m_pCamera->Frame(pNexusSystem->GetSceneBounds());

	m_isCreated = true;

	m_time = 0;

	return Tu::Status::SUCCESS;
}

void NexusRender::Destroy()
{
	if (m_pISceneRenderer)
	{
		//Disable text output
		m_pISceneRenderer->SetOutputNotificationCallback(0);
	}

	m_pNexusSystem = 0;

	if (m_pISceneRenderer)
	{
		m_pISceneRenderer->Destroy();
		delete m_pISceneRenderer;
		m_pISceneRenderer = 0;
	}

	if (m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = 0;
	}

	SRD::SR_D3D_TEXTURE_MANAGER.Destroy();
//	SRD::SR_D3D_SHADER_MANAGER.Destroy();

	m_d3dWindow.Destroy();

	m_isCreated = false;
}

bool NexusRender::IsCreated() const
{
	return m_isCreated;
}

void NexusRender::SetViewportRectangle(const RECT& rect)
{
	m_viewportRect = rect;
}

Tu::Status LoadTexturesTask(void* pThis, void* pArgument)
{
	NexusProgressDialog* pThisDialog = (NexusProgressDialog*)pThis;

	pThisDialog->SetTitle("Loading textures...");

	return Nexus::g_pNexusRender->ISceneRendererPtr()->OnLoadTextures((SRD::IProgress*)pThisDialog);
}

Tu::Status NexusRender::OnLoadTextures()
{
	if (!m_pISceneRenderer)
	{
		return Tu::Status::FAILURE;
	}

	//If we've enabled texturing and we've not yet loaded any textures, then let's do so.
	if (!m_pISceneRenderer->TexturesLoaded())
	{
		NexusProgressDialog dlg;
		dlg.SetTaskCallbackProc(LoadTexturesTask);
		dlg.DoModal();
		dlg.WaitForThreadToFinish();
	}

	return Tu::Status::SUCCESS;
}

void NexusRender::SetGlobalShadingMode(NexusShadingTypeEnum state)
{
	m_globalShadingMode = state;
}

NexusShadingTypeEnum NexusRender::GlobalShadingMode() const
{
	return m_globalShadingMode;
}

//TODO - MAKE THIS USE SIZE START/STOP EVENTS
void NexusRender::Resize(U32 width, U32 height)
{
	if (m_pISceneRenderer)
	{
		//Make sure the renderer correctly handles the D3D device being reset
		m_pISceneRenderer->OnPreResetDevice();
	}
	m_d3dWindow.Resize(width, height);
	if (m_pISceneRenderer)
	{
		//Make sure the renderer correctly handles the D3D device being reset
		m_pISceneRenderer->OnPostResetDevice(0);
	}

	if (m_pCamera != 0)
	{
		m_pCamera->OnResize();
	}
/*
//#ifdef DEFAULT_POOL
	if (m_pISceneRenderer)
	{
		OnUpdateScene();
	}
//#endif
*/
}

void NexusRender::Clear()
{
	if (m_pISceneRenderer)
	{
		m_pISceneRenderer->Clear();
	}

	SRD::SR_D3D_TEXTURE_MANAGER.Clear();
//	SRD::SR_D3D_SHADER_MANAGER.Clear();
}

Tu::Status RebuildTask(void* pThis, void* pArgument)
{
	NexusProgressDialog* pThisDialog = (NexusProgressDialog*)pThis;

	pThisDialog->SetTitle("Rebuilding renderer...");

	return Nexus::g_pNexusRender->ISceneRendererPtr()->OnUpdateScene((SRD::IProgress*)pThisDialog);
}

Tu::Status NexusRender::OnUpdateScene()
{
	Tu::Status status;

	NexusProgressDialog dlg;
	dlg.SetTaskCallbackProc(RebuildTask);
	dlg.DoModal();
	dlg.WaitForThreadToFinish();

	status = dlg.TaskResult();
	if (!status) return status;

//Uncomment and swap with above to avoid progress bar for rebuilding scene renderer.
//	status = Nexus::g_pNexusRender->ISceneRendererPtr()->OnUpdateScene(0);
//	if (!status) return status;

	const Aftk::C_Scene* pAftkScene = m_pNexusSystem->GetScenePtr();
	if (pAftkScene)
	{
		m_time = pAftkScene->GetStartTime();
	}

	FrameAll();

	return Tu::Status::SUCCESS;
}

Tu::Status ReloadDeviceTask(void* pThis, void* pArgument)
{
	NexusProgressDialog* pThisDialog = (NexusProgressDialog*)pThis;

	pThisDialog->SetTitle("Reloading graphics data...");

	return Nexus::g_pNexusRender->ISceneRendererPtr()->OnPostResetDevice((SRD::IProgress*)pThisDialog);
}

Tu::Status NexusRender::OnResetDevice()
{
	HRESULT hr;
	// Focus lost
	if (!m_pISceneRenderer->OnPreResetDevice())
	{
		//Try again next time
		return Tu::Status::SUCCESS;
	}
	hr = m_d3dWindow.HandleLostDevice();

	if (hr == -2005532292)
	{
		//Out of video memory.
		return Tu::Status(Tu::Status::FAILURE, "Out of video memory.");
	}
	else if (FAILED(hr))
	{
		return Tu::Status(Tu::Status::FAILURE, "Unknown D3D error.");
	}
	else
	{
		//Ensure camera zoom level is accurate.
		m_pCamera->OnResize();
	}

	NexusProgressDialog dlg;
	dlg.SetTaskCallbackProc(ReloadDeviceTask);
	dlg.DoModal();
	dlg.WaitForThreadToFinish();

	return dlg.TaskResult();
}

extern bool gbDisablePainting;

Tu::Status NexusRender::Draw()
{
	if (gbDisablePainting)
		return Tu::Status::SUCCESS;


	if (!m_isCreated)
	{
		return Tu::Status::SUCCESS;
	}
	if (!m_pNexusSystem)
	{
		return Tu::Status::SUCCESS;
	}

	m_pISceneRenderer->SetTime(m_time);

	HRESULT hr = m_d3dWindow.D3DDevicePtr()->TestCooperativeLevel();

	if ((hr == D3DERR_DEVICELOST) || (hr == D3DERR_DEVICENOTRESET)) 
	{ 
		Tu::Status resetStatus = OnResetDevice();
		if (!resetStatus)
		{
			//Return upon critical failure.
			return resetStatus;
		}
	} 
	else if(FAILED(hr)) 
	{
		//Try again next time
		return Tu::Status::SUCCESS;
	}

	m_d3dWindow.D3DDevicePtr()->Clear(
		0, 
		NULL, 
		D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
		0x87878787, 
		1.0f, 
		0);

	m_d3dWindow.D3DDevicePtr()->BeginScene();
	if (!Draw3D())
	{
		m_d3dWindow.D3DDevicePtr()->EndScene();
	}
	if (!Draw2D())
	{
		m_d3dWindow.D3DDevicePtr()->EndScene();
	}

    m_d3dWindow.D3DDevicePtr()->EndScene();

	m_d3dWindow.D3DDevicePtr()->Present(NULL, &m_viewportRect, NULL, NULL);

	if (m_startTime.QuadPart == 0)
	{
		//First call of render code.
		LARGE_INTEGER startTime;
		if (!QueryPerformanceCounter(&startTime))
		{
			return Tu::Status(Tu::Status::FAILURE, "Could not query performance timer.");
		}
		m_startTime = startTime;
	}
	else
	{
		LARGE_INTEGER endTime;
		if (!QueryPerformanceCounter(&endTime))
		{
			return Tu::Status(Tu::Status::FAILURE, "Could not query performance timer.");
		}

		//Advance time.
		U64 ticks = (endTime.QuadPart - m_startTime.QuadPart);
		m_startTime = endTime;

		//Frequency is the number of ticks/second
		//Therefore ticks represents ticks / freq of a second
		F32 dtf = (F32)ticks / (F32)m_timerFrequency.QuadPart;

		F32 frameRate = m_pNexusSystem->GetScenePtr()->GetFrameRate();
		UpdateTime(dtf*frameRate);
	}

	return Tu::Status::SUCCESS;
}

bool NexusRender::Draw2D()
{
	RECT rc;
	SetRect( &rc, 2, 0, 0, 0 );
/*
	F64 bytesToMB = 1.0 / 1048576.0;
	{
		MEMORYSTATUS memStatus;
		// read the memory status
		memStatus.dwLength = sizeof(memStatus);
		GlobalMemoryStatus(&memStatus);
		memStatus.dwAvailPhys;

		std::stringstream stream;
		stream << "Physical: ";
		stream << (F64)((F64)(memStatus.dwTotalPhys - memStatus.dwAvailPhys) * bytesToMB) << " / " << (F64)((F64)memStatus.dwTotalPhys * bytesToMB) << "MB";
		m_d3dWindow.Print(0, 2, 14, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}
	{
		MEMORYSTATUS memStatus;
		// read the memory status
		memStatus.dwLength = sizeof(memStatus);
		GlobalMemoryStatus(&memStatus);
		memStatus.dwAvailPhys;

		std::stringstream stream;
		stream << "Virtual: ";
		stream << (F64)((F64)(memStatus.dwTotalVirtual - memStatus.dwAvailVirtual) * bytesToMB) << " / " << (F64)((F64)memStatus.dwTotalVirtual * bytesToMB) << "MB";
		m_d3dWindow.Print(0, 2, 34, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}
	//m_d3dWindow.D3DDevicePtr()->QueryInterface
	*/
/*
	{
	LPDIRECTDRAW lpDD  ;
	DirectDrawCreate(NULL, &lpDD, NULL);
	LPDIRECTDRAW2 lpDD2;
	DDSCAPS      ddsCaps;
	DWORD         dwTotal;
	DWORD         dwFree;
		lpDD->QueryInterface(IID_IDirectDraw2, (void**)&lpDD2);
	ZeroMemory(&ddsCaps, sizeof(ddsCaps)); // Initialize the structure
	ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	lpDD2->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree);
	// Print the free and total memory
	printf("FREE: %d TOTAL: %d\n",dwFree,dwTotal);
	}
*//*
		U32 tMemory = (U32)((F32)m_d3dWindow.D3DDevicePtr()->GetAvailableTextureMem() * bytesToMB);
		std::stringstream stream;
		stream << "Texture: ";
		stream << tMemory;
		stream << "MB";
		m_d3dWindow.Print(0, 2, 54, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}
*/
	/*
	{
		std::stringstream stream;
		stream << "Last Frame Time: ";
		stream << m_pISceneRenderer->LastFrameTime();
		m_d3dWindow.Print(0, 2, 154, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}
	{
		F32 fps;
		if (m_pISceneRenderer->LastFrameTime() > 1e-3f)
		{
			fps = 1.f/m_pISceneRenderer->LastFrameTime();
		}
		else
		{
			fps = 0.f;
		}
		std::stringstream stream;
		stream << "Rendering FPS: ";

		stream << fps;
		m_d3dWindow.Print(0, 2, 184, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}
*/
	const Aftk::C_Scene* pAftkScene = m_pNexusSystem->GetScenePtr();
	if (pAftkScene->GetDuration() < 0.001f)
	{
		return true;
	}

	{
		std::stringstream stream;
		stream << "Time: ";
		stream << m_time;
		m_d3dWindow.Print(0, 2, 34, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}
	{
		std::stringstream stream;
		stream << "Duration: ";
		stream << pAftkScene->GetDuration();
		m_d3dWindow.Print(0, 2, 54, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}
	{
		std::stringstream stream;
		stream << "Start Time: ";
		stream << pAftkScene->GetStartTime();
		m_d3dWindow.Print(0, 2, 74, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}
	{
		std::stringstream stream;
		stream << "End Time: ";
		stream << pAftkScene->GetEndTime();
		m_d3dWindow.Print(0, 2, 94, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}
	{
		std::stringstream stream;
		stream << "Frame Rate: ";
		stream << pAftkScene->GetFrameRate();
		m_d3dWindow.Print(0, 2, 124, stream.str(), VecMat::HVec4(1.0f,1.0f,0.0f,1.0f));
	}

	return true;
}

bool NexusRender::Draw3D()
{
	if (m_pCamera == 0)
	{
		return true;
	}

	m_pCamera->OnUpdateDevice();

	if (!m_pNexusSystem->IsSceneLoaded())
	{
		return true;
	}

	if (!m_pISceneRenderer)
	{
		return true;
	}
	m_pISceneRenderer->SetCameraPtr(m_pCamera);

	switch (m_globalShadingMode)
	{
		case NEXUS_SHADING_TYPE_WIRE_FRAME:
		{
			DWORD value;
			if (FAILED(m_d3dWindow.D3DDevicePtr()->GetRenderState(D3DRS_FILLMODE, &value)))
			{
				assert(false);
			}
			if (value != D3DFILL_WIREFRAME)
			{
				m_d3dWindow.D3DDevicePtr()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			}
			break;
		}

		case NEXUS_SHADING_TYPE_FLAT:
		{
			DWORD value;
			if (FAILED(m_d3dWindow.D3DDevicePtr()->GetRenderState(D3DRS_FILLMODE, &value)))
			{
				assert(false);
			}
			if (value != D3DFILL_SOLID)
			{
				m_d3dWindow.D3DDevicePtr()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			}
			break;
		}

		case NEXUS_SHADING_TYPE_POINTS:
		{
			DWORD value;
			if (FAILED(m_d3dWindow.D3DDevicePtr()->GetRenderState(D3DRS_FILLMODE, &value)))
			{
				assert(false);
			}
			if (value != D3DFILL_POINT)
			{
				m_d3dWindow.D3DDevicePtr()->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
			}
			break;
		}

		case NEXUS_SHADING_TYPE_SMOOTH:
		default:
		{
			DWORD value;
			if (FAILED(m_d3dWindow.D3DDevicePtr()->GetRenderState(D3DRS_FILLMODE, &value)))
			{
				assert(false);
			}
			if (value != D3DFILL_SOLID)
			{
				m_d3dWindow.D3DDevicePtr()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			}
			break;
		}
	}

	if (m_settings.BackFaceCulling())
	{
		DWORD value;
		if (FAILED(m_d3dWindow.D3DDevicePtr()->GetRenderState(D3DRS_CULLMODE, &value)))
		{
			assert(false);
		}
		if (value == D3DCULL_NONE)
		{
			m_d3dWindow.D3DDevicePtr()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}
	}
	else
	{
		DWORD value;
		if (FAILED(m_d3dWindow.D3DDevicePtr()->GetRenderState(D3DRS_CULLMODE, &value)))
		{
			assert(false);
		}
		if (value == D3DCULL_CCW)
		{
			m_d3dWindow.D3DDevicePtr()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		}
	}

	m_pISceneRenderer->OnDraw();
	
	return true;
}

void NexusRender::CentreCamera()
{
	if (m_pCamera == 0)
	{
		return;
	}

	m_pCamera->SetOrigin(VecMat::HVec3(0,0,0));
	m_pCamera->SetRotation(0,0,0);
}

void NexusRender::FrameAll()
{
	if (m_pCamera == 0)
	{
		return;
	}
	
	m_pCamera->Frame(m_pNexusSystem->GetSceneBounds());
	if (m_pISceneRenderer)
	{
		m_pISceneRenderer->OnCameraZoom();
	}
}

void NexusRender::FrameSelected()
{
	/*
	if (m_pNexusSystem->GetSelectedSceneNodePtr())
	{
		Frame(m_pNexusSystem->GetSceneNodeBounds(m_pNexusSystem->GetSelectedSceneNodePtr()));
	}
	if (m_pISceneRenderer)
	{
		m_pISceneRenderer->OnCameraZoom();
	}
	*/
}

void NexusRender::Frame(const Tu::Bounds& bounds)
{
	if (m_pCamera == 0)
	{
		return;
	}
	
	m_pCamera->Frame(bounds);
	if (m_pISceneRenderer)
	{
		m_pISceneRenderer->OnCameraZoom();
	}
}

SRD::Camera* NexusRender::CameraPtr()
{
	return m_pCamera;
}

const SRD::D3DWindow* NexusRender::D3DWindowPtr() const
{
	return &m_d3dWindow;
}

const SRD::ISceneRenderer* NexusRender::ISceneRendererPtr() const
{
	return m_pISceneRenderer;
}

SRD::ISceneRenderer* NexusRender::ISceneRendererPtr()
{
	return m_pISceneRenderer;
}

void NexusRender::OnCameraZoom()
{
	if (m_pISceneRenderer)
	{
		m_pISceneRenderer->OnCameraZoom();
	}
}

void NexusRender::Pick()
{
/*	const SD::SpatialHierachy* pSpatialHierachy = m_pNexusSystem->GetSpatialHierachyPtr();
	if (!m_pCamera || !pSpatialHierachy)
	{
		return;
	}

    // Get the pick ray from the mouse position
	if (!m_pView)
	{
		return;
	}

	RECT rect;
	m_pView->GetWindowRect(&rect);
	int iWidth = (rect.right-rect.left);
	int iHeight = (rect.bottom-rect.top);

	D3DXMATRIX m;

	FLOAT fAspect = (FLOAT)iWidth / (FLOAT)iHeight;

    POINT ptCursor;
    GetCursorPos( &ptCursor );
	m_pView->ScreenToClient( &ptCursor );

     // Compute the vector of the pick ray in screen space
	D3DXMATRIX matProj, matView, matWorld;
	m_pCamera->GetProjectionMatrix(matProj);
	m_pCamera->GetViewMatrix(matView);
	m_pCamera->GetWorldMatrix(matWorld);

    D3DXVECTOR3 v;
	v.x =  ( ( ( 2.0f * ptCursor.x ) / iWidth  ) - 1 ) / matProj._11;
    v.y = -( ( ( 2.0f * ptCursor.y ) / iHeight ) - 1 ) / matProj._22;
    v.z =  1.0f;

    // Get the inverse of the composite view and world matrix
	m = matView;

    D3DXMatrixInverse( &m, NULL, &m );

    // Transform the screen space pick ray into 3D space
	float3 vOrigin = float3( m._41, m._42, m._43);
	float3 vDirection = float3(
		v.x*m._11 + v.y*m._21 + v.z*m._31,
		v.x*m._12 + v.y*m._22 + v.z*m._32,
		v.x*m._13 + v.y*m._23 + v.z*m._33);

	SD::Ray pickRay(vOrigin,vDirection,true);
	SD::RayTraceResult result;
	pSpatialHierachy->RayTrace(pickRay,result);

	// results in:
	//result.iPolygon;
	if (result.bHit)
	{
		Aftk::C_SceneNode* pAftkSceneNode = result.pMeshNode;
		m_pNexusSystem->SetSelectedSceneNodePtr(pAftkSceneNode);
		CDocument* pDocument = m_pView->GetDocument();
		pDocument->UpdateAllViews(0);
	}
	*/
}

bool NexusRender::AnimationPaused() const
{
	return m_animationPaused;
}

void NexusRender::ToggleAnimation()
{
	m_animationPaused = !m_animationPaused;
}

void NexusRender::ResetAnimation()
{
	m_time = m_pNexusSystem->GetScenePtr()->GetStartTime();
}

void NexusRender::FrameSkip(S32 skip)
{
	if (m_pNexusSystem->GetScenePtr()->GetFrameRate() > 0.01f)
	{
		m_time += (1 / m_pNexusSystem->GetScenePtr()->GetFrameRate() * skip);
	}
	if (m_time < 0)
	{
		m_time = m_pNexusSystem->GetScenePtr()->GetEndTime() + m_time;
	}
	else if (m_time > m_pNexusSystem->GetScenePtr()->GetEndTime())
	{
		m_time = 
			m_pNexusSystem->GetScenePtr()->GetStartTime() + 
			(m_time) - m_pNexusSystem->GetScenePtr()->GetEndTime();
	}

	if (m_time < 0)
	{
		m_time = 0;
	}
}

void NexusRender::SetTime(F32 time)
{
	m_time = time;
}

void NexusRender::UpdateTime(F32 time)
{
	if (m_animationPaused)
	{
		return;
	}

	if (time > m_pNexusSystem->GetScenePtr()->GetDuration())
	{
		m_time = m_pNexusSystem->GetScenePtr()->GetDuration();
	}
	else if (m_time + time >= m_pNexusSystem->GetScenePtr()->GetEndTime())
	{
		m_time = 
			m_pNexusSystem->GetScenePtr()->GetStartTime() + 
			(m_time+time) - m_pNexusSystem->GetScenePtr()->GetEndTime();
	}
	else
	{
 		m_time += time;
	}
}

F32 NexusRender::Time() const
{
	return m_time;
}

void NexusRender::CleanUpAfterFailure()
{
}

void NexusRender::SetSettings(const SRD::SceneRendererSettings& settings)
{
	m_settings = settings;
}

SRD::SceneRendererSettings& NexusRender::Settings()
{
	return m_settings;
}

const SRD::SceneRendererSettings& NexusRender::Settings() const
{
	return m_settings;
}

bool NexusRender::TexturesLoaded() const
{
	return m_pISceneRenderer->TexturesLoaded();
}
