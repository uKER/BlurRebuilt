#ifndef DISPLAY_RESOURCE_BUNDLER_RESOURCE_TEXTURE_H
#define DISPLAY_RESOURCE_BUNDLER_RESOURCE_TEXTURE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	namespace ResourceBundler
	{
		struct C_ResourceTexture : C_ResourceHeader
		{
			U16		format;
			U16		mipLevels;

#ifdef BUNDLER_APP
			INLINE void Serialise(Sys::BinarySerialiserAutoEndian& s);
#endif
		};

#ifdef BUNDLER_APP
		void C_ResourceTexture::Serialise(Sys::BinarySerialiserAutoEndian& s)
		{
			C_ResourceHeader::Serialise(s);
			s << format;
			s << mipLevels;
		}
#endif
	}
}

#endif
