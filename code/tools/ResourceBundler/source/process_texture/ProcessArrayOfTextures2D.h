#pragma once

namespace ResourceBundler
{
	class I_ExportTexture;
	struct C_OutputResourceHeader;

	// This is used by Volume textures and Array textures
	class C_ProcessArrayOfTextures2D : public I_ProcessTexture
	{
	public:
		virtual ~C_ProcessArrayOfTextures2D() {}

		virtual bool Read(
			const C_Bundler& bundler, 
			const Sys::C_XmlAttributeList &attributes);

	protected:
		virtual bool _Process(
			C_Bundler& bundler,
			I_ExportTexture& exportTexture,
			C_OutputTextureData& outputData,
			C_OutputResourceHeader& outputHeader);

		Btl::Vector<Btl::String> m_sources;
		Btl::Vector<Btl::String> m_alphaSources;
		S32	m_widthOverride;
		S32	m_heightOverride;
	};
}
