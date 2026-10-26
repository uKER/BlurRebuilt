#pragma once


namespace ResourceBundler
{
	class C_OutputTextureData;
	struct C_OutputResourceHeader;

	struct C_BundlerOutputOptions;
	struct C_ExportTextureOptions;

	class C_ProcessedMipmapChain;

	class I_ExportTexture
	{
	public:
		I_ExportTexture();

		~I_ExportTexture();

		bool AddMipmapChain(C_ProcessedMipmapChain* pMipMapChain);

		U32 GetNumMipmapChains();

		bool Export(
			C_OutputTextureData& output, 
			const C_BundlerOutputOptions& bundlerOptions,
			const C_ExportTextureOptions& exportOptions);

		bool GetOutputHeader(C_OutputResourceHeader& header);

	protected:
		bool SetHeaderInfo(C_OutputTextureData *pTex, U8 *pPtr, U32 size);

		virtual bool _Export(
			C_OutputTextureData& output, 
			const C_BundlerOutputOptions& bundlerOptions,
			const C_ExportTextureOptions& exportOptions) = 0;

		virtual bool _GetOutputHeader(C_OutputResourceHeader& header) = 0;

		Util::C_String		m_name;
		Ut::UtList<C_ProcessedMipmapChain*>	
							m_mipmapChains;
		Display::ResourceBundler::E_TextureFormat		m_textureFormat;
		bool				m_gamma;
	};
}
