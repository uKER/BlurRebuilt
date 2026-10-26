#include "stdafx.h"
#include "system/misc/StringUtil.h"

#include "TreeRenderer.h"
#include "Shaders/vsConstants.h"
#include "Shaders/psConstants.h"

using namespace SpeedtreeBillboard;

/*
C_RenderNormalsShaderDelegate::~C_RenderNormalsShaderDelegate()
{
}

void C_RenderNormalsShaderDelegate::Execute(const AftkRender::C_RenderTask& task)
{
	AftkRender::I_RenderMeshNode *pMeshNode = task.GetRenderMeshNode();

	Display::C_Matrix worldMatrix;
	Maths::Store(pMeshNode->GetWorldMatrix(), &worldMatrix);

	Display::C_Matrix worldViewITMatrix;
	Maths::HMat44 worldViewITMatrixTmp = Maths::HMat44(m_pTreeRenderer->m_Views[0]->m_ViewSetup.m_ViewSetup3D.m_CameraToWorld) * pMeshNode->GetWorldMatrix();
	worldViewITMatrixTmp.AW(Maths::HVec4(0.0f, 0.0f, 0.0f, 1.0f));
	worldViewITMatrixTmp = worldViewITMatrixTmp.Inverse();
	worldViewITMatrixTmp.Transposed();
	Maths::Store(worldViewITMatrixTmp, &worldViewITMatrix);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);
	Display::Device()->SetVertexShaderConstantF(CV_SPEEDTREE_BILLBOARD_WORLDVIEW_INV_TRANS_MATRIX, (F32*)&worldViewITMatrix, 4);

	S32 materialIndex = task.GetRenderMeshClusterPtr()->GetAftkMaterialIndex();
	if (materialIndex == -1)
		m_pTreeRenderer->m_NormalEffects[E_NormalRenderEffect_Geometric]->OnEnable();
	else
	{
		E_NormalRenderEffect newEffect;
		if (task.GetRenderMeshClusterPtr()->GetClusterFlags().speedtree_leaf)
			if (m_pTreeRenderer->m_pAftkSceneRenderer->GetRenderMaterial(materialIndex)->HasNormalMap())
				newEffect = E_NormalRenderEffect_NormalMapBillboard;
			else
				newEffect = E_NormalRenderEffect_GeometricBillboard;
		else
			if (m_pTreeRenderer->m_pAftkSceneRenderer->GetRenderMaterial(materialIndex)->HasNormalMap())
				newEffect = E_NormalRenderEffect_NormalMap;
			else
				newEffect = E_NormalRenderEffect_Geometric;

		m_pTreeRenderer->m_NormalEffects[newEffect]->OnEnable();
	}
}
*/

C_TreeRenderer::C_TreeRenderer()
: AftkRender::C_SceneRenderer()
{
}

C_TreeRenderer::~C_TreeRenderer()
{
}

void C_TreeRenderer::Render(C_TreeRenderer::Flags flags)
{
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	//AA alpha testing
	if (Display::Device()->m_pd3dObject->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A')) == S_OK)
		Display::Device()->m_pd3dDevice->SetRenderState(D3DRS_ADAPTIVETESS_Y, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A'));


	for (S32 i=0; i<m_NodeContextArray.Size(); ++i)
	{
		const AftkRender::C_SceneGraphNode *pNode = m_NodeContextArray[i].first;
		U32 index = m_NodeContextArray[i].second;

		AftkRender::C_RenderContext& rc = m_RenderContextArray[index];
		const AftkRender::C_GeometryCluster *pCluster = rc.GetGeometryCluster();
		U32 batch = rc.GetBatch();


		if (flags.CullLeaves && rc.GetFlags().isBillboard)
			continue;

		// view frustrum culling
		const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
		Display::C_CullResult cullResult = frustum.CullAxisBox(pNode->GetWorldAABBox());
		if (cullResult.Visible() > 0)
		{
			S16 materialIndex = rc.GetRemappedMaterialIndex();
			if (materialIndex != -1)
			{
				const AftkRender::C_Scene *pScene = pNode->GetScene();
				if (pScene)
				{
					S32 materialIndexMapping = pScene->GetMaterialIndexMapping(materialIndex);
					const AftkRender::C_Material *pMaterial = pScene->GetMaterial(materialIndexMapping);
					assert(pMaterial && "Invalid material index");

					pMaterial->Enable();
					if (!pMaterial->TexturesLoaded())
						Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());
				}
			}
			rc.Apply();

			if (flags.VertexColourLighting)
				Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicSHL.shader"));

			Display::Device()->SetCullMode(Display::E_CullMode_None);
			pCluster->RenderGeometry(batch);
		}
	}

	m_NodeContextArray.Clear();
	m_RenderContextArray.Clear();
}

void C_TreeRenderer::AddRenderContext(AftkRender::C_RenderContext& rc, const AftkRender::C_SceneGraphNode* pNode)
{
	const AftkRender::C_GeometryCluster *pCluster = rc.GetGeometryCluster();
	assert (pCluster);
	
	//set shader for cluster
	{
		if (rc.GetFlags().isBillboard)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader("shaders/vsBillboard.shader"));
		else if (rc.GetFlags().isSkinned)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader("shaders/vsSkinned.shader"));
		else
			rc.SetVertexShader(m_ShaderManager.GetVertexShader("shaders/vsBasic.shader"));

		if (rc.GetFlags().isTextured)
			rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicTextured.shader"));
		else
			rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasic.shader"));
	}

	C_Renderer::AddRenderContext(rc, pNode);
	m_NodeContextArray.PushBack(Ut::Pair<const AftkRender::C_SceneGraphNode*, U32>(pNode, m_RenderContextArray.Size()-1));
}

C_TreeRendererApp::C_TreeRendererApp()
: m_pSettings(0)
, m_pTreeRenderer(0)
, m_pAftkRenderScene(0)
, m_pDiffuseAtlas(0)
{
}

C_TreeRendererApp::~C_TreeRendererApp()
{
}

bool C_TreeRendererApp::Create(const C_TreeRendererSettings *pSettings)
{
	m_pSettings = pSettings;
	if(!AftkRender::C_Application::Create(1,1))
		return false;


	m_pAftkRenderViewContext = AftkRender::C_Application::AddViewContext(m_DeviceParams);
	m_pAftkRenderViewContext->Resize(pSettings->m_AtlasWidth, pSettings->m_AtlasHeight);

	m_pTreeRenderer = new C_TreeRenderer();
	if (!m_pTreeRenderer)
		return false;

	m_pTreeRenderer->LoadDefaultShaders();
	m_pTreeRenderer->Create();
	m_pTreeRenderer->CreateVolatile();

	m_pAftkRenderScene = new AftkRender::C_Scene;

	m_pDiffuseAtlas = new Attk::C_Image();
	if (!m_pDiffuseAtlas)
		return false;

	//m_pNormalAtlas = new Attk::C_Image();
	//if (!m_pNormalAtlas)
	//	return false;

	Attk::C_Status status;
	status = m_pDiffuseAtlas->NewPixelData(pSettings->m_AtlasWidth, pSettings->m_AtlasHeight, 1, Attk::E_PixelFormat_R8G8B8A8U);
	if (!status)
		return false;

	//status = m_pNormalAtlas->NewPixelData(pSettings->m_AtlasWidth, pSettings->m_AtlasHeight, 1, Attk::E_PixelFormat_R8G8B8A8U);
	//if (!status)
	//	return false;

	return true;
}

void C_TreeRendererApp::Destroy()
{
	SAFE_DELETE_PTR(m_pAftkRenderScene);

	if (m_pTreeRenderer)
	{
		m_pTreeRenderer->Destroy();
		m_pTreeRenderer->DestroyVolatile();
		SAFE_DELETE_PTR(m_pTreeRenderer);
	}
	C_Application::Destroy();

	m_pAftkRenderViewContext = 0;

	ClearUniqueTreeList();

	SAFE_DELETE_PTR(m_pDiffuseAtlas);
	//SAFE_DELETE_PTR(m_pNormalAtlas);
}


void C_TreeRendererApp::Run(Aftk::C_Scene *pScene)
{
	ClearUniqueTreeList();

	if (!GetUniqueTrees(pScene))
		return;

	if (m_UniqueTrees.Size() == 0)
	{
		LOGINFO (g_Log, "No trees have been found in scene\n");
		return;
	}

	#ifdef NONUNIFORM_TEXATLAS
	C_NonUniformTextureLayout<C_TreeBillboard> billboardLayoutMgr(m_pSettings->m_AtlasWidth, m_pSettings->m_AtlasHeight);
	#else
	C_UniformTextureLayout<C_TreeBillboard> billboardLayoutMgr(m_pSettings->m_AtlasWidth, m_pSettings->m_AtlasHeight, m_pSettings->m_MinBillboardWidth, m_pSettings->m_MinBillboardHeight);
	#endif

	U32 nBillboardsInCurrent = 0;
	Util::C_String diffuseFilename, normalFilename;
	diffuseFilename.Printf("%s%i_DIFF.%s", m_pSettings->m_Filename.Chars(), 0, m_pSettings->m_FileFormat.Chars());
	normalFilename.Printf("%s%i_NORM.%s", m_pSettings->m_Filename.Chars(), 0, m_pSettings->m_FileFormat.Chars());

	C_TreeBillboardSet billboardSet;
	U32 currentTextureSet = billboardSet.CreateNewTextureSet(diffuseFilename, normalFilename);

	ClearRenderSurfaces();

	U32 i=0;
	Ut::UtVector< Ut::DynamicVector< Ut::Pair< C_TreeBillboard*, Aftk::C_SceneMeshNode*> > >::Iterator it = m_UniqueTrees.Begin();
	while (it != m_UniqueTrees.End())
	{
		Aftk::C_SceneMeshNode *pMeshNode = it->second;
		SetSceneNodePtr(pMeshNode);

		for (U32 iView=0; iView<(U32)m_pSettings->m_Views.Size(); ++iView)
		{	
			#ifdef NONUNIFORM_TEXATLAS
			#pragma message ("FIXME: ensure all weather conditions for a tree reside in the same atlas")
			#else

			// keep billboards of the same tree in different conditions on the same atlas
			if (billboardLayoutMgr.GetTextureCapacity()-billboardLayoutMgr.GetNumTextures() < (U32)m_pSettings->m_Conditions.Size())
			{
				WriteRenderSurfaces(diffuseFilename, normalFilename);
				diffuseFilename.Printf("%s%i_DIFF.%s", m_pSettings->m_Filename.Chars(), currentTextureSet+1, m_pSettings->m_FileFormat.Chars());
				normalFilename.Printf("%s%i_NORM.%s", m_pSettings->m_Filename.Chars(), currentTextureSet+1, m_pSettings->m_FileFormat.Chars());
				currentTextureSet = billboardSet.CreateNewTextureSet(diffuseFilename, normalFilename);
				nBillboardsInCurrent = 0;
				ClearRenderSurfaces();
				billboardLayoutMgr.Clear();
			}
			#endif
			for (U32 jCondition=0; jCondition<(U32)m_pSettings->m_Conditions.Size(); ++jCondition)
			{
				#ifdef NONUNIFORM_TEXATLAS
				const C_TextureBounds* pBillboardExtents = billboardLayoutMgr.Insert(
					Aftk::Vector2U32(m_pSettings->m_MinBillboardWidth, m_pSettings->m_MinBillboardHeight), it->GetKey());
				#else
				const C_TextureBounds* pBillboardExtents = billboardLayoutMgr.Insert(it->first);
				#endif
				
				if (!pBillboardExtents)
				{
					if (nBillboardsInCurrent)
					{
						// we've ran out of space ... write the surface out to file
						WriteRenderSurfaces(diffuseFilename, normalFilename);
						diffuseFilename.Printf("%s%i_DIFF.%s", m_pSettings->m_Filename.Chars(), currentTextureSet+1, m_pSettings->m_FileFormat.Chars());
						normalFilename.Printf("%s%i_NORM.%s", m_pSettings->m_Filename.Chars(), currentTextureSet+1, m_pSettings->m_FileFormat.Chars());
						currentTextureSet = billboardSet.CreateNewTextureSet(diffuseFilename, normalFilename);
						
						nBillboardsInCurrent = 0;
						ClearRenderSurfaces();
						billboardLayoutMgr.Clear();

						#ifdef NONUNIFORM_TEXATLAS
						pBillboardExtents = billboardLayoutMgr.Insert(
							Aftk::Vector2U32(m_pSettings->m_MinBillboardWidth, m_pSettings->m_MinBillboardHeight), it->GetKey());
						#else
						pBillboardExtents = billboardLayoutMgr.Insert(it->first);
						#endif
					}
					else // we've no space & have yet to insert a billboard
						LOGFATAL(g_Log, "billboard size is too big to fit into texture atlas");
				}

				Maths::HVec2 imageSize(m_pSettings->m_AtlasWidth, m_pSettings->m_AtlasHeight);
				m_pAftkRenderViewContext->GetView().windowCoords.SetBounds(0.f ,0.f ,1.f ,1.f);
				m_pAftkRenderViewContext->GetView().camera.SetAspectRatio((F32)m_pSettings->m_AtlasWidth/(F32)m_pSettings->m_AtlasHeight);

				Maths::HVec3 viewVec(1.0f, 0.0f, 0.0f);
				Maths::HMat33 rotateY(Maths::RotY(Maths::DegToRad((F32)m_pSettings->m_Views[iView].m_Azimuth)));
				Maths::HMat33 rotateZ(Maths::RotZ(Maths::DegToRad((F32)m_pSettings->m_Views[iView].m_Elevation)));
				viewVec = rotateY * rotateZ * viewVec;
				viewVec = viewVec * -1;

				m_pAftkRenderViewContext->GetView().camera.SetType(AftkRender::C_Camera::E_CameraType_Orthographic);
				AftkRender::Utility::FrameObjectOrthographic(m_pAftkRenderViewContext->GetView().camera, pMeshNode, viewVec);

				//Nasty horrible hack to ensure images start along the bottom edge of the billboard (on side on views).
				//This is because framing uses the mesh radius
				//if (m_pSettings->m_Views[iView].m_Elevation == 0)
				//{
				//	const Aftk::C_Bounds& bounds = pMeshNode->GetWorldBounds();
				//	F32 centreHeight = bounds.GetCentre().Y() - bounds.GetMin().Y();
				//	F32 radius = sqrt(bounds.GetObjectRadiusSquared());

				//	F32 adjust = radius - centreHeight;
				//	Maths::HVec3 newPosition = m_Views[0]->m_ViewSetup.m_ViewSetup3D.m_CameraToWorld.AW();
				//	newPosition.Y(newPosition.Y()+adjust);
				//	m_Views[0]->m_ViewSetup.m_ViewSetup3D.m_CameraToWorld.AW(newPosition);
				//}

				Render((E_Conditions)m_pSettings->m_Conditions[jCondition], m_UniqueTrees_Evergreen[i]);

				C_TreeBillboard::TreeBillboardVariation variation;
					variation.m_Azimuth = (F32)m_pSettings->m_Views[iView].m_Azimuth;
					variation.m_Elevation = (F32)m_pSettings->m_Views[iView].m_Elevation;
					variation.m_Condition = (U32)m_pSettings->m_Conditions[jCondition];
					variation.m_TextureIndex = currentTextureSet;

					variation.m_MinX = (F32)(pBillboardExtents->m_Min.X() + 1)/m_pSettings->m_AtlasWidth;
					variation.m_MinY = (F32)(pBillboardExtents->m_Min.Y() + 1)/m_pSettings->m_AtlasHeight;
					variation.m_MaxX = (F32)(pBillboardExtents->m_Max.X() - 1)/m_pSettings->m_AtlasWidth;
					variation.m_MaxY = (F32)(pBillboardExtents->m_Max.Y() - 1)/m_pSettings->m_AtlasHeight;

					Attk::C_Image capture;
					m_pAftkRenderViewContext->BeginRender();
					if (AftkRender::Utility::CaptureRenderTarget(capture))
					{
						AttkUt::C_ImageResize::Resize(capture, m_pSettings->m_MinBillboardWidth, m_pSettings->m_MinBillboardHeight, 1);
						m_pDiffuseAtlas->Insert(pBillboardExtents->m_Min.X(), pBillboardExtents->m_Min.Y(), capture);
					}
					m_pAftkRenderViewContext->EndRender(false);
					capture.Clear();
					//m_pDevice->SetColourSurface(&m_NormalMapRenderTarget);
					//if (AftkRender::Utility::CaptureRenderTarget(capture))
					//{
					//	AttkUt::C_ImageResize::Resize(capture, m_pSettings->m_MinBillboardWidth, m_pSettings->m_MinBillboardHeight, 1);
					//	m_pNormalAtlas->Insert(pBillboardExtents->m_Min.X(), pBillboardExtents->m_Min.Y(), capture);
					//}

					ClearRenderSurfaces();

				it->first->AddVariant(variation);
				nBillboardsInCurrent++;
			}
		}
		billboardSet.AddTreeBillboardRecord(*(it->first));
		it++;
		i++;
	}

	if (nBillboardsInCurrent)
		WriteRenderSurfaces(diffuseFilename, normalFilename);

	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(m_pSettings->m_DefinitionFilename, Sys::E_FileAccess_Write);
	if (pFile)
		billboardSet.Serialise(*pFile);
	else
		LOGERROR(g_Log, "Failed to open file %s for writing\n", m_pSettings->m_DefinitionFilename.Chars());

	delete pFile;


	#ifdef _DEBUG
	{
		//read it back in
		Sys::I_File *pFile2 = Sys::C_FileSystem::Instance().Open(m_pSettings->m_DefinitionFilename, Sys::E_FileAccess_Read);
		if (pFile2)
		{
			C_TreeBillboardSet set;
			bool isok = set.Deserialise(*pFile2);
		}
		delete pFile2;
	}
	#endif


}


void C_TreeRendererApp::Render(E_Conditions condition, bool isEvergreen)
{
	if (!m_pTreeRenderer)
		return;

	Display::Device()->SetView(NULL, false);
	Display::Device()->BeginScene();
	m_pAftkRenderViewContext->BeginRender();

	C_TreeRenderer::Flags renderFlags = {0,0};
	if (m_pSettings->m_Lighting == E_Lighting_None)
		renderFlags.VertexColourLighting = false;
	else if (m_pSettings->m_Lighting == E_Lighting_VertexColours)
		renderFlags.VertexColourLighting = true;

	if (condition == E_Condition_Snow && !isEvergreen)
		renderFlags.CullLeaves = true;

	m_pTreeRenderer->Update(0);
	m_pTreeRenderer->Render(renderFlags);

	Display::Device()->SetView(NULL, false);
	Display::Device()->EndScene();
	m_pAftkRenderViewContext->EndRender(false);
}


void C_TreeRendererApp::Update(const Maths::C_TimeS& timeStep)
{
}

bool C_TreeRendererApp::SetSceneNodePtr(Aftk::C_SceneNode *pSceneNode)
{
	if (m_pAftkRenderScene && m_pAftkRenderScene->Load(pSceneNode, false))
	{
		m_pAftkRenderScene->BuildMeshClusters();
		m_pAftkRenderScene->LoadTextures(-1);

		m_pTreeRenderer->Clear();
		m_pTreeRenderer->AttachSceneGraphNode(m_pAftkRenderScene->GetRootSceneGraphNodePtr());
		return true;
	}
	else
		return false;
}

bool C_TreeRendererApp::GetUniqueTrees(Aftk::C_Scene *pScene)
{
	Ut::OpenHashSet<
		C_TreeBillboard*, 
		HashKeyOpTreeKey > treeSet;

	for (Aftk::C_SceneNodeConstIterator it(pScene); it; ++it)
	{
		if ((*it).GetType() == Aftk::E_ObjType_SceneMesh)
		{
			Aftk::C_SceneMeshNode *pMeshNode = (Aftk::C_SceneMeshNode*) &(*it);

			if (!pMeshNode->GetBlindDataResourcePtr())
				continue;

			const Aftk::C_BlindDataNode& blindData = pMeshNode->GetBlindDataResourcePtr()->GetContent();
			bool hasSPTData;
			for (U32 jAttr=0; jAttr<blindData.GetNumAttributes(); ++jAttr)
			{
				if (blindData.GetAttribute(jAttr).GetName() == "SPTData")
				{
					hasSPTData = true;
					break;
				}
			}

			if (!hasSPTData)
				continue;

			C_TreeBillboard *pNewTreeKey = new C_TreeBillboard();
			if (!pNewTreeKey)
			{
				LOGFATAL(g_Log, "Failed to allocate memory for spt key\n");
				return false;
			}

			for (U32 jAttr=0; jAttr<blindData.GetNumAttributes(); ++jAttr)
			{
				const Aftk::C_GenericAttribute& attribute = blindData.GetAttribute(jAttr);
				if (attribute.GetName() == "Filename")
				{
					std::string filename;
					attribute.GetValue(filename);
					Util::C_String filename_cstr = Util::ExtractFilename(Util::C_String(filename.c_str()));
					pNewTreeKey->SetName(filename_cstr);
				}
				if (attribute.GetName() == "Seed")
				{
					S32 seed;
					attribute.GetValue(seed);
					pNewTreeKey->SetSeed(seed);
				}
				/*
				else if (attribute.GetName() == "Size")
				{
					F32 size;
					attribute.GetValue(size);
					pNewTreeKey->SetSize(size);
				}
				else if (attribute.GetName() == "SizeVariance")
				{
					F32 sizeVariance;
					attribute.GetValue(sizeVariance);
					pNewTreeKey->SetSizeVariance(sizeVariance);
				}
				*/
			}

			Ut::OpenHashSet<C_TreeBillboard*, HashKeyOpTreeKey >::ConstIterator treeSetIt = 
				treeSet.Find(pNewTreeKey);

			if (treeSetIt == treeSet.End())
			{
				treeSet.Insert(pNewTreeKey);
				//m_UniqueTrees[pNewTreeKey] = pMeshNode;
				Ut::Pair< C_TreeBillboard*, Aftk::C_SceneMeshNode*> billlboardMeshPair(pNewTreeKey, pMeshNode);
				m_UniqueTrees.PushBack(billlboardMeshPair);

				#ifdef _DEBUG
				LOGDEBUG(AftkRender::g_Log, "Adding key %s\n", pMeshNode->GetName().c_str());
				#endif
			}
			else
				delete pNewTreeKey;
		}
	}

	m_UniqueTrees_Evergreen.Clear();
	m_UniqueTrees_Evergreen.Resize(m_UniqueTrees.Size());

	Ut::UtVector< Ut::DynamicVector<Ut::Pair<C_TreeBillboard*, Aftk::C_SceneMeshNode*> > >::Iterator uniquetreeIt = m_UniqueTrees.Begin();
	U32 i=0;
	while (uniquetreeIt != m_UniqueTrees.End())
	{
		Aftk::C_SceneMeshNode* pTreeMesh = uniquetreeIt->second;
		const Aftk::C_BlindDataNode& blindData = pTreeMesh->GetBlindDataResourcePtr()->GetContent();

		for (U32 iAttr=0; iAttr<blindData.GetNumAttributes(); ++iAttr)
		{
			const Aftk::C_GenericAttribute& attribute = blindData.GetAttribute(iAttr);
			if (attribute.GetName() == "Filename")
			{
				std::string filename;
				attribute.GetValue(filename);
				filename += ".xml";

				m_UniqueTrees_Evergreen[i] = IsSpeedtreeEvergreen(filename.c_str());
			}
		}
		uniquetreeIt++;
		i++;
	}

	return true;
}

void C_TreeRendererApp::ClearUniqueTreeList()
{
	Ut::UtVector< Ut::DynamicVector<Ut::Pair<C_TreeBillboard*, Aftk::C_SceneMeshNode*> > >::Iterator it =	m_UniqueTrees.Begin();

	while (it != m_UniqueTrees.End())
	{
		delete it->first;
		++it;
	}

	m_UniqueTrees.Clear();
	m_UniqueTrees_Evergreen.Clear();
}


bool C_TreeRendererApp::IsSpeedtreeEvergreen(const char* filename)
{
	Sys::C_XmlDom dom;
	if (!dom.Read(filename))
		return false;

	Sys::C_XmlDomBaseNode* pLeavesNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "leaves");
	if (!pLeavesNode)
		return false;

	const Sys::C_XmlAttribute *pTypeAttribute = ((Sys::C_XmlDomElement*)(pLeavesNode))->GetAttributes().GetAttributeByName("type");
	if (pTypeAttribute && pTypeAttribute->value == "evergreen")
		return true;

	return false;
}

void C_TreeRendererApp::WriteRenderSurfaces(const Util::C_String& diffuseFilename, const Util::C_String& normalFilename)
{
	Attk::C_Status status;
	LOGINFO(g_Log, "Writing %s\n",diffuseFilename.Chars());
	#ifndef _DEBUG
	{
		Attk::C_Image floodfilledImage;
		status = AttkUt::C_FloodFill::FloodFill(floodfilledImage, *m_pDiffuseAtlas);
		if (!status)
			LOGERROR(g_Log, "Failed to perform flood filling operation of diffuse atlas");

		status = floodfilledImage.Write(diffuseFilename.Chars());
	}
	#else
	status = m_pDiffuseAtlas->Write(diffuseFilename.Chars());
	#endif
	if (!status)
		LOGERROR(g_Log, "Failed to create diffuse texture atlas\n");

	//LOGINFO(g_Log, "Writing %s\n",normalFilename.Chars());
	//status = m_pNormalAtlas->Write(normalFilename.Chars());
	//if (!status)
	//	LOGERROR(g_Log, "Failed to create normal map texture atlas\n");

	m_pDiffuseAtlas->Fill(Attk::C_PixelR32G32B32A32F(0.f, 0.f, 0.f, 0.f));
	//m_pNormalAtlas->Fill(Attk::C_PixelR32G32B32A32F(0.5f, 0.5f, 0.5f, 1.f));

}


void C_TreeRendererApp::ClearRenderSurfaces()
{
	m_pAftkRenderViewContext->BeginRender();
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 0, 0, 0));
	m_pAftkRenderViewContext->EndRender(false);
}