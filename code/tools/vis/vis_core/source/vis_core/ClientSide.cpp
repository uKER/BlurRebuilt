#include <stdafx.h>

#include <vis_core/Vis.h>
#include <vis_core/FaceFlags.h>
#include <vis_core/Scene.h>
#include <vis_core/Mesh.h>
#include <vis_core/ClientSide.h>
#include <vis_core/D3DRenderer.h>
#include <vis_core/D3DMesh.h>
#include <vis_core/SampleArea.h>
#include <vis_core/PVSResult.h>
#include <vis_core/Frustum.h>

bool gbExitIfNoD3D = true;

IMPLEMENT_PERSISTENCE(ClientSide,"ClientSide");

ClientSide::ClientSide()
{
	m_pVS2Data = NULL;
}

ClientSide::~ClientSide()
{
	if (m_pVS2Data) delete m_pVS2Data;
}

void ClientSide::SetScene(Scene *pScene)
{
	m_pScene = pScene;
}

void ClientSide::SetVS2(Ut::Vector<BYTE> *pVS2Data)
{
    m_pVS2Data = pVS2Data;
}

// initialize the client side
void ClientSide::Initialize()
{
	CD3DRenderer::Initialize(SD3DRI(1024,1024,1024));
	m_pD3DR = CD3DRenderer::m_pInterface;

	if (!m_pD3DR->m_pD3D)
	{
		LOGWARN( g_Log, "\n         It is not possible to calculate PVS on this machine\n");
		if (gbExitIfNoD3D)
		{
			LOGWARN( g_Log, "         This program will now exit. \n");
			exit(-1);
		}
		else
		{
			LOGWARN( g_Log, "         Waiting for other PCs to complete the task. Press CTRL+C to exit.\n");
		}
		m_bInitialized = false;
		return;
	}
	m_bInitialized = true;

	int nVertices = 0;
	int nFaces = 0;

	// create a D3D Mesh for each scene meshnode
	for (int i=0; i<m_pScene->GetNumNodes(); i++)
	{
		m_D3DMeshList.PushBack(new CD3DMesh);
		m_D3DMeshList[i]->CreateFromMesh(m_pScene->GetNode(i));

		nVertices += m_pScene->GetNode(i)->GetVertices().Size();
		nFaces += m_pScene->GetNode(i)->GetIndices().Size();

		m_pScene->GetNode(i)->DeleteBuffers();
	}

	// create a D3D mesh for each base instance
	for (int i=0; i<m_pScene->GetNumBaseInstances(); i++)
	{
		m_D3DBaseInstanceMeshList.PushBack(new CD3DMesh);
		m_D3DMeshList[i]->CreateFromMesh(m_pScene->GetBaseInstance(i));

		nVertices += m_pScene->GetBaseInstance(i)->GetVertices().Size();
		nFaces += m_pScene->GetBaseInstance(i)->GetIndices().Size();

		m_pScene->GetBaseInstance(i)->DeleteBuffers();
	}

//	printf("vertices: %d\nfaces: %d\n",nVertices,nFaces/3);

	// build bounding box vertex buffer
	m_pBBoxMesh = new CD3DMesh;

	ID3DXMesh* pD3DXMesh = 0;
	D3DXCreateBox(
		CD3DRenderer::m_pInterface->m_pD3DDevice, 
		1,
		1,
		1, 
		&pD3DXMesh, 
		NULL);
	 m_pBBoxMesh->SetD3DXMesh(pD3DXMesh);

	// initialize occlusion query
	if ( D3DERR_NOTAVAILABLE == CD3DRenderer::m_pInterface->m_pD3DDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, &m_pD3DQuery ) )
	{
		LOGERROR( g_Log, "ERROR: Self test failed. Your graphics card does not support occlusion\n"
				         "       queries. The program will now exit.\n");
		exit(-1);
	}

	m_Timer = GetTickCount();
}

// clean up the client before exit
void ClientSide::CleanUp()
{
	m_Timer = GetTickCount() - m_Timer;
	LOGINFO( g_Log, "\n");
	LOGINFO( g_Log, "Calculated %d samples in %5.2f seconds - %5.2f frames per second\n",gnTotalSamples, (float)m_Timer/1000.0f, (float)gnTotalSamples*1000.0f/(float)m_Timer);

	if (!m_bInitialized)
		return;

	m_pD3DR->Destroy();
	for (int i=0; i<m_D3DMeshList.Size(); i++)
		delete m_D3DMeshList[i];
	m_D3DMeshList.Destroy();
	delete m_pScene;
	delete m_pBBoxMesh;
}


// process a job
BaseObject *ClientSide::Process(BaseObject *pJob)
{
	if (!m_bInitialized)
		return NULL;

	SampleArea *pArea = (SampleArea*)pJob;

	m_pResult = new PVSResult(m_pScene->GetNumIds());
	Frustum frustum;

	m_pVisCount = new U32[m_pScene->GetNumIds()];
	m_pClusterVisCount = new U32[m_pScene->GetNumClusterIds()];

	// calculate visibility for this sample cell
	int nSamples = pArea->GetNumSamples();
	for (int iSample = 0; iSample < nSamples; iSample++)
	{
		float3 vPos;
		float3 vDir;

		gnTotalSamples++;

		if (!pArea->GetSample( iSample, vPos, vDir ))
			continue;	

		// render this sample
		for (int iPass=0; iPass<m_pScene->m_Passes.Size(); iPass++)
		{
			memset(m_pVisCount, 0, sizeof(U32) * m_pScene->GetNumIds());
			memset(m_pClusterVisCount, 0, sizeof(U32) * m_pScene->GetNumClusterIds());

			m_iCurrentPass = iPass;
			m_pD3DR->BeginRenderSceneToAtlas(0);
			m_pD3DR->SetCamera(vPos, vDir, &frustum);

			Render(frustum);

			m_pD3DR->m_pD3DDevice->EndScene();

			// get framebuffer and analyze data
			U32 *pData = (U32*)m_pD3DR->GetFrameBuffer();
			int c = m_pD3DR->m_iWidth * m_pD3DR->m_iHeight;
			for (int i=0; i<c; i++)
			{
				int x = pData[i] & 0x00ffffff;
				if (x == 0x00ffffff)
					continue;
				IncVisCount(x, 1);
			}

			if (gbDebugMode)
			{
				// flip texture upside down
				U32 *pTex = new U32[m_pD3DR->m_iWidth * m_pD3DR->m_iHeight];
				for (int i=0; i<m_pD3DR->m_iHeight; i++)
					memcpy(pTex + m_pD3DR->m_iWidth *i, (int*)pData + m_pD3DR->m_iWidth * (m_pD3DR->m_iHeight-1-i), sizeof(int)*m_pD3DR->m_iWidth);

				char buffer[512];
				sprintf(buffer,"c:/temp/testpics/pic-%d-p%d.bmp",iSample, iPass);
				SaveTexture(buffer, pTex, m_pD3DR->m_iWidth, m_pD3DR->m_iHeight, D3DFMT_A8R8G8B8, true);

				sprintf(buffer,"c:/temp/testpics/pic-%d-p%d_cluster.bmp",iSample, iPass);
				//colourise using cluster index
				for (int i=0; i<c; i++)
				{
					int x = pTex[i] & 0x00ffffff;
					if (x == 0x00ffffff)
						continue;
					
					x = m_pScene->GetClusterRef(x);
					pTex[i] = x & 0x00ffffff;
				}
				SaveTexture(buffer, pTex, m_pD3DR->m_iWidth, m_pD3DR->m_iHeight, D3DFMT_A8R8G8B8, true);
				delete[] pTex;
			}

			delete [] pData;

			m_pD3DR->m_pD3DDevice->BeginScene();
			ProcessInstances(m_pResult, &frustum, vPos);
			m_pD3DR->EndRenderScene();

			FlushResults();

#if 0
			{
				U32 *pData = (U32*)m_pD3DR->GetFrameBuffer();
				U32 *pTex = new U32[m_pD3DR->m_iWidth * m_pD3DR->m_iHeight];
				for (int i=0; i<m_pD3DR->m_iHeight; i++)
					memcpy(pTex + m_pD3DR->m_iWidth *i, (int*)pData + m_pD3DR->m_iWidth * (m_pD3DR->m_iHeight-1-i), sizeof(int)*m_pD3DR->m_iWidth);

				char buffer[512];
				sprintf(buffer,"c:/temp/testpics/pic-%d-p%d_culled.bmp",iSample, iPass);
				for (int i=0; i<c; i++)
				{
					int x = pTex[i] & 0x00ffffff;
					if (x == 0x00ffffff)
						continue;
					
					if (!m_pResult->GetBit(x))
						pTex[i] = 0xffffffff;
				}
				SaveTexture(buffer, pTex, m_pD3DR->m_iWidth, m_pD3DR->m_iHeight, D3DFMT_A8R8G8B8, true);
				delete[] pTex;
				delete[] pData;
			}
#endif

		}
	}

	delete [] m_pVisCount;
	delete [] m_pClusterVisCount;

	// return result
	return m_pResult;
}

void ClientSide::IncVisCount(int iID, int iAmount)
{
	assert(iID>=0 && iID<m_pScene->GetNumIds());
	int icID = m_pScene->GetClusterRef(iID);

	m_pVisCount[iID] += iAmount;
	m_pClusterVisCount[icID] += iAmount;
}

void ClientSide::FlushResults()
{
	const int nClusters = (int) m_pScene->GetNumClusterIds();
	const int nPrimitives = (int) m_pScene->GetNumIds();
	const U32 clusterThreshold = (U32)m_pScene->GetClusterThreshold();
	const U32 polygonThreshold = (U32)m_pScene->GetPolygonThreshold();

	for (int p=0; p<nPrimitives; ++p)
	{
		if (m_pVisCount[p] >= polygonThreshold)
		{
			U32 c = m_pScene->GetClusterRef(p);
			if (m_pClusterVisCount[c] >= clusterThreshold)
				m_pResult->SetBit(p);
		}
	}
}

void ClientSide::TestAndSaveVisibilityOfInstanceBatch(PVSResult *pResult, Ut::Vector<int> &list)
{
	// test visibility of the whole group in one go
	if (TestInstanceBatch(&list[0], list.Size()))
		return;

	// if some of them are visible, test each one individually
	for (int i=0; i<list.Size(); i++)
	{
		IncVisCount(m_pScene->GetInstance(list[i]).m_iRefIdx, TestInstanceBatch(&list[i], 1));
	}
}

// test a list of instances for visibility
int ClientSide::TestInstanceBatch(int *piBatch, int n)
{
	m_pD3DQuery->Issue( D3DISSUE_BEGIN );

	for (int idx=0; idx<n; idx++)
	{
		const Instance &instance = m_pScene->GetInstance(piBatch[idx]);
		if (instance.m_iBaseInstance == -1)
		{
			// no base instance mesh - render using the instance's bounding box
			const box3f &instancebbox = instance.m_bbox;

			// convert bounding box to matrix
			float3 vBoxCentre = instancebbox.GetCenter();
			float3 vBoxSize = instancebbox.GetSize();
			D3DXMATRIX mtx1,mtx2;
			D3DXMatrixTranslation(&mtx1, vBoxCentre.x, vBoxCentre.y, vBoxCentre.z);
			D3DXMatrixScaling(&mtx2, vBoxSize.x, vBoxSize.y, vBoxSize.z);
			D3DXMatrixMultiply(&mtx1, &mtx2, &mtx1);

			CD3DRenderer::m_pInterface->m_pD3DDevice->SetTransform(D3DTS_WORLD, &mtx1);

			// render instance
			m_pBBoxMesh->Render();
		}
		else
		{
			// render the instance's mesh

			// setup world matrix
			D3DXMATRIX mtx;
			D3DXMatrixIdentity( &mtx );
			CD3DRenderer::m_pInterface->m_pD3DDevice->SetTransform(D3DTS_WORLD, &mtx);
			
			m_D3DBaseInstanceMeshList[instance.m_iBaseInstance]->Render();
		}
	}

    // End the query, get the data
    m_pD3DQuery->Issue( D3DISSUE_END );

    // Loop until the data becomes available
    DWORD pixelsVisible = 0;

    while (m_pD3DQuery->GetData((void *) &pixelsVisible, sizeof(DWORD), D3DGETDATA_FLUSH) == S_FALSE);

	return pixelsVisible/2;
}

bool ClientSide::IsInCurrentPass(int iLayer)
{
	const PVSPass *pPass = &m_pScene->m_Passes[m_iCurrentPass];

	for (int i=0; i<pPass->m_OcclusionLayers.Size(); i++)
		if (pPass->m_OcclusionLayers[i] == iLayer)
			return true;

	for (int i=0; i<pPass->m_PVSLayers.Size(); i++)
		if (pPass->m_PVSLayers[i] == iLayer)
			return true;

	for (int i=0; i<pPass->m_BothLayers.Size(); i++)
		if (pPass->m_BothLayers[i] == iLayer)
			return true;
	return false;
}

// calc vis on instances
void ClientSide::ProcessInstances(PVSResult *pResult, Frustum *pFrustum, float3 &vCameraPos)
{
	// temporarily turn off the z-buffer
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	CD3DRenderer::m_pInterface->m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_CONSTANT );
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetTextureStageState( 0, D3DTSS_CONSTANT,	0xff00ff );

	// temporarily turn off backface culling, so that an instance is visible even when we are inside it's bounding box
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	Ut::Vector<int> batch;

#define INSTANCE_BATCH_SIZE 20
	batch.Reserve( INSTANCE_BATCH_SIZE );

	// check each instance, if they intersect the frustum add them to a batch to render
    for (int iCurInst=0; iCurInst<m_pScene->GetNumInstances(); iCurInst++)
	{
		//m_pD3DQuery->Issue( D3DISSUE_BEGIN );

		box3f &instancebbox = m_pScene->GetInstanceBBox(iCurInst);
		float fInstanceDrawDist = m_pScene->GetInstanceDrawDistance(iCurInst);

		// check maximum draw-distance
		if (fInstanceDrawDist < Length(instancebbox.GetCenter() - vCameraPos))
			continue;

		if (!IsInCurrentPass(m_pScene->GetInstance(iCurInst).m_iAftkLayer))
			continue;

		// frustum culling
		if (!pFrustum->IntersectBox(instancebbox))
			continue;

		// add this instance to the batch
		batch.PushBack(iCurInst);
		if (batch.Size() >= INSTANCE_BATCH_SIZE)
		{
			TestAndSaveVisibilityOfInstanceBatch(pResult, batch);
			batch.Clear();
		}
	}

	if (batch.Size())
		TestAndSaveVisibilityOfInstanceBatch(pResult, batch);

	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetTextureStageState( 0, D3DTSS_CONSTANT,	0xffffffff );
}

double gnStats_NumObjectsRendered = 0;
double gnStats_NumObjectsCulled   = 0;

#define D3DCOLORWRITEENABLE_ALL (D3DCOLORWRITEENABLE_RED + D3DCOLORWRITEENABLE_GREEN + D3DCOLORWRITEENABLE_BLUE)

// render the scene
void ClientSide::Render(Frustum &frustum)
{
	const PVSPass *pPass = &m_pScene->m_Passes[m_iCurrentPass];

	// render occlusion
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

	for (int i=0; i<pPass->m_OcclusionLayers.Size(); i++)
		RenderLayer(pPass->m_OcclusionLayers[i], frustum);

	// render PVS
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);

	for (int i=0; i<pPass->m_PVSLayers.Size(); i++)
		RenderLayer(pPass->m_PVSLayers[i], frustum);

	// render occlusion + PVS
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);

	for (int i=0; i<pPass->m_BothLayers.Size(); i++)
		RenderLayer(pPass->m_BothLayers[i], frustum);
}

void ClientSide::RenderLayer(int iLayer, Frustum &frustum)
{
	// TODO: make a list of which nodes are in each layer
	for (int i=0; i<m_pScene->m_MeshNodeList.Size(); i++)
	{
		if (m_pScene->m_MeshNodeList[i]->GetAftkLayer() == iLayer)
		{
			int nFaces = m_D3DMeshList[i]->GetD3DXMesh()->GetNumFaces();

			gnStats_NumObjectsRendered += nFaces;

			box3f &bbox = *m_pScene->GetNode(i)->GetBoundingBox();
			if (frustum.IntersectBox(bbox))
				m_D3DMeshList[i]->Render();
			else
				gnStats_NumObjectsCulled += nFaces;
		}
	}
}

// serialization code
void ClientSide::write( ost::Engine &ar )
{
	ar << m_pScene;

	ClientInterface::write(ar);
}

void ClientSide::read( ost::Engine &ar )
{
	ar >> m_pScene;

	ClientInterface::read(ar); 
}
