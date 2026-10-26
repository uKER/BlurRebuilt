#include "stdafx.h"

#include "vis_core\PolyRef.h"
#include "vis_core\D3DRenderer.h"
#include "vis_core\D3DMesh.h"

#include "vis_core\database.h"
#include "vis_core\DatabaseFile.h"
#include "Vis_Core\Sampler.h"

#include "Vis_Core\PVSResult.h"

#include <algorithm>

using namespace std;


MeshInfo::MeshInfo(Aftk::C_Scene *pScene)
{
	for (Aftk::C_SceneNodeIterator it(pScene->GetRootSceneNodePtr()); it; it++)
	{
		Aftk::C_SceneNode *pNode = it;

		if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pNode;

		m_NodePtrs.PushBack( pMesh );
	}
}

MeshInfo::~MeshInfo()
{
}

// read a backbuffer, recognize each polygon that was drawn from their colour and store the visibility information
void MeshInfo::AnalyzeData(int *pData, int c, int iCell, BitfieldDatabase *pDB)
{
	for (int i=0; i<c; i++)
	{
		int x = pData[i];
		if (x == 0x00ffffff)
			continue;
		pDB->Set(x, iCell);
	}
}

// make a list of cells that have no visible polygons in them
void BitfieldDatabase::FindCellsWithNothingVisible(int nCells)
{
	char buffer[512];
	PVSResult result(nCells);

	int iMaxErrorMsg = 20;

	// check each poly
	for (int i=0; i<GetNumRefs(); i++)
	{
		bool bPolyIsVisibleFromAnywhere = false;

		for (int iCell=0; iCell<nCells; iCell++)
		{
			// is this poly visible from this cell?
			if (Get(i, iCell))
			{
				// set this cell's visible flag
				result.SetBit(iCell);
				bPolyIsVisibleFromAnywhere = true;
			}
		}

		if (!bPolyIsVisibleFromAnywhere)
		{
			if (iMaxErrorMsg ==  1)
			{
				LOGDEBUG( g_Log, "... more\n");
				iMaxErrorMsg--;
			}
			else if (iMaxErrorMsg != 0)
			{
				iMaxErrorMsg--;

				Aftk::C_Polygon *pPoly = GetPolygon( m_bitfields[i].m_iID );

				if (!pPoly)
					continue;

				SetPolygonColour( pPoly, FLOAT_PACK_RGBA32(0,1,1,0) );
				LOGDEBUG( g_Log, "Polyref %d is not visible from anywhere\n",i);
			}
		}
	}

	// done - check whether any cells have no visibility at all
	iMaxErrorMsg = 20;
	for (int i=0; i<nCells; i++)
	{
		if (!result.GetBit(i))
		{
			if (iMaxErrorMsg ==  1)
			{
				LOGDEBUG( g_Log, "... more\n");
				iMaxErrorMsg--;
			}
			else if (iMaxErrorMsg != 0)
			{
				iMaxErrorMsg--;
				LOGDEBUG( g_Log, "Cell %d has nothing visible\n",i);
			}
		}
	}
}

// display visibility by setting vertex colours in the mesh
void BitfieldDatabase::DisplayVisibilityInfo(int iCell)
{
	for (int i=0; i<GetNumRefs(); i++)
	{
		Aftk::C_Polygon *pPoly = GetPolygon( m_bitfields[i].m_iID );

		if (!pPoly)
		{
			// must be visibility for an entire node
			Aftk::C_SceneNode *pNode = (Aftk::C_SceneMeshNode*)m_pScene->GetSceneNodePtrByUniqueID((*m_pPolyRefs)[m_bitfields[i].m_iID].m_iSceneNodeID );
			if ( pNode && !Get(i, iCell) )
			{
				// not visible - change the matrix to identity
				Aftk::Matrix44 mtx;

				UtMaths::SetIdentity(&mtx);

				pNode->SetLocalMatrix(mtx);
			}
			continue;
		}

		// is this polygon visibile from the selected cell?
		if ( Get(i, iCell) )
			SetPolygonColour( pPoly, FLOAT_PACK_RGBA32(0,1,0,0) );
		else
			SetPolygonColour( pPoly, FLOAT_PACK_RGBA32(0.5,0,0,0) );
	}
}

// display visibility by setting vertex colours in the mesh
void BitfieldDatabase::DisplayUsageInfo()
{
	int nCells = m_bitfields[0].m_pData->Size() * 8;

	for (int i=0; i<GetNumRefs(); i++)
	{
		Aftk::C_Polygon *pPoly = GetPolygon( m_bitfields[i].m_iID );

		if (!pPoly)
		{
			// must be visibility for an entire node - skip
			continue;
		}

		//
		int nVisibleFrom = 0;
		for (int iCell=0; iCell<nCells; iCell++)
		{
			// is this polygon visibile from the selected cell?
			if ( Get(i, iCell) )
				nVisibleFrom++;
		}

		//
		float f = (float)nVisibleFrom / 64.0f;
		if (f>1.0f)
			f = 1.0f;
		SetPolygonColour( pPoly, FLOAT_PACK_RGBA32(f,0,0,0) );
	}
}

// bitfield classes
void Bitfield::Set(int n)
{
	int iByte = n/8;
	int iBit  = n&7;

	int iBitmask = 1<<iBit;
	(*m_pData)[iByte] |= iBitmask;
}

bool Bitfield::Get(int n)
{
	int iByte = n/8;
	int iBit  = n&7;

	int iBitmask = 1<<iBit;
	return ((*m_pData)[iByte] & iBitmask) != 0;
}

bool Bitfield::operator<(const Bitfield &e) const
{
	assert( (*e.m_pData).Size() == (*m_pData).Size() );

	for (int i=0; i<(*m_pData).Size(); i++)
	{
		if ((*m_pData)[i] != (*e.m_pData)[i])
			return (*m_pData)[i] > (*e.m_pData)[i];
	}

	return false;
}

// bitfield database classes
BitfieldDatabase::BitfieldDatabase(Aftk::C_Scene *pScene, Ut::Vector<PolygonRef> *pPolyRefs, int nCells)
{
	int nBytes = (nCells+7)/8;
	int nPolygons = pPolyRefs->Size();

	m_pScene = pScene;

	m_bitfields.Resize( nPolygons );
	for (int i=0; i<nPolygons; i++)
	{
		m_bitfields[i].m_iID = i;
		m_bitfields[i].m_pData = new Ut::Vector<unsigned char>();
		m_bitfields[i].m_pData->Resize( nBytes );
		memset( &(*m_bitfields[i].m_pData)[0], 0, sizeof(unsigned char)*nBytes );
	}

	// take ownership of ref data
	m_pPolyRefs = pPolyRefs;
}

BitfieldDatabase::~BitfieldDatabase()
{
	for (int i=0; i<m_bitfields.Size(); i++)
	{
		delete m_bitfields[i].m_pData;
	}

	if (m_pPolyRefs)
		delete m_pPolyRefs;
}

void BitfieldDatabase::Set(int iID, int iBit)
{
	m_bitfields[iID].Set(iBit);
}

bool BitfieldDatabase::Get(int iID, int iBit)
{
	return m_bitfields[iID].Get(iBit);
}

// sort by bitfield and combine matching ones
void BitfieldDatabase::Combine()
{
	// TODO: program this!	
	assert(0);

	sort( &m_bitfields[0], &m_bitfields[0] + m_bitfields.Size() );
}

// save database to binary file
bool BitfieldDatabase::Save(const char *psz)
{
	if (!m_bitfields.Size())
		return false;

	// open file
	FILE *f = fopen(psz, "wb");

	if (!f)
		return false;

	// save header
	DatabaseFileHeader header;
	header.iVersion				= DATABASE_FILE_VERSION;
	header.nPrimitives			= m_bitfields.Size();
	header.iBitfieldSizeInBytes	= m_bitfields[0].m_pData->Size();
	fwrite( &header, sizeof(DatabaseFileHeader), 1, f );

	// save primitives
	for (int i=0; i<m_bitfields.Size(); i++)
	{
		int iID = m_bitfields[i].m_iID;
		if (fwrite( &m_pPolyRefs->At(iID), sizeof(PolygonRef), 1, f ) == 0)
			return false;
	}

	// save bitfields
	for (int i=0; i<m_bitfields.Size(); i++)
	{
		/*
		int iCheckCell = 77;

		if ((*m_bitfields[i].m_pData)[iCheckCell/8] & (1<<5))
		{
			LOGDEBUG( g_Log, "prim: %d node: %d poly: %d\n", i, 
											m_pPolyRefs->Element(i).m_iSceneNodeID,
											m_pPolyRefs->Element(i).m_iPolygon);
		}
		//*/

		if (fwrite( &(*m_bitfields[i].m_pData)[0], header.iBitfieldSizeInBytes, 1, f ) == 0)
			return false;
	}

	// done
	fclose( f );

	return true;
}

// load a bitfield database back into memory from a file
BitfieldDatabase::BitfieldDatabase(Aftk::C_Scene *pScene, const std::string filename)
{
	FILE *f = fopen(filename.c_str(), "rb");
	if (!f)
		return; // oops

	m_pScene = pScene;

    DatabaseFileHeader header;
	fread( &header, 1, sizeof(DatabaseFileHeader), f );

	m_bitfields.Resize( header.nPrimitives );
	m_pPolyRefs  = new Ut::Vector<PolygonRef>;
	m_pPolyRefs->Resize( header.nPrimitives );

	// load primitives
	for (int i=0; i<header.nPrimitives; i++)
	{
		fread( &m_pPolyRefs->At(i), 1, sizeof(PolygonRef), f );
	}

	int nBytes = header.iBitfieldSizeInBytes;

	// load bitfields
	for (int i=0; i<header.nPrimitives; i++)
	{
		m_bitfields[i].m_iID = i;
		m_bitfields[i].m_pData = new Ut::Vector<unsigned char>();
		m_bitfields[i].m_pData->Resize( nBytes );
		fread( &(*m_bitfields[i].m_pData)[0], 1, nBytes, f );
	}

	// done
	fclose( f );
}


Aftk::C_SceneMeshNode *BitfieldDatabase::GetSceneMeshNode(int iRef)
{
	if ((*m_pPolyRefs)[iRef].m_iPolygon == -1)
		return NULL;

	return (Aftk::C_SceneMeshNode*)m_pScene->GetSceneNodePtrByUniqueID((*m_pPolyRefs)[iRef].m_iSceneNodeID);
}

Aftk::C_Polygon	*BitfieldDatabase::GetPolygon(int iRef)
{
	Aftk::C_SceneMeshNode *pMesh = GetSceneMeshNode(iRef);

	if (!pMesh || pMesh->GetType() != Aftk::E_ObjType_SceneMesh)
		return NULL;

	return &pMesh->GetPolygon((*m_pPolyRefs)[iRef].m_iPolygon);
}

int	BitfieldDatabase::GetNumRefs()
{
	return m_pPolyRefs->Size();
}

int BitfieldDatabase::GetVisibilityCount(int i)
{
	int nCells = m_bitfields[0].m_pData->Size() * 8;

	Aftk::C_Polygon *pPoly = GetPolygon( m_bitfields[i].m_iID );

	if (!pPoly)
		return -1;

	//
	int nVisibleFrom = 0;
	for (int iCell=0; iCell<nCells; iCell++)
	{
		// is this polygon visibile from the selected cell?
		if ( Get(i, iCell) )
			nVisibleFrom++;
	}

	return nVisibleFrom;
}