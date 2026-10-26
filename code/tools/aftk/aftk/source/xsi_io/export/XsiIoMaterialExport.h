#ifndef XSI_IO_MATERIAL_EXPORT_H
#define XSI_IO_MATERIAL_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{

	class C_SceneExport;

	class C_MaterialExport
	{
	public:

		static Aftk::C_Status Export(
			C_SceneExport *pExporter,
			const Aftk::C_MaterialResource *pAftkMaterialResource,
			CSLBaseMaterial *pXsiBaseMaterial);

	private:

		static Aftk::C_Status ExportDummyMaterial(
			C_SceneExport *pExporter,
			const Aftk::C_MaterialResource *pAftkMaterialResource,
			CSLXSIMaterial *pXsiMaterial);

		static Aftk::C_Status ExportXsiMaterial(
			C_SceneExport *pExporter,
			const Aftk::C_MaterialResource *pAftkMaterialResource,
			CSLXSIMaterial *pXsiMaterial);

		static Aftk::C_Status ExportXsiShader(
			CSLXSIShader *pXsiShader,
			const Aftk::C_ShaderNode *pAftkShader,
			Ut::UtMap<const Aftk::C_Node*, CSLXSIShader*>& aftkXsiShaderNodeMapping);

		static Aftk::C_Status ExportXsiShaderConnection(
			CSLShaderConnectionPoint *pShaderConnection,
			const Aftk::I_Plug *pPlug,
			Ut::UtMap<const Aftk::C_Node*, CSLXSIShader*>& aftkXsiShaderNodeMapping);

		static Aftk::C_Status ExportXsiShaderParameter(
			CSLVariantParameter *pShaderParameter,
			const Aftk::I_Plug *pPlug);
	};
}

#endif