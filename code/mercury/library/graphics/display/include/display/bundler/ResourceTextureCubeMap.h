#ifndef DISPLAY_RESOURCE_BUNDLER_RESOURCE_CUBE_MAP_H
#define DISPLAY_RESOURCE_BUNDLER_RESOURCE_CUBE_MAP_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	namespace ResourceBundler
	{
		struct C_ResourceTextureCubeMap : C_ResourceTexture
		{
			U16		size;
			U16		pad;

#ifdef BUNDLER_APP
			INLINE void Serialise(Sys::BinarySerialiserAutoEndian& s);
#endif
		};

#ifdef BUNDLER_APP
		void C_ResourceTextureCubeMap::Serialise(Sys::BinarySerialiserAutoEndian& s)
		{
			C_ResourceTexture::Serialise(s);
			s << size;
			s << pad;
		}
#endif
	}
}

#endif
