#pragma once

namespace ResourceBundler
{
class C_ExportTextureCubeMap360 : public C_ExportArrayOfTextures2D360
	{
	public:
		C_ExportTextureCubeMap360();

	protected:
		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);

		virtual bool _GetD3DHeader(
			D3DBaseTexture& header, 
			U32& tiledSize,
			U32 *pBaseSize, 
			U32 *pMipSize);
	};
}