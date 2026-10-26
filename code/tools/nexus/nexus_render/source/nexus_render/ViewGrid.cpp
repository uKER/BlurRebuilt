/*
 * A small class that draws a grid in the viewport
 */

#include <nexus_render/stdafx.h>
#include <nexus_render/Camera.h>
#include <nexus_render/TargetCamera.h>
#include <nexus_render/ViewGrid.h>

using namespace NexusRender;

const DWORD ViewGrid::m_FVF			= D3DFVF_XYZ | D3DFVF_DIFFUSE;

void ViewGrid::Initialize()
{
	SVertex *pVertices = NULL;
	HRESULT hr;

	float fRadius = 1024.0f;

	if (m_pVertexBuffer)
	{
		delete m_pVertexBuffer;
		m_pVertexBuffer = NULL;
	}

#define GRID_INIT() \
		InitializeGrid(24,	fRadius,		0xa0a0a0a0,		pVertices) +\
		InitializeGrid(0,	fRadius,		0,				pVertices)

	m_nVertices = GRID_INIT();

	D3DManager::GetDevicePtr()->CreateVertexBuffer( 
		m_nVertices * sizeof(SVertex),
		0, 
		m_FVF,
		D3DPOOL_MANAGED,
		&m_pVertexBuffer,
		NULL);

	// Lock	the	vertex buffer, to generate a simple	bounding sphere
	hr = m_pVertexBuffer->Lock(
		0, 
		sizeof(SVertex) * m_nVertices, 
		(void**)&pVertices,
		0);

	m_nVertices = GRID_INIT();

	m_pVertexBuffer->Unlock();
	m_bInitialized = true;
}

int ViewGrid::InitializeGrid(int nCells, float nWidth, int iColour, LPSVertex &pCurV)
{
	float f, fSpacing = 0;
	int nVerts = 0;

	if (nCells) fSpacing = nWidth / nCells;

	// create the grid
	for (int iAxis=0; iAxis<2; iAxis++)
	{
		for (int i=-nCells; i<=nCells; i++)
		{
			f = i*fSpacing;

			if (pCurV)
			{
				if (iAxis == 0)
				{
					pCurV[0].vPos[0] = f;
					pCurV[0].vPos[1] = 0;
					pCurV[0].vPos[2] = -nWidth;

					pCurV[1].vPos[0] = f;
					pCurV[1].vPos[1] = 0;
					pCurV[1].vPos[2] = +nWidth;
				}
				else
				{
					pCurV[0].vPos[0] = -nWidth;
					pCurV[0].vPos[1] = 0;
					pCurV[0].vPos[2] = f;

					pCurV[1].vPos[0] = +nWidth;
					pCurV[1].vPos[1] = 0;
					pCurV[1].vPos[2] = f;
				}
				pCurV[0].iColour = pCurV[1].iColour = iColour;

				pCurV += 2;
			}

			nVerts += 2;
		}
	}
	return nVerts;
}

ViewGrid::~ViewGrid(void)
{
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
	}
}

const float ViewGrid::m_matAxis[3][16] =
{
	{ 0,0,1,0, 0,1,0,0, 1,0,0,0, 0,0,0,1 },
	{ 0,1,0,0, 1,0,0,0, 0,0,1,0, 0,0,0,1 },
	{ 1,0,0,0, 0,0,1,0, 0,1,0,0, 0,0,0,1 },
};

void ViewGrid::Render(const NexusRender::Camera *pView)
{
	if (!m_bInitialized) return;
	float fScale;
	int nScaleFactors=0;

	D3DXMATRIX matTemp;
	D3DXMATRIX mView;
	pView->GetViewMatrix(mView);

	// the scale of the grid depends on the distance between the camera and the origin
/*
	if (!pView->m_bParallelProj)
		fScale = pView->m_matView._43;//Length(float3(pView->m_matView._41, , pView->m_matView._43));
	else
		fScale = pView->m_ArcBall.m_fRadiusTranslation * 0.5f;
*/

	fScale = mView._43;
	int dummy = (int)(fScale/((1024.0f/24.0f)));
	while (dummy)
	{
		nScaleFactors++;
		dummy/=8;
	}

	fScale = 1;
	for (int i=0; i<nScaleFactors; i++)
		fScale*=8;
	fScale *= 0.05f;

	// the grid is always centered around the origin
	D3DXMatrixScaling(&matTemp, fScale, fScale, fScale);
//	D3DXMatrixMultiply(&matTemp, (LPD3DXMATRIX)&m_matAxis[pView->m_iAxisAlign][0], &matTemp);
	D3DManager::GetDevicePtr()->SetTransform( D3DTS_WORLD, &matTemp );
	D3DManager::GetDevicePtr()->SetTransform( D3DTS_VIEW, &mView );
//	D3DManager::GetDevicePtr()->SetTransform( D3DTS_PROJECTION, &pView->m_matProj );

	// set the right rendermodes
	D3DManager::GetDevicePtr()->SetTextureStageState( 0, D3DTSS_COLOROP,	D3DTOP_SELECTARG2 );
	D3DManager::GetDevicePtr()->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );
	D3DManager::GetDevicePtr()->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE );
	D3DManager::GetDevicePtr()->SetTextureStageState( 0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );

	D3DManager::GetDevicePtr()->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );
	D3DManager::GetDevicePtr()->SetTextureStageState( 1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );

	// temporarily turn off the z-buffer
//	D3DManager::GetDevicePtr()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	// draw XY axis in black lines
	D3DManager::GetDevicePtr()->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(SVertex));
	D3DManager::GetDevicePtr()->SetFVF(m_FVF);

	D3DManager::GetDevicePtr()->DrawPrimitive(D3DPT_LINELIST, 0, m_nVertices/2);

//	D3DManager::GetDevicePtr()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}
