#pragma once

#include "system\serialise.h"
#include "core\containers.h"

#include "system\xml\xmlparser.h"
#include "core\containers\string.h"
#include "attk\attk.h"


namespace ResourceBundler
{
	class C_OutputTextureData;
	class C_Bundler;
	class I_ExportTextureFactory;
	class C_ProcessedMipmapChain;

	class I_ProcessTexture
	{
	public:
		virtual ~I_ProcessTexture() {};

		virtual bool Read(
			const C_Bundler& bundler, 
			const Sys::C_XmlAttributeList &attributes);

		virtual bool Process(C_Bundler& bundler) = 0;

	protected:
		bool LoadTexture(
			Attk::C_Image& image,
			const C_Bundler& bundler, 
			Util::C_String imgFile, 
			Util::C_String alphaFile);

		bool ResizeTexture(
			Attk::C_Image& image,
			S32 widthOverride,
			S32 heightOverride);

		bool GenerateProcessedMipmapChain(
			const C_Bundler& bundler, 
			C_ProcessedMipmapChain& processedMipmapChain,	// output
			Attk::C_Image& img);							// input

		bool GenerateProcessedMipmapChain(
			const C_Bundler& bundler, 
			C_ProcessedMipmapChain** processedMipmapChains,	// output
			Attk::C_CubeMap& img,
			const AttkUt::C_GenerateMipmaps::CubeMapFilterParameters& filterParams);							// input

		bool ProcessMipmapChain(
			const C_Bundler& bundler,
			Attk::C_MipmapImageChain& unprocessedMipmapChain,
			C_ProcessedMipmapChain& processedMipmapChain);

		Display::ResourceBundler::E_Platform GetPlatform(const Util::C_String& platform) const;

		const I_ExportTextureFactory* GetExportTextureFactory(const Display::ResourceBundler::E_Platform platform);

		CompressLib::E_CompressionFormat GetCompressLibFormat() const;

		bool SetOutputFormatFromString(const Util::C_String& format);

		bool SetNumMipLevelsToGenerateFromString(const Util::C_String& numMipLevels);

		bool SetCompressionSettingsFromString(const Util::C_String& compressor);

		bool SetGammaCorrectionFromString(const Util::C_String& gamma);
		bool SetDiscardAlphaFromString(const Util::C_String& gamma);

		C_ExportTextureOptions	m_exportOptions;
		bool					m_useAlpha;
	};
}
