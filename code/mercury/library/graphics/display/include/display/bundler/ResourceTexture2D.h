#ifndef DISPLAY_RESOURCE_BUNDLER_RESOURCE_TEXTURE_2D_H
#define DISPLAY_RESOURCE_BUNDLER_RESOURCE_TEXTURE_2D_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	namespace ResourceBundler
	{
		struct C_ResourceTexture2D : C_ResourceTexture
		{
			U16		width;
			U16		height;

#ifdef BUNDLER_APP
			INLINE void Serialise(Sys::BinarySerialiserAutoEndian& s);
#endif
		};

#ifdef BUNDLER_APP
		void C_ResourceTexture2D::Serialise(Sys::BinarySerialiserAutoEndian& s)
		{
			C_ResourceTexture::Serialise(s);
			s << width;
			s << height;
		}
#endif
	}
}

#endif
