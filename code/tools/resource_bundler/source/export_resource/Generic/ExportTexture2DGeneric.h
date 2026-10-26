#pragma once

namespace ResourceBundler
{
	class C_ExportTexture2DGeneric : public C_ExportArrayOfTextures2DGeneric
	{
	protected:
		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);
	};
}
