#pragma once

namespace ResourceBundler
{
	class C_ExportArrayOfTextures2DWii : public C_ExportTextureWii
	{
	public:
		C_ExportArrayOfTextures2DWii();

	protected:
		virtual bool _Export(
			C_OutputTextureData& output, 
			const C_BundlerOutputOptions& bundlerOptions,
			const C_ExportTextureOptions& exportOptions);

		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);
	};
}
