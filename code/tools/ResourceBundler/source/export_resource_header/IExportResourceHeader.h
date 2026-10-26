#pragma once

namespace ResourceBundler
{
	class I_ExportResourceHeader
	{
	public:
		I_ExportResourceHeader(
			const C_ExportTextureOptions& exportOptions,
			const C_OutputResourceHeader& outputHeader,
			U32 offset,
			U32 uncompressedDataSize,
			U32 compressedDataSize) : 
				m_exportOptions(exportOptions), 
				m_outputHeader(outputHeader),
				m_offset(offset), 
				m_uncompressedDataSize(uncompressedDataSize),
				m_compressedDataSize(compressedDataSize) {}

		virtual ~I_ExportResourceHeader() {}

		const Util::C_String& GetName() const
		{
			return m_exportOptions.name;
		}

		const U32 GetCompressedDataSize() const
		{
			return m_compressedDataSize;
		}

		const U32 GetUncompressedDataSize() const
		{
			return m_uncompressedDataSize;
		}

		const U32 GetOffset() const
		{
			return m_offset;
		}

		virtual U32 GetType() const = 0;

		virtual U32 CalculateHeaderSize() const = 0;

		virtual void Serialise(
			Sys::BinarySerialiserAutoEndian& serialiser,
			U16		offsetNext,					// Offset to next resource, or Zero for last one
			U16		offsetString,				// Offset to string entry from start of this header
			U32		offsetResource) const = 0;	// Offset to platform specific resource data from start of platform specific data

	protected:
		C_OutputResourceHeader	m_outputHeader;
		C_ExportTextureOptions	m_exportOptions;

		U32				m_offset;	// Offset into platform specific data of this resource
		U32				m_uncompressedDataSize;
		U32				m_compressedDataSize;
	};
}
