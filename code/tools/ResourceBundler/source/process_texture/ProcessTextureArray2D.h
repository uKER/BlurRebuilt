#pragma once

namespace ResourceBundler
{
	class C_ProcessTextureArray2D : public C_ProcessArrayOfTextures2D
	{
	public:
		virtual ~C_ProcessTextureArray2D() {}

		virtual bool Read(
			const C_Bundler& bundler, 
			const Sys::C_XmlAttributeList &attributes);

		virtual bool Process(C_Bundler& bundler);
	};
}
