#pragma once

namespace ResourceBundler
{
	struct C_BundlerOutputOptions
	{
		Display::ResourceBundler::E_Platform		
							platform;
		Display::ResourceBundler::E_HeaderFlag	
							dataCompressionType;
		bool				bigEndianFile;
		bool				bigEndianTexture;
		bool				tileTextures;
		bool				swizzleTextures;
		U32					resourceAlignment;
	};
}
