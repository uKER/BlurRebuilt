
#include "stdafx.h"

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/VertexBuffer.h>

#include <scene_renderer/ObjectRenderingFragment.h>
#include <scene_renderer/ObjectRenderingBatch.h>
#include <scene_renderer/ShaderProperties.h>
#include <scene_renderer/LightmapShaderProperties.h>

#include <scene_renderer/SceneRenderer.h>
#include <scene_renderer/GenericSceneRenderer.h>
#include <scene_renderer/VertexFormats.h>

#include <scene_renderer/D3dImageLoader.h>
#include <scene_renderer/D3dTextureManager.h>
#include <scene_renderer/D3dShaderManager.h>
#include <scene_renderer/GenericShaderOperations.h>

#include <scene_renderer/ViewGrid.h>
#include <lux_common/SamplePoint.h>

// TODO: refactor this
extern int gnSamplePoints;
void ComputeSHIrradEnvMapConstants( ID3DXEffect* pShader, float* pSHCoeffsRed, float* pSHCoeffsGreen, float* pSHCoeffsBlue );

namespace LUX
{
	bool DoesNodeNeedProcessing(Aftk::C_SceneNode *pNode);
}

const std::string g_lightmapShaderName = "IDR_LIGHTMAP_SHADER";

using namespace SRD;

// temporary debug drawing code
struct LineVertex
{
	float3 vPos;
	DWORD iColour;
};
extern int gnLines;
extern float3 gvLines[];
int gnCurLines = 0;
const DWORD gLineFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
D3DVertexBuffer *gpLineVertexBuffer;

GenericSceneRenderer::GenericSceneRenderer(D3dWindow* pD3dWindow, const Aftk::C_Scene* pScene) 
	: SceneRenderer(pD3dWindow, pScene)
{
	m_pRenderableObjects = 0;
	m_numRenderableNodes = 0;
	m_settings.bViewLightPaths = false;
	m_settings.bRealtimeLighting = false;
	m_settings.bViewSamplePoints = false;
	m_pSphereMesh = NULL;
	gbDisplaySamplePoints = true;
}

GenericSceneRenderer::~GenericSceneRenderer()
{
	Destroy();
}

bool GenericSceneRenderer::Create()
{
	m_pViewGrid = new CViewGrid;
	m_pSphereMesh = NULL;

	return true;
}

void GenericSceneRenderer::Destroy()
{
	Clear();
	Reset();

	if (m_pViewGrid)
	{
		delete m_pViewGrid;
		m_pViewGrid = NULL;
	}

	if (m_pSphereMesh)
	{
		m_pSphereMesh->Release();
		m_pSphereMesh = NULL;
	}
}

bool GenericSceneRenderer::Clear()
{
	if (m_pRenderableObjects != 0)
	{
		delete[] m_pRenderableObjects;
	}

	m_fragmentArrays.Clear();
	m_vertexBuffersBuiltFlag = false;

	//Loop thru all of our textures and unload them.
	{
		SR_D3D_TEXTURE_MANAGER.Clear();
		SR_D3D_SHADER_MANAGER.Clear();
	}

	//Delete the shader properties we allocated
	std::map<const Aftk::C_MaterialResource* , ShaderProperties*>::iterator it;
	for (it = m_shaderMapping.begin(); it != m_shaderMapping.end(); it++)
	{
		ShaderProperties* pShader = it->second;
		delete pShader;
	}

	m_fragmentArrayIndices.clear();
	m_shaderMapping.clear();

	m_vertexBuffersBuiltFlag = false;
	m_pRenderableObjects = 0;
	m_numRenderableNodes = 0;
	m_fragmentArrays.Clear();
	gnCurLines = 0;

	if (gpLineVertexBuffer)
	{
		gpLineVertexBuffer->Release();
		gpLineVertexBuffer = NULL;
	}
	return true;
}

bool GenericSceneRenderer::OnResetDevice()
{
	{
		std::map<ID3DXEffect* , Ut::UtVector< Ut::DynamicVector<U32> > >::iterator it;

		for (it = m_fragmentArrayIndices.begin(); it != m_fragmentArrayIndices.end(); it++)
		{
			ID3DXEffect* pShader = it->first;
			//D3DXH handle = pShader->GetTechniqueByName( "RenderSceneWithTexture1Light" );

			if (FAILED(pShader->OnLostDevice()))
				return false;
			if (FAILED(pShader->OnResetDevice()))
				return false;

			pShader = LoadAndInitialiseShader(g_lightmapShaderName);
		}
	}

	return true;
}

// check whether any materials in the scene are lightmaps
bool GenericSceneRenderer::DoesSceneUseLightmaps()
{
	U32 materialCount = m_pScene->GetNumMaterialNodes();
	for (U32 i=0; i<materialCount; i++)
	{
		if (materialCount > 0)
		{
			const Aftk::C_MaterialNode* pMaterialNode = &m_pScene->GetMaterialResourcePtr(i)->GetContent();
			for (U32 iMap=0; iMap<pMaterialNode->GetNumShaderImageNodes(); iMap++)
			{
				std::string imageType;
				pMaterialNode->GetShaderImageNodePtr(iMap)->GetImageTypePlug().GetValue(imageType);
				if (!strcmp(imageType.c_str(),"lightmap"))
					return true;
			}
		}
	}
	return false;
}

void GenericSceneRenderer::OnUpdateDevice()
{
	if (m_pCamera == 0)
	{
		return;
	}

	// draw the background grid
	if (m_pViewGrid)
	{
		if (!m_pViewGrid->m_bInitialized)
		{
			m_pViewGrid->Initialize(m_pCamera->m_pD3dWindow->GetD3dDevicePtr());
		}
		m_pViewGrid->Render(m_pCamera);
	}

	D3DXMATRIX cameraViewMatrix;
	D3DXMATRIX cameraProjectionMatrix;
	D3DXMATRIX cameraWorldMatrix;
	// Get the projection & view matrix from the camera class
	m_pCamera->GetViewMatrix(cameraViewMatrix);
	m_pCamera->GetProjectionMatrix(cameraProjectionMatrix);
	m_pCamera->GetWorldMatrix(cameraWorldMatrix);

    D3DXMATRIX mWorldViewProjection = cameraViewMatrix * cameraProjectionMatrix;
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	Position3 camOrigin = m_pCamera->GetPosition();
	identity._41 = camOrigin[0];
	identity._42 = camOrigin[1];
	identity._43 = camOrigin[2];

	if (m_vertexBuffersBuiltFlag == false)
	{
		RebuildVertexBuffers();
		m_vertexBuffersBuiltFlag = true;
	}

	std::map<ID3DXEffect* , Ut::UtVector< Ut::DynamicVector<U32> > >::iterator it;

	ID3DXEffect* pShader;

	for (it = m_fragmentArrayIndices.begin(); it != m_fragmentArrayIndices.end(); it++)
	{
	    //Loop thru all of the fragments, ordered by the shader to be used for their material
		pShader = it->first;
		if (pShader == 0)
			continue;

		//Todo, determine what shader type this is here.

		Ut::UtVector< Ut::DynamicVector<U32> >* pFragmentArrayIndices = &(it->second);

		//Start using the shader
		U32 cPasses;
		// Apply the technique contained in the effect 

		bool bUsesLightmaps = false;

		// 
		bUsesLightmaps = DoesSceneUseLightmaps();

//		pShader->SetTexture("g_BumpTexture", SR_D3D_TEXTURE_MANAGER.Load("c:\\temp\\lpvs_bumptex.dds") );

		// switch shaders depending on whether we are using lightmaps
		if (m_settings.bRealtimeLighting && !bUsesLightmaps && m_settings.iLightmapType == 0)
			pShader->SetTechnique( "RenderSceneWithTexture1Light" );
		else
		{
			// render with lighting coming from the lightmaps - select one of 3 different lightmap modes
			char buffer[512];
			sprintf(buffer,"RenderSceneMode%d",m_settings.iLightmapType);
			pShader->SetTechnique( buffer );
		}			

		pShader->Begin(&cPasses, 0);
		for (U32 iPass = 0; iPass < cPasses; iPass++)
		{
#if (D3D_SDK_VERSION == 31)
			pShader->Pass(iPass);
#else
			pShader->BeginPass(iPass);
#endif

			//Everything pointed at by the fragmentArrayIndices uses the same shader
			for (U32 i = 0; i < (U32)pFragmentArrayIndices->Size(); i++)
			{
				//Loop thru all of the fragment arrays [it->second] that use this shader [it->first]
				U32 fragmentArrayIndex = (*pFragmentArrayIndices)[i];

				if (m_fragmentArrays[fragmentArrayIndex].Size() == 0)
					continue;

				//Update the properties of this shader
				const Aftk::C_MaterialResource* pMaterial = 
					m_fragmentArrays[fragmentArrayIndex][0].GetMaterialResourcePtr();

				ShaderProperties* pShaderProperties = 
					m_shaderMapping[pMaterial];
                
				//If lightmap shader ...
				{
					LightmapShaderProperties* pLmProperties = 
						(LightmapShaderProperties*)pShaderProperties;
					//Update lightmap shader parameters
					UpdateGenericLightmapShader(
						pLmProperties,
						cameraViewMatrix,
						mWorldViewProjection,
						identity);

#if (D3D_SDK_VERSION != 31)
					pShader->CommitChanges();
#endif
				}

				for (U32 j = 0; j < (U32)m_fragmentArrays[fragmentArrayIndex].Size(); j++)
				{
					m_fragmentArrays[fragmentArrayIndex][j].OnUpdateDevice();
				}
			}
		
#if (D3D_SDK_VERSION != 31)
			pShader->EndPass();
#endif
		}

		//Finish using this shader
		pShader->End();
	}

	IDirect3DDevice9 *pD3DDevice = m_pCamera->m_pD3dWindow->GetD3dDevicePtr();

	if (m_settings.bViewSamplePoints)
	{
		if (!m_pSphereMesh)
		{
			D3DXCreateSphere(pD3DDevice, 0.04f, 20, 20, &m_pSphereMesh, NULL);
		}

		if (m_pSphereMesh)
		{
			D3DXMATRIX worldMatrix, tmpmat;

			// update the device - if we fail then return the error code
			pD3DDevice->SetTransform(D3DTS_VIEW,&cameraViewMatrix);
			pD3DDevice->SetTransform(D3DTS_PROJECTION,&cameraProjectionMatrix);

			for (int i=0; i<gSamplePoints.Size(); i++)
			{
				SamplePoint *p = &gSamplePoints[i];

				D3DXMatrixTranslation(&worldMatrix, p->m_vPos.x, p->m_vPos.y, p->m_vPos.z);
//				D3DXMatrixScaling(&tmpmat, cameraViewMatrix._43, cameraViewMatrix._43, cameraViewMatrix._43);

				float fScale = 5.f;

				D3DXMatrixScaling(&tmpmat, fScale, fScale, fScale);
				D3DXMatrixMultiply(&worldMatrix, &tmpmat,	&worldMatrix);
				D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &cameraViewMatrix);
				D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &cameraProjectionMatrix);

				pShader->SetTechnique( "RenderWithSHIrradEnvMap" );
				pShader->SetMatrix( "g_mWorldViewProjection", &worldMatrix );

				// set shader
				ComputeSHIrradEnvMapConstants(pShader, p->m_fSH_R, p->m_fSH_G, p->m_fSH_B);

				UINT cPasses;
				pShader->Begin(&cPasses, 0);
				for (U32 iPass = 0; iPass < cPasses; iPass++)
				{
					pShader->BeginPass(iPass);
					m_pSphereMesh->DrawSubset(0);
					pShader->EndPass();
				}
				pShader->End();
			}

		}
	}


	// optionally draw the lighting paths as purple lines
	if (m_settings.bViewLightPaths)
	{
		D3DXMATRIX worldMatrix;

		D3DXMatrixIdentity(&worldMatrix);

		// update the device - if we fail then return the error code
		pD3DDevice->SetTransform(D3DTS_VIEW,&cameraViewMatrix);
		pD3DDevice->SetTransform(D3DTS_PROJECTION,&cameraProjectionMatrix);
		pD3DDevice->SetTransform(D3DTS_WORLD,&worldMatrix);

		// draw debugging stuff
		if (gnLines!=gnCurLines)
		{
			LineVertex *pVertices = NULL;

			if (gpLineVertexBuffer)
				gpLineVertexBuffer->Release();

			pD3DDevice->CreateVertexBuffer( gnLines * sizeof(LineVertex),
							0, gLineFVF,
							D3DPOOL_MANAGED, &gpLineVertexBuffer, NULL );

			gpLineVertexBuffer->Lock( 0, sizeof(LineVertex) * gnLines, (void**)&pVertices, 0 );

			for (int i=0; i<gnLines; i++)
			{
				pVertices[i].vPos = gvLines[i];
				pVertices[i].iColour = 0xff00ff;
			}

			gpLineVertexBuffer->Unlock();
			gnCurLines = gnLines;
		}

		if (gnCurLines)
		{
			// set the right rendermodes
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,	D3DTOP_SELECTARG2 );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );

			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );

			pD3DDevice->SetStreamSource(0, gpLineVertexBuffer, 0, sizeof(LineVertex));
			pD3DDevice->SetFVF(gLineFVF);

			pD3DDevice->DrawPrimitive(D3DPT_LINELIST, 0, gnLines/2);
		}
	}
}

bool GenericSceneRenderer::BuildVertexBuffers()
{
	//This map is used to organise object fragments by material for the entire scene. From this map,
	//a conventional 2D array is later created to store this information. This is in order to provide
	//more efficient iteration over the data to be rendered.

	std::map< 
		const Aftk::C_MaterialResource*, 
		Ut::UtVector< Ut::DynamicVector<ObjectRenderingFragment > > >	fragmentArraysByMaterial;

	m_numRenderableNodes = 0;
	//Calculate the number of renderable objects in the scene
	{
		Aftk::C_SceneNodeConstIterator it(m_pScene->GetRootSceneNodePtr());
		while (it) 
		{
			const Aftk::C_SceneNode* pNode = it;
			switch (pNode->GetType()) 
			{
				case Aftk::E_ObjType_SceneMesh:
				{
					m_numRenderableNodes++;
				}
				default:
					break;
			}
			it++;
		}
	}

	if (m_pRenderableObjects != 0)
	{
		delete m_pRenderableObjects;
	}
	m_pRenderableObjects = new ObjectRenderingBatch[m_numRenderableNodes];

	U32 currentRenderBatchIndex = 0;
	Aftk::C_SceneNodeConstIterator it(m_pScene->GetRootSceneNodePtr());
	while (it) 
	{
		const Aftk::C_SceneNode* pNode = it;
		switch (pNode->GetType()) 
		{
			case Aftk::E_ObjType_SceneMesh:
			{
				Aftk::C_SceneMeshNode* pMeshInterface = 
					(Aftk::C_SceneMeshNode*)pNode;

				if (!LUX::DoesNodeNeedProcessing((Aftk::C_SceneNode*)pNode))
					break;

				U32 numVPos = pMeshInterface->GetNumVertexPositions();
#ifdef KEN
				{
					char buf[1024];
					_snprintf(buf, sizeof(buf), "Adding mesh with %i vertex positions\r\n", numVPos );
					OutputDebugString(buf);
				}
#endif


				m_pRenderableObjects[currentRenderBatchIndex].SetD3dWindowPtr(m_pD3dWindow);
				m_pRenderableObjects[currentRenderBatchIndex].SetAftkScenePtr(m_pScene);
				m_pRenderableObjects[currentRenderBatchIndex].SetAftkSceneNodePtr(pMeshInterface);
				if (!m_pRenderableObjects[currentRenderBatchIndex].RebuildVertexBuffers())
				{
					assert(false);
					return false;
				}
				ExtractObjectRenderingFragments(currentRenderBatchIndex, fragmentArraysByMaterial);
				currentRenderBatchIndex++;
				break;
			}
			default:
				break;
		}
		it++;
	}

	{
		//Okay, now we need to convert our map of fragments into a simple 2D array for efficiency.
		std::map< 
			const Aftk::C_MaterialResource*, 
			Ut::UtVector< Ut::DynamicVector< ObjectRenderingFragment > > >::iterator fragIt;

		U32 arrayIndex = 0;
		for (fragIt = fragmentArraysByMaterial.begin(); fragIt != fragmentArraysByMaterial.end(); fragIt++)
		{
			Ut::UtVector< Ut::DynamicVector< ObjectRenderingFragment > >* pArray = 
				&(fragIt->second);
			m_fragmentArrays.PushBack();
			m_fragmentArrays[arrayIndex] = *pArray;
#ifdef KEN
			char buf[1024];
			_snprintf(buf, sizeof(buf), "Fragment array %i num fragments : %i\r\n", arrayIndex, pArray->Size() );
			OutputDebugString(buf);
#endif

			arrayIndex++;
		}
	}

	//Finally, build our AFTK material -> real-time shader mapping table.
	if (!BuildMaterialShaderMapping())
		return false;

	return true;
}

void GenericSceneRenderer::ExtractObjectRenderingFragments(
	U32 currentRenderBatchIndex,
	std::map< 
		const Aftk::C_MaterialResource*, 
		Ut::UtVector< Ut::DynamicVector<ObjectRenderingFragment > > >&	fragmentArraysByMaterial)
{
	//This function extracts all of the rendering fragments from the given object rendering batch.
	//A fragment is all of the polys of an object that sharing the same material.
	//A fragment consists of a key (the material), and a value (an array of vertex buffer indices).

	ObjectRenderingBatch* pObjectRenderingBatch = &(m_pRenderableObjects[currentRenderBatchIndex]);

	//Loop through all the vertex buffer index arrays, which are grouped by material.
	for (U32 i = 0; i < pObjectRenderingBatch->GetNumVertexBufferIndexArrays(); i++)
	{
		//Okay, this is the material that we're currently interested in.
		const Aftk::C_MaterialResource* pMaterialResource = 
			pObjectRenderingBatch->GetMaterialResourcePtrFromInstance(i);

		//We now need to add a new entry into our fragment map.
		std::map< 
			const Aftk::C_MaterialResource*, 
			Ut::UtVector< Ut::DynamicVector< ObjectRenderingFragment > > >::iterator it;
		it = fragmentArraysByMaterial.find(pMaterialResource);

		//Our new object rendering fragment
		ObjectRenderingFragment* pRenderingFragment;
		Ut::UtVector< Ut::DynamicVector< ObjectRenderingFragment > >* pArray;

		if (it == fragmentArraysByMaterial.end())
		{
			//We couldn't find an existing instance of this material. Create a new entry in our
			//fragment list.
			fragmentArraysByMaterial[pMaterialResource] = 
				Ut::UtVector< Ut::DynamicVector< ObjectRenderingFragment > >();
			pArray = &(fragmentArraysByMaterial[pMaterialResource]);
		}
		else
		{
			//We found an existing instance of this material. Add this polygon index to the material's 
			//associated array of polygon indices.
			pArray = &(fragmentArraysByMaterial[pMaterialResource]);
		}
		//Add space for our new fragment
		pArray->PushBack();
		U32 fragmentIndex = pArray->Size()-1;
		//Get a pointer to where the fragement is stored in our array.
		pRenderingFragment = &((*pArray)[fragmentIndex]);
        
		//Copy vertex buffer indices into our rendering fragment
		U32 numVertexBufferIndices = pObjectRenderingBatch->GetNumVertexBufferIndices(pMaterialResource);
		for (U32 j = 0; j < numVertexBufferIndices; j++)
		{
			pRenderingFragment->AddVertexBufferIndex(
				pObjectRenderingBatch->GetVertexBufferIndex(pMaterialResource, j)
				);
		}

		//Attach fragment to window
		pRenderingFragment->SetD3dWindowPtr(m_pD3dWindow);
		//Set rendering fragment's material resource pointer
		pRenderingFragment->SetMaterialResourcePtr(pMaterialResource);
		//Attach this fragment to the batch object it comes from
		pRenderingFragment->SetObjectRenderingBatchPtr(pObjectRenderingBatch);

		//Okay, we're done with this fragment! All necessary information has been added.
	}
}

bool GenericSceneRenderer::RebuildVertexBuffers()
{
	if (m_vertexBuffersBuiltFlag == false)
	{
		return BuildVertexBuffers();
	}
	else
	{
		m_vertexBuffersBuiltFlag = false;
		for (U32 i = 0; i < m_numRenderableNodes; i++)
		{
			if (!m_pRenderableObjects[i].RebuildVertexBuffers())
			{
				return false;
			}
		}
	}

	if (!BuildMaterialShaderMapping())
	{
		return false;
	}

	m_vertexBuffersBuiltFlag = true;
	return true;
}

bool GenericSceneRenderer::BuildMaterialShaderMapping()
{
	/*
	{
		//Delete the shader properties we allocated
		std::map<const Aftk::C_MaterialResource* , ShaderProperties*>::iterator it;
		for (it = m_shaderMapping.begin(); it != m_shaderMapping.end(); it++)
		{
			ShaderProperties* pShader = it->second;
			delete pShader;
		}
		m_shaderMapping.clear();
		//Loop thru all of our textures and unload them.
		{
			SR_D3D_TEXTURE_MANAGER.Clear();
			SR_D3D_SHADER_MANAGER.Clear();
		}
	}
*/
	//For each material that is _actually_used_ within our scene, find the most appropriate
	//real-time shader.

	for (U32 i = 0; i < (U32)m_fragmentArrays.Size(); i++)
	{
		if (m_fragmentArrays[i].Size() > 0)
		{
			const Aftk::C_MaterialResource* pMaterial = 
				m_fragmentArrays[i][0].GetMaterialResourcePtr();

			//What is the most appropriate shader for pMaterial?
			ID3DXEffect* pShader;
			//if (...
			{			
				//At the moment, it is always our default lightmap shader
				LightmapShaderProperties* pLmProperties = new LightmapShaderProperties;

				pShader = LoadAndInitialiseShader("e:/devroot/code/tools/lux/scene_renderer/Shaders/lightmap.fx");
				if (!pShader)
					pShader = LoadAndInitialiseShader(g_lightmapShaderName);

				pLmProperties->SetShaderPtr(pShader);

				if (pMaterial != 0)
				{
					//Copy relevant shader properties from aftk (if we have a material that is).
					ExtractLightmapShaderPropertiesFromAftk(&pMaterial->GetContent(), pLmProperties);
				}

				m_shaderMapping[pMaterial] = pLmProperties;
			}

			if (pShader == 0)
			{
				assert(0); // couldn't load shader! check shader path "d:/subversion..." above
				return false;
			}

			//Now add indices into our fragment array for this shader.
			std::map< 
				ID3DXEffect* , Ut::UtVector< Ut::DynamicVector<U32> > >::iterator it;
			it = m_fragmentArrayIndices.find(pShader);

			if (it == m_fragmentArrayIndices.end())
			{
				m_fragmentArrayIndices[pShader] = Ut::UtVector< Ut::DynamicVector< U32 > >();
				m_fragmentArrayIndices[pShader].PushBack(i);
			}
			else
			{
				m_fragmentArrayIndices[pShader].PushBack(i);
			}
		}
	}

	return true;
}

ID3DXEffect* GenericSceneRenderer::LoadAndInitialiseShader(const std::string& rcName)
{
	U32 numShadersStart = SR_D3D_SHADER_MANAGER.GetNumShaders();
	//For the moment, we assume that the only shader we have is the simple lightmap shader.
	ID3DXEffect* pShader = SR_D3D_SHADER_MANAGER.Load(rcName);
	if (pShader == 0)
	{
		return 0;
	}

	U32 numShadersEnd = SR_D3D_SHADER_MANAGER.GetNumShaders();

	if (numShadersEnd > numShadersStart)
	{
		//We actually loaded in a new shader, we must now initialise it.
		if (!InitialiseGenericLightmapShader(pShader))
		{
			//Print warning here if we couldn't initialise our shader
		}
	}

	return pShader;
}

void GenericSceneRenderer::Reset()
{
	SceneRenderer::Reset();

	m_pRenderableObjects = 0;
	m_numRenderableNodes = 0;
	m_fragmentArrays.Clear();
}




void ComputeSHIrradEnvMapConstants( ID3DXEffect* pShader, float* pSHCoeffsRed, float* pSHCoeffsGreen, float* pSHCoeffsBlue )
{ 
    float* fLight[3] = { pSHCoeffsRed, pSHCoeffsGreen, pSHCoeffsBlue };

    // Lighting environment coefficients
    D3DXVECTOR4 vCoefficients[3];

	// (3rd order SH)

    // These constants are described in the article by Peter-Pike Sloan titled 
    // "Efficient Evaluation of Irradiance Environment Maps" in the book 
    // "ShaderX 2 - Shader Programming Tips and Tricks" by Wolfgang F. Engel.
    static const float s_fSqrtPI = ((float)sqrtf(D3DX_PI));
    const float fC0 = 1.0f/(2.0f*s_fSqrtPI);
    const float fC1 = (float)sqrt(3.0f)/(3.0f*s_fSqrtPI);
    const float fC2 = (float)sqrt(15.0f)/(8.0f*s_fSqrtPI);
    const float fC3 = (float)sqrt(5.0f)/(16.0f*s_fSqrtPI);
    const float fC4 = 0.5f*fC2;

	int iChannel;
    for( iChannel=0; iChannel<3; iChannel++ )
    {
        vCoefficients[iChannel].x = -fC1*fLight[iChannel][3];
        vCoefficients[iChannel].y = -fC1*fLight[iChannel][1];
        vCoefficients[iChannel].z =  fC1*fLight[iChannel][2];
        vCoefficients[iChannel].w =  fC0*fLight[iChannel][0] - fC3*fLight[iChannel][6];
    }

    pShader->SetVector( "cAr", &vCoefficients[0] );
    pShader->SetVector( "cAg", &vCoefficients[1] );
    pShader->SetVector( "cAb", &vCoefficients[2] );

    for( iChannel=0; iChannel<3; iChannel++ )
    {
        vCoefficients[iChannel].x =      fC2*fLight[iChannel][4];
        vCoefficients[iChannel].y =     -fC2*fLight[iChannel][5];
        vCoefficients[iChannel].z = 3.0f*fC3*fLight[iChannel][6];
        vCoefficients[iChannel].w =     -fC2*fLight[iChannel][7];
    }

    pShader->SetVector( "cBr", &vCoefficients[0] );
    pShader->SetVector( "cBg", &vCoefficients[1] );
    pShader->SetVector( "cBb", &vCoefficients[2] );

    vCoefficients[0].x = fC4*fLight[0][8];
    vCoefficients[0].y = fC4*fLight[1][8];
    vCoefficients[0].z = fC4*fLight[2][8];
    vCoefficients[0].w = 1.0f;

    pShader->SetVector( "cC", &vCoefficients[0] );
}
