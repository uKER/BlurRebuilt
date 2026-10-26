//-----------------------------------------------------------------------------
// File: rasterize2.cpp
//
// Desc: rasterize a polygon's texels and output spans + some more info
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "Lux/Rasterize.h"
#include "Lux/RasterizedTriIterator.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/TextureTypes/ITexture.h"
#include "Lux/Common.h"
#include "Lux/Typedefs.h"
#include "Lux/Sampler.h"

using namespace LUX;

void AddLine(float3, float3);

// check that the tangent space is valid. create our own if it isn't.
void Rasterizer::VerifyTangentSpace()
{
	bool bBroken = false;
	for (int i=0; i<3; i++)
	{
		if (CheckBuggeredFloat(m_tri.tangent[i].x) || CheckBuggeredFloat(m_tri.tangent[i].y) || CheckBuggeredFloat(m_tri.tangent[i].z) ||
			CheckBuggeredFloat(m_tri.binormal[i].x) || CheckBuggeredFloat(m_tri.binormal[i].y) || CheckBuggeredFloat(m_tri.binormal[i].z))
		{
			bBroken = true;
			break;
		}
	}
	if (!bBroken)
		return; // no problems

	// something is wrong with it - invent out own based on the vertex normal
	// calculate world-space tangent space vectors (don't relate to UV mapping at the moment)
	float3 t,s,v;

	for (int i=0; i<3; i++)
	{
		CoordinateSystem(m_tri.norm[i], &m_tri.tangent[i], &m_tri.binormal[i]);
	}
}

// load aftk polygon data into the rasterizer
bool Rasterizer::LoadPolygon(Aftk::I_SceneMeshNode *m_pMesh, int polygonIndex, Aftk::Matrix44 *pWorldMatrix, Aftk::Matrix44 *pNormalMatrix, int m_iLMapUVSet, bool bTangents)
{
	int iTangentUVSet = 0; // TODO: somehow figure out the UV channel of the bump map

	// get polygon
	const Aftk::C_Polygon *p = &m_pMesh->GetPolygon(polygonIndex);
	// store the vertex data into our tri structure 
	for (int i=0; i<3; i++)
	{
		int iPolyVert = p->GetPolygonVertexIndex(i);

		// copy position + normal from Aftk to our own structure
		LUX::Position3 v = m_pMesh->GetEvaluatedPolygonVertexPosition( iPolyVert );
		v = LPVS::ApplyMatrix(*pWorldMatrix, v );
		*(Aftk::Position3*)&m_tri.vert[i] = v;

		LUX::Normal3 n = m_pMesh->GetEvaluatedPolygonVertexNormal( iPolyVert );
		n = LPVS::ApplyMatrix(*pNormalMatrix, n );
		*(Aftk::Normal3*)&m_tri.norm[i] = n;

		// protect against AFTK spitting out bogus vertices
		if (CheckBuggeredFloat(v.x) || CheckBuggeredFloat(v.y) || CheckBuggeredFloat(v.z))
		{
			if (gbDebugMode)
				S_WARN( g_Log, "bad position on polyvert " << iPolyVert << " on mesh " << m_pMesh->GetPath() << "\n");

			return false;
		}

		if (CheckBuggeredFloat(n.x) || CheckBuggeredFloat(n.y) || CheckBuggeredFloat(n.z))
		{
			if (gbDebugMode)
				S_WARN( g_Log, "bad normal on polyvert " << iPolyVert << " on mesh " << m_pMesh->GetPath() << "\n");

			return false;
		}

		// optionally load tangent space
		if (bTangents)
		{
			// get tangent space vectors - WELCOME TO MATHS LIBRARY HELL
			Aftk::Vector3F32 vt = m_pMesh->GetEvaluatedPolygonVertexTangent( iTangentUVSet, iPolyVert );
			Normal3 t; t.x = vt.x; t.y = vt.y; t.z = vt.z;
			m_tri.tangent[i] = (float3)LPVS::ApplyMatrix(*pNormalMatrix, t );
			Aftk::Vector3F32 vb = m_pMesh->GetEvaluatedPolygonVertexBinormal( iTangentUVSet, iPolyVert );
			Normal3 b; b.x = vb.x; b.y = vb.y; b.z = vb.z;
			m_tri.binormal[i] = (float3)LPVS::ApplyMatrix(*pNormalMatrix, b );
		}

		Aftk::TextureCoordinate4 uv = m_pMesh->GetPolygonVertexTextureCoordinate( m_iLMapUVSet, iPolyVert );
		m_tri.texCoord[i].u = uv.x;
		m_tri.texCoord[i].v = uv.y;
	}

	// check the tangent space returned from AFTK
	if (bTangents)
		VerifyTangentSpace();

	// load subchart dimensions
    m_tri.m_x  = POLY_DATA(polygonIndex)->m_x;
	m_tri.m_y  = POLY_DATA(polygonIndex)->m_y;
	m_tri.m_cx = POLY_DATA(polygonIndex)->m_cx;
	m_tri.m_cy = POLY_DATA(polygonIndex)->m_cy;

	return true;
}


// Get edge info
void Rasterizer::GetEdge(NmEdge* edge, int idx0, int idx1)
{
	int e1,e2;

	// Based on the Y/v coordinate fill in the structure.
	if (m_tri.texCoord[idx0].v <= m_tri.texCoord[idx1].v)
	{
		e1 = idx0, e2 = idx1;
	}
	else
	{
		e1 = idx1, e2 = idx0;
	}

	edge->idx0 = e1;
	edge->idx1 = e2;

	edge->yMin = m_tri.texCoord[e1].v * m_iHeight -0.5f;
	edge->yMax = m_tri.texCoord[e2].v * m_iHeight -0.5f;
	edge->x = m_tri.texCoord[e1].u * m_iWidth	 -0.5f;
	edge->x1 = m_tri.texCoord[e2].u * m_iWidth	  -0.5f;;

	edge->increment = edge->denominator = edge->yMax - edge->yMin;
	edge->numerator = edge->x1 - edge->x;

	edge->dx_over_dy = (edge->x1 - edge->x) / (edge->yMax - edge->yMin);

} // GetEdge

// Find	the	minimum	and	maximum	Y value	for	the	given set of edges.
void Rasterizer::GetYMinMax(NmEdge edge[3], float *minY, float *maxY)
{
	// Find	the	minimum	and	maximum	Y values for these edges.
	(*minY)	= edge[0].yMin;
	(*maxY) = edge[0].yMax;
	if (ceil(edge[1].yMax) > (*maxY))
	{
		(*maxY)	= edge[1].yMax;
	}
	if (ceil(edge[2].yMax) > (*maxY))
	{
		(*maxY)	= edge[2].yMax;
	}
}

// Find	the	minimum	and	maximum	X value	for	the	given set of edges and a given Y value.
void Rasterizer::GetXMinMax(NmEdge edge[3], int y, float *minX, float *maxX)
{
	// Using active	edges find the min/max X values.
	float fminx = 100000, fmaxx = -10;
	// calculate the horizontal max and min x for the vertical area of [y,y+1]

	for	(int e = 0;	e <	3; e++)
	{
		// See if this edge	is active.
		if ((edge[e].yMin <= y+1) && (edge[e].yMax >= y))
		{
			// is the start position within this pixel?)
			if (edge[e].yMin>=y)
			{
				// yes, take it's X coordinate
				float d = edge[e].x;
				if (d < fminx) fminx = d;
				if (d > fmaxx) fmaxx = d;
			}
			else
			{
				// no, calculate intersection
				float d = y;

				float x = (d-edge[e].yMin) * edge[e].dx_over_dy + edge[e].x; // CHECK

				if (x < fminx) fminx = x;
				if (x > fmaxx) fmaxx = x;                
			}

			// is the end position within this pixel?)
			if (edge[e].yMax<=y+1.0f)
			{
				// yes, take it's X coordinate
				float d = edge[e].x1;
				if (d < fminx) fminx = d;
				if (d > fmaxx) fmaxx = d;
			}
			else
			{
				// no, calculate intersection
				float d = y+1.0f;

				float x = (d-edge[e].yMin) * edge[e].dx_over_dy + edge[e].x; // CHECK

				if (x < fminx) fminx = x;
				if (x > fmaxx) fmaxx = x;                
			}
		} // end if	edge is	active
	} // end for number	of edges

	// verify that we got a min and max value
	assert(fminx<100000 && fmaxx>-10);

	(*minX)	= fminx;
	(*maxX)	= fmaxx;
}

// Get the sorted edges	from the given triangle
void Rasterizer::GetSortedEdges(NmEdge edge[3])
{
	// Get the edges from the triangle
	GetEdge	(&edge[0],	0, 1);
	GetEdge	(&edge[1],	0, 2);
	GetEdge	(&edge[2],	1, 2);

	// Sort	by minimum Y
	NmEdge tmp;
	if (edge[2].yMin < edge[1].yMin)
	{
		memcpy (&tmp, &edge[1],	sizeof (NmEdge));
		memcpy (&edge[1], &edge[2],	sizeof (NmEdge));
		memcpy (&edge[2], &tmp,	sizeof (NmEdge));
	}
	if (edge[1].yMin < edge[0].yMin)
	{
		memcpy (&tmp, &edge[0],	sizeof (NmEdge));
		memcpy (&edge[0], &edge[1],	sizeof (NmEdge));
		memcpy (&edge[1], &tmp,	sizeof (NmEdge));
	}
}

// set 3 bits of the return value depending on whether the sx/sy is outside that edge
bool Rasterizer::ClampToEdge(float3 vPlane, float2 &s)
{
	float fDotS = vPlane.x * s.x + vPlane.y * s.y;

	// clamp fDotS to fDotP if needed
	if (fDotS>vPlane.z-0.1)
	{
		float f = fDotS-vPlane.z;
		s.x = s.x - vPlane.x * f;
		s.y = s.y - vPlane.y * f;
		return true;
	}

	return false;
}

// calculate normal + constant for one of the edge planes
void Rasterizer::CalcEdgePlane(float2 a, float2 b, float fDir, float3 &vPlane)
{
	float2 vEdge, vENormal;

	vEdge = b - a;

	vENormal.x = -vEdge.y;
	vENormal.y = vEdge.x;
	vENormal.Normalize();

	vENormal = vENormal * fDir;

	vPlane.x = vENormal.x;
	vPlane.y = vENormal.y;
	vPlane.z = vENormal.x * a.x + vENormal.y * a.y;
}

// calculate three edge planes for each edge of the polygon
void Rasterizer::CalculateEdgePlanes(float2 a, float2 b, float2 c)
{
	// calculate whether this poly is CW or CCW in UV space
	float2 vE1, vE2;
	vE1 = b - a;
	vE2 = c - b;

	float fDir;
	float fDot = vE1.x * vE2.y - vE1.y * vE2.x;
	if (fDot > 0)
		fDir = -1;
	else
		fDir = 1;

	CalcEdgePlane(a,b,fDir, m_vPlane[0]);
	CalcEdgePlane(b,c,fDir, m_vPlane[1]);
	CalcEdgePlane(c,a,fDir, m_vPlane[2]);
}

bool Rasterizer::IsTexelCentreInsideTriangle(int x, int y)
{
	// this code clamps the rasterization to the triangle
	for (int i=0; i<3; i++)
	{
		float fDotS = m_vPlane[i].x * (x+0.5f) + m_vPlane[i].y * (y+0.5f);
		if (fDotS > m_vPlane[i].z) // outside this edge - another tri from this chart will draw here
			return false;
	}
	return true;
}

bool Rasterizer::ShouldWeWriteToThisPixel(int x, int y, RasterizedTriInfo *pOut)
{
	switch (m_iPass)
	{
	case 0:
		// first pass. only flags texels totaly inside each triangle
		return IsTexelCentreInsideTriangle(x,y);

	case 1:
		// second pass, only allow unused texels, which are either totally inside the triangle or not inside any triangle
		if (m_pTex->CheckPixelUsed(x,y))
			return false;

		// disallow texels outside of this triangle which are inside another triangle
		if (m_pTex->CheckPixelUsed2(x,y) && !IsTexelCentreInsideTriangle(x,y))
			return false;

		return true;

	case 2:
		// old style
		if (m_pTex->CheckPixelUsed(x,y))
			return false;

		return true;
	}

	return true;
}

// rasterize an NwRawTri
void Rasterizer::Rasterize(TexelIteratorInput *pTI, NmRawTriangle *pTri, int iPass)
{
	// load triangle info into our own structure
	memcpy(&m_tri, pTri, sizeof(NmRawTriangle));

	Rasterize(pTI, iPass);
}

// rasterize a polygon, export the edges and some other info
// pass:
//   0 = preprocess - just write to the texture
//   1 = actual rasterization, using previous pass info
//   2 = rasterization, old-style
void Rasterizer::Rasterize(TexelIteratorInput *pTI, int iPass)
{
	NmEdge edge[3];
	bool bDegenerate = false;
	m_pTex = pTI->m_pTex;

	m_iPass = iPass;

	m_iWidth  = m_pTex->m_cX;
	m_iHeight = m_pTex->m_cY;

	// Find	initial	Barycentric	parameters
    float x1 = m_tri.texCoord[0].u*((float)(m_iWidth));
    float x2 = m_tri.texCoord[1].u*((float)(m_iWidth));
    float x3 = m_tri.texCoord[2].u*((float)(m_iWidth));
    float y1 = m_tri.texCoord[0].v*((float)(m_iHeight));
    float y2 = m_tri.texCoord[1].v*((float)(m_iHeight));
    float y3 = m_tri.texCoord[2].v*((float)(m_iHeight));

	CalculateEdgePlanes(float2(x1,y1),float2(x2,y2),float2(x3,y3));

	float b0 =	((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1));

	// protect against divide by zero
	if (fabs(b0)<EPSILON)
	{
		b0 = 1;
		bDegenerate = true;
		return;
	}

	// add a new polygon
	pTI->m_Tris.PushBack();
	RasterizedTriInfo *pOut = &pTI->m_Tris.Back();
	pOut->m_iRasterizedEdge = pTI->m_Edges.Size();

	pOut->m_vPlane[0] = m_vPlane[0];
	pOut->m_vPlane[1] = m_vPlane[1];
	pOut->m_vPlane[2] = m_vPlane[2];

	// store parameters so we can do barycentric calculations later on
	pOut->m_x1 = x1;	pOut->m_x2 = x2;	pOut->m_x3 = x3;
	pOut->m_y1 = y1;	pOut->m_y2 = y2;	pOut->m_y3 = y3;
	pOut->m_b0 = b0;

	// store subchart dimensions
	pOut->m_x  = m_tri.m_x;		pOut->m_y = m_tri.m_y;
	pOut->m_cx = m_tri.m_cx;	pOut->m_cy = m_tri.m_cy;

	// store position + normals + tangent space
	for (int i=0; i<3; i++)
	{
		pOut->m_vPos[i] = m_tri.vert[i];
		pOut->m_vNormal[i] = m_tri.norm[i];
		pOut->m_vTangent[i] = m_tri.tangent[i];
		pOut->m_vBinormal[i] = m_tri.binormal[i];
	}

	pOut->m_nEdges = 0;

	// Find	edges and sort by minimum Y.
	GetSortedEdges(edge);

	// Find	min/max	Y.
	int	minY, maxY;
	float fminY, fmaxY;
	GetYMinMax (edge, &fminY, &fmaxY);
	minY = floor(fminY);
	maxY = ceil(fmaxY);

	float fOldMinX = 100000, fOldMaxX = -100000;
	float fNewMinX, fNewMaxX;

    int minX, maxX;
	float fminX, fmaxX;
    int firstY = minY;
    int lastY = maxY;

	// calculate on which edges to use clamping
	for (int i=0; i<3; i++)
	{
		pOut->m_bClampEdge[i] = false;//true;// true disables the clamping(!)
/*
		int iAdjacent = m_pMesh->GetPolygon(m_tri.iIndex).GetAdjacentPolygonIndex(i);

		// don't clamp if we have an adjacent poly on the same chart
		if (iAdjacent==-1)
			continue;

		if (POLY_DATA(iAdjacent)->iChart ==	POLY_DATA(m_tri.iIndex)->iChart)
			pOut->m_bClampEdge[i] = false;
		// TODO: improve this heuristic
*/
		// calculate whether this edge is 'open' (i.e. no adjacent polygon on the same chart)
	}

	// store y-coordinate of first edge
	pOut->m_iFirstEdgeY = max(0,firstY);

	// do the rasterizing
	bool bFirst = true;
    for (int y = firstY; y < lastY + 1; y++) // +1... we expand diagonally by one
    {
		if (bFirst || y<lastY) // the last horizontal line has the same width as the previous one
		{
			GetXMinMax (edge, y, &fminX, &fmaxX);

			fNewMinX = (fminX<fOldMinX)?fminX:fOldMinX; // this is to expand vertically downwards
			fNewMaxX = (fmaxX>fOldMaxX)?fmaxX:fOldMaxX;

			fOldMinX = fminX;
			fOldMaxX = fmaxX;
		}
		bFirst = false;

		minX = (int) floor(fNewMinX);
		maxX = (int) ceil(fNewMaxX);

        // Make sure this is a valid texture coordinate
        if ((y < 0) || (y >= m_iHeight))
        {
            continue;
        }

		if (minX < 0) minX = 0;
		if (minX >= m_iWidth) minX = m_iWidth-1;
		if (maxX < 0) maxX = 0;
		if (maxX >= m_iWidth) maxX = m_iWidth-1;

		while (minX<maxX && !ShouldWeWriteToThisPixel(minX,y, pOut))
			minX++;
		while (minX<=maxX && !ShouldWeWriteToThisPixel(maxX,y, pOut))
			maxX--;

		if (iPass == 0)
		{
			// Loop over the Xs touching each texel of the texture
			for (int x = (minX); x <= (maxX); x++)
			{
				m_pTex->TouchPixel2(x,y);
			}
		}
		else
		{
			// add an edge
			RasterizedEdge newEdge;
			newEdge.x1 = minX;
			newEdge.x2 = maxX;
			pTI->m_Edges.PushBack(newEdge);

			pOut->m_nEdges++;

			// Loop over the Xs touching each texel of the texture
			for (int x = (minX); x <= (maxX); x++)
			{
				m_pTex->TouchPixel(x,y);            
			}
		}
	}
}

// texel rasterizer input class

TexelIteratorInput::TexelIteratorInput()
{
	m_Edges.Reserve(1024);
	m_Tris.Reserve(100);
}

void TexelIteratorInput::Clear()
{
	m_Edges.Resize(0);
	m_Tris.Resize(0);
}
