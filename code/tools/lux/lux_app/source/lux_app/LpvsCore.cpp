#include "stdafx.h"

#include "lux_common/MultiProgressNotification.h"
#include "Lux/Core.h"
#include "lux_app/LpvsCore.h"
#include "lux_app/PolygonPicker.h"
#include "Lux/Common.h"

using namespace LPVS;


LPVS::LpvsCore* LPVS::LpvsCore::m_pSingleton = 0;
bool LPVS::LpvsCore::m_bShuttingDown = false;

LPVS::LpvsCore::LpvsCore()
{
	m_bShuttingDown = false;
	m_runningFlag = false;
	Reset();

	m_pPicker = new CPolygonPicker;
}

LPVS::LpvsCore::~LpvsCore()
{
	Clear();
	if (m_pPicker)
	{
		delete m_pPicker;
		m_pPicker = NULL;
	}
}

void LPVS::LpvsCore::Clear()
{
	delete m_pCamera;
	Reset();
}

void LPVS::LpvsCore::Init()
{
	m_pSingleton->Reset();
}

LPVS::LpvsCore& LPVS::LpvsCore::GetSingleton()
{
	if (!m_pSingleton) 
	{
		m_pSingleton = new LpvsCore;
		Init();
	}

	return *m_pSingleton;
}

bool LPVS::LpvsCore::Create(HWND renderWindowHandle)
{
	if (m_runningFlag) return true;

	m_renderWindowHandle = renderWindowHandle;

	if (renderWindowHandle != 0)
	{
		m_d3dWindow.AttachToWindow(0, renderWindowHandle);
		m_debugRender.Init(&m_d3dWindow);

		// set default renderer (even when we haven't loaded a scene yet)
		// (we're doing this to make the grid appear)
		SRD::SR_D3D_TEXTURE_MANAGER.Create(m_d3dWindow.GetD3dDevicePtr());
		SRD::SR_D3D_SHADER_MANAGER.Create(m_d3dWindow.GetD3dDevicePtr());

		m_pHierarchyRenderer = NULL;
		SRD::GenericSceneRenderer* pGSR = new SRD::GenericSceneRenderer(&m_d3dWindow, (Aftk::C_Scene*)&m_aftk);
		m_pSceneRenderer = (SRD::GenericSceneRenderer*)pGSR;
		if (!m_pSceneRenderer->Create())
		{
			return false;
		}

		// set arbitrary bounds
		Bounds bounds;
		bounds.AddVertex(*(Position3*)&float3(100,100,100));
		bounds.AddVertex(*(Position3*)&float3(-100,-100,-100));
		m_pCamera->Frame(bounds);
	}

	m_runningFlag = false;

	return true;
}

// Close source scene and delete scene_renderer
void LPVS::LpvsCore::Destroy()
{
	m_bShuttingDown = true;

	Core::Destroy();

	m_debugRender.Close();

	m_d3dWindow.Destroy();

	SRD::SR_D3D_TEXTURE_MANAGER.Destroy();
	SRD::SR_D3D_SHADER_MANAGER.Destroy();

	Clear();

	delete m_pSingleton;
	m_pSingleton = NULL;
}

bool LPVS::LpvsCore::GetIsRunningFlag() const
{
	return m_runningFlag;
}

void LPVS::LpvsCore::SetViewportRectangle(const RECT& rect)
{
	m_rect = rect;
}

void LPVS::LpvsCore::SetCameraType(SRD::SrCameraTypeEnum cameraType)
{
	if (m_pCamera != 0)
	{
		delete m_pCamera;
		m_pCamera = 0;
	}
	switch (cameraType)
	{
	case SRD::SR_CAMERA_TYPE_TARGET:
		m_pCamera = new SRD::TargetCamera(&m_d3dWindow);
		break;
	default:
		break;
	}
}

void LPVS::LpvsCore::Resize(U32 width, U32 height)
{
	if (m_pSceneRenderer != 0)
	{
		//Make sure the renderer correctly handles the D3D device being reset
		m_pSceneRenderer->OnResetDevice();
	}
	m_d3dWindow.Resize(width, height);

	if (m_pCamera != 0)
	{
		m_pCamera->OnResize();
	}
}

void LPVS::LpvsCore::SetIsRunningFlag(bool flag)
{
	m_runningFlag = flag;
}

SRD::Camera* LPVS::LpvsCore::GetCameraPtr()
{
	return m_pCamera;
}

const SRD::Camera* LPVS::LpvsCore::GetCameraPtr() const
{
	return m_pCamera;
}

const SRD::D3dWindow* LPVS::LpvsCore::GetD3dWindowPtr() const
{
	return &m_d3dWindow;
}

bool LPVS::LpvsCore::ReadProject(
	const std::string& fileName,
	MultiProgressNotification* pProgress)
{
	m_state = APP_MAIN_STATE_READING_PROJECT;
	if (!m_lux.ReadProjectProperties(fileName))
	{
		return false;
	}

	if (!m_lux.ReadLightingModelProperties())
	{
		return false;
	}

	if (pProgress)
	{
		pProgress->SetNumSubProcesses(4);
		pProgress->SetSubProcess(0);
	}
	if (!ReadScene(pProgress))
	{
		return false;
	}

	if (!m_lux.GetLightingModelPropertiesPtr()->AddLightsFromScene(m_lux.GetAftkScene()))
		LOGWARN( LUX::g_Log, "Multiple directional lights exist. Check your input scene and your lighting.xml file.\n");

	m_state = APP_MAIN_STATE_IDLE;
	return true;
}

bool LPVS::LpvsCore::WriteProject(
	const std::string& fileName,
	MultiProgressNotification* pProgress)
{
	return m_lux.WriteProjectProperties(fileName);
}

void LPVS::LpvsCore::CloseProject()
{
	CloseScene();
	m_lux.Clear();
}

void LPVS::LpvsCore::SetLightingModelPropertiesPtr(LUX::LightingModelProperties* pProperties)
{
	m_lux.SetLightingModelPropertiesPtr(pProperties);
}

const LUX::LightingModelProperties* LPVS::LpvsCore::GetLightingModelPropertiesPtr() const
{
	return m_lux.GetLightingModelPropertiesPtr();
}

LUX::LightingModelProperties* LPVS::LpvsCore::GetLightingModelPropertiesPtr()
{
	return m_lux.GetLightingModelPropertiesPtr();
}

namespace LPVS
{
	Aftk::C_SceneNode* GetSourceNodePtr(Aftk::C_SceneInstanceNode* pInstance)
	{
		Aftk::C_SceneNode* pSourceNode = 0;
		pSourceNode = pInstance->GetSourceNodePtr();
		while(pSourceNode)
		{
			if (pSourceNode == pInstance)
			{
				//Bail - we've encountered an infinite instancing loop.
				return 0;
			}

			if (pSourceNode->GetType() == Aftk::E_ObjType_SceneInstance)
			{
				pSourceNode = ((Aftk::C_SceneInstanceNode*)pSourceNode)->GetSourceNodePtr();
				continue;
			}

			//If we get here then we've finally found a non-instance source node.
			break;
		}

		return pSourceNode;
	}
}


U32 ResourceIndirectInstances(Aftk::C_Scene *pScene)
{
	U32 count = 0;
	//Remove instances of instances.
	Aftk::C_SceneNodeIterator it(pScene);
	for (; it ; it++)
	{
		Aftk::C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			Aftk::C_SceneInstanceNode* pInst = (Aftk::C_SceneInstanceNode*)pSceneNode;
			//Correct source node if needed.
			Aftk::C_SceneNode* pCorrectSourceNode = LPVS::GetSourceNodePtr(pInst);
			if (pInst->GetSourceNodePtr() != pCorrectSourceNode)
			{
				pInst->SetSourceNodePtr(pCorrectSourceNode);
				count++;
			}
		}
	}

	return count;
}

// read scene + set up some D3D stuff
bool LPVS::LpvsCore::ReadScene(MultiProgressNotification* pProgress)
{
	bool bReturn = Core::ReadScene(0,0);
	if (!bReturn)
		return false;

	ResourceIndirectInstances(&m_aftk);

	ComputeSceneBounds();

	if (m_renderWindowHandle)
	{
		m_pHierarchyRenderer = NULL;
		SRD::GenericSceneRenderer* pGSR = new SRD::GenericSceneRenderer(&m_d3dWindow, (Aftk::C_Scene*)&m_aftk);
		m_pSceneRenderer = (SRD::GenericSceneRenderer*)pGSR;

		if (!m_pSceneRenderer->Create())
		{
			return false;
		}

		FrameAll();
	}

	return true;
}

void LPVS::LpvsCore::ComputeSceneBounds()
{

	Aftk::C_SceneNodeIterator it(&m_aftk);

	while (it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;

		switch (pAftkSceneNode->GetType())
		{
			default:
				break;
			case Aftk::E_ObjType_SceneMesh:
			{
				Aftk::C_SceneMeshNode* pAftkMeshInterfaceNode = (Aftk::C_SceneMeshNode*)pAftkSceneNode;
				const Aftk::Matrix44* pMatrix = &pAftkMeshInterfaceNode->GetWorldMatrix();

				Bounds b;

				U32 numPolys = pAftkMeshInterfaceNode->GetNumPolygons(0);
				for (U32 i = 0; i < numPolys; i++)
				{
					const Aftk::C_Polygon* pAftkPoly = &pAftkMeshInterfaceNode->GetPolygon(i);

					U32 numPolyVerts = pAftkPoly->GetNumPolygonVertices();
					for (U32 j = 0; j < numPolyVerts; j++)
					{
						const Aftk::C_PolygonVertex* pVertex = &pAftkPoly->GetPolygonVertex(j);

						U32 posIndex = pVertex->GetPositionIndex();
						const Aftk::Position3* pos = &pAftkMeshInterfaceNode->GetVertexPosition(posIndex);
						Aftk::Position3 posWorld = ApplyMatrix(*pMatrix,*pos);

						//Update local bounds
						b.AddVertex(posWorld);
						//Update global bounds
						m_sceneBounds.AddVertex(posWorld);
					}
				}

				m_aftkSceneNodeBounds[pAftkSceneNode] = b;
			break;
			}
		}

		it++;
	}
}

void LPVS::LpvsCore::CloseScene()
{
	Core::CloseScene();

	// Delete the scene_renderer
	if (m_pHierarchyRenderer != 0)
	{
		delete m_pHierarchyRenderer;
		m_pHierarchyRenderer = 0;
	}

	if (m_pSceneRenderer != 0)
	{
		delete m_pSceneRenderer;
		m_pSceneRenderer = 0;
	}

	//Clear the scene node bounds
	m_sceneBounds.Clear();
	m_aftkSceneNodeBounds.clear();
}

// set progress counter update callback function
void LPVS::LpvsCore::SetProgressCallBackProc(void *pThis, LUX::ProgressCallbackProc *pCallback)
{
	m_lux.SetProgressCallback(pThis, pCallback);
}

// call this function to update the mesh after lightmapping
void LPVS::LpvsCore::UpdateVertexBuffers()
{
	if (m_d3dWindow.GetHWnd() != 0)
	{
		//Rebuild our vertex buffers so that we can see the newly lit scene.
		//m_pHierarchyRenderer->RebuildVertexBuffers();
//		m_pSceneRenderer->RebuildVertexBuffers();
		m_pSceneRenderer->Clear();
	}
}

void LPVS::LpvsCore::CentreCamera()
{
	if (m_pCamera == 0)
	{
		return;
	}
	if (m_pSpatialHierarchy == 0)
	{
		return;
	}
	// This currently only frames the rootnode, it will eventually frame an entire scene
	const box3f bbox = *m_pSpatialHierarchy->GetBoundingBox();
	Position3 p;
	m_pCamera->SetPosition((Position3&)bbox.GetCenter());
	m_pCamera->SetRotation(0,0,0);
}

void LPVS::LpvsCore::FrameAll()
{
	if (m_pCamera == 0)
	{
		return;
	}
	if (m_pSpatialHierarchy == 0)
	{
		return;
	}
	// This currently only frames the rootnode, it will eventually frame an entire scene
	m_pCamera->Frame(m_sceneBounds);
}

bool LPVS::LpvsCore::Render2D()
{
	return true;
}

bool LPVS::LpvsCore::Render3D()
{
	HRESULT hr = D3D_OK;

	if (m_pCamera == 0)
	{
		return true;
	}

	m_pCamera->OnUpdateDevice();

	if (m_pSceneRenderer != 0)
	{
		m_pSceneRenderer->SetCameraPtr(m_pCamera);
		m_pSceneRenderer->OnUpdateDevice();
	}

	// draw picked polygons
	m_pCamera->OnUpdateDevice();
	m_pPicker->Draw();

	return true;
}


bool LPVS::LpvsCore::RenderScene()
{
	if (!m_runningFlag)
	{
		return true;
	}
	if (m_state != APP_MAIN_STATE_IDLE)
	{
		return true;
	}


	m_state = APP_MAIN_STATE_RENDERING;

	HRESULT hr = D3D_OK;

	if ((hr == D3DERR_DEVICELOST) || (hr == D3DERR_DEVICENOTRESET)) 
	{ 
		// Focus lost
		m_d3dWindow.HandleLostDevice();
	} 
	else if(FAILED(hr)) 
	{
		m_runningFlag = false;
		return false;
	}

	m_d3dWindow.GetD3dDevicePtr()->Clear(
		0, 
		NULL, 
		D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
		0x87878787, 
//		0,
		1.0f, 
		0);

	m_d3dWindow.GetD3dDevicePtr()->BeginScene();
	hr = Render3D();
	if (FAILED(hr)) {
		m_d3dWindow.GetD3dDevicePtr()->EndScene();
		return false;
	}
	hr = Render2D();
	if (FAILED(hr)) {
		m_d3dWindow.GetD3dDevicePtr()->EndScene();
		return false;
	}

    m_d3dWindow.GetD3dDevicePtr()->EndScene();

    m_d3dWindow.GetD3dDevicePtr()->Present(NULL, &m_rect, NULL, NULL);

	m_state = APP_MAIN_STATE_IDLE;
	return true;
}

LPVS::LpvsCoreStateEnum LPVS::LpvsCore::GetState() const
{
	return m_state;
}

const Bounds &LPVS::LpvsCore::GetSceneBounds() const
{
	return m_sceneBounds;
}


void LPVS::LpvsCore::Reset()
{
	m_renderWindowHandle = 0;
	m_aftk.Clear();
	m_pHierarchyRenderer = 0;
	m_pSceneRenderer = 0;
	m_pHierarchyRenderer = 0;
	m_lux.Clear();

	m_rect.left = 0;
	m_rect.right = 0;
	m_rect.top = 0;
	m_rect.bottom = 0;

	m_pCamera = 0;	

	m_state = APP_MAIN_STATE_IDLE;

	m_pLogCallbackFunc = 0;

	Core::Reset();
}

void LPVS::LpvsCore::SetSelection(SD::RayTraceResult *pResult)
{
	m_pPicker->Init(m_d3dWindow.GetD3dDevicePtr());
	m_pPicker->AddIntersectionsToMesh(pResult);
}


