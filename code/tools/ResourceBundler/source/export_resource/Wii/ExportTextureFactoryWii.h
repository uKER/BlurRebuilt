#pragma once

namespace ResourceBundler
{
	class C_ExportTextureFactoryWii : public I_ExportTextureFactory
	{
	public:
		I_ExportTexture* CreateTexture2D() const;

		I_ExportTexture* CreateTextureArray2D() const;

		I_ExportTexture* CreateTexture3D() const;

		I_ExportTexture* CreateTextureCubeMap() const;

		static const C_ExportTextureFactoryWii& Instance();

	private:
		C_ExportTextureFactoryWii();

		static C_ExportTextureFactoryWii	m_instance;
	};
};
