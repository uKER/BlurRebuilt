#include "StdAfx.h"
#include "lux_app/polygonpicker.h"

const DWORD CPolygonPicker::D3DVERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

CPolygonPicker::CPolygonPicker(void)
{
	m_pVB					= NULL;
	m_dwNumIntersections	= 0;
	m_bClosestOnly			= true;
	m_bInitialized			= false;
	m_bFaceSelected			= false;
	m_pMDL					= NULL;
}

CPolygonPicker::~CPolygonPicker(void)
{
	if (m_pVB)
	{
		m_pVB->Release();
		m_pVB = NULL;
	}
}

void CPolygonPicker::Init(LPDIRECT3DDEVICE9 pD3DDevice)
{
	if (m_bInitialized) return;

	m_pd3dDevice = pD3DDevice;
    // Create the vertex buffer for the picked polygons
	m_pVB = NULL;
    DWORD dwNumVertices = MAX_INTERSECTIONS * 3;
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( dwNumVertices*sizeof(D3DVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DVERTEX::FVF,
												  D3DPOOL_MANAGED, &m_pVB, NULL ) ) )
    {
        return;
    }
	m_bInitialized = true;
}

void CPolygonPicker::Draw()
{
	if (!m_bInitialized) return;

    if( m_dwNumIntersections > 0 )
    {
		D3DXMATRIX identityMtx;

		D3DXMatrixIdentity(&identityMtx);

		m_pd3dDevice->SetTransform(D3DTS_WORLD,&identityMtx);

        // Set render mode to unlit, wireframe triangles
        m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
        m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1 );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE );

		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,	  FALSE );

		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );

        // Draw the picked triangle
        m_pd3dDevice->SetFVF( D3DVERTEX::FVF );
        m_pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof(D3DVERTEX) );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_dwNumIntersections*3 );

		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,	  TRUE );

		m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
    }
}

HRESULT CPolygonPicker::Pick(SD::RayTraceResult *pResult)
{
	AddIntersectionsToMesh(pResult);

    return S_OK;
}


void CPolygonPicker::AddIntersectionsToMesh(SD::RayTraceResult *pResult)
{
    // Now, for each intersection, add a triangle to m_pVB and compute texture coordinates
    if( pResult->bHit )
    {
        D3DVERTEX* v;
        D3DVERTEX* vThisTri;
        D3DVERTEX  v1, v2, v3;

        m_pVB->Lock( 0, 0, (void**)&v, 0 );

        vThisTri = &v[0];
        // get vertices hit
		vThisTri[0].p[0] = pResult->vVerts[0].x;
		vThisTri[0].p[1] = pResult->vVerts[0].y;
		vThisTri[0].p[2] = pResult->vVerts[0].z;
		vThisTri[1].p[0] = pResult->vVerts[1].x;
		vThisTri[1].p[1] = pResult->vVerts[1].y;
		vThisTri[1].p[2] = pResult->vVerts[1].z;
		vThisTri[2].p[0] = pResult->vVerts[2].x;
		vThisTri[2].p[1] = pResult->vVerts[2].y;
		vThisTri[2].p[2] = pResult->vVerts[2].z;

		vThisTri[0].iColour = 0xff00ff;
		vThisTri[1].iColour = 0xff00ff;
		vThisTri[2].iColour = 0xff00ff;

        m_pVB->Unlock();

		m_dwNumIntersections = 1;
    }
	else
		m_dwNumIntersections = 0;
}

