#ifndef RESOURCE_BUNDLER_DEFINES_H
#define RESOURCE_BUNDLER_DEFINES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace ResourceBundler
{
	enum E_Platform
	{
		E_Platform_Unknown	= 0,
		E_Platform_Xbox360,
		E_Platform_PC,
		E_Platform_PS3_OpenGL,
		E_Platform_PS3_LibGCM,
		E_Platform_Wii
	};

	#ifdef BUNDLER_APP
	static const char* g_platformName[] = 
	{ 
		"Unknown",
		"XBox 360",
		"PC",
		"PS3 - OpenGL",
		"PS3 - LibGCM",
		"Wii",
		"\0"
	};
	#endif

	enum E_TextureFormat
	{
		E_TextureFormat_Unknown	= 0,
		E_TextureFormat_DXT1,
		E_TextureFormat_DXT2,
		E_TextureFormat_DXT3,
		E_TextureFormat_DXT4,
		E_TextureFormat_DXT5,
		E_TextureFormat_A8R8G8B8,
		E_TextureFormat_L8,
		E_TextureFormat_A32R32G32B32,
		E_TextureFormat_DXN
	};
	
	#ifdef BUNDLER_APP
	//Much as globals are Evil, we need to place this one here.
	//This is so that people don't forget to update this string table when adding new formats.
	static const char* g_textureFormat[] = 
	{ 
		"Unknown",
		"DXT1",
		"DXT2",
		"DXT3",
		"DXT4",
		"DXT5",
		"A8R8G8B8",
		"L8",
		"A32R32G32B32",
		"DXN",
		"\0"
	};

	//Requires power of 2 texture size?
	static const bool g_requiresPow2Size[] = 
	{ 
		false,	//Unknown
		true,	//DXT1
		true,	//DXT2
		true,	//DXT3
		true,	//DXT4
		true,	//DXT5
		false,	//A8R8G8B8
		false,	//L8
		false,	//A32R32G32B32
		true	//DXN
	};

	//Bytes per block. A DXT block is a 4x4 block of pixels.
	static const U32 g_textureFormatBytesPerBlock[] = 
	{ 
		0,	//Unknown
		8,	//DXT1
		16,	//DXT2
		16,	//DXT3
		16,	//DXT4
		16,	//DXT5
		4,	//A8R8G8B8
		1,	//L8
		16,	//A32R32G32B32
		16	//DXN
	};

	//Block width in pixels.
	static const U32 g_textureFormatBlockWidth[] = 
	{ 
		0,	//Unknown
		4,	//DXT1
		4,	//DXT2
		4,	//DXT3
		4,	//DXT4
		4,	//DXT5
		1,	//A8R8G8B8
		1,	//L8
		1,	//A32R32G32B32
		4	//DXN
	};

	#endif

	enum E_ResourceType
	{
		E_ResourceType_Unknown			= 0,
		E_ResourceType_Texture2D		= 1,
		E_ResourceType_TextureArray2D	= 2,
		E_ResourceType_Texture3D		= 3,
		E_ResourceType_TextureCubeMap	= 4
	};

	enum E_HeaderMask
	{
		E_HeaderMask_Compression = 0x000f
	};

	enum E_HeaderFlag
	{
		E_HeaderFlag_Uncompressed = 0x0000,
		E_HeaderFlag_Compressed = 0x0001,
		E_HeaderFlag_CompressedIndividually = 0x0002,
		E_HeaderFlag_Unknown = 0x000f
	};

	enum E_ResourceFlags
	{
		E_ResourceFlag_Gamma = 0x0001
	};
}
}

#endif
