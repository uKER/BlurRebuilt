#pragma once

namespace NexusRender
{
	//The convention used here is that _effects_ represent different materials.
	//Each effect should be able to render all object types.
	class GMaterialEffectWrapper : public GEffectWrapper
	{
	public:
		Tu::Status SetTechnique(
			const SceneRendererSettings* pSettings, 
			GRenderingMode renderingMode = G_RENDERING_MODE_DEFAULT);

	private:
		Tu::Status SetTechniqueNormal(
			const SceneRendererSettings* pSettings, 
			GRenderingMode renderingMode);

		Tu::Status SetTechniqueCropSheet(
			const SceneRendererSettings* pSettings, 
			GRenderingMode renderingMode);
	};
};