//-----------------------------------------------------------------------------
// File: TextureAtlas.cpp
//
// Desc: UV parameterization for a CMesh
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/Parameterization/GenerateFaceNormals.h"
#include "Lux/Parameterization/ChartSeams.h"
#include "lux_common/LpvsUtility.h"
#include "Lux/typedefs.h"
#include "lux_common/LpvsUtility.h"
#include "Lux/DebugDraw.h"

#define SCALE_AMOUNT 2 // 2

using namespace LUX;


LUX::Position3 *g_pVerts;		// vertex positions in world-space

// calculate the major axis of a normal
majoraxis_t CalculateMajorAxis(float3 vNormal)
{
	float v[3];
	v[0]=fabsf(vNormal[0]);
	v[1]=fabsf(vNormal[1]);
	v[2]=fabsf(vNormal[2]);

	int iAxis=(v[0]>v[1])?0:1;
	iAxis=(v[iAxis]>v[2])?iAxis:2;

	if (vNormal[iAxis]>0) iAxis+=3;
	return (majoraxis_t)iAxis;
}


static CTextureAtlas *g_pCurrentTextureAtlas = NULL;

// --- Texture Chart


const int CTextureChart::axistypes[3][2]={
		{1,2},
		{0,2},
		{0,1},
	};

#define VERTEX(IFACE)

// manual initialize - start with one face
void CTextureChart::Initialize(Aftk::C_SceneMeshNode *pMesh, int iFace)
{
	// link the face to this chart
	m_iFace = iFace;
	m_iLastFace = iFace;
	m_pMesh = pMesh;
	m_pMatrixNode = pMesh;

	POLY_DATA(iFace)->iChart = FACE_CHARTED; // set this face 

	// calculate major axis
	// only adjacent faces with the same major axis will be added to this chart
	float *pf = (float*)&POLY_DATA(iFace)->vNormal;
	m_iMajorAxis = CalculateMajorAxis( float3(pf[0],pf[1],pf[2]) );

	// decide which axis gets mapped to U and V
	int iMajorAxis = m_iMajorAxis % 3;
	m_iUAxis = axistypes[iMajorAxis][0],
	m_iVAxis = axistypes[iMajorAxis][1];

	// initialize edge-list
//	m_EdgeList.EnsureCapacity(128);
}

void CTextureChart::Free()
{
}

void CTextureChart::RecalcBoundingBox()
{
	int iCurFace = m_iFace;

	m_bbox = box3f();

	Aftk::Matrix44 matrix = m_pMatrixNode->GetWorldMatrix();

	// walk through the link list, add bounding boxes of all the charts
	while (iCurFace != -1)
	{
		const Aftk::C_Polygon *pPoly = &m_pMatrixNode->GetPolygon(iCurFace);

		for (int i=0; i<3; i++)
		{
			float3 v = LPVS::ApplyMatrix( matrix, 
					m_pMatrixNode->GetEvaluatedPolygonVertexPosition( pPoly->GetPolygonVertexIndex( i ) ) );

			m_bbox.UpdateBox(v);
		}
        
		iCurFace = POLY_DATA(iCurFace)->iChartLink;
	}
}

// decide wether iNewFace should be added to our chart or not

// TODO: a much better algorithm could be found for this - 
//			as it is it produces small separate charts inside big ones on some curves.
//			definitely needs changing to something more advanced

// return values:
// 0 = ok to add
// 1 = can't add because of a mismatch in polyflags or major axis
// 2 = can't add because of a mismatch in materials
int CTextureChart::TestAddFacesToChart(int iCurFace, int iNewFace, bool bStrict)
{
	majoraxis_t ma = CalculateMajorAxis( POLY_DATA(iNewFace)->vNormal );
    
	if (ma != m_iMajorAxis) return CHART_TEST_POTENTIAL_SEAM;

	// stop charting when we encounter a "stop lightmap" polyflag
	int iLightmapEndFlag = (1<<26);

	// if it's got a lightmap end flag, then don't add it
	PolyFlags flags1 = GetPolyFlags(
		&m_pMesh->GetPolygon(iCurFace), 
		0, //double-sided mask
		0, //do not light mask
		iLightmapEndFlag,
		0, //emit mask
		g_pCurrentTextureAtlas->m_iLowResLightmapPolyflag, //low res lightmap mask
		g_pCurrentTextureAtlas->m_iLowResLightmapBits //low res lightmap bits
	);

	PolyFlags flags2 = GetPolyFlags(
		&m_pMesh->GetPolygon(iNewFace), 
		0, //double-sided mask
		0, //do not light mask
		iLightmapEndFlag,
		0, //emit mask
		g_pCurrentTextureAtlas->m_iLowResLightmapPolyflag, //low res lightmap mask
		g_pCurrentTextureAtlas->m_iLowResLightmapBits //low res lightmap bits
	);
	if ((flags1.lightmapStop) && ((flags2.lightmapStop) == 0))
		return CHART_TEST_POTENTIAL_SEAM;

	// if the low-res-texel-density flags don't match, then don't add it
	if (flags1.lightmapRes != flags2.lightmapRes)
		return CHART_TEST_NOT_OK;

	if (!bStrict)
		return CHART_TEST_OK;

	// compare materials
	if ( m_pMesh->GetPolygon(iCurFace).GetMaterialMappingIndex() != m_pMesh->GetPolygon(iNewFace).GetMaterialMappingIndex() )
		return CHART_TEST_NOT_OK;
	else
		return CHART_TEST_OK;
}
//

// returns true if this chart originates from a remappable instance
bool CTextureChart::IsInstance()
{
	return m_pMatrixNode->HasType( Aftk::E_ObjType_SceneMeshRemappedInstance );
}

// check wether two edges intersect
bool CTextureChart::TestEdges(int iA1, int iA2, int iB1, int iB2)
{
	float2 vA1, vA2, vB1, vB2;

	vA1.x = g_pVerts[iA1][m_iUAxis];
	vA1.y = g_pVerts[iA1][m_iVAxis];
	vA2.x = g_pVerts[iA2][m_iUAxis];
	vA2.y = g_pVerts[iA2][m_iVAxis];

	vB1.x = g_pVerts[iB1][m_iUAxis];
	vB1.y = g_pVerts[iB1][m_iVAxis];
	vB2.x = g_pVerts[iB2][m_iUAxis];
	vB2.y = g_pVerts[iB2][m_iVAxis];

	if ((vA1 == vB1) || (vA1 == vB2) ||
		(vA2 == vB1) || (vA2 == vB2)) return false;

	return Intersect(vA1,vA2,vB1,vB2);
}


// compare the edges of a new face to the current edge of the chart
// this is to make sure we don't get polygons folding back onto themselves and
// creating duplicate UV mapping
// TODO: write this - hasn't happened so far but could feasibly happen in some oddly modeled meshes
bool CTextureChart::TestFaceAgainstEdgeList(int iFace)
{
	int iCurFace = m_iFace;

	// check to see if this face overlaps with any of the faces already in the chart
	while (iCurFace != -1)
	{
		for (int i=0; i<3; i++)
		{
			Aftk::C_Polygon *pPoly1 = &m_pMesh->GetPolygon(iCurFace);

			for (int j=0; j<3; j++)
			{
				Aftk::C_Polygon *pPoly2 = &m_pMesh->GetPolygon(iFace);

				if (TestEdges(	pPoly1->GetPolygonVertex(i).GetPositionIndex(),
								pPoly1->GetPolygonVertex((i+1)%3).GetPositionIndex(),
								pPoly2->GetPolygonVertex(j).GetPositionIndex(),
								pPoly2->GetPolygonVertex((j+1)%3).GetPositionIndex() ))
					return true;
			}
		}

		iCurFace = POLY_DATA(iCurFace)->iChartLink;
	}

	return false; // no overlapping edges
}

// compare two vertices - do they have the same position and normal
bool CTextureChart::ComparePolygonVertices(
	const Aftk::C_Polygon* pAftkPoly1,
	const Aftk::C_Polygon* pAftkPoly2,
	int v1, 
	int v2)
{
	if (!VectorSame(g_pVerts[pAftkPoly1->GetPolygonVertex(v1).GetPositionIndex()],  
					g_pVerts[pAftkPoly2->GetPolygonVertex(v2).GetPositionIndex()] ))  return false;

	if (!VectorSame(m_pMesh->GetVertexNormal(pAftkPoly1->GetPolygonVertex(v1).GetNormalIndex()),  
					m_pMesh->GetVertexNormal(pAftkPoly2->GetPolygonVertex(v2).GetNormalIndex()) )) return false;

	// TODO: check that the tangent space vectors are the same
//	if (Length(pV1->normal - pV2->normal) > 0.01) return false;
//	if (Length(pV1->binormal- pV2->binormal) > 0.01) return false;
//	if (Length(pV1->tangent - pV2->tangent) > 0.01) return false;

	return true;
}

// check wether a face folds back onto the previous face
int CTextureChart::CheckFaceFoldBack(int iFace, int iNewFace, int iEdge)
{
	// get the vertices of the common edge
	Aftk::C_Polygon *p2, *p = &m_pMesh->GetPolygon(iFace);

	int iA1, iA2;

	iA1 = iEdge;
	iA2 = (iEdge+1)%3;

	float3 vpE1 = POLYVERT_POS(p->GetPolygonVertex(iA1));
	float3 vpE2 = POLYVERT_POS(p->GetPolygonVertex(iA2));
	float3 vE;

	// get the additional vertex of 'iFace'
	//Position3 vpA  = POLYVERT_POS(p->GetPolygonVertexIndex((iEdge+2)%3));
	float3 vpA  = POLYVERT_POS(p->GetPolygonVertex((iEdge+2)%3));
	float3 vpB;
	float3 vA, vB;
	float3 vE2;

	// find the additional vertex of 'iNewFace'
	int i;
	for (i=0; i<3; i++)
	{
		p2 = &m_pMesh->GetPolygon(iNewFace);
		vpB  = g_pVerts[ p2->GetPolygonVertex(i).GetPositionIndex() ];

		float3 vl1 = vpB - vpE1;
		float3 vl2 = vpB - vpE2;
		float l1 = Length(vl1);
		float l2 = Length(vl2);
		if ( l1 > VERTEX_WELD_EPSILON &&
			 l2 > VERTEX_WELD_EPSILON) break; // found it
	}
	if (i==3) return CHART_TEST_NOT_OK; // appears to be a degenerate triangle. return true; don't add it to this list

	// compare the edge vertices - see if they match up
	// this will reject any triangles with different normals

	if (!ComparePolygonVertices(
		p,
		&m_pMesh->GetPolygon(iNewFace),
		iA2,
		(i+1)%3)) return CHART_TEST_NOT_OK;

	if (!ComparePolygonVertices(
		p,
		&m_pMesh->GetPolygon(iNewFace),
		iA1,
		(i+2)%3)) return CHART_TEST_NOT_OK;


//		iA2, m_pMesh->GetPolygon(iNewFace).GetPolygonVertexIndex((i+1)%3))) return true;
//	if (!ComparePolygonVertices(iA1, m_pMesh->GetPolygon(iNewFace).GetPolygonVertexIndex((i+2)%3))) return true;
//KEN - REMOVE
//	if (!CompareVertices(iA2, m_pMesh->GetPolygon(iNewFace).GetPolygonVertexIndex((i+1)%3))) return true;
//	if (!CompareVertices(iA1, m_pMesh->GetPolygon(iNewFace).GetPolygonVertexIndex((i+2)%3))) return true;

	// now we check wether vA and vB are on the same side of vE1-vE2
	vA = vpA - vpE1;
	vB = vpB - vpE1;
	vE2 = vpE2 - vpE1;

	vA.x = vA[m_iUAxis];
	vA.y = vA[m_iVAxis];

	vB.x = vB[m_iUAxis];
	vB.y = vB[m_iVAxis];

	vE2.x = vE2[m_iUAxis];
	vE2.y = vE2[m_iVAxis];

	// normal of the edge
	vE.x = vE2.y;
	vE.y = -vE2.x;

	float fA = vA.x * vE.x + vA.y * vE.y;
	float fB = vB.x * vE.x + vB.y * vE.y;

	// return true if the polygons are both on the same side of the edge
	if ((fA<0) == (fB<0))
		return CHART_TEST_POTENTIAL_SEAM;

	return CHART_TEST_OK;
}

box3f CTextureChart::GetBBox(int iFace)
{
	box3f bbox;

	Aftk::C_Polygon *pPoly = &m_pMesh->GetPolygon(iFace);

	for (int i=0; i<3; i++)
	{
		float3 v = POLYVERT_POS(pPoly->GetPolygonVertex(i));

		bbox.UpdateBox(v);
	}

	return bbox;
}

// don't make the charts bigger than the size of an atlas
bool CTextureChart::CheckFaceIsInBounds(int iFace)
{
	box3f newbbox = box3f(m_bbox, GetBBox(iFace));

	// if the bounding box is exactly the same size as before then it's always ok
	if ((Length(m_bbox.GetCenter() - newbbox.GetCenter())<0.00000001f) &&
		(Length(m_bbox.GetSize() - newbbox.GetSize())<0.00000001f))
		return true;

	float3 vNewSize = newbbox.GetSize();
	float fMaxSize = m_fMaxSize;

	if (vNewSize.x > fMaxSize ||
		vNewSize.y > fMaxSize ||
		vNewSize.z > fMaxSize)
		return false;

	m_bbox = newbbox;
	return true;
}

// check whether we reckon two faces were originally part of a quad
// TODO: get some information from Ken's AFTK so we can actually -know- for sure whether these triangles were part of the same polygon
bool CTextureChart::AreFacesPartOfQuad(int iFace1, int iFace2, int iEdge)
{
#ifndef WIN32
	return false;	// this doesn't work on GNUC. TODO: figure out why
#endif

	Aftk::C_Polygon *p1 = &m_pMesh->GetPolygon(iFace1);
	Aftk::C_Polygon *p2 = &m_pMesh->GetPolygon(iFace2);

	int i1 = p1->GetBasePolygonIndex();
	if (i1 == -1) i1 = iFace1;
	int i2 = p2->GetBasePolygonIndex();
	if (i2 == -1) i2 = iFace2;

	return i1 == i2;    
}

// returns true if this polygon should not be treated with strict material chart splitting
bool CTextureChart::NonStrictPolygon(int iPoly)
{
	// strict nodes (i.e. track-collision mesh) on instances is not allowed
	if (m_pMesh->GetNumInstancesOfThisNode() > 0)
		return false;

	return GetPolyFlags(
		&m_pMesh->GetPolygon(iPoly), 
		0,
		0,
		0,
		0, 
		0, 
		0).groundCollision; 
}

// checks whether it's ok to add a given face to the current chart
// return values are:
//	0 = ok
//	1 = not ok because of fold-back, out of bounds, or off-axis (this split will cause a possible lightmap seam)
//  2 = not ok because the material is different or the normals don't match (this is a natural split)
int CTextureChart::TestOkToAddFace(int iFace, int iAdjacentFace, int i, bool bFixAllSeams)
{
	bool bStrict = true;

	// if both polys are non-strict, turn off strict mode
	bStrict = !(NonStrictPolygon(iAdjacentFace) && NonStrictPolygon(iFace));

	if (!bStrict)
		m_bTrackSurface = true;
	
	// faces with differing materials can't be part of the same chart even if they were part of the same polygon
	// otherwise supergroup code doesn't work properly
	// NOTE: is this still true? maybe this code can actually be removed; not 100% sure
	if (bStrict)
	{
		if ( m_pMesh->GetPolygon(iFace).GetMaterialMappingIndex() != 
			m_pMesh->GetPolygon(iAdjacentFace).GetMaterialMappingIndex() )
				return CHART_TEST_NOT_OK;
	}

	// if this face is part of a polygon of the original face then always add it to the chart
	if ( !AreFacesPartOfQuad(iFace, iAdjacentFace, i) )
	{
		// check for faces that fold back onto themselves
		// also checks for faces with vertices with differing normals
		int iRet;
		iRet = CheckFaceFoldBack(iFace, iAdjacentFace, i);
		if ( iRet != CHART_TEST_OK )
			return iRet;

		// is it ok to add this face
		iRet = TestAddFacesToChart(iFace, iAdjacentFace, bStrict);
		if ( iRet != CHART_TEST_OK )
			return iRet;

		// check that we don't grow this chart too big
		// depending on the mode, if the chart is too big we don't count it as a seam. this is to stop massive
		// monolithic jobs (e.g. all of the track-surface in a city). these jobs would parallelize badly
		if ( !CheckFaceIsInBounds(iAdjacentFace) )
		{
			if (bFixAllSeams)
				return CHART_TEST_POTENTIAL_SEAM;	// count out of bounds as seam
			else
				return CHART_TEST_NOT_OK;			// don't count it as a seam
		}
	}

	return CHART_TEST_OK; // ok
}

// link other faces with similar normals into this chart
void CTextureChart::TryGrowOneFace(int iFace, ChartSeamGroup *pChartSeams, Ut::Vector<int> &seamFaceIndices, bool bFixAllSeams)
{
	// every face has three neighbours
	for (int i=0; i<3; i++)
	{
		int iAdjacentFace = m_pMesh->GetPolygon(iFace).GetAdjacentPolygonIndex(i);
		if ( iAdjacentFace == -1 ) continue;
		if ( POLY_DATA(iAdjacentFace)->iChart == FACE_CHARTED ) continue;

		int iRet = TestOkToAddFace(iFace, iAdjacentFace, i, bFixAllSeams);
		if (iRet == CHART_TEST_POTENTIAL_SEAM)
		{
			// remember edges that will potentially have a seam on
			pChartSeams->seams.PushBack();
			pChartSeams->seams.Back().iTriangle1 = iFace;
			pChartSeams->seams.Back().iTriangle2 = iAdjacentFace;
			pChartSeams->seams.Back().iEdge1 = i;

			// remember to process the other triangle as part of this seam group, unless it has been charted already
			S32 polyChart = POLY_DATA(iAdjacentFace)->iChart;
			if (polyChart == FACE_NOT_CHARTED) 
				seamFaceIndices.PushBack(iAdjacentFace);
		}

		if (iRet != CHART_TEST_OK)
			continue;

		// yes - add this face to our chart
		assert( iAdjacentFace != iFace );
		assert( m_iLastFace != iAdjacentFace );
		AddFace( iAdjacentFace );
	}
}

// add a face to the linked list of faces in this chart
void CTextureChart::AddFace(int iAdjacentFace)
{
	POLY_DATA(m_iLastFace)->iChartLink = iAdjacentFace;
	POLY_DATA(iAdjacentFace)->iChart = FACE_CHARTED;
	m_iLastFace = iAdjacentFace;
}

// keep adding faces to this chart until we can't add any more
void CTextureChart::Grow(ChartSeamGroup *pChartSeams, Ut::Vector<int> &seamFaceIndices, bool bFixAllSeams)
{
	int iCurFace = m_iFace;
	m_bbox = GetBBox(iCurFace);

	// walk through the link list, linking in more faces as we go
	while (iCurFace != -1)
	{
        TryGrowOneFace(iCurFace, pChartSeams, seamFaceIndices, bFixAllSeams);
		iCurFace = POLY_DATA(iCurFace)->iChartLink;
	}
}

// CalculateBBoxAndMatrix for a particular angle
void CTextureChart::CalculateBBoxAndMatrix(box3f &boundingBox, float fAngle, float3 &vAverageNormal, float4x4 &m_UVMatrix)
{
	int iCurFace;

	boundingBox = box3f();

	m_UVMatrix.Align(180+fAngle, vAverageNormal);
	HMat44 hmat = *(HMat44*)&m_UVMatrix;
	hmat = hmat.Inverse();	// invert matrix using vecmat.h
	m_UVMatrix = hmat;

	float3 testx(1,0,0), testy(0,1,0);

	testx = testx * m_UVMatrix;
	testy = testy * m_UVMatrix;

	// calculate the bounding box
	for (iCurFace = m_iFace; iCurFace != -1; iCurFace = POLY_DATA(iCurFace)->iChartLink)
	{
		Aftk::C_Polygon *p = &m_pMesh->GetPolygon(iCurFace);

		// update bounding box with this face's three vertices
		for (int i=0; i<3; i++)
		{
			float3 v = POLYVERT_POS( p->GetPolygonVertex(i) );
			v = v * m_UVMatrix;
			boundingBox.UpdateBox(v);
		}
	}

	// pad out track-surface bounding box to whole integers, this is to line up adjacent charts to fix track-surface seams
	if (m_bTrackSurface)
	{
		float fScale = 1.f;
		boundingBox.vmin.x -= 0.1f;
		boundingBox.vmin.y -= 0.1f;
		boundingBox.vmin.z -= 0.1f;
		boundingBox.vmax.x += 0.1f;
		boundingBox.vmax.y += 0.1f;
		boundingBox.vmax.z += 0.1f;
		boundingBox.vmin.x = fScale*floor(boundingBox.vmin.x/fScale);
		boundingBox.vmin.y = fScale*floor(boundingBox.vmin.y/fScale);
		boundingBox.vmin.z = fScale*floor(boundingBox.vmin.z/fScale);
		boundingBox.vmax.x = fScale*ceil(boundingBox.vmax.x/fScale);
		boundingBox.vmax.y = fScale*ceil(boundingBox.vmax.y/fScale);
		boundingBox.vmax.z = fScale*ceil(boundingBox.vmax.z/fScale);
	}
}

// generate UVs
ChartMatrix CTextureChart::GenerateUVs(Aftk::Matrix44 *pMat, float fAtlasScale, float fMaxSize)
{
	// calculate bounding box
	int iCurFace;
	box3f boundingBox;
	float fBestAngle;
	float fBestBBoxSize;
	float fAngle, l;

	float3 vAverageNormal(0,0,0);

	m_pWorldMatrix = pMat;
	// TODO get scale from matrix and scale the UV maps accordingly
//	float fMatrixScale = pMat->At<0>().X() 

	// calculate the average normal
	for (iCurFace = m_iFace; iCurFace != -1; iCurFace = POLY_DATA(iCurFace)->iChartLink)
		vAverageNormal += POLY_DATA(iCurFace)->vNormal;

	vAverageNormal.Normalize();

	float4x4 m_UVMatrix;		// matrix that transforms world position into UV coordinates

	// calculate the UV matrix's rotation
	// TODO: might be able to speed this up with a binary search or something like that
	fBestBBoxSize = FLT_MAX;

	if (m_bTrackSurface)
	{
		// for track-surface the angle is always 0. this minimises chart seams by making sure adjacent charts always line up perfectly
		fBestAngle = 0;

		// quantize normal
		int iAxis = 0;
		bool bPositive = false;
		float fBest = fabs(vAverageNormal.x);
		if (fabs(vAverageNormal.y) > fBest)
		{
			iAxis = 1;
			fBest = fabs(vAverageNormal.y);
		}
		if (fabs(vAverageNormal.z) > fBest)
		{
			iAxis = 2;
		}
		bPositive = (vAverageNormal[iAxis]>0);
		vAverageNormal[0] = vAverageNormal[1] = vAverageNormal[2] = 0;
		vAverageNormal[iAxis] = bPositive?1.f:-1.f;
	}
	else
	{
		// find the best angle that minizes the size of this chart
		for (fAngle = 0; fAngle<90.0f; fAngle+=1.0f)
		{
			CalculateBBoxAndMatrix(boundingBox, fAngle, vAverageNormal, m_UVMatrix);

			// calculate surface area of this bbox in X/Y
			float3 bc = boundingBox.GetSize();
			l = bc.x*bc.y;

			if (l<fBestBBoxSize)
			{
				fBestBBoxSize = l;
				fBestAngle = fAngle;	
			}
		}
	}
	CalculateBBoxAndMatrix(boundingBox, fBestAngle, vAverageNormal, m_UVMatrix);

	// calculate the UV matrix's offset
	m_UVMatrix.PostMultTranslate(-boundingBox.vmin.x, -boundingBox.vmin.y, -boundingBox.vmin.z);

	// calculate the UV matrix's scale
	float3 vBoxSize = boundingBox.GetSize();

	// make the box horizontal
	if (vBoxSize.x < vBoxSize.y)
	{
		fBestAngle += 90.f;
		CalculateBBoxAndMatrix(boundingBox, fBestAngle, vAverageNormal, m_UVMatrix);
		// calculate the UV matrix's offset
		m_UVMatrix.PostMultTranslate(-boundingBox.vmin.x, -boundingBox.vmin.y, -boundingBox.vmin.z);
		// calculate the UV matrix's scale
		vBoxSize = boundingBox.GetSize();
	}

//	assert(vBoxSize.x >= vBoxSize.y);

	fAtlasScale *= m_fRelativeTexelDensity; 

	int iHeight = (int) (vBoxSize.y * fAtlasScale );
	int iWidth = (int) (vBoxSize.x * fAtlasScale );

	CTextureAtlas::ClampTextureSize(iWidth, iHeight, fMaxSize);

	m_iTexHeight = iHeight;
	m_iTexWidth  = iWidth;

	// prevent divide by zero
	if (vBoxSize.x == 0) vBoxSize.x=1.0f;
	if (vBoxSize.y == 0) vBoxSize.y=1.0f;
	vBoxSize.z = 1.0f;

	vBoxSize.x = 1.0f / vBoxSize.x;
	vBoxSize.y = 1.0f / vBoxSize.y;
	vBoxSize.z = 1.0f / vBoxSize.z;
	m_UVMatrix.PostMultScale(vBoxSize.x, vBoxSize.y, vBoxSize.z);

	int iNewTextureCoordsStart = m_pMesh->GetNumTextureCoordinates(m_iLMapUVSet);

	// do the mapping
	for (iCurFace = m_iFace; iCurFace != -1; iCurFace = POLY_DATA(iCurFace)->iChartLink)
	{
		Aftk::C_Polygon *p = &m_pMesh->GetPolygon(iCurFace);

		// map the UVs
		for (int i=0; i<3; i++)
		{
			Aftk::C_PolygonVertex* pPolyVert = &p->GetPolygonVertex(i);

			float3 v = POLYVERT_POS((*pPolyVert));
			v = v * m_UVMatrix;

			// this vertex may already be mapped to the right coordinates
			//Aftk::TextureCoordinate4 *pUV = POLYVERT_UV(vi); 

			S32 textureCoordIdx = pPolyVert->GetTextureCoordinateIndex(m_iLMapUVSet);
			Aftk::TextureCoordinate4 *pUV = &m_pMesh->GetTextureCoordinate(m_iLMapUVSet, textureCoordIdx);

			if ( textureCoordIdx >= iNewTextureCoordsStart)
			{
				// this texture coordinate was generated by this chart, so no need to generate another one
				continue;
			}

			// if this UV was already mapped we need to clone (split) the 'polygonvertex'
			if (pUV->x != -1)
			{
				Aftk::C_PolygonVertex myNewVert;
				myNewVert = *pPolyVert;
				U32 iNewVtx = m_pMesh->AddPolygonVertex(myNewVert);	// create a new polygonvert
				p->SetPolygonVertexIndex(i, iNewVtx);
			}

			// create the new texture coordinate and set it to this vertex
			Aftk::TextureCoordinate4 uv(0,0,0,0);
			uv.x = v.x;
			uv.y = v.y;
			int iUV = m_pMesh->AddTextureCoordinate(m_iLMapUVSet, uv);

			p->GetPolygonVertex(i).SetTextureCoordinateIndex(m_iLMapUVSet, iUV);
		}
	}

	ChartMatrix rtn;
	rtn.uTransform.x = m_UVMatrix.AX().x;
	rtn.uTransform.y = m_UVMatrix.AY().x;
	rtn.uTransform.z = m_UVMatrix.AZ().x;
	rtn.uTransform.w = m_UVMatrix.AW().x;

	rtn.vTransform.x = m_UVMatrix.AX().y;
	rtn.vTransform.y = m_UVMatrix.AY().y;
	rtn.vTransform.z = m_UVMatrix.AZ().y;
	rtn.vTransform.w = m_UVMatrix.AW().y;

	return rtn;
}

// set this chart's number into it's faces
void CTextureChart::SetChartNumberToPolygons(int iNum)
{
	int iCurFace = m_iFace;

	while (iCurFace != -1)
	{
		POLY_DATA(iCurFace)->iChart = iNum;
		POLY_DATA(iCurFace)->iAtlas = iNum;

		// also set chart dimensions to polydata
		POLY_DATA(iCurFace)->m_x	= 0;
		POLY_DATA(iCurFace)->m_y	= 0;
		POLY_DATA(iCurFace)->m_cx   = m_iTexWidth;
		POLY_DATA(iCurFace)->m_cy   = m_iTexHeight;

		iCurFace = POLY_DATA(iCurFace)->iChartLink;
	}
}

void CTextureChart::RescaleUVs(int *piHashTable, int iHashPass)
{
	int iCurFace = m_iFace;
	float fScaleFactor[2];

#define NEW_METHOD
	int iNewTexWidth	= m_iTexWidth ;//+SCALE_AMOUNT;
	int iNewTexHeight	= m_iTexHeight;//+SCALE_AMOUNT;

	// pad out texture to 4x4 sizes, and reserve some space around the edge of the chart (experimental)
	// we don't do this for track-surface because we don't want to mess with the aligmnent of adjacent track-surface charts
	if (m_bTrackSurface)
		return;

	iNewTexWidth  = (iNewTexWidth +3) & ~3;
	iNewTexHeight = (iNewTexHeight+3) & ~3;

#define EPSILON (1.0e-5)

#ifdef NEW_METHOD
	fScaleFactor[0] = (float)(iNewTexWidth-1) / ((float)(iNewTexWidth)+EPSILON);
	fScaleFactor[1] = (float)(iNewTexHeight-1) / ((float)(iNewTexHeight)+EPSILON);
#else
	fScaleFactor[0] = (float)(m_iTexWidth) / (float)(iNewTexWidth);
	fScaleFactor[1] = (float)(m_iTexHeight) / (float)(iNewTexHeight);
#endif

	float xmin = (0.5f+EPSILON) / (float)(iNewTexWidth);
	float ymin = (0.5f+EPSILON) / (float)(iNewTexHeight);
	double xmax = ((float)(iNewTexWidth)-0.5-EPSILON) / (float)(iNewTexWidth);
	double ymax = ((float)(iNewTexHeight)-0.5-EPSILON) / (float)(iNewTexHeight);

	int nUVVerts = m_pMesh->GetNumTextureCoordinates(m_iLMapUVSet);

	while (iCurFace != -1)
	{
		Aftk::C_Polygon *p = &m_pMesh->GetPolygon(iCurFace);

		// rescale the UVs
		for (int i=0; i<3; i++)
		{
			Aftk::C_PolygonVertex* pv = &p->GetPolygonVertex(i);

			S32 uvIndex = pv->GetTextureCoordinateIndex(m_iLMapUVSet);

			assert(uvIndex>=0 && uvIndex<nUVVerts);
			if (piHashTable[uvIndex] == iHashPass) continue; // don't process the same vertex twice

			Aftk::TextureCoordinate4 *pUV = &m_pMesh->GetTextureCoordinate(m_iLMapUVSet, uvIndex);

			if (pUV->x < 0) pUV->x = 0;
			if (pUV->y < 0) pUV->y = 0;

#ifdef NEW_METHOD
			pUV->x =
				((pUV->x - 0.5f) * fScaleFactor[0]) + 0.5f;
				
			pUV->y =
				((pUV->y - 0.5f) * fScaleFactor[1]) + 0.5f;
#else
			pUV->x =
				((pUV->x - 0.5f) * fScaleFactor[0]) + 0.5f;
				
			pUV->y =
				((pUV->y - 0.5f) * fScaleFactor[1]) + 0.5f;
#endif

			piHashTable[uvIndex] = iHashPass;
		}

		// next
		iCurFace = POLY_DATA(iCurFace)->iChartLink;
	}

	m_iTexWidth = iNewTexWidth;
	m_iTexHeight = iNewTexHeight;
}

// --- Texture Atlas

CTextureAtlas::CTextureAtlas()
	: m_pMesh(NULL)
	, m_iDoNotLightPolyflagMask(1<<11)
	, m_iLowResLightmapPolyflag(1<<12)
	, m_bFixAllSeams(false)
{
}

// return nearest power of two
int PowerOf2(int a)
{
	int x=2;
	while (1)
	{
		if (a<=x) return x;
		x=x*2;
	}
}

// get the index of the mesh chart UV channel
bool CTextureAtlas::IsMeshCharted()
{
	bool bNew = true;

	for (int i=0; i<m_pMesh->GetNumTextureCoordinateSets(); i++)
	{
		if ( m_pMesh->GetTextureCoordinateSetName(i) == LIGHT_MAP_UV_SET_NAME )
		{
			m_iLMapUVSet = i;
			bNew = false;
			break;
		}
	}

	MEMORY_TRACKER_PUSH("tools.aftk.uvs");

	if (bNew)
	{
		// add a texture coordinate set to use as UV set for charting
		m_iLMapUVSet = m_pMesh->GetNumTextureCoordinateSets();
		m_pMesh->SetTextureCoordinateSetName( m_iLMapUVSet, LIGHT_MAP_UV_SET_NAME);
	}

	// mark all vertices with UV2 = -1,-1   this way we know which ones have already been mapped
	// this helps us find vertices that need splitting because they are referenced by polygons from different charts
	Aftk::TextureCoordinate4 uv(0,0,0,0);
	uv.x = -1;
	uv.y = 0;
	int iv = m_pMesh->AddTextureCoordinate(m_iLMapUVSet, uv);

	for (U32 i = 0; i < m_pMesh->GetNumPolygons(0); i++)
	{
		Aftk::C_Polygon* pPoly = &m_pMesh->GetPolygon(i);
		for (U32 j = 0; j < pPoly->GetNumPolygonVertices(); j++)
		{
			Aftk::C_PolygonVertex *ppv = &pPoly->GetPolygonVertex(j);

			ppv->SetTextureCoordinateIndex(m_iLMapUVSet, iv);
		}
	}
	MEMORY_TRACKER_POP("tools.aftk.uvs");

	return false;
}

// attach an LMapData structure to each polygon
void AttachLMapData(Aftk::C_SceneMeshNode *m_pMesh, int iSkipPolyflag)
{
	for (int i=0; i<m_pMesh->GetNumPolygons(0); i++)
	{
		Aftk::C_Polygon* pPoly = &m_pMesh->GetPolygon(i);

		if (pPoly->GetAttachDataPtr())
			pPoly->DeleteAttachData();

		LMPolyAttributes data;

		// skip polygons that have been flagged with "donotlight"
		// check double-sided flag
		if (GetPolyFlags(
			pPoly, 
			0, //double-sided mask
			iSkipPolyflag, //do not light mask
			0,
			0, //emit mask
			0, //low res lightmap mask
			0 //low res lightmap bits
			).doNotReceiveShadows)
			data.iChart = FACE_CHARTED;

		pPoly->SetAttachData(data);
	}
}

// main function
void CTextureAtlas::ParameterizeMeshUVs(Aftk::C_SceneMeshNode *pMesh, TChartList *pChartList, float fAtlasScale, float fMaxSize, Aftk::Matrix44 *pMatrix, int iLightingPass, float *pfAltTexDensityRatios, TSeamGroupList &seamGroups, TChartMatrixList *pChartMatrixList, bool bFirst)
{
	m_pMesh		   = pMesh;
	m_pChartList   = pChartList;
	m_pWorldMatrix = pMatrix;
	m_pfAlternateTexelDensityRatios = pfAltTexDensityRatios;
	g_pCurrentTextureAtlas = this;

	int i, nFaces = m_pMesh->GetNumPolygons(0);
	int nCharts = pChartList->Size(); // number of current chart
	int iOffset = nCharts;

	// generate lightmapping UV set for this mesh
	IsMeshCharted();

	// handle the case of "UV override" UV set
//	if (CheckForUVOverride(fMaxSize / fAtlasScale / 2.0f, iLightingPass, fMaxSize, seamGroups, bFirst))
//		return;

	// before we start, apply the world-space matrix to all the vertices and cache that
	int nVP = m_pMesh->GetNumVertexPositions();
	for (i=0; i<nVP; i++)
		g_pVerts[i] = LPVS::ApplyMatrix(*pMatrix, pMesh->GetVertexPosition( i ) );

	MEMORY_TRACKER_PUSH("tools.lux.server.unwrapper.lmapdata");

	AttachLMapData(m_pMesh, m_iDoNotLightPolyflagMask);

	// generate face normals
	CGenerateFaceNormals gf;
	gf.GenerateFaceNormals(pMesh);

	MEMORY_TRACKER_POP("tools.lux.server.unwrapper.lmapdata");

	// create a chart seam group for this whole node
	// TODO: ability to generate multiple chart seam groups per node, for better job granularity
	ChartSeamGroup *pChartSeamGroup = NULL;

	// list of faces that should be added to the current seam group
	Ut::Vector<int> seamFaceIndices;

	// create a Texture Chart for each face of the mesh
	for (i=0; i<nFaces; i++)
	{
		// skip this face if it already belongs to another chart
		S32 polyChart = POLY_DATA(i)->iChart;
		if (polyChart != FACE_NOT_CHARTED) 
			continue;

		seamFaceIndices.PushBack(i);

		MEMORY_TRACKER_PUSH("tools.lux.server.unwrapper.seamgroups");

		// create a new group for this chart
		seamGroups.PushBack();
		pChartSeamGroup = &seamGroups.Back();

		// associate these charts with the seam group
		pChartSeamGroup->iChart = pChartList->Size();
		pChartSeamGroup->iLightingPass = iLightingPass;

		while (seamFaceIndices.Size()>0)
		{
			int iCurFace = seamFaceIndices.Back();
			seamFaceIndices.PopBack();

			S32 polyChart = POLY_DATA(iCurFace)->iChart;
			if (polyChart != FACE_NOT_CHARTED) 
				continue;

			MEMORY_TRACKER_PUSH("tools.lux.server.unwrapper.charts");

			// initialize a new chart
			CTextureChart *pNewChart = new CTextureChart();
			m_pChartList->PushBack(pNewChart);
			pNewChart->Initialize(m_pMesh,iCurFace);
			pNewChart->m_iLMapUVSet = m_iLMapUVSet;
			pNewChart->m_fMaxSize   = fMaxSize / fAtlasScale / 2.0f;
			pNewChart->m_bRemap = bFirst;
			// check polyflag of the first face - if it's set to 'low res lightmaps' then make the whole chart low-res
			PolyFlags data = GetPolyFlags(
				&m_pMesh->GetPolygon(pNewChart->m_iFace), 
				0, //double-sided mask
				0, //do not light mask
				0, //light end maks
				0, //emit mask
				m_iLowResLightmapPolyflag, //low res lightmap mask
				m_iLowResLightmapBits); //low res lightmap bits
			pNewChart->m_fRelativeTexelDensity = 
				CalcLightmapResolutionFromFlags(data.lightmapRes, fAtlasScale);
			// adjust maximum size if low res poly flags are set
			pNewChart->m_fMaxSize /= m_pChartList->At(nCharts)->m_fRelativeTexelDensity;

			// collect all other eligible faces for this chart
			m_pChartList->At(nCharts)->Grow(pChartSeamGroup, seamFaceIndices, m_bFixAllSeams);
			m_pChartList->At(nCharts)->m_bLastInChartSeamGroup = false;

			MEMORY_TRACKER_POP("tools.lux.server.unwrapper.charts");

			nCharts++;
		}

		MEMORY_TRACKER_POP("tools.lux.server.unwrapper.seamgroups");

		// set this chart to be the last on in the seam group
		m_pChartList->At(nCharts-1)->m_bLastInChartSeamGroup = true;
	}

	MEMORY_TRACKER_PUSH("tools.aftk.uvs");

	// actually generate UVs
	for (i=iOffset; i<nCharts; i++)
	{
		ChartMatrix mtx = m_pChartList->At(i)->GenerateUVs(m_pWorldMatrix, fAtlasScale, fMaxSize - SCALE_AMOUNT*2);
		if (pChartMatrixList)
			pChartMatrixList->PushBack(mtx);
		m_pChartList->At(i)->SetChartNumberToPolygons(i);
	}

	// rescale UVs to add an extra pixel around the edge of the texture
	int nUVVerts = m_pMesh->GetNumTextureCoordinates(m_iLMapUVSet);
	int *m_piHashTable = new int [nUVVerts];
	memset(m_piHashTable, -1, sizeof(int) * nUVVerts);
	for (i=iOffset; i<nCharts; i++)
		m_pChartList->At(i)->RescaleUVs(m_piHashTable, i);

	MEMORY_TRACKER_POP("tools.aftk.uvs");

	delete [] m_piHashTable;
}

// verify that all the faces in this chart have the same material
void CTextureChart::Validate()
{
	int iFace = m_iFace;
	const Aftk::C_MaterialResource *pMat = NULL;

	// loop through the faces of this chart
	while (iFace != -1)
	{
		// check everything has the same material assigned
		if (!pMat)
			pMat = GetMaterialResourcePtr(&m_pMesh->GetPolygon(iFace));
		else
			assert(pMat == GetMaterialResourcePtr(&m_pMesh->GetPolygon(iFace)));

		iFace = POLY_DATA(iFace)->iChartLink;
	}		
}

// constructor
CTextureChart::CTextureChart() 
{
	m_bRemap = true;
	m_bReferenceGeometry = false;
	m_bBaseInstanceDoNotExport = false;
	m_fRelativeTexelDensity = 1.0f;
	m_bTrackSurface = false;
}

// clone an existing chart (for instancing)
CTextureChart::CTextureChart(CTextureChart *pOrg)
{
	m_pMesh			= pOrg->m_pMesh;
	m_iLMapUVSet	= pOrg->m_iLMapUVSet;
	m_fMaxSize		= pOrg->m_fMaxSize;

	m_iFace			= pOrg->m_iFace;
	m_iLastFace		= pOrg->m_iLastFace;
	m_iTexWidth		= pOrg->m_iTexWidth;
	m_iTexHeight	= pOrg->m_iTexHeight;
	m_fRelativeTexelDensity = pOrg->m_fRelativeTexelDensity;
	m_bRemap		= pOrg->m_bRemap;
	m_bBaseInstanceDoNotExport = false;
	m_bTrackSurface	= pOrg->m_bTrackSurface;
}

// check whether we have a UV override set, and if so copy it over and fill in the CTextureChart class
bool CTextureAtlas::CheckForUVOverride(float fScale, int iLightingPass, float fMaxSize, TSeamGroupList &seamGroups, bool bFirst)
{
	// look for override UV set
	int i;
	for (i=0; ; i++)
	{
		if ( i>=m_pMesh->GetNumTextureCoordinateSets() )
			return false;	// didn't find it

		if ( m_pMesh->GetTextureCoordinateSetName(i) == OVERRIDE_LIGHT_MAP_UV_SET_NAME)
			break;			// found it
	}
	int iOverrideSet = i;
	int iLMapSet = m_iLMapUVSet;

	// copy over the UV data
	int n = m_pMesh->GetNumTextureCoordinates(iOverrideSet);
	m_pMesh->SetTextureCoordinateInitialCapacity(iLMapSet,n);
	for (U32 x=0; i<n; i++)
		m_pMesh->AddTextureCoordinate(iLMapSet, m_pMesh->GetTextureCoordinate(iOverrideSet, x));

	// attach poly data
	AttachLMapData(m_pMesh, m_iDoNotLightPolyflagMask);

	// generate face normals
	CGenerateFaceNormals gf;
	gf.GenerateFaceNormals(m_pMesh);

	// create a chart containing all the faces in this meshnode
	CTextureChart *pNewChart = new CTextureChart();
	m_pChartList->PushBack(pNewChart);
	pNewChart->m_iLMapUVSet = m_iLMapUVSet;
	pNewChart->m_fMaxSize   = fScale;
	pNewChart->m_pMatrixNode	= m_pMesh;
	pNewChart->m_pMesh			= m_pMesh;
	pNewChart->m_bRemap			= bFirst;

	// generate a seam group for this chart
	seamGroups.PushBack();
	ChartSeamGroup *pChartSeamGroup = &seamGroups.Back();
	pChartSeamGroup->iChart = m_pChartList->Size()-1;
	pChartSeamGroup->iLightingPass = iLightingPass;
	// this is the only chart in the group
	pNewChart->m_bLastInChartSeamGroup = true;

	// copy over the uv indices + add polygons to chart + generate bounding box
	box3f bbox;
	{
	for (U32 i=0; i<m_pMesh->GetNumPolygons(0); i++)
	{
		Aftk::C_Polygon* pPoly = &m_pMesh->GetPolygon(i);
		for (U32 j = 0; j < pPoly->GetNumPolygonVertices(); j++)
		{
			Aftk::C_PolygonVertex *ppv = &pPoly->GetPolygonVertex(j);

			bbox.UpdateBox( m_pMesh->GetVertexPosition(ppv->GetPositionIndex()) );

			ppv->SetTextureCoordinateIndex(m_iLMapUVSet, ppv->GetTextureCoordinateIndex(iOverrideSet));
		}

		// add face to chart
		if (i==0)
			pNewChart->m_iFace = pNewChart->m_iLastFace = i;
		else
			pNewChart->AddFace(i);
	}
	}

	// calculate chart dimensions from bounding box
	float3 v = bbox.GetSize();
	float f = v.x;
	if (v.y>f) f = v.y;
	if (v.z>f) f = v.z;
	int iWidth = f * fScale * 4;
	int iHeight = f * fScale * 4;
	ClampTextureSize(iWidth,iHeight,fMaxSize);
	pNewChart->m_iTexWidth		= iWidth;
	pNewChart->m_iTexHeight		= iHeight;
	pNewChart->m_bReferenceGeometry = false;
	pNewChart->m_bbox = bbox;

	// create pixel-boundary along the edge of the chart
	int nUVVerts = m_pMesh->GetNumTextureCoordinates(m_iLMapUVSet);
	int *m_piHashTable = new int [nUVVerts];
	memset(m_piHashTable, -1, sizeof(int) * nUVVerts);
	pNewChart->RescaleUVs(m_piHashTable, i);

	delete [] m_piHashTable;

	return true;
}

void CTextureAtlas::ClampTextureSize(int &iWidth, int &iHeight, float fMaxSize)
{
	// clamp texture sizes - no texture that are too big
	if (iHeight>fMaxSize)
	{
		float fScale = fMaxSize/iHeight;
		iHeight*=fScale;
		iWidth *=fScale;
	}
	if (iWidth>fMaxSize)
	{ 
		float fScale = fMaxSize/iWidth;
		iHeight*=fScale;
		iWidth *=fScale;
	}

	if (iHeight<1) iHeight = 1;
	if (iWidth<1)  iWidth = 1;
}
/*
// returns the low res lightmap bits given a polyflag value
float CTextureAtlas::GetLowResLightmapBits(int iBits)
{
	iBits = iBits >> m_iLowResLightmapPolyflag;
	int iMaxValue = (1<<m_iLowResLightmapBits)-1;
	iBits &= iMaxValue;
	return (iBits / (float)iMaxValue);
}
*/

// calculate relative resolution from bits
float CTextureAtlas::CalcLightmapResolutionFromFlags(U32 lightmapRatio, float fTexelDensity)
{
	assert(lightmapRatio >= 0 && lightmapRatio < 5);
	return fTexelDensity * m_pfAlternateTexelDensityRatios[lightmapRatio];
}
