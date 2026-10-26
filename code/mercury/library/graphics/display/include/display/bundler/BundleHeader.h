#ifndef DISPLAY_RESOURCE_BUNDLER_BUNDLE_HEADER_H
#define DISPLAY_RESOURCE_BUNDLER_BUNDLE_HEADER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	namespace ResourceBundler
	{
		struct C_BundleHeader
		{
	#ifdef BUNDLER_APP
			U32		magicID;			// Unique value so we can verify the file is Ok
			U8		version;			// Which revision of this format is this?
	#endif
			U8		platformCode;		// 360, PS3, Wii?
			U16		resourceInfoSize;	// Size of resource info = Offset to platform specific resource data
			U16		stringTableOffset;
			U16		flags;
			U32		uncompressedDataSize;	// Size of platform specific data
			U32		compressedDataSize;

	#ifdef BUNDLER_APP
			INLINE void Serialise(Sys::BinarySerialiserAutoEndian& s);
	#else
			INLINE void Deserialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s);
	#endif
		};

	#ifdef BUNDLER_APP
		void C_BundleHeader::Serialise(Sys::BinarySerialiserAutoEndian& s)
		{
			s << magicID;
			s << version;
			s << platformCode;
			s << resourceInfoSize;
			s << stringTableOffset;
			s << flags;
			s << uncompressedDataSize;
			s << compressedDataSize;
		}
	#else
		void C_BundleHeader::Deserialise(Sys::BinaryDeserialiser<Ut::NoEndianSwap>& s)
		{
			//Read seperately
//			s >> magicID;
//			s >> version;
			s >> platformCode;
			s >> resourceInfoSize;
			s >> stringTableOffset;
			s >> flags;
			s >> uncompressedDataSize;
			s >> compressedDataSize;
		}
	#endif
	}
}

#endif
