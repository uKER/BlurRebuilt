#pragma once

namespace ResourceBundler
{
	class C_ExportTextureFactory360 : public I_ExportTextureFactory
	{
	public:
		I_ExportTexture* CreateTexture2D() const;

		I_ExportTexture* CreateTextureArray2D() const;

		I_ExportTexture* CreateTexture3D() const;

		I_ExportTexture* CreateTextureCubeMap() const;

		static const C_ExportTextureFactory360& Instance();

	private:
		C_ExportTextureFactory360();

		static C_ExportTextureFactory360	m_instance;
	};
};
