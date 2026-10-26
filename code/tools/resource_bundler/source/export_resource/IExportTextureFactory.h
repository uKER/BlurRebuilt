#pragma once

namespace ResourceBundler
{
	class I_ExportTexture;

	class I_ExportTextureFactory 
	{
	public:
		virtual I_ExportTexture* CreateTexture2D() const = 0;

		virtual I_ExportTexture* CreateTextureArray2D() const = 0;

		virtual I_ExportTexture* CreateTexture3D() const = 0;

		virtual I_ExportTexture* CreateTextureCubeMap() const = 0;
	};
}
