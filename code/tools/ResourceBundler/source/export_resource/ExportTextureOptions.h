#pragma once

namespace ResourceBundler
{
	struct C_ExportTextureOptions
	{
		C_ExportTextureOptions()
		{
			name = "";
			outputFormat = Display::ResourceBundler::E_TextureFormat_Unknown;
			compressionLibrary = CompressLib::E_CompressionLibrary_Unknown;
			compressionTechnique = 0;
			numMipLevelsToGenerate = 0;
			gamma = false;
			discardAlpha = false;
		}

		C_ExportTextureOptions(const C_ExportTextureOptions& rhs)
		{
			*this = rhs;
		}

		const C_ExportTextureOptions& operator=(const C_ExportTextureOptions& rhs)
		{
			name = rhs.name;
			outputFormat = rhs.outputFormat;
			compressionLibrary = rhs.compressionLibrary;
			compressionTechnique = rhs.compressionTechnique;
			gamma = rhs.gamma;
			discardAlpha = rhs.discardAlpha;

			return *this;
		}

		Util::C_String						name;
		//Generic texture format enum, e.g.E_TextureFormat_DXT5
		Display::ResourceBundler::E_TextureFormat						outputFormat;
		//Parameters for compression lib. Of course, it's quite possible that the user might
		//not want a texture to be compressed at all, in which case these values will be unused.
		CompressLib::E_CompressionLibrary	compressionLibrary;
		S32									compressionTechnique;
		S32									numMipLevelsToGenerate;
		bool								gamma;
		bool								discardAlpha;
	};
}
