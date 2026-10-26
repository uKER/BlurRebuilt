#pragma once

namespace ResourceBundler
{
	class C_ExportTextureGeneric : public I_ExportTexture
	{
	public:
		C_ExportTextureGeneric();

	protected:
		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);
	};
}
