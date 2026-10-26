#pragma once

namespace ResourceBundler
{
	class C_ExportHeaderTexture3D : public C_ExportHeaderTexture
	{
	public:
		C_ExportHeaderTexture3D(
			const C_ExportTextureOptions& exportOptions,
			const C_OutputResourceHeader& outputHeader,
			U32 dataOffset,
			U32 uncompressedDataSize,
			U32 compressedDataSize);

		virtual U32 CalculateHeaderSize() const;

		virtual U32 GetType() const;

		virtual void Serialise(
			Sys::BinarySerialiserAutoEndian& serialiser,
			U16		offsetNext,				// Offset to next resource, or Zero for last one
			U16		offsetString,			// Offset to string entry from start of this header
			U32		offsetData) const;	// Offset to platform specific resource data from start of platform specific data
	};
}