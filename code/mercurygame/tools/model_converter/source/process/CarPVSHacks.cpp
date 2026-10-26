#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <converter_shared/helpers/AftkProgress.h>

#include <data/DataTypes.h>
#include <data/PvsData.h>
#include <data/AftkSceneData.h>

#include <data/ConversionOptions.h>

#include <process/ReadPvsData.h>

#include <process/CarPVSHacks.h>

// nLongSegments(16)
// nLatSegments(8)
// nDistanceSegments(4)
// this means that there are 128 PVS cells per distance segment, and there are 4 distance segments (512 cells in total)

/// constructor
C_PVSDataEditor::C_PVSDataEditor(const Aftk::C_Scene *pScene, C_DataPvs *pData)
: m_pData(pData)
, m_pScene(pScene)
{
}

/// destructor
C_PVSDataEditor::~C_PVSDataEditor()
{
}

/// unify PVS data for all polygons on the object, such that if any polygon is visible, the entire object is visible
void C_PVSDataEditor::Merge(const Aftk::C_SceneNode *pNode, int iMinCell, int iMaxCell)
{
	U32 iGUID = pNode->GetUniqueId();

	Pvs::Build::PvsData *pPVSData = (Pvs::Build::PvsData*)&m_pData->GetPVSData();

	Btl::BitVector mergedBits;
	mergedBits.Resize(iMaxCell-iMinCell);
	mergedBits.ClearBits();

	const Btl::Vector<Pvs::DatabaseFilePrimitive> &prims = pPVSData->GetPvsPrimitives();

	// first pass - merge
	for (int iPrim=0; iPrim<prims.Size(); iPrim++)
	{
		if (prims[iPrim].iNode == iGUID)
		{
			Btl::BitVector &bits = pPVSData->GetPvsBits(iPrim);

			for (int iCell=iMinCell; iCell<iMaxCell; iCell++)
				mergedBits[iCell-iMinCell] = mergedBits[iCell-iMinCell] | bits[iCell];
		}
	}	

	// second pass - splat
	for (int iPrim=0; iPrim<prims.Size(); iPrim++)
	{
		if (prims[iPrim].iNode == iGUID)
		{
			Btl::BitVector &bits = pPVSData->GetPvsBits(iPrim);

			for (int iCell=iMinCell; iCell<iMaxCell; iCell++)
				bits.SetBitValue(iCell, mergedBits[iCell-iMinCell]);
		}
	}	
}

/// force all polygons on the object to be visible for the given range of PVS cells
void C_PVSDataEditor::ForceVisible(const Aftk::C_SceneNode *pNode, int iMinCell, int iMaxCell)
{
	U32 iGUID = pNode->GetUniqueId();

	Pvs::Build::PvsData *pPVSData = (Pvs::Build::PvsData*)&m_pData->GetPVSData();

	const Btl::Vector<Pvs::DatabaseFilePrimitive> &prims = pPVSData->GetPvsPrimitives();

	for (int iPrim=0; iPrim<prims.Size(); iPrim++)
	{
		if (prims[iPrim].iNode == iGUID)
		{
			Btl::BitVector &bits = pPVSData->GetPvsBits(iPrim);

			for (int iCell=iMinCell; iCell<iMaxCell; iCell++)
				bits[iCell] = 1;				
		}
	}	
}

void ApplyCarPVSHacks(C_PVSDataEditor *pEditor, const Aftk::C_SceneNode *pNode, bool bForceVisible, bool bUnify)
{
	Util::C_String name(pNode->GetName().c_str());
	if (name.ISearch("tyre_front") != -1)
		bForceVisible = true;

	if (name.ISearch("_bonnet") != -1)
		bUnify = true;

	if (pNode->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		if (bForceVisible)
			pEditor->ForceVisible(pNode, 0, 128);

		if (bUnify)
			pEditor->Merge(pNode, 0, 128);
	}

	for (U32 i=0; i<pNode->GetNumChildren(); i++)
	{
		ApplyCarPVSHacks(pEditor, pNode->GetChildPtr(i), bForceVisible, bUnify);
	}
}

// hack the PVS data slightly to account for PVS problems on damaged cars
void ApplyCarPVSHacks(const Aftk::C_Scene *pScene, C_DataPvs *pData)
{
	C_PVSDataEditor editor(pScene,pData);

	ApplyCarPVSHacks(&editor, pScene->GetRootSceneNodePtr(), false, false);
}
