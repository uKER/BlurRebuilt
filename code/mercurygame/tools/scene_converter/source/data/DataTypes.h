#ifndef APP_DATA_TYPES_H
#define APP_DATA_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif


enum E_GraphData
{
	E_GraphData_Invalid = -1,

	E_GraphData_ConversionOptions = 0,
	E_GraphData_PreprocessedAftk,
	E_GraphData_PreprocessedAftkFile,
	E_GraphData_LightSamplePointsFile,
	E_GraphData_LitAftkFile,
	E_GraphData_LightSamplesFile,
	E_GraphData_LitAftk,
	E_GraphData_SceneDescription,
	E_GraphData_LightSamples,
	E_GraphData_PvsSamplePointsFile,
	E_GraphData_PvsDataFile,
	E_GraphData_PvsData,
	E_GraphData_RenderData,
	E_GraphData_PortalRooms,
	E_GraphData_PhysicsData,
	E_GraphData_LightList,
	E_GraphData_NavMesh,
	E_GraphData_GameSceneFile,
	E_GraphData_FileList,
	E_GraphData_CubemapPoints,
	E_GraphData_CrowdData,
	E_GraphData_CropSheet,
	E_GraphData_AdvertisingData,
	
	E_GraphData_Count
};

inline const Util::C_String GraphDataName( const E_GraphData graphData )
{
	const char names[E_GraphData_Count][64] = 
	{
		"E_GraphData_ConversionOptions",
		"E_GraphData_PreprocessedAftk",
		"E_GraphData_PreprocessedAftkFile",
		"E_GraphData_LightSamplePointsFile",
		"E_GraphData_LitAftkFile",
		"E_GraphData_LightSamplesFile",
		"E_GraphData_LitAftk",
		"E_GraphData_SceneDescription",
		"E_GraphData_LightSamples",
		"E_GraphData_PvsSamplePointsFile",
		"E_GraphData_PvsDataFile",
		"E_GraphData_PvsData",
		"E_GraphData_RenderData",
		"E_GraphData_PortalRooms",
		"E_GraphData_PhysicsData",
		"E_GraphData_LightList",
		"E_GraphData_NavMesh",
		"E_GraphData_GameSceneFile",
		"E_GraphData_FileList",
		"E_GraphData_CubemapPoints",
		"E_GraphData_CrowdData",
	};

	return names[graphData];
}


#endif
