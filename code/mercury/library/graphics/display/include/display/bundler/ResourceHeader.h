#ifndef DISPLAY_RESOURCE_BUNDLER_RESOURCE_H
#define DISPLAY_RESOURCE_BUNDLER_RESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	namespace ResourceBundler
	{
		struct C_ResourceHeader
		{
			U16		offsetNext;		// Offset to next resource, or Zero for last one
			U16		offsetString;	// Offset to string entry from start of this header
			U32		offsetData;		// Offset to platform specific resource data from start of platform specific data
			U16		type;
			U16		flags;
			U32		uncompressedDataSize;
			U32		compressedDataSize;

#ifdef BUNDLER_APP
			void Serialise(Sys::BinarySerialiserAutoEndian& s)
			{
				s << offsetNext;
				s << offsetString;
				s << offsetData;
				s << type;
				s << flags;
				s << uncompressedDataSize;
				s << compressedDataSize;
			}
#endif
		};
	}
}

#endif
