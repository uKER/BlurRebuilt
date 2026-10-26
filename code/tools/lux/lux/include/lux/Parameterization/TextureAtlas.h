#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#ifdef _WIN32
#pragma once
#endif

#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/Parameterization/ChartSeams.h"

#define FACE_NOT_CHARTED -1
#define FACE_CHARTED	 -2
#define SUPERGROUP_NOT_ASSIGNED -1

#define VERTEX_WELD_EPSILON 0 //0.003

// save us some typing
#define POLYVERT_POS(VERT) g_pVerts[ VERT.GetPositionIndex() ]
#define POLYVERT_NORMAL(VERT) m_pMesh->GetVertexNormal( VERT.GetNormalIndex() )
#define POLY_DATA(IFACE) ((LMPolyAttributes*)m_pMesh->GetPolygon(IFACE).GetAttachDataPtr())
#define POLYVERT_UV(VERT) ( &m_pMesh->GetTextureCoordinate(m_iLMapUVSet, VERT.GetTextureCoordinateIndex(m_iLMapUVSet) ))

class CTextureAtlas;
class CTextureChart;
struct ChartSeamGroup;

typedef Ut::Vector<CTextureChart*> TChartList;

typedef struct
{
	float3		vEdgeA, vEdgeB;
	int			iFace;
} edge_t;

// a 4x2 matrix that transforms a world position into a lightmap uv
struct ChartMatrix
{
	float4	uTransform;
	float4	vTransform;
};
typedef Ut::Vector<ChartMatrix> TChartMatrixList;

extern LUX::Position3 *g_pVerts;		// vertex positions in world-space

typedef enum
{
	AXIS_X_NEGATIVE,
	AXIS_Y_NEGATIVE,
	AXIS_Z_NEGATIVE,
	AXIS_X_POSITIVE,
	AXIS_Y_POSITIVE,
	AXIS_Z_POSITIVE,
} majoraxis_t;

#define CHART_TEST_OK 0
#define CHART_TEST_POTENTIAL_SEAM 1
#define CHART_TEST_NOT_OK 2

class CTextureChart
{
public:

	// iterate through the polygons associated with this chart
	struct PolyIterator
	{
		// init
		PolyIterator(CTextureChart *pChart)
		{
			m_pMesh = pChart->m_pMesh;
			m_iFace = pChart->m_iFace;
		}

		// get current polygon pointer
		Aftk::C_Polygon *GetPoly()
		{
			return &m_pMesh->GetPolygon(m_iFace);
		}

		// get lightmap polygon data associated with current polygon
		LMPolyAttributes *GetData()
		{
			return ((LMPolyAttributes*)m_pMesh->GetPolygon(m_iFace).GetAttachDataPtr());
		}

		// get next polygon
		void Next()
		{
			m_iFace = ((LMPolyAttributes*)m_pMesh->GetPolygon(m_iFace).GetAttachDataPtr())->iChartLink;
		}

		// returns true if we've reached the end of the list
		bool End()
		{
			return m_iFace == -1;
		}
	
		Aftk::C_SceneMeshNode *m_pMesh;
		int m_iFace;
	};

						CTextureChart();
						CTextureChart(CTextureChart *pClone);
						~CTextureChart() {};

	int					m_iFace;		// index to the first face of a linked-list of faces that belong to this chart
	int					m_iLastFace;	// index of end of linked list, so we can easily add extra faces to the end of the list

	unsigned char		m_iMajorAxis;
	unsigned char		m_iUAxis, m_iVAxis;
	unsigned char		m_iLMapUVSet;
	unsigned char		m_bLastInChartSeamGroup;

	int					m_iTexWidth, m_iTexHeight;
	box3f				m_bbox;
	int					m_iAtlas;

	int					m_iUsedSpace;
	float				m_fRelativeTexelDensity;
	float				m_fMaxSize;		// TODO: remove from the structure and pass into the Grow function directly
	bool				m_bRemap;

	int					m_iMaterial;

	int					m_iSuperGroup;

	bool				m_bReferenceGeometry;	// true if this chart originates from a mesh which is itself instanced
	bool				m_bBaseInstanceDoNotExport;		// true if this is a base instance which does not need to be packed
	bool				m_bTrackSurface;		// PGR4 track surface gets treated in a special way to avoid seams

	static const int	axistypes[3][2];

	Aftk::C_SceneMeshNode			*m_pMesh;		// the reference geometry scene node
	Aftk::C_SceneMeshRemappableNode	*m_pMatrixNode;	// the instance scene node
	Aftk::Matrix44					*m_pWorldMatrix;

	void				Initialize(Aftk::C_SceneMeshNode *pMesh, int iFace);
	void				Free();

	int					TestAddFacesToChart(int iCurFace, int iNewFace, bool);
	void				TryGrowOneFace(int iFace, ChartSeamGroup *pChartSeams, Ut::Vector<int> &seamFaceIndices, bool bFixAllSeams);
	void				Grow(ChartSeamGroup *pChartSeams, Ut::Vector<int> &seamFaceIndices, bool bFixAllSeams);
	ChartMatrix			GenerateUVs(Aftk::Matrix44 *pMat, float fAtlasScale, float fMaxSize);
	void				SetChartNumberToPolygons(int iNum);
	bool				TestFaceAgainstEdgeList(int iFace);
	int					CheckFaceFoldBack(int iFace, int iNewFace, int iEdge);
	void				RescaleUVs(int *piHashTable, int iPass);
	bool				TestEdges(int iA1, int iA2, int iB1, int iB2);
	bool				IsInstance();
	bool				NonStrictPolygon(int iPoly);
	int					TestOkToAddFace(int iFace, int iAdjacentFace, int i, bool bFixAllSeams);
	void				CalculateBBoxAndMatrix(box3f &boundingBox, float fAngle, float3 &vAverageNormal, float4x4 &m_UVMatrix);
	bool				ComparePolygonVertices(	const Aftk::C_Polygon* pAftkPoly1,
												const Aftk::C_Polygon* pAftkPoly2,
												int v1, 
												int v2);
	bool				CheckFaceIsInBounds(int iFace);
	void				RecalcBoundingBox();
	box3f				GetBBox(int iFace);
	bool				AreFacesPartOfQuad(int iFace1, int iFace2, int iEdge);
	void				AddFace(int iFace);

	void				ValidateMeshUVs();
	void				Validate();
};

class CTextureAtlas
{
public:
					CTextureAtlas();
					~CTextureAtlas() {};
	
	TChartList							*m_pChartList;
	Aftk::C_SceneMeshNode				*m_pMesh;
	Aftk::Matrix44						*m_pWorldMatrix;
    
	int				m_iLMapUVSet;
	float			m_fMaxSize;
	float			*m_pfAlternateTexelDensityRatios;
	int				m_iHashPass;
	int				m_iDoNotLightPolyflagMask;
	int				m_iLowResLightmapPolyflag;
	int				m_iLowResLightmapBits;
	int				*m_piHashTable;
	bool			m_bFixAllSeams;

	void			ParameterizeMeshUVs(Aftk::C_SceneMeshNode *pMesh, TChartList *pChartList, float fAtlasScale, float fMaxSize, Aftk::Matrix44 *pMatrix, int iLP, float *pfAltTexDensityRatios, TSeamGroupList &seamGroups, TChartMatrixList *pChartMatrixList, bool bFirst);
	void			CreateChartFaceLists();
	void			Purge();
	bool			IsMeshCharted();
	bool			CheckForUVOverride(float fScale, int iLightingPass, float fMaxSize, TSeamGroupList &seamGroups, bool bFirst);
	static void		ClampTextureSize(int &iWidth, int &iHeight, float fMaxSize);
	float			CalcLightmapResolutionFromFlags(U32 lightmapRatio, float fTexelDensity);
};

// inline compare functions

inline bool VectorSame(LUX::Position3 a, LUX::Position3 b)
{
#define DIFF_EPSILON 0.0001f

	for (int i=0; i<3; i++)
	{
		float f = a[i] - b[i];
		if (f>DIFF_EPSILON || f<-DIFF_EPSILON) return false;
	}

	return true;
}

inline bool VectorSame(Aftk::Normal3 a, Aftk::Normal3 b)
{
#define NORM_EPSILON 0.01f

	for (int i=0; i<3; i++)
	{
		float f = a[i] - b[i];
		if (f>NORM_EPSILON || f<-NORM_EPSILON) return false;
	}

	return true;
}

#endif
