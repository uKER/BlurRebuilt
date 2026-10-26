#pragma once

namespace ResourceBundler
{
	class C_ExportTextureArray2DWii : public C_ExportArrayOfTextures2DWii
	{
	public:
		C_ExportTextureArray2DWii();

	protected:
		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);		
	};
}
