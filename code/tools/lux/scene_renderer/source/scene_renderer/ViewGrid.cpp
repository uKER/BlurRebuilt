/*
 * A small class that draws a grid in the viewport
 */

#include "stdafx.h"
#include "scene_renderer/Camera.h"
#include "scene_renderer/TargetCamera.h"
#include "scene_renderer/ViewGrid.h"

using namespace SRD;

const DWORD CViewGrid::m_FVF			= D3DFVF_XYZ | D3DFVF_DIFFUSE;

void CViewGrid::Initialize(IDirect3DDevice9 *pD3DDevice)
{
	SVertex *pVertices = NULL;
	HRESULT hr;

	float fRadius = 1024.0f;

	m_pD3DDevice = pD3DDevice;

	if (m_pVertexBuffer)
	{
		delete m_pVertexBuffer;
		m_pVertexBuffer = NULL;
	}

#define GRID_INIT() \
		InitializeGrid(24,	fRadius,		0xa0a0a0a0,		pVertices) +\
		InitializeGrid(0,	fRadius,		0,				pVertices)

	m_nVertices = GRID_INIT();

    GetD3DDevice()->CreateVertexBuffer( m_nVertices * sizeof(SVertex),
					0, m_FVF,
					D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );

	// Lock	the	vertex buffer, to generate a simple	bounding sphere
	hr = m_pVertexBuffer->Lock( 0, sizeof(SVertex) * m_nVertices, (void**)&pVertices, 0 );

	m_nVertices = GRID_INIT();

	m_pVertexBuffer->Unlock();
	m_bInitialized = true;
}

int CViewGrid::InitializeGrid(int nCells, float nWidth, int iColour, LPSVertex &pCurV)
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
					pCurV[0].vPos = float3(f,0,-nWidth);
					pCurV[1].vPos = float3(f,0,+nWidth);
				}
				else
				{
					pCurV[0].vPos = float3(-nWidth,0,f);
					pCurV[1].vPos = float3(+nWidth,0,f);
				}
				pCurV[0].iColour = pCurV[1].iColour = iColour;

				pCurV += 2;
			}

			nVerts += 2;
		}
	}
	return nVerts;
}

CViewGrid::~CViewGrid(void)
{
	m_pVertexBuffer->Release();
}

const float CViewGrid::m_matAxis[3][16] =
{
	{ 0,0,1,0, 0,1,0,0, 1,0,0,0, 0,0,0,1 },
	{ 0,1,0,0, 1,0,0,0, 0,0,1,0, 0,0,0,1 },
	{ 1,0,0,0, 0,0,1,0, 0,1,0,0, 0,0,0,1 },
};

void CViewGrid::Render(const SRD::Camera *pView)
{
	if (!m_bInitialized) return;
	float fScale;
	int nScaleFactors=0;

	IDirect3DDevice9 *pD3DDevice = GetD3DDevice();

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
	pD3DDevice->SetTransform( D3DTS_WORLD, &matTemp );
	pD3DDevice->SetTransform( D3DTS_VIEW, &mView );
//	pD3DDevice->SetTransform( D3DTS_PROJECTION, &pView->m_matProj );

	// set the right rendermodes
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,	D3DTOP_SELECTARG2 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );

	pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );
	pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );

	// temporarily turn off the z-buffer
//	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	// draw XY axis in black lines
	pD3DDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(SVertex));
	pD3DDevice->SetFVF(m_FVF);

	pD3DDevice->DrawPrimitive(D3DPT_LINELIST, 0, m_nVertices/2);

//	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}
