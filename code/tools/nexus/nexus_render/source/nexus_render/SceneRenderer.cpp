#include <nexus_render/stdafx.h>
#include <tex_atlas/Database.h>
#include <core/utility/stringproc.h>
#include <nexus_render/SceneBuilder.h>
#include <nexus_render/MeshEffectConstantsBuilder.h>
#include <nexus_render/AreaLightGizmoRenderer.h>
#include <set>

using namespace NexusRender;

Tu::DebugManager NexusRender::g_debugManager;

Maths::HMat44 D3DToNightmareMatrix44(const D3DMATRIX& rhs);

//#pragma optimize("", off)

SceneRenderer::SceneRenderer()
{
	m_pAftkScene = 0;

	m_pCamera = 0;
	m_pViewGrid = 0;
	m_pRootRenderNode = 0;
	m_time = 0;
	m_zoomScale = 0;

	m_pSettings = 0;
	m_lastTime = -1;
	m_oldZoomScale = -1;

	m_pRenderBoundingBox = 0;

	//Callbacks
	m_pDoWeRenderThisNodeCallback = 0;
	m_pDoWeRenderThisPolygonCallback = 0;
	m_pMessageCallback = 0;
	m_pWarningCallback = 0;
	m_pErrorCallback = 0;

	m_numProcessingStagesCompleted = 0;
	m_numProcessingStages = 0;

	///State information
	m_isBuilt = false;
	m_texturesLoaded = false;
	m_cropSheetTexturesLoaded = false;
	m_skinningModeChanged = false;

	memset(m_effectWrappers, 0, sizeof(void*)*EffectWrapper::EFFECT_TYPE_COUNT);
	m_pMeshBasicEffectConstants = 0;
	m_pMeshSelectionEffectConstants = 0;
	m_pGizmoEffectConstantsMeshNormal = 0;
	m_pGizmoEffectConstantsMeshTangent = 0;
	m_pGizmoEffectConstantsMeshBinormal = 0;

	m_pShaderManager = new D3DShaderManager;
	m_pTextureManager = new D3DTextureManager;
}

SceneRenderer::~SceneRenderer()
{
	Destroy();

	delete m_pShaderManager;
	delete m_pTextureManager;
}

Tu::Status SceneRenderer::Create(
	IDirect3DDevice9* pDevice, 
	Aftk::C_Scene* pScene,
	Aftk::ObjTypeArray* pBoundsFilter,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallbackFunc)(const std::string& text),
	void (*pErrorCallbackFunc)(const std::string& text))
{
	m_pBoundsFilter = pBoundsFilter;
	m_pMessageCallback = pMessageCallbackFunc;
	m_pWarningCallback = pWarningCallbackFunc;
	m_pErrorCallback = pErrorCallbackFunc;

	Tu::Status status;

	#ifdef _DEBUG
		g_debugManager.SetDebugEnabledFlag(true);
		g_debugManager.SetConsoleEnabledFlag(true);
		g_debugManager.SetLogFileEnabledFlag(true);
		g_debugManager.Create("c:/nexusx.log");
	#else 
		g_debugManager.SetDebugEnabledFlag(false);
		g_debugManager.SetConsoleEnabledFlag(false);
		g_debugManager.SetLogFileEnabledFlag(true);
		g_debugManager.Create("c:/nexusr.log");
	#endif

	if (!pDevice)
		return Tu::Status(Tu::Status::FAILURE, "D3D device was null.");
	if (!pScene)
		return Tu::Status(Tu::Status::FAILURE, "AftkScene pointer was null.");

	D3DManager::Create(pDevice);
	GetTextureManagerPtr()->Create(pDevice);
	GetShaderManagerPtr()->Create(pDevice);

	m_pAftkScene = pScene;
	m_time = 0;

	m_effectWrappers[EffectWrapper::EFFECT_TYPE_MESH_SELECTION] = new MeshSelectionEffectWrapper;
	if (!m_effectWrappers[EffectWrapper::EFFECT_TYPE_MESH_SELECTION])
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	m_pMeshSelectionEffectConstants = new MeshSelectionEffectConstants;
	if (!m_pMeshSelectionEffectConstants)
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	m_effectWrappers[EffectWrapper::EFFECT_TYPE_MESH_BASIC] = new MeshBasicEffectWrapper;
	if (!m_effectWrappers[EffectWrapper::EFFECT_TYPE_MESH_BASIC])
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	m_pMeshBasicEffectConstants = new MeshBasicEffectConstants;
	if (!m_pMeshBasicEffectConstants)
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	m_effectWrappers[EffectWrapper::EFFECT_TYPE_MESH_LIGHTMAP] = new MeshLightmapEffectWrapper;
	if (!m_effectWrappers[EffectWrapper::EFFECT_TYPE_MESH_LIGHTMAP])
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	m_effectWrappers[EffectWrapper::EFFECT_TYPE_MESH_TEXTURE] = new MeshTextureEffectWrapper;
	if (!m_effectWrappers[EffectWrapper::EFFECT_TYPE_MESH_TEXTURE])
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	m_effectWrappers[EffectWrapper::EFFECT_TYPE_GIZMO] = new GizmoEffectWrapper;
	if (!m_effectWrappers[EffectWrapper::EFFECT_TYPE_GIZMO])
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	m_pGizmoEffectConstantsMeshNormal = new GizmoEffectConstants;
	if (!m_pGizmoEffectConstantsMeshNormal)
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);
	m_pGizmoEffectConstantsMeshTangent = new GizmoEffectConstants;
	if (!m_pGizmoEffectConstantsMeshTangent)
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);
	m_pGizmoEffectConstantsMeshBinormal = new GizmoEffectConstants;
	if (!m_pGizmoEffectConstantsMeshBinormal)
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	for (U32 iEffectWrapper = 0; iEffectWrapper < EffectWrapper::EFFECT_TYPE_COUNT; iEffectWrapper++)
	{
		status = m_effectWrappers[EffectWrapper::EEffectType(iEffectWrapper)]->Build(m_pShaderManager);
		if (!status) return status;
	}

	status = m_pMeshBasicEffectConstants->Build(0);
	if (!status) return status;

	status = m_pMeshSelectionEffectConstants->Build();
	if (!status) return status;

	status = m_pGizmoEffectConstantsMeshNormal->Build();
	if (!status) return status;
	status = m_pGizmoEffectConstantsMeshTangent->Build();
	if (!status) return status;
	status = m_pGizmoEffectConstantsMeshBinormal->Build();
	if (!status) return status;

	//Create scene bounding box renderer
	m_pRenderBoundingBox = new BoundingBox();
	if (!m_pRenderBoundingBox) 
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	//Create the home grid.
	m_pViewGrid = new ViewGrid;
	if (!m_pViewGrid) 
		return Tu::Status(Tu::Status::OUT_OF_MEMORY);

	return Tu::Status::SUCCESS;
}

void SceneRenderer::Destroy()
{
	//Clear the scene renderer to its initial flags.
	Clear();

	delete m_pViewGrid;
	m_pViewGrid = 0;

	delete m_pRenderBoundingBox;
	m_pRenderBoundingBox = 0;

	for (U32 iEffectWrapper = 0; iEffectWrapper < EffectWrapper::EFFECT_TYPE_COUNT; iEffectWrapper++)
	{
		delete m_effectWrappers[EffectWrapper::EEffectType(iEffectWrapper)];
		m_effectWrappers[EffectWrapper::EEffectType(iEffectWrapper)] = 0;
	}

	delete m_pMeshSelectionEffectConstants;
	m_pMeshSelectionEffectConstants = 0;

	delete m_pMeshBasicEffectConstants;
	m_pMeshBasicEffectConstants = 0;

	delete m_pGizmoEffectConstantsMeshNormal;
	m_pGizmoEffectConstantsMeshNormal = 0;
	delete m_pGizmoEffectConstantsMeshTangent;
	m_pGizmoEffectConstantsMeshTangent = 0;
	delete m_pGizmoEffectConstantsMeshBinormal;
	m_pGizmoEffectConstantsMeshBinormal = 0;

	//Erase shaders.
	GetShaderManagerPtr()->Destroy();
	GetTextureManagerPtr()->Destroy();
	//Destroy vertex declarations and any other global D3D stuff.
	D3DManager::Destroy();
	//We don't need our debug manager anymore.
	g_debugManager.Destroy();
}

Tu::Status SceneRenderer::PreResetDevice()
{
	Tu::Status status;
	if (!GetShaderManagerPtr()->OnPreResetDevice())
		return Tu::Status(Tu::Status::FAILURE, "Failed to reset shaders.");

	for (U32 i = 0; i < (U32)m_renderNodes.Size(); i++)
	{
		status = m_renderNodes[i]->OnPreResetDevice();
		if (!status) return status;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::PostResetDevice(Nexus::Progress* pProgress)
{
	Tu::Status status;
	for (U32 i = 0; i < (U32)m_renderNodes.Size(); i++)
	{
		status = m_renderNodes[i]->OnPostResetDevice();
		if (!status) return status;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::UpdateRenderNodes()
{
	if (!m_pRootRenderNode)
		return Tu::Status::SUCCESS;

	Tu::Status status;
	bool scaleChanged = false;

	if (fabsf(fabsf(m_oldZoomScale) - fabsf(m_zoomScale)) > 0.02f)
	{
		scaleChanged = true;
		m_oldZoomScale = m_zoomScale;
	}
	else if (m_lastTime != m_time)
	{
		scaleChanged = true;
	}

	bool timeChanged;
	if (m_lastTime != m_time)
		timeChanged = true;
	else
		timeChanged = false;

	bool settingsChanged;
	if (!memcmp(&m_lastSettings, m_pSettings, sizeof(m_lastSettings)))
		settingsChanged = false;
	else
	{
		settingsChanged = true;
		m_lastSettings = *m_pSettings;
	}

	//Traverse scene in hierarchy order. We need to do this as our parent may have changed scale,
	//this will need to be compensated for.
	status = UpdateRenderNodeSources(
		scaleChanged,
		timeChanged,
		m_pRootRenderNode);
	if (!status) return status;

	status = UpdateRenderNodeInstances(
		scaleChanged,
		timeChanged);
	if (!status) return status;

	if (timeChanged || scaleChanged || settingsChanged)
	{
		status = m_axisRenderer.Update(m_renderNodes);
		if (!status) return status;

		status = m_directionalLightGizmoRenderer.Update(m_renderDirectionalLightNodes);
		if (!status) return status;

		status = m_pointLightGizmoRenderer.Update(m_renderPointLightNodes);
		if (!status) return status;

		status = m_spotLightGizmoRenderer.Update(m_renderSpotLightNodes);
		if (!status) return status;

		status = m_areaLightGizmoRenderer.Update(m_renderAreaLightNodes);
		if (!status) return status;
	}

	m_skinningModeChanged = false;
	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::UpdateRenderNodeInstances(
	bool updateScale,
	bool timeChanged)
{
	for (U32 i = 0; i < (U32)m_renderInstanceNodes.Size(); i++)
	{
		RenderNode* pRenderNode = m_renderInstanceNodes[i];
		Tu::Status status = pRenderNode->Update(
			timeChanged,
			updateScale,
			m_skinningModeChanged);
		if (!status) return status;
	}
	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::UpdateRenderNodeSources(
	bool updateScale,
	bool timeChanged,
	RenderNode* pRenderNode)
{
	if (pRenderNode->GetType() == RenderNode::RENDER_NODE_TYPE_INSTANCE)
		return Tu::Status::SUCCESS;

	Tu::Status status = pRenderNode->Update(
		timeChanged,
		updateScale,
		m_skinningModeChanged);
	if (!status) return status;

	for (U32 iChild = 0; iChild < pRenderNode->GetNumChildren(); iChild++)
	{
		status = UpdateRenderNodeSources(
			updateScale,
			timeChanged,
			pRenderNode->GetChildPtr(iChild));
		if (!status) return status;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::OnDraw()
{
	Tu::Status status;

	if (m_pCamera == 0)
		return Tu::Status(Tu::Status::FAILURE, "Failed to update scene renderer. Camera is null.");

	if (!m_isBuilt)
		//Not built yet...
		return Tu::Status::SUCCESS;

	//Build crop-sheet texture indices if required.
	if (m_pSettings->useCropSheet)
	{
		if (!m_cropSheetTextureIDsSet)
			SetCropSheetTextureIDs();
	}

	status = UpdateRenderNodes();
	if (!status)
	{
		return status;
	}

	// draw the background grid
	if (m_pViewGrid)
	{
		if (!m_pViewGrid->m_bInitialized)
			m_pViewGrid->Initialize();

		if (m_pSettings->showGrid)
			m_pViewGrid->Render(m_pCamera);
	}

	//Draw all of the meshes in the scene.
	if (m_pSettings->renderMeshes)
	{
		IDirect3DStateBlock9* pStateBlock;
		if (FAILED(D3DManager::GetDevicePtr()->CreateStateBlock(D3DSBT_ALL, &pStateBlock)))
			return Tu::Status::FAILURE;
		pStateBlock->Capture();

		switch(m_pSettings->fillMode)
		{
		case E_FillMode_WireFrame:
			D3DManager::GetDevicePtr()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			break;
		case E_FillMode_Point:
			D3DManager::GetDevicePtr()->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
			break;
		case E_FillMode_Solid:
		default:
			D3DManager::GetDevicePtr()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			break;
		};

		if (m_pSettings->backFaceCulling)
			D3DManager::GetDevicePtr()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		else
			D3DManager::GetDevicePtr()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		//If we're using textures, then batch draw mesh fragments by material.
		if (m_pSettings->useTextures || m_pSettings->useCropSheet)
		{
			status = DrawMeshBatches();
			if (!status) return status;
		}
		//If we're not using textures, minimise the number of draw calls per mesh.
		else
		{
			status = DrawMeshes(
				m_meshBatchesNoMaterial,
				m_pMeshBasicEffectConstants);
			if (!status) return status;
		}

		pStateBlock->Apply();
		pStateBlock->Release();
	}

	if (m_pSettings->renderSplines)
	{
		status = DrawSplines();
		if (!status) return status;
	}

	status = DrawGizmos();
	if (!status) return status;

	status = DrawSceneBoundingBox();
	if (!status) return status;

	status = DrawMeshComponents();
	if (!status) return status;

	//Reset dirty flag.
	m_timeChanged = false;

	//Update timing flags.
	m_lastTime = m_time;
	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::DrawMeshBatches()
{
	Tu::Status status;

	for (U32 i = 0; i < D3DManager::VERTEX_DECL_COUNT; i++)
	{
		status = D3DManager::SetVertexDeclaration(D3DManager::EVertexDecl(i));
		if (!status) return status;

		for (U32 j = 0; j < RenderMeshNode::EFFECT_MODE_COUNT; j++)
		{
			status = DrawMeshBatches(
				D3DManager::EVertexDecl(i),
				RenderMeshNode::EEffectMode(j));
			if (!status) return status;
		}
	}

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::DrawMeshBatches(
	D3DManager::EVertexDecl decl,
	RenderMeshNode::EEffectMode mode)
{
	Tu::Status status;

	//Get the array of meshes to render.
	Ut::UtVector< Ut::DynamicVector<MeshClusterBatch*> >* pMeshClusterArray =
		&m_meshClusterBatches[decl][mode];
	if (pMeshClusterArray->Size() == 0)
		return Tu::Status::SUCCESS;

	EffectWrapper* ppLastEffectWrapperUsed = 0;
	for (U32 iBatch = 0; iBatch < (U32)pMeshClusterArray->Size(); iBatch++)
	{
		status = DrawMeshBatch(
			(*pMeshClusterArray)[iBatch],
			decl,
			mode,
			&ppLastEffectWrapperUsed);
		if (!status) return status;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::DrawMeshBatch(
	MeshClusterBatch* pMeshClusterBatch,
	D3DManager::EVertexDecl decl,
	RenderMeshNode::EEffectMode mode,
	EffectWrapper** ppLastEffectWrapperUsed)
{
	Tu::Status status;

	EffectWrapper* pEffectWrapper = 
		m_effectWrappers[pMeshClusterBatch->pEffectConstants->GetEffectType()];

	if (pEffectWrapper != *ppLastEffectWrapperUsed)
	{
		status = pEffectWrapper->SetTechnique(
			*m_pSettings,
			decl, 
			mode);
		ASSERT(status);
		if (!status)
			return status;
		*ppLastEffectWrapperUsed = pEffectWrapper;
	}

	U32 numPasses;
	pEffectWrapper->Begin(numPasses, 0);
	for (U32 iPass = 0; iPass < numPasses; iPass++)
	{
		pEffectWrapper->BeginPass(iPass);

		//Update camera proporties that apply to everything drawn.
		pEffectWrapper->UpdateCameraConstants(*m_pCamera);

		//Update shader constants specific to the material being used.
		//In this case, everything is being drawn with the same material.
		pEffectWrapper->UpdateMaterialConstants(
			*this,
			*pMeshClusterBatch->pEffectConstants);
	        
		for (U32 jMeshCluster = 0; jMeshCluster < (U32)pMeshClusterBatch->meshClusters.Size(); jMeshCluster++)
		{
			MeshCluster* pMeshCluster = pMeshClusterBatch->meshClusters[jMeshCluster].first;
			//Update shader constants that are specific to each node.
			IRenderMeshNode* pMesh = pMeshClusterBatch->meshClusters[jMeshCluster].second;

			//Visibility is determined by the internal flags set during the node's update routine.
			if (!pMesh->IsVisible())
			{
				//Commit effect changes to avoid D3D warnings.
				pEffectWrapper->Commit();
				continue;
			}

			pEffectWrapper->UpdateNodeConstants(*pMesh);

			//Commit effect changes.
			pEffectWrapper->Commit();

			//Draw mesh cluster.
			pMeshCluster->Draw();
		}

		pEffectWrapper->EndPass();
	}
	//Finish using this shader
	pEffectWrapper->End();
	return Tu::Status::SUCCESS;
}


Tu::Status SceneRenderer::DrawMeshes(
	Ut::UtVector< Ut::DynamicVector<IRenderMeshNode*> >
		meshes[D3DManager::VERTEX_DECL_COUNT][IRenderMeshNode::EFFECT_MODE_COUNT],
	EffectConstants* pEffectConstants)
{
	Tu::Status status;
	for (U32 i = 0; i < D3DManager::VERTEX_DECL_COUNT; i++)
	{
		status = D3DManager::SetVertexDeclaration(D3DManager::EVertexDecl(i));
		if (!status) return status;

		for (U32 j = 0; j < RenderMeshNode::EFFECT_MODE_COUNT; j++)
		{
			status = DrawMeshes(
				meshes,
				D3DManager::EVertexDecl(i),
				RenderMeshNode::EEffectMode(j),
				pEffectConstants);
			if (!status) return status;
		}
	}
	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::DrawMeshes(
	Ut::UtVector< Ut::DynamicVector<IRenderMeshNode*> >
		meshes[D3DManager::VERTEX_DECL_COUNT][IRenderMeshNode::EFFECT_MODE_COUNT],
	D3DManager::EVertexDecl decl,
	RenderMeshNode::EEffectMode mode,
	EffectConstants* pEffectConstants)
{
	Tu::Status status;

	//Get the array of meshes to render.
	Ut::UtVector< Ut::DynamicVector<IRenderMeshNode*> >* pMeshArray =
		&meshes[decl][mode];
	if (pMeshArray->Size() == 0)
		return Tu::Status::SUCCESS;

	EffectWrapper* pEffectWrapper = m_effectWrappers[pEffectConstants->GetEffectType()];
	ASSERT(pEffectWrapper);
	ASSERT(pEffectConstants);

	status = pEffectWrapper->SetTechnique(
		*m_pSettings,
		decl, 
		mode);
	ASSERT(status);
	if (!status) return status;

	U32 numPasses;
	pEffectWrapper->Begin(numPasses, 0);
	for (U32 iPass = 0; iPass < numPasses; iPass++)
	{
		pEffectWrapper->BeginPass(iPass);

		//Update camera proporties that apply to everything drawn.
		pEffectWrapper->UpdateCameraConstants(*m_pCamera);

		//Update shader constants specific to the material being used.
		//In this case, everything is being drawn with the same material.
		pEffectWrapper->UpdateMaterialConstants(
			*this,
			*pEffectConstants);
	        
		for (U32 jMesh = 0; jMesh < (U32)pMeshArray->Size(); jMesh++)
		{
			IRenderMeshNode* pMesh = (*pMeshArray)[jMesh];

			//Visibility is determined by the internal flags set during the node's update routine.
			if (!pMesh->IsVisible())
			{
				//Commit effect changes to avoid D3D warnings.
				pEffectWrapper->Commit();
				continue;
			}

			//Update shader constants that are specific to each node.
			pEffectWrapper->UpdateNodeConstants(*pMesh);

			//Commit effect changes.
			pEffectWrapper->Commit();

			//Draw mesh.
			pMesh->Draw();
		}

		pEffectWrapper->EndPass();
	}
	//Finish using this shader
	pEffectWrapper->End();

	return Tu::Status::SUCCESS;
}

const Camera* SceneRenderer::GetCameraPtr() const
{
	return m_pCamera;
}

void SceneRenderer::OnCameraZoom()
{
	if (!m_pCamera)
	{
		return;
	}

	if (m_pCamera->GetType() == Camera::CAMERA_TYPE_TARGET)
	{
        TargetCamera* pTargetCamera = (TargetCamera*)m_pCamera;
		m_zoomScale = pTargetCamera->GetZDistance() * 0.05f;
	}
}

void SceneRenderer::OnSkinningModeChanged()
{
	//Force update
	m_lastTime = -1;
	if (m_pSettings->skinningMode == 0 
		||
		m_pSettings->skinningMode == 2)
	{
	}
	m_oldZoomScale = -1;
	m_skinningModeChanged = true;
}

void SceneRenderer::OnGizmoRenderingChanged()
{
	//Force update
	m_lastTime = -1;
	m_oldZoomScale = -1;
}

void SceneRenderer::SetCameraPtr(Camera* pCamera)
{
	m_pCamera = pCamera;
}

bool SceneRenderer::TexturesLoaded() const
{
	return m_texturesLoaded;
}

bool SceneRenderer::CropSheetTexturesLoaded() const
{
	return m_cropSheetTexturesLoaded;
}

void SceneRenderer::SetSettingsPtr(const SceneRendererSettings* pSettings)
{
	m_pSettings = pSettings;
}

const SceneRendererSettings* SceneRenderer::GetSettingsPtr() const
{
	return m_pSettings;
}

Camera* SceneRenderer::GetCameraPtr()
{
	return m_pCamera;
}

RenderNode* SceneRenderer::GetRootRenderNodePtr()
{
	return m_pRootRenderNode;
}

const RenderNode* SceneRenderer::GetRootRenderNodePtr() const
{
	return m_pRootRenderNode;
}

void SceneRenderer::ClearSelection()
{
	m_selectedRenderNodes.Clear();

	for (U32 iDecl = 0; iDecl < D3DManager::VERTEX_DECL_COUNT; iDecl++)
		for (U32 jEffectMode = 0; jEffectMode < IRenderMeshNode::EFFECT_MODE_COUNT; jEffectMode++)
			m_meshBatchesNoMaterialSelected[iDecl][jEffectMode].Clear();
}

void SceneRenderer::ClearDynamicMeshEffectConstantsList()
{
	for (U32 i = 0; i < (U32)m_dynamicMeshEffectConstantsList.Size(); i++)
	{
		//Free our effect constants.
		delete m_dynamicMeshEffectConstantsList[i];
	}
	m_dynamicMeshEffectConstantsList.Clear();
}

void SceneRenderer::ClearSceneHierachy()
{
	for (U32 i = 0; i < (U32)m_renderNodes.Size(); i++)
		delete m_renderNodes[i];

	m_pRootRenderNode = 0;
	m_renderNodes.Clear();
	m_renderInstanceNodes.Clear();
	m_renderDirectionalLightNodes.Clear();
	m_renderPointLightNodes.Clear();
	m_renderSpotLightNodes.Clear();
	m_renderAreaLightNodes.Clear();
	m_renderSplineNodes.Clear();
	m_renderMeshNodes.Clear();

	m_aftkNodeToRenderNodeMapping.Clear();

	for (U32 iDecl = 0; iDecl < D3DManager::VERTEX_DECL_COUNT; iDecl++)
		for (U32 jEffectMode = 0; jEffectMode < IRenderMeshNode::EFFECT_MODE_COUNT; jEffectMode++)
		{
			m_meshBatchesNoMaterial[iDecl][jEffectMode].Clear();
			m_meshBatchesNoMaterialSelected[iDecl][jEffectMode].Clear();
			m_meshBatchesNoMaterialSelected[iDecl][jEffectMode].Clear();

			Ut::UtVector< Ut::DynamicVector<MeshClusterBatch*> >& clusterBatches = 
				m_meshClusterBatches[iDecl][jEffectMode];
			for (U32 k = 0; k < (U32)clusterBatches.Size(); k++)
			{
				delete clusterBatches[k];
			}
			clusterBatches.Clear();
		}

	ClearSelection();
}


void SceneRenderer::Clear()
{
	//Erase the scene graph and all batching lists.
	ClearSceneHierachy();

	//Erase our effect constants.
	ClearDynamicMeshEffectConstantsList();

	//Erase all textures.
	m_pTextureManager->Clear();
	m_cropSheetImageDB.Clear();
	m_cropSheetAtlasDB.Clear();
	m_cropSheetImageDBMapping.clear();
	m_cropSheetTextureIDsSet = false;

	m_isBuilt = false;
	m_texturesLoaded = false;
	m_cropSheetTexturesLoaded = false;
	m_pRootRenderNode = 0;
	m_timeChanged = true;

	m_lastTime = -1;
	m_oldZoomScale = -1;

	m_numProcessingStagesCompleted = 0;
	m_numProcessingStages = 0;
	
	m_axisRenderer.Clear();
	m_directionalLightGizmoRenderer.Clear();
	m_spotLightGizmoRenderer.Clear();
	m_pointLightGizmoRenderer.Clear();
	m_areaLightGizmoRenderer.Clear();
	m_layerInfo.Clear();
}

RenderNode* SceneRenderer::GetRenderNodeFromAftkNode(const Aftk::C_SceneNode* pAftkSceneNode)
{
	Ut::OpenHashMap<
		const void*, 
		RenderNode*, 
		Ut::HashKeyOpPtr >::Iterator it;

	it = m_aftkNodeToRenderNodeMapping.Find(pAftkSceneNode);
	ASSERT(it != m_aftkNodeToRenderNodeMapping.End());
	return it->GetValue();
}

const SceneRenderer::LayerInfo* SceneRenderer::GetLayerInfoPtr(U32 index) const
{
	return &(m_layerInfo[index]);
}

SceneRenderer::LayerInfo* SceneRenderer::GetLayerInfoPtr(U32 index)
{
	return &(m_layerInfo[index]);
}

S32 SceneRenderer::GetLayerIndex(const Aftk::C_LayerNode* pLayer) const
{
	for (U32 i = 0; i < (U32)m_layerInfo.Size(); i++)
	{
		if (m_layerInfo[i].pLayer == pLayer)
			return i;
	}
	return -1;
}

Aftk::C_Scene* SceneRenderer::GetAftkScenePtr()
{
	return m_pAftkScene;
}

const Aftk::C_Scene* SceneRenderer::GetAftkScenePtr() const
{
	return m_pAftkScene;
}

void SceneRenderer::Pick(
	 Nexus::C_PickResult& result,
	 E_SelectionMode selectionMode,
	 const Aftk::C_Ray& ray)
{
	result.pSceneNode = 0;
	result.selectedComponent = -1;
	result.selectedSubComponent = -1;

	bool foundIntersection = false;
	F32 closestIntersection;
	for (U32 i = 0; i < (U32)m_renderMeshNodes.Size(); i++)
	{
		RenderNode* pRenderNode = m_renderMeshNodes[i];
		if (!pRenderNode->IsVisible())
			continue;

		bool evaluate;
		if (m_pSettings->skinningMode != 0)
			evaluate = true;
		else 
			evaluate = false;
		
		if (pRenderNode->GetAftkSceneNodePtr()->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
			evaluate = true;

		Aftk::C_RayMeshResult rayResult;
		if (selectionMode == E_SelectionMode_Object)
		{
			//Find first point of intersection.
			ray.IntersectMesh(
				rayResult, 
				pRenderNode->GetAftkSceneNodePtr(), 
				pRenderNode->GetActiveLodLevel(),
				evaluate,
				false);
		}
		else
		{
			//Find closest point of intersection.
			ray.IntersectMesh(
				rayResult, 
				pRenderNode->GetAftkSceneNodePtr(), 
				pRenderNode->GetActiveLodLevel(),
				evaluate,
				true);
		}
		if (rayResult.Hit())
		{
			Aftk::Vector3F32 delta = 
				(Aftk::Vector3F32&)rayResult.GetIntersectionPoint() - (Aftk::Vector3F32&)ray.Origin();

			F32 distanceSq = delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2];
			if (!foundIntersection || distanceSq < closestIntersection)
			{
				result.pSceneNode = (Aftk::I_SceneMeshNode*)pRenderNode->GetAftkSceneNodePtr();
				result.lodLevel = pRenderNode->GetActiveLodLevel();
				if (selectionMode == E_SelectionMode_Face)
				{
					result.selectedComponent = rayResult.GetPolygonIndex();
					result.selectedSubComponent = rayResult.GetPolygonTriangleIndex();
				}
				else
				{
					result.selectedComponent = -1;
					result.selectedSubComponent = -1;
				}
				closestIntersection = distanceSq;
				foundIntersection = true;
			}
		}
	}
}

Tu::Status SceneRenderer::OnUpdateGeometry(Nexus::Progress* pProgress)
{
	Tu::Status status;
	m_isBuilt = false;

    if (pProgress)
	{
		pProgress->OnProgress(0.f);
		pProgress->OnPrintProgress("Rebuilding geometry...\n");
	}

	m_numProcessingStages = (U32)m_renderNodes.Size();
	m_numProcessingStagesCompleted = 0;

	for (U32 i = 0; i < (U32)m_renderNodes.Size(); i++)
	{
		//Rebuild geometry node contents.
		RenderNode* pRenderNode = m_renderNodes[i];

		status = pRenderNode->Rebuild();
		if (!status) return status;

		m_numProcessingStagesCompleted++;
		F32 progress = ((F32)m_numProcessingStagesCompleted / (F32)m_numProcessingStages) * 100.0f;
		if (pProgress)
			if (pProgress->OnProgress(progress))
				return Tu::Status(Tu::Status::FAILURE, "Scene update aborted.");
	}

	//Syncronise scene renderer with current rendering settings.
	OnSkinningModeChanged();
	OnGizmoRenderingChanged();

    if (pProgress)
	{
		pProgress->OnProgress(100.0f);
		pProgress->OnPrintProgress("Scene Renderer Built.\n");
	}

	m_isBuilt = true;
	m_timeChanged = true;
	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::OnUpdateTextures(Nexus::Progress* pProgress)
{
	if (m_pSettings->useTextures)
	{
		m_pTextureManager->Clear();
		Tu::Status status = OnLoadTextures(pProgress);
		if (!status) return status;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::OnUpdateScene(Nexus::Progress* pProgress)
{
	Clear();

	if (pProgress)
	{
		if (pProgress->OnProgress(0))
		{
			return Tu::Status(Tu::Status::FAILURE, "Scene update aborted.");
		}
		pProgress->OnPrintProgress("Building scene hierachy...\n");
	}

	m_isBuilt = false;
	Tu::Status status;

	//Clear layer states.
	m_layerInfo.Resize(m_pAftkScene->GetNumLayers());
	for (U32 iLayer = 0; iLayer < m_pAftkScene->GetNumLayers(); iLayer++)
	{
		m_layerInfo[iLayer].pLayer = m_pAftkScene->GetLayerPtr(iLayer);
		m_layerInfo[iLayer].isVisible = true;
	}

	//Create our own copy of the AFTK scene graph.
	status = SceneBuilder::Build(
		&m_pRootRenderNode,
		this,
		m_renderNodes,
		m_renderInstanceNodes,
		m_renderDirectionalLightNodes,
		m_renderPointLightNodes,
		m_renderSpotLightNodes,
		m_renderAreaLightNodes,
		m_renderSplineNodes,
		m_renderMeshNodes,
		m_aftkNodeToRenderNodeMapping,
		m_pAftkScene,
		pProgress);
	if (!status) return status;

	if (pProgress) pProgress->OnPrintProgress("Building vertex data...\n");

	//Update the geometry vertex and index buffers.
	status = OnUpdateGeometry(pProgress);
	if (!status) return status;

	//Rebuild real-time materials.
	if (pProgress) pProgress->OnPrintProgress("Building effect constants...\n");
	status = BuildDynamicMeshEffectConstantsList();
	if (!status) return status;

	//Rebuild effect batches.
	if (pProgress) pProgress->OnPrintProgress("Building mesh batches...\n");
	status = BuildMeshBatches();
	if (!status) return status;

	m_axisRenderer.UpdateScene((U32)m_renderNodes.Size());
	m_directionalLightGizmoRenderer.UpdateScene((U32)m_renderDirectionalLightNodes.Size());
	m_spotLightGizmoRenderer.UpdateScene((U32)m_renderSpotLightNodes.Size());
	m_pointLightGizmoRenderer.UpdateScene((U32)m_renderPointLightNodes.Size());
	m_areaLightGizmoRenderer.UpdateScene((U32)m_renderAreaLightNodes.Size());

	m_isBuilt = true;
	m_oldZoomScale = -1;
	memset(&m_lastSettings, 0, sizeof(m_lastSettings));

	if (pProgress)
	{
		pProgress->OnProgress(100.0f);
		pProgress->OnPrintProgress("Scene Renderer Built.\n");
	}

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::OnLoadTextures(Nexus::Progress* pProgress)
{
	Tu::Status status;

	//Pre-cache textures, so that the 'LoadTextures' function of the materials
	//only builds the association between the cached textures and the material.
	std::set< std::string > uniqueTexturesToLoad;
	std::set< std::string > uniqueTexturesToLoadFullRes;

	Ut::UtVector<Ut::DynamicVector<const EffectConstants::TextureInfo*> > tempTextureInfo;
	for (U32 iConstant = 0; iConstant < (U32)m_dynamicMeshEffectConstantsList.Size(); iConstant++)
	{
		EffectConstants* pEffectConstants = m_dynamicMeshEffectConstantsList[iConstant];
		ASSERT(pEffectConstants);

		tempTextureInfo.Clear();
		//Get a list of the textures used by this effect.
		pEffectConstants->GetTextureInfo(tempTextureInfo);
		for (U32 jTex = 0; jTex < (U32)tempTextureInfo.Size(); jTex++)
		{
			if (tempTextureInfo[jTex]->fileName.length() != 0)
			{
				//Perform some additional work to build a list of lightmap textures.
				//This is so that we can avoid resizeing these textures.
				if (tempTextureInfo[jTex]->forceFullRes)
				{
					uniqueTexturesToLoadFullRes.insert(tempTextureInfo[jTex]->fileName);
				}
				else
				{
					uniqueTexturesToLoad.insert(tempTextureInfo[jTex]->fileName);
				}
			}
		}
	}

	//Now that we've got a unique list of the relevant materials to load, lets do this.
	std::set< std::string >::iterator it;
	U32 texturesToLoad = (U32)uniqueTexturesToLoad.size()+uniqueTexturesToLoadFullRes.size();
	U32 texturesLoaded = 0;

	if (pProgress)
		pProgress->OnProgress(0);
	
	for (it = uniqueTexturesToLoadFullRes.begin(); it != uniqueTexturesToLoadFullRes.end(); it++)
	{
		std::string textureName = *it;
		
		if (!m_pTextureManager->Load(textureName, -1))
		{
			std::stringstream ss;
			ss << "Failed to load texture [" << textureName << "]\n";
			DoWarning(ss.str());
			m_pTextureManager->LoadBlank(textureName);
		}
		texturesLoaded++;
		F32 percent = (F32)texturesLoaded / (F32)texturesToLoad;
		percent *= 100.0f;

		if (pProgress)
		{
			if (pProgress->OnProgress(percent))
			{
				//We've cancelled the texture loading! Make sure the texture pointers for those we have loaded
				//are associated with the correct material proporties.
				LoadTexturesIntoMaterials();
				m_texturesLoaded = false;
				m_cropSheetTexturesLoaded = false;
				return Tu::Status::SUCCESS;
			}
			pProgress->OnPrintProgress(textureName + std::string("\n"));
		}
	}

	for (it = uniqueTexturesToLoad.begin(); it != uniqueTexturesToLoad.end(); it++)
	{
		std::string textureName = *it;
		
		S32 textureCap = 256;
		if (!m_pTextureManager->Load(textureName, textureCap))
		{
			std::stringstream ss;
			ss << "Failed to load texture [" << textureName << "]\n";
			DoWarning(ss.str());
			m_pTextureManager->LoadBlank(textureName);
		}
		texturesLoaded++;
		F32 percent = (F32)texturesLoaded / (F32)texturesToLoad;
		percent *= 100.0f;

		if (pProgress)
		{
			if (pProgress->OnProgress(percent))
			{
				//We've cancelled the texture loading! Make sure the texture pointers for those we have loaded
				//are associated with the correct material proporties.
				LoadTexturesIntoMaterials();
				m_texturesLoaded = false;
				m_cropSheetTexturesLoaded = false;
				return Tu::Status::SUCCESS;
			}
			pProgress->OnPrintProgress(textureName + std::string("\n"));
		}
	}

	//Associate the materials we've loaded with the correct material proporties.
	LoadTexturesIntoMaterials();

	m_texturesLoaded = true;
	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::OnLoadCropSheetTextures(Nexus::Progress* pProgress)
{
	Tu::Status status;

	//Add crop sheet atlases to our unique texture list.
	U32 texturesToLoad = (U32)m_cropSheetAtlasDB.Size();
	if (texturesToLoad == 0)
	{
		return Tu::Status::SUCCESS;
	}
	U32 texturesLoaded = 0;

	if (pProgress)
	{
//		m_pProgress->SetTitle("Loading textures...");
		pProgress->OnProgress(0);
	}

	for (U32 iAtlas = 0; iAtlas < (U32)m_cropSheetAtlasDB.Size(); iAtlas++)
	{
		CropSheetAtlasRecord& r = m_cropSheetAtlasDB[iAtlas];

		//Crop sheets are not resampled to lower resolution.
		IDirect3DTexture9* pD3DCSTexture = m_pTextureManager->Load(
			r.fileName, 
			-1,
			&r.width, 
			&r.height);

		if (!pD3DCSTexture)
		{
			std::stringstream ss;
			ss << "Failed to load texture [" << r.fileName << "]\n";
			DoWarning(ss.str());
			pD3DCSTexture = m_pTextureManager->LoadBlank(r.fileName);
		}
		//Set atlas texture.
		r.pTexture = pD3DCSTexture;

		texturesLoaded++;
		F32 percent = (F32)texturesLoaded / (F32)texturesToLoad;
		percent *= 100.0f;

		if (pProgress)
		{
			if (pProgress->OnProgress(percent))
			{
				//We've cancelled the texture loading! Make sure the texture pointers for those we have loaded
				//are associated with the correct material proporties.
				status = LoadCropSheetTexturesIntoMaterials();
				if (!status) 
				{
					return status;
				}
				m_texturesLoaded = false;
				m_cropSheetTexturesLoaded = false;
				return Tu::Status::SUCCESS;
			}
			pProgress->OnPrintProgress(r.fileName + std::string("\n"));
		}
	}

	//Associate the materials we've loaded with the correct material proporties.
	status = LoadCropSheetTexturesIntoMaterials();
	if (!status) return status;

	m_cropSheetTexturesLoaded = true;
	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::OnUpdateCropSheetDatabase(
	const std::string& csDBFileName)
{
	Tu::Status status;
	//Clear crop-sheet database.
	m_cropSheetImageDB.Clear();
	m_cropSheetAtlasDB.Clear();
	m_cropSheetImageDBMapping.clear();
	m_cropSheetTextureIDsSet = false;

	//Load and associate crop sheet database.
	TexAtlas::Database csGenDB;
	if (!csGenDB.Read(csDBFileName))
		return status = Tu::Status::FAILURE;

	//Load atlas textures.
	for (U32 iAtlasRecord = 0; iAtlasRecord < csGenDB.NumAtlasRecords(); iAtlasRecord++)
	{
		const TexAtlas::Database::DBAtlasRecord& csGenAtlasRecord = 
			csGenDB.AtlasRecord(iAtlasRecord);

		CropSheetAtlasRecord atlasRecord;
		atlasRecord.fileName = csGenAtlasRecord.fileName;
		atlasRecord.pTexture = 0;
		m_cropSheetAtlasDB.PushBack(atlasRecord);
	}

	for (U32 iImageRecord = 0; iImageRecord < csGenDB.NumImageRecords(); iImageRecord++)
	{
		const TexAtlas::Database::DBImageRecord& csGenImageRecord = 
			csGenDB.ImageRecord(iImageRecord);
		CropSheetImageRecord r;
		r.fileName = csGenImageRecord.key.FileName();
		r.textureIndex = -1;
		r.uOffset = csGenImageRecord.uOffset;
		r.vOffset = csGenImageRecord.vOffset;
		r.uScale = csGenImageRecord.uScale;
		r.vScale = csGenImageRecord.vScale;
		r.rotated = csGenImageRecord.rotated;
		r.atlasIndex = csGenImageRecord.atlasIndex;
		//Clean up filename
		std::string cleanFileName;
		Tu::SearchAndReplace(r.fileName, "\\", "/");
		for (U32 j = 0; j < r.fileName.length(); j++)
		{
			char lastChar = ' ';
			if (!((r.fileName[j] == '/') && (lastChar == '/')))
			{
				cleanFileName += r.fileName[j];
			}
			lastChar = r.fileName[j];
		}
		std::transform(cleanFileName.begin(), cleanFileName.begin(), cleanFileName.end(), std::tolower);
		m_cropSheetImageDBMapping[cleanFileName] = (U32)m_cropSheetImageDB.Size();
		r.fileName = cleanFileName;
		m_cropSheetImageDB.PushBack(r);
	}

	return Tu::Status::SUCCESS;
}

void SceneRenderer::SetCropSheetTextureIDs()
{
	if (!m_cropSheetTexturesLoaded)
		return;

	for (U32 i = 0; i < (U32)m_cropSheetImageDB.Size(); i++)
	{
		CropSheetImageRecord& r = m_cropSheetImageDB[i];
		if (r.textureIndex == -1)
		{
			r.textureIndex = m_pTextureManager->TextureIndex(r.fileName);
		}
	}

	m_cropSheetTextureIDsSet = true;
}

void SceneRenderer::LoadTexturesIntoMaterials()
{
	for (U32 iConstant = 0; iConstant < (U32)m_dynamicMeshEffectConstantsList.Size(); iConstant++)
		m_dynamicMeshEffectConstantsList[iConstant]->LoadTextures(*this);
}

Tu::Status SceneRenderer::LoadCropSheetTexturesIntoMaterials()
{
	for (U32 iConstant = 0; iConstant < (U32)m_dynamicMeshEffectConstantsList.Size(); iConstant++)
	{
		Tu::Status status;
		status = m_dynamicMeshEffectConstantsList[iConstant]->LoadCropSheetTexture(*this);
		if (!status)
		{
			std::stringstream ss;
			ss << "Failed to find crop sheet record for [" <<
				status.ErrorString() << "].";
			DoWarning(ss.str());
		}
	}

	return Tu::Status::SUCCESS;
}

F32 SceneRenderer::GetZoomScale() const
{
	return m_zoomScale;
}

Tu::Status SceneRenderer::DrawSelectedNonMesh()
{
	return Tu::Status::SUCCESS;
/*	if (m_selectedRenderNodes.Size() == 0)
	{
		return Tu::Status::SUCCESS;
	}

	Tu::Status status;

	D3DXMATRIX cameraViewMatrix;
	D3DXMATRIX cameraProjectionMatrix;
	// Get the projection & view matrix from the camera class
	m_pCamera->ViewMatrix(cameraViewMatrix);
	m_pCamera->ProjectionMatrix(cameraProjectionMatrix);

	status = m_pSelectionEffectWrapper->SetTechnique(m_pSettings);
	#ifdef _DEBUG
	ASSERT(status);
	#endif
	if (!status) return status;

	U32 numPasses;
	m_pSelectionEffectWrapper->Begin(numPasses, 0);
	for (U32 iPass = 0; iPass < numPasses; iPass++)
	{
		m_pSelectionEffectWrapper->BeginPass(iPass);
		for (U32 iRenderNode = 0; iRenderNode < (U32)m_selectedRenderNodes.Size(); iRenderNode++)
		{
			RenderNode* pRenderNode = m_selectedRenderNodes[iRenderNode];
			if ((pRenderNode->GetType() == G_RENDER_NODE_TYPE_MESH) || 
				(pRenderNode->GetType() == G_RENDER_NODE_TYPE_MESH_REMAPPED_INSTANCE) || 
				(pRenderNode->GetType() == G_RENDER_NODE_TYPE_MESH_BONED_INSTANCE))
			{
				//Skip mesh nodes
				continue;
			}

//			status = DrawSceneNode(pRenderNode);
//			if (!status) return status;
		}
		m_pSelectionEffectWrapper->EndPass();
	}
	//Finish using this shader
	m_pSelectionEffectWrapper->End();
*/
	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::DrawMeshComponents() 
{
	Tu::Status status;

	IDirect3DStateBlock9* pStateBlock;
	if (FAILED(D3DManager::GetDevicePtr()->CreateStateBlock(D3DSBT_ALL, &pStateBlock)))
		return Tu::Status::FAILURE;

	pStateBlock->Capture();
	const F32 bias = -0.0001f;
	D3DManager::GetDevicePtr()->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)(&bias));
	D3DManager::GetDevicePtr()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	//Draw selected meshes in wireframe.
	status = DrawMeshes(
		m_meshBatchesNoMaterialSelected,
		m_pMeshSelectionEffectConstants);
	if (!status) return status;

	status = D3DManager::SetVertexDeclaration(D3DManager::VERTEX_DECL_POS_COL);
	if (!status) return status;

	EffectWrapper* pEffectWrapper = 
		m_effectWrappers[EffectWrapper::EFFECT_TYPE_GIZMO];

	status = pEffectWrapper->SetTechnique(
		*m_pSettings,
		D3DManager::VERTEX_DECL_POS_COL);
	ASSERT(status);
	if (!status) return status;

	U32 numPasses;
	pEffectWrapper->Begin(numPasses, 0);
	for (U32 iPass = 0; iPass < numPasses; iPass++)
	{
		pEffectWrapper->BeginPass(iPass);

		//Update camera proporties that apply to everything drawn.
		pEffectWrapper->UpdateCameraConstants(*m_pCamera);

		//Draw selected mesh components filled.
		D3DManager::GetDevicePtr()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		for (U32 j = 0; j < (U32)m_renderMeshNodes.Size(); j++)
		{
			IRenderMeshNode* pMesh = (IRenderMeshNode*)m_renderMeshNodes[j];
			AFTK_ASSERT(pMesh->HasType(RenderNode::RENDER_NODE_TYPE_IMESH));
			if (!pMesh->IsVisible())
				continue;

			pEffectWrapper->UpdateNodeConstants(*pMesh);

			//Commit effect changes.
			pEffectWrapper->Commit();

			U32 activeLodLevel = pMesh->GetActiveLodLevel();
			if (m_pSettings->renderNormals)
				//Render mesh normals.
				pMesh->DrawNormals(activeLodLevel);
		
			if (m_pSettings->tangentSpaceMode != -1)
				pMesh->DrawTangentSpace(activeLodLevel, m_pSettings->tangentSpaceMode);
		}

		D3DManager::GetDevicePtr()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		D3DManager::GetDevicePtr()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		D3DManager::GetDevicePtr()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		for (U32 j = 0; j < (U32)m_renderMeshNodes.Size(); j++)
		{
			IRenderMeshNode* pMesh = (IRenderMeshNode*)m_renderMeshNodes[j];
			AFTK_ASSERT(pMesh->HasType(RenderNode::RENDER_NODE_TYPE_IMESH));
			if (!pMesh->IsVisible())
				continue;

			pEffectWrapper->UpdateNodeConstants(*pMesh);

			//Commit effect changes.
			pEffectWrapper->Commit();

			pMesh->DrawSelectedComponents(pMesh->GetActiveLodLevel());
		}

		pEffectWrapper->EndPass();
	}

	//Finish using this shader
	pEffectWrapper->End();
	
	pStateBlock->Apply();
	pStateBlock->Release();

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::DrawSplines()
{
	Tu::Status status;

	status = D3DManager::SetVertexDeclaration(D3DManager::VERTEX_DECL_POS_COL);
	if (!status) return status;

	EffectWrapper* pEffectWrapper = 
		m_effectWrappers[EffectWrapper::EFFECT_TYPE_GIZMO];

	status = pEffectWrapper->SetTechnique(
		*m_pSettings,
		D3DManager::VERTEX_DECL_POS_COL);
	ASSERT(status);
	if (!status) return status;

	U32 numPasses;
	pEffectWrapper->Begin(numPasses, 0);
	for (U32 iPass = 0; iPass < numPasses; iPass++)
	{
		pEffectWrapper->BeginPass(iPass);

		//Update camera proporties that apply to everything drawn.
		pEffectWrapper->UpdateCameraConstants(*m_pCamera);

		for (U32 j = 0; j < (U32)m_renderSplineNodes.Size(); j++)
		{
			RenderSplineNode* pSpline = (RenderSplineNode*)m_renderSplineNodes[j];
			if (!pSpline->IsVisible())
				continue;
			pEffectWrapper->UpdateNodeConstants(*pSpline);
			//Commit effect changes.
			pEffectWrapper->Commit();

			pSpline->Draw(0);
		}

		pEffectWrapper->EndPass();
	}
	//Finish using this shader
	pEffectWrapper->End();

	return Tu::Status::SUCCESS;
}

void SceneRenderer::SetTime(F32 time)
{
	if (fabsf(m_time - time) < AFTK_EPSILON)
		return;

	m_time = time;
	m_timeChanged = true;
}

const SceneRenderer::CropSheetAtlasRecord& SceneRenderer::GetCropSheetAtlasRecord(U32 index) const
{
	return m_cropSheetAtlasDB[index];
}

U32 SceneRenderer::GetNumCropSheetAtlasRecords(U32 index) const
{
	return (U32)m_cropSheetAtlasDB.Size();
}

const SceneRenderer::CropSheetImageRecord& SceneRenderer::GetCropSheetImageRecord(U32 index) const
{
	return m_cropSheetImageDB[index];
}

S32 SceneRenderer::GetCropSheetImageRecord(const std::string& fileName) const
{
	std::map<std::string, U32>::const_iterator it;

	it = m_cropSheetImageDBMapping.find(fileName);
	if (it != m_cropSheetImageDBMapping.end())
	{
		return it->second;
	}
	return -1;
}

U32 SceneRenderer::GetNumCropSheetImageRecords(U32 index) const
{
	return (U32)m_cropSheetImageDB.Size();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

class HashKeyOpEffectConstants
{
public:
	//Hashing function.
	static unsigned int GetHash(const EffectConstants* pEffectConstants)
	{
		return pEffectConstants->GetHash();
	}

	static bool Equal(const EffectConstants* pEffectConstants1, const EffectConstants* pEffectConstants2)
	{
		if (*pEffectConstants1 == *pEffectConstants2)
			return true;

		return false;
	}
};

Tu::Status SceneRenderer::BuildDynamicMeshEffectConstantsList()
{
	Tu::Status status;
	//Store one record for each material, mapping Aftk materials to effect constants.
	//This array is accessed by Aftk material index. Multiple aftk materials can potentially be renderered
	//using the same effect constants.
	m_aftkMaterialToEffectConstantsMapping.Resize(m_pAftkScene->GetNumMaterialNodes());

	//Store a list of the effect constants that we've created so far.
	Ut::OpenHashSet<
		EffectConstants*, 
		HashKeyOpEffectConstants > effectConstants;

	//Store a list of the aftk materials actually in use.
	for (U32 i = 0; i < (U32)m_renderNodes.Size(); i++)
	{
		RenderNode* pRenderNode = m_renderNodes[i];
		if (!pRenderNode->HasType(RenderNode::RENDER_NODE_TYPE_IMESH))
			continue;

		if (pRenderNode->GetAftkSceneNodePtr()->GetType() == Aftk::E_ObjType_SceneInstance)
			continue;

		Aftk::C_SceneMeshRemappableNode* pAftkMesh = 
			(Aftk::C_SceneMeshRemappableNode*)pRenderNode->GetAftkSceneNodePtr();

		for (U16 jMaterial = 0; jMaterial < pAftkMesh->GetNumPolygonMaterialMappings(); jMaterial++)
		{
			const Aftk::C_MaterialResource* pRes = pAftkMesh->GetPolygonMaterialMappingPtr(jMaterial);
			if (pRes)
				m_aftkMaterialToEffectConstantsMapping[pRes->GetIndex()] = 0;
		}
	}

	//Run thru our aftk material set and build effect constants as needed.
	for (U32 i = 0; i < (U32)m_aftkMaterialToEffectConstantsMapping.Size(); i++)
	{
		const Aftk::C_MaterialNode* pAftkMaterial = &m_pAftkScene->GetMaterialResourcePtr(i)->GetContent();
		EffectConstants* pEffectConstants;

		status = MeshEffectConstantsBuilder::Build(&pEffectConstants, pAftkMaterial);
		if (!status) return status;
		
		//Insert our effect constants into our hash set
		Ut::OpenHashSet<
			EffectConstants*, 
			HashKeyOpEffectConstants >::ConstIterator hashIt = 
			effectConstants.Find(pEffectConstants);

		//Store any effect constants that we've not encountered yet.
		if (hashIt == effectConstants.End())
		{
			//We've not encountered this effect before.
			//Insert into our unique effect constant list.
			effectConstants.Insert(pEffectConstants);
			//Insert into our unique effect constant array.
			m_dynamicMeshEffectConstantsList.PushBack(pEffectConstants);
		}
		else
		{
			//Delete this effect, we've already got an identical one.
			delete pEffectConstants;
			//Use the existing effect.
			pEffectConstants = hashIt->GetKey();
		}

		//Store the effect constants for this Aftk material. The effect constants pointer may be shared
		//by other Aftk materials.
		m_aftkMaterialToEffectConstantsMapping[i] = pEffectConstants;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::BuildMeshBatches()
{
	Tu::Status status;

	for (U32 i = 0; i < (U32)m_renderNodes.Size(); i++)
	{
		RenderNode* pRenderNode = m_renderNodes[i];

		if (!pRenderNode->HasType(RenderNode::RENDER_NODE_TYPE_IMESH))
			continue;

		IRenderMeshNode* pMesh = 
			(IRenderMeshNode*)pRenderNode;

		Ut::UtVector< Ut::DynamicVector<MeshClusterBatch*> >& clusterBatches = 
			m_meshClusterBatches[pMesh->GetVertexDecl()][pMesh->GetEffectMode()];

		//Add this to our list of meshes.
		m_meshBatchesNoMaterial[pMesh->GetVertexDecl()][pMesh->GetEffectMode()].PushBack(pMesh);

		//Build mesh cluster batches.
		Aftk::C_SceneMeshRemappableNode* pAftkMesh;
		if (pRenderNode->GetAftkSceneNodePtr()->GetType() == Aftk::E_ObjType_SceneInstance)
			pAftkMesh = 
				//Cast the render node's Aftk equivalant to an aftk instance, and then cast its source node
				//to an aftk remappable mesh class.
				(Aftk::C_SceneMeshRemappableNode*)(((Aftk::C_SceneInstanceNode*)pRenderNode->GetAftkSceneNodePtr())->GetSourceNodePtr());
		else
			pAftkMesh = (Aftk::C_SceneMeshRemappableNode*)pMesh->GetAftkSceneNodePtr();

		for (U32 jLod = 0; jLod < pAftkMesh->GetNumLodLevels(); jLod++)
		{
			for (U32 kCluster = 0; kCluster < pMesh->GetNumClusters(jLod); kCluster++)
			{
				//Don't build batches for the 'entire mesh' cluster.
				if (kCluster == RenderMeshNode::SPECIAL_CLUSTER_ENTIRE_MESH)
					continue;

				MeshCluster* pMeshCluster = pMesh->GetClusterPtr(jLod, kCluster);
				if (!pMeshCluster)
					continue;
				//Lookup the effect constants used to render this cluster.
				EffectConstants* pMeshEffectConstants;
				S32 aftkMaterialIndex = pMeshCluster->GetAftkMaterialIndex();
				if (aftkMaterialIndex == -1)
				{
					//No material.
					pMeshEffectConstants = m_pMeshBasicEffectConstants;
				}
				else
				{
					//Material present, so we'll have a corresponding set of effect constants.
					pMeshEffectConstants = m_aftkMaterialToEffectConstantsMapping[aftkMaterialIndex];
				}

				//Insert clusters into batches by effect constants.
				bool inserted = false;
				for (U32 lClusterBatch = 0; lClusterBatch < (U32)clusterBatches.Size(); lClusterBatch++)
				{
					//If the effect constants used to render this cluster are the same as that of the cluster
					//batch we're looking at, then insert this cluster into the cluster batch. Otherwise
					//keep looking.
					MeshClusterBatch* pClusterBatch = clusterBatches[lClusterBatch];
					if (pMeshEffectConstants == pClusterBatch->pEffectConstants)
					{
						Ut::Pair<MeshCluster*, IRenderMeshNode*> pair;
						pair.first = pMeshCluster;
						pair.second = pMesh;
						pClusterBatch->meshClusters.PushBack(pair);
						inserted = true;
					}
				}
				if (!inserted)
				{
					//If we get here then we didn't find an existing batch for this mesh cluster.
					//Create a new cluster batch.
					MeshClusterBatch* pClusterBatch = new MeshClusterBatch();

					Ut::Pair<MeshCluster*, IRenderMeshNode*> pair;
					pair.first = pMeshCluster;
					pair.second = pMesh;
					pClusterBatch->meshClusters.PushBack(pair);

					pClusterBatch->pEffectConstants = pMeshEffectConstants;
					clusterBatches.PushBack(pClusterBatch);
				}
			}
		}
	}

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::DrawSceneBoundingBox()
{
	if (m_timeChanged)
	{
		//Update bounding box.
		m_pRenderBoundingBox->SetBounds(m_pAftkScene->GetEvaluatedBounds(m_pBoundsFilter));
	}

	if (m_pSettings->boundingBoxMode != 1)
		return Tu::Status::SUCCESS;

	Maths::HMat44 id;
	id.SetIdentity();
	D3DManager::GetDevicePtr()->SetTransform(D3DTS_WORLD, (const D3DMATRIX*)&id);

	m_pRenderBoundingBox->Draw();

	return Tu::Status::SUCCESS;
}

Tu::Status SceneRenderer::DrawGizmos()
{
	Tu::Status status;

	status = D3DManager::SetVertexDeclaration(D3DManager::VERTEX_DECL_POS_COL);
	if (!status) return status;

	EffectWrapper* pEffectWrapper = 
		m_effectWrappers[EffectWrapper::EFFECT_TYPE_GIZMO];

	status = pEffectWrapper->SetTechnique(
		*m_pSettings,
		D3DManager::VERTEX_DECL_POS_COL);
	ASSERT(status);
	if (!status) return status;

	U32 numPasses;
	pEffectWrapper->Begin(numPasses, 0);
	for (U32 iPass = 0; iPass < numPasses; iPass++)
	{
		pEffectWrapper->BeginPass(iPass);

		//Update camera proporties that apply to everything drawn.
		pEffectWrapper->UpdateCameraConstants(*m_pCamera);

		//Commit effect changes.
		pEffectWrapper->Commit();

		if (m_pSettings->renderAxes)
		{
			m_axisRenderer.Draw();
		}

		if (m_pSettings->renderLights)
		{
			if (m_pSettings->renderDirectionalLights)
			{
				m_directionalLightGizmoRenderer.Draw();
			}
			if (m_pSettings->renderPointLights)
			{
				m_pointLightGizmoRenderer.Draw();
			}
			if (m_pSettings->renderSpotLights)
			{
				m_spotLightGizmoRenderer.Draw();
			}
			if (m_pSettings->renderAreaLights)
			{
				m_areaLightGizmoRenderer.Draw();
			}
		}
		pEffectWrapper->EndPass();
	}
	//Finish using this shader
	pEffectWrapper->End();

	return Tu::Status::SUCCESS;
}

void SceneRenderer::SetDoWeRenderThisNodeCallback(bool (*pDoWeRenderThisNodeCallback)(const Aftk::C_SceneNode* pAftkSceneNode))
{
	m_pDoWeRenderThisNodeCallback = pDoWeRenderThisNodeCallback;
}

void SceneRenderer::SetDoWeRenderThisPolygonCallback(bool (*pDoWeRenderThisPolygonCallback)(const Aftk::C_Polygon* pAftkPolygon))
{
	m_pDoWeRenderThisPolygonCallback = pDoWeRenderThisPolygonCallback;
}

bool SceneRenderer::DoWeRenderThisPolygon(const Aftk::C_Polygon* pAftkPolygon) const
{
	if (!m_pDoWeRenderThisPolygonCallback)
	{
		return true;
	}

	return m_pDoWeRenderThisPolygonCallback(pAftkPolygon);
}

bool SceneRenderer::DoWeRenderThisNode(const Aftk::C_SceneNode* pAftkSceneNode) const
{
	if (!m_pDoWeRenderThisNodeCallback)
	{
		return true;
	}

	return m_pDoWeRenderThisNodeCallback(pAftkSceneNode);

}

bool SceneRenderer::IsBuilt() const
{
	return m_isBuilt;
}

D3DShaderManager *SceneRenderer::GetShaderManagerPtr()
{
	return m_pShaderManager;
}

D3DTextureManager *SceneRenderer::GetTextureManagerPtr()
{
	return m_pTextureManager;
}

void SceneRenderer::SetSelected(
	E_SelectionMode selectionMode,
	const Nexus::AftkSceneNodeMap& aftkSceneNodeMap)
{
	//Use a set to optimise our search below.
	{
		//Run thru the list of nodes that were previously selected.
		for (U32 iSelectedNode = 0; iSelectedNode < (U32)m_selectedRenderNodes.Size(); iSelectedNode++)
		{
			Nexus::AftkSceneNodeMap::ConstIterator it = 
				aftkSceneNodeMap.Find(m_selectedRenderNodes[iSelectedNode]->GetAftkSceneNodePtr());
			if (it == aftkSceneNodeMap.End())
			{
				//This node was previously selected, but is no longer.
				m_selectedRenderNodes[iSelectedNode]->OnDeselected();
			}
		}
	}

	//Clear our list of selected render nodes.
	ClearSelection();

	//Build a new list of selected render nodes.
	for (
		Nexus::AftkSceneNodeMap::ConstIterator it = aftkSceneNodeMap.Begin();
		it != aftkSceneNodeMap.End(); 
		++it)
	{
		Ut::OpenHashMap<
			const void*, 
			RenderNode*, 
			Ut::HashKeyOpPtr >::Iterator it2;

		it2 = m_aftkNodeToRenderNodeMapping.Find(it->GetKey());
		ASSERT(it2 != m_aftkNodeToRenderNodeMapping.End());
		RenderNode* pRenderNode = it2->GetValue();
		m_selectedRenderNodes.PushBack(pRenderNode);
		if (pRenderNode->HasType(RenderNode::RENDER_NODE_TYPE_IMESH))
		{
			IRenderMeshNode* pMesh = (IRenderMeshNode*)pRenderNode;
			m_meshBatchesNoMaterialSelected[pMesh->GetVertexDecl()][pMesh->GetEffectMode()].PushBack(pMesh);
		}

		if (
			pRenderNode->HasType(RenderNode::RENDER_NODE_TYPE_MESH) || 
			pRenderNode->HasType(RenderNode::RENDER_NODE_TYPE_MESH_INSTANCE))
		{
			RenderMeshRemappableNode* pMesh = (RenderMeshRemappableNode*)pRenderNode;
			pMesh->OnSelected();
			if (selectionMode == E_SelectionMode_Face)
			{
				pMesh->SelectFaces(it->GetValue().selectedComponents);
			}
		}
	}
}

void SceneRenderer::DoMessage(const std::string& message)
{
	if (m_pMessageCallback)
		m_pMessageCallback(message);
}

void SceneRenderer::DoWarning(const std::string& message)
{
	if (m_pWarningCallback)
		m_pWarningCallback(std::string("Warning: ") + message);
}

void SceneRenderer::DoError(const std::string& message)
{
	if (m_pErrorCallback)
		m_pErrorCallback(std::string("Error: ") + message);
}

#endif
