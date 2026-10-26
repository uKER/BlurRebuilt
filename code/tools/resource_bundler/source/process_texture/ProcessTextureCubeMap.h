#pragma once

namespace ResourceBundler
{
	class C_ProcessTextureCubeMap : public C_ProcessArrayOfTextures2D
	{
	public:
		virtual ~C_ProcessTextureCubeMap() {}

		virtual bool Read(
			const C_Bundler& bundler, 
			const Sys::C_XmlAttributeList &attributes);

		virtual bool Process(C_Bundler& bundler);

	protected:
		virtual bool _Process(
			C_Bundler& bundler,
			I_ExportTexture& exportTexture,
			C_OutputTextureData& outputData,
			C_OutputResourceHeader& outputHeader);

		//ATI cubemapgen variables;
		AttkUt::C_GenerateMipmaps::CubeMapFilterParameters m_CubeMapVariables;
	};
}
