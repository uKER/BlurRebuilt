#pragma once

namespace ResourceBundler
{
	class C_ExportArrayOfTextures2D360 : public C_ExportTexture360
	{
	public:
		C_ExportArrayOfTextures2D360();

	protected:
		virtual bool _Export(
			C_OutputTextureData& output, 
			const C_BundlerOutputOptions& bundlerOptions,
			const C_ExportTextureOptions& exportOptions);

		virtual bool _GetD3DHeader(
			D3DBaseTexture& header, 
			U32& tiledSize,
			U32 *pBaseSize,
			U32 *pMipSize) = 0;

		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);
		bool	m_NonPackedMipTail;
		bool	m_Linear;
	};
}
