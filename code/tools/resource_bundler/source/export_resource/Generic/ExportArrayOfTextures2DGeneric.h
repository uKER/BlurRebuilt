#pragma once

namespace ResourceBundler
{
	class C_ExportArrayOfTextures2DGeneric : public I_ExportTexture
	{
	protected:
		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);

		virtual bool _Export(
			C_OutputTextureData& output, 
			const C_BundlerOutputOptions& bundlerOptions,
			const C_ExportTextureOptions& exportOptions);
	};
}
