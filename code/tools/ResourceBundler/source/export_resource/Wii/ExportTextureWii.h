#pragma once

namespace ResourceBundler
{
	class C_ExportTextureWii : public I_ExportTexture
	{
	protected:
		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);
	};
}