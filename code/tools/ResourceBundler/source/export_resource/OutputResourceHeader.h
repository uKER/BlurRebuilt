#pragma once

namespace ResourceBundler
{
	struct C_OutputResourceHeader
	{
		struct Texture2D
		{
			U32		width;
			U32		height;
		};

		struct TextureArray2D
		{
			U32		width;
			U32		height;
			U32		numImages;
		};

		struct TextureCubeMap
		{
			U32		size;
		};

		U32		textureFormat;
		U16		numMipLevels;
		U16		flags;

		union
		{
			Texture2D		texture2D;
			TextureArray2D	arrayTexture2D;
			TextureCubeMap	textureCubeMap;
		};
	};
}
