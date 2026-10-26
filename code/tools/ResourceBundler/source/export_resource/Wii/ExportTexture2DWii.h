#pragma once

namespace ResourceBundler
{
	class C_ExportTexture2DWii : public C_ExportArrayOfTextures2DWii
	{
	public:
		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);
	};
}