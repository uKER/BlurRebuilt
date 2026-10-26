#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeComponentCluster.h"
#include "NativePolygonCluster.h"

using namespace Aftk;

C_Status C_NativePolygonCluster::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_PolygonCluster* pPolygonCluster,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_POLYGON_CLUSTER);
	}
	{
		//Serialise baseclass
		{
			status = C_NativeComponentCluster::Serialise(
				pDiskWriter,
				pNativeFtk,
				pPolygonCluster,
				false);
			if (!status) return status;
		}

		U32 numPolygonIndices = 0;
		for (U32 iLodLevel = 0; iLodLevel < pPolygonCluster->GetNumLodLevels(); iLodLevel++)
		{
			numPolygonIndices += pPolygonCluster->GetNumPolygonIndices(iLodLevel);
		}
		*pDiskWriter << numPolygonIndices;

		for (U32 iLodLevel = 0; iLodLevel < pPolygonCluster->GetNumLodLevels(); iLodLevel++)
		{
			for (U32 i = 0; i < pPolygonCluster->GetNumPolygonIndices(iLodLevel); i++)
			{
				U32 polyIndex = pPolygonCluster->GetPolygonIndex(iLodLevel, i);
				*pDiskWriter << iLodLevel;
				*pDiskWriter << polyIndex;
			}
		}
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	
	return C_Status::E_Status_Success;
}

C_Status C_NativePolygonCluster::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_PolygonCluster* pPolygonCluster,
	C_SceneMeshNode* pSceneMeshNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_POLYGON_CLUSTER))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find polygon cluster block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		//Deserialise baseclass
		{
			status = C_NativeComponentCluster::Deserialise(
				pDiskReader,
				pNativeFtk,
				pPolygonCluster,
				false);
			if (!status) return status;
		}

		U32 numPolygonIndices;
		*pDiskReader >> numPolygonIndices;
		pPolygonCluster->SetPolygonIndexInitialCapacity(0, numPolygonIndices);
		for (U32 i = 0; i < numPolygonIndices; i++)
		{
			if (pDiskReader->GetVersion() < 150)
			{
				U32 polygonIndex;
				*pDiskReader >> polygonIndex;
				pPolygonCluster->AddPolygonIndex(0, polygonIndex);
			}
			else
			{
				U32 lodLevel;
				U32 polygonIndex;
				*pDiskReader >> lodLevel;
				*pDiskReader >> polygonIndex;
				pPolygonCluster->AddPolygonIndex(lodLevel, polygonIndex);
			}

		}

		//Attach the cluster to its mesh
//		pPolygonCluster->AttachToMesh(pSceneMeshNode);
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}

	return C_Status::E_Status_Success;
}

