#pragma once

namespace CompressLib
{
	enum E_CompressionLibrary
	{
		E_CompressionLibrary_Squish,
		E_CompressionLibrary_X360,
		E_CompressionLibrary_ATI,
		E_CompressionLibrary_NVidia,

		E_CompressionLibrary_Unknown
	};

/*
	Format 	Description 			Alpha premultiplied? 	Compression ratio 	Texture Type
	DXT1 	Opaque / 1-bit Alpha 	N/A 					8:1 / 6:1 			Simple non-alpha
	DXT2 	Explicit alpha 			Yes 					4:1 				Sharp alpha
	DXT3 	Explicit alpha 			No						4:1					Sharp alpha
	DXT4 	Interpolated alpha 		Yes 					4:1					Gradient alpha
	DXT5 	Interpolated alpha 		No						4:1					Gradient alpha
*/
	enum E_CompressionFormat
	{
		// 0 or 1 bit alpha
		E_CompressionFormat_DXT1 = 0,

		//Explicit 4-bit alpha, color data is premultiplied by alpha.
		E_CompressionFormat_DXT2 = 1,

		//Explicit 4-bit alpha, not premultiplied.
		E_CompressionFormat_DXT3 = 2,

		//Interpolated alpha, color data is premultiplied by alpha.
		E_CompressionFormat_DXT4 = 3,

		//Interpolated alpha, not premultiplied.
		E_CompressionFormat_DXT5 = 4,

		//Normal map compression. Two-component 8-bit per pixel format made up of two DXT4/5 alpha blocks. 
		E_CompressionFormat_DXN = 5,

		//Library specific or unknown format.
		E_CompressionFormat_Unknown
	};

	enum E_Endianness
	{
		E_Endianness_LittleEndian,
		E_Endianness_BigEndian
	};
}