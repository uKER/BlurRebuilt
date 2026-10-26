#ifndef XSI_IO_MATERIAL_IMPORT_H
#define XSI_IO_MATERIAL_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_MaterialImport
	{
	public:
		static Aftk::C_Status Import(
			C_SceneImport* pSceneImport,
			CSLBaseMaterial* pXsiBaseMaterial, 
			Aftk::C_MaterialNode* pAftkMaterial);

	private:
		static Aftk::C_Status ImportSiMaterial(
			C_SceneImport* pSceneImport,
			CSLBaseMaterial* pXsiBaseMaterial, 
			Aftk::C_MaterialNode* pAftkMaterial);

		static Aftk::C_Status ImportXsiMaterial(
			C_SceneImport* pSceneImport,
			CSLBaseMaterial* pXsiBaseMaterial, 
			Aftk::C_MaterialNode* pAftkMaterial);

		static Aftk::C_ShaderNode*	ConvertAftkImageShaderNode(
			const std::string& nodeName, 
			std::string& imageName,
			Aftk::C_Status& status);

		static Aftk::C_Status BuildRelationships(
			C_SceneImport* pSceneImport,
			CSLXSIShader* pShader,
			Aftk::C_MaterialNode* pAftkMaterial,
			Ut::UtMap<CSLXSIShader*, Aftk::C_ShaderNode*>& xsiAftkShaderNodeMapping);

		static Aftk::C_ShaderNode* CreateAftkShaderNode(
			CSLXSIShader* pShader,
			Ut::UtMap<CSLXSIShader*, Aftk::C_ShaderNode*>& xsiAftkShaderNodeMapping,
			Aftk::C_Status& status);
	};
}

#endif
