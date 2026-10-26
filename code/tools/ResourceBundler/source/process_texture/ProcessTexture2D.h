#pragma once

namespace ResourceBundler
{
	class C_ProcessTexture2D : public I_ProcessTexture
	{
	public:
		virtual ~C_ProcessTexture2D() {};

		virtual bool Read(
			const C_Bundler& bundler, 
			const Sys::C_XmlAttributeList &attributes);

		virtual bool Process(C_Bundler& bundler);

	public:
		Btl::String			m_source;
		Btl::String			m_alphaSource;
		S32					m_widthOverride;
		S32					m_heightOverride;
	};
}
