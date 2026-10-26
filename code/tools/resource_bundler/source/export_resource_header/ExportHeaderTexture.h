#pragma once

namespace ResourceBundler
{
	class C_ExportHeaderTexture : public I_ExportResourceHeader
	{
	public:
		C_ExportHeaderTexture(
			const C_ExportTextureOptions& exportOptions,
			const C_OutputResourceHeader& outputHeader,
			U32 dataOffset,
			U32 uncompressedDataSize,
			U32 compressedDataSize);
	};
}
