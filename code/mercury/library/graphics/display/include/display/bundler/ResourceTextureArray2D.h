#ifndef DISPLAY_RESOURCE_BUNDLER_RESOURCE_TEXTURE_ARRAY_2D_H
#define DISPLAY_RESOURCE_BUNDLER_RESOURCE_TEXTURE_ARRAY_2D_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	namespace ResourceBundler
	{
		struct C_ResourceTextureArray2D : C_ResourceTexture
		{
			U16		width;
			U16		height;
			U16		numImages;
			U16		pad;

#ifdef BUNDLER_APP
			INLINE void Serialise(Sys::BinarySerialiserAutoEndian& s);
#endif
		};

#ifdef BUNDLER_APP
		void C_ResourceTextureArray2D::Serialise(Sys::BinarySerialiserAutoEndian& s)
		{
			C_ResourceTexture::Serialise(s);
			s << width;
			s << height;
			s << numImages;
			s << U16(0); // pad
		}
#endif
	}
}

#endif
