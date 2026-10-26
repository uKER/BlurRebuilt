#pragma once

namespace ResourceBundler
{
	class C_ExportTextureFactoryGeneric : public I_ExportTextureFactory
	{
	public:
		I_ExportTexture* CreateTexture2D() const;

		I_ExportTexture* CreateTextureArray2D() const;

		I_ExportTexture* CreateTexture3D() const;

		I_ExportTexture* CreateTextureCubeMap() const;

		static const C_ExportTextureFactoryGeneric& Instance();

	private:
		C_ExportTextureFactoryGeneric();

		static C_ExportTextureFactoryGeneric	m_instance;
	};
};

