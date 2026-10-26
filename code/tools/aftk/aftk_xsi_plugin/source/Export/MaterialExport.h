#pragma once

namespace XSITrans
{
	class C_MaterialExport
	{
	public:
		static Aftk::C_Status Export(
			Aftk::C_MaterialNode& aftkMaterial, 
			XSI::Material& xsiMaterial);

	private:
		static Aftk::C_ShaderImageNode* CreateAftkShaderImageNode(
			Aftk::C_MaterialNode& aftkMaterial,
			XSI::ImageClip2 imageClip2,
			Ut::UtMap<
				Util::C_String, 
				Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping);

		static Aftk::C_Status BuildShaderRelationships(
			XSI::CRef shader,
			const Util::C_String& parameterName,
			XSI::CRef connectedObject,
			Aftk::C_MaterialNode& aftkMaterial,
			Ut::UtMap<
				Util::C_String, 
				Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping);

		static Aftk::C_Status CreateAndBuildImageRelationships(
			XSI::CRef shader,
			const Util::C_String& parameterName,
			XSI::CRef connectedObject,
			Aftk::C_MaterialNode& aftkMaterial,
			Ut::UtMap<
				Util::C_String, 
				Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping);

		static Aftk::C_Status BuildRelationships(
			XSI::CRef shader,
			Aftk::C_MaterialNode& aftkMaterial,
			Ut::UtMap<
				Util::C_String, 
				Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping);

		static Aftk::C_Status CreateAftkShaderNode(
			Aftk::C_MaterialNode& aftkMaterial,
			XSI::CRef _shader,
			Ut::UtMap<
				Util::C_String, 
				Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping);
	};
}
