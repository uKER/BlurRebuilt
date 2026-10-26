#ifndef MAYA_IO_MATERIAL_HELPER_H
#define MAYA_IO_MATERIAL_HELPER_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace MayaIo
{
	class C_MaterialHelper
	{
	public:
		static Aftk::C_Status ValidateTextures(
			C_Importer* pImporter,
			C_ImportOptions* pOptions,
			Aftk::I_Progress* pProgress);

		///
		///Find the shading node for the given shading group set node.
		///
		///@param shading group set node to acquire shader from.
		///@return Shader object.
		static MObject FindShader(
			MObject& setNode, 
			Aftk::C_Status& status);

		static void CleanUpFileName(const std::string& input, std::string& output);

	private:
		static bool HasMissingTextures(MObject& materialObj);
	};
}

#endif

