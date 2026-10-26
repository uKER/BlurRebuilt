#pragma once

namespace ResourceBundler
{
	class C_ProcessTexture3D : public C_ProcessArrayOfTextures2D
	{
	public:
		virtual ~C_ProcessTexture3D() {};

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
	};
}
