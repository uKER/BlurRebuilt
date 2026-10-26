#ifndef __TEXTURE_EXPORT_TEXTURE_PROCESSING_H__
#define __TEXTURE_EXPORT_TEXTURE_PROCESSING_H__

#include "texturemanager.h"
#include "texture_export/Rescale.h"

namespace TextureExport
{

/// example textureinfo structure
class TextureInfo : public TextureInfoBase
{
public:
	/// constructor
	TextureInfo() 
		: m_iFlags(0), mipmapped(true), m_iDefaultColour(0xffffffff), m_nMipMaps(0), m_MipFilter(ActTools::FT_LANCZOS)
		, m_EdgeFilterU(ActTools::EDGE_TILE), m_EdgeFilterV(ActTools::EDGE_TILE)
	{}
	/// destructor
	~TextureInfo() {}

	/// write contents of this class to a binary stream
	void Write(Sys::I_BinaryStream *pStream);
	/// read contents of this class from a binary stream
	void Read(Sys::I_BinaryStream *pStream);

	// if the file is not found, this colour is used
	U32 m_iDefaultColour;

	U32 m_nMipMaps;
	ActTools::FilterType m_MipFilter;
	ActTools::EdgeFilter m_EdgeFilterU, m_EdgeFilterV;

	// flags
	union
	{
		U32 m_iFlags;

		struct
		{
			// this should only contain information about texture conversion, with
			// rendering information moved to the materials or another structure.
			bool mipmapped : 1;
			bool convertToNormalMap : 1;
			bool invertNormalMapX : 1;
			bool preMultiplyAlpha : 1;
			bool bSRGBTexture : 1;
			bool bINR0 : 1;
			bool bForceGreyScaleAlpha : 1;
			bool bEnhanceMipMapAlpha : 1;
			bool bReduce : 1;
		};
	};

	Util::C_String m_sTexelFmtOverride;
};

};

#endif
