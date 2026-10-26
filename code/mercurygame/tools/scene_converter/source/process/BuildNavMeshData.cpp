#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <data/PhysicsData.h>
#include <data/NavMeshData.h>
#include "converter_shared/NavMeshDataConverter.h"
#include <process/BuildNavMeshData.h>

#include <Polyflags.h>

C_ProcessBuildNavMeshData::C_ProcessBuildNavMeshData()
{
}

const Util::C_String C_ProcessBuildNavMeshData::GetName() const
{
	return Util::C_String("C_ProcessBuildNavMeshData");
}

bool C_ProcessBuildNavMeshData::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progress( pProgress, GetName() );

	const C_DataConversionOptions* pInputConversionOptions = (const C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataSceneDescription* pInputSceneDescription = (const C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	assert(pInputSceneDescription);

	C_DataNavMesh* pOutputNavMeshData = (C_DataNavMesh*)GetOutput( E_GraphData_NavMesh );
	assert(pOutputNavMeshData);

	
	pOutputNavMeshData->AddNavMeshVector(pInputSceneDescription->GetNavMeshNodes());

	return true;
}

