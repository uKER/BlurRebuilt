#pragma once

namespace ResourceBundler
{
	class C_ExportTexture360 : public I_ExportTexture
	{
	protected:
		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);

		U32 _GetXboxFormat(Display::ResourceBundler::E_TextureFormat format);
	};
}