#pragma once

namespace ResourceBundler
{
	class C_ExportTextureCubeMapGeneric : public C_ExportArrayOfTextures2DGeneric
	{
	protected:
		virtual bool _GetOutputHeader(C_OutputResourceHeader& header);
	};
}
