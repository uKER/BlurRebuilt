#pragma once

namespace NexusRender
{
	struct SceneRendererSettings
	{
		SceneRendererSettings()
		{
			useRealTimeLighting = true;
			useTextures = false;
			useDiffuseTextures = true;
			useLightmapTextures = true;
			renderNormals = false;

			renderAxes = false;
			renderSplines = true;
			renderNulls = true;
			renderJoints = true;
			renderPointLights = true;
			renderDirectionalLights = true;
			renderSpotLights = true;
			renderAreaLights = true;
			renderLights = true;
			renderInstances = true;
			renderMeshes = true;
			lightmapMode = 0;
			componentMode = 0;
			showGrid = true;
			backFaceCulling = true;
			tangentSpaceMode = -1;
			skinningMode = 0;
			boundingBoxMode = 0;
			useCropSheet = false;
			globalScaleCompensation = false;
			fillMode = E_FillMode_Solid;
		}

		bool useRealTimeLighting;
		bool useTextures;
		bool useDiffuseTextures;
		bool useLightmapTextures;
		bool renderNormals;
		bool globalScaleCompensation;

		bool showGrid;
		U32 componentMode;
		
		bool renderAxes;
		bool renderSplines;
		bool renderNulls;
		bool renderJoints;
		bool renderPointLights;
		bool renderDirectionalLights;
		bool renderSpotLights;
		bool renderAreaLights;
		bool renderLights;
		bool renderInstances;
		bool renderMeshes;
		S32  tangentSpaceMode;
		U32	 lightmapMode;
		bool useCropSheet;

		bool backFaceCulling;

		U32 boundingBoxMode;

		U32 skinningMode;

		E_FillMode fillMode;
	};
}
