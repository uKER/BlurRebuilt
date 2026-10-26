#ifndef AFTK_RENDER_TEXTURE_MANAGER
#define AFTK_RENDER_TEXTURE_MANAGER

#ifdef _MSC_VER
#pragma once
#endif

#include "common/ResourcePtr.h"
#include "common/Synchronise.h"

namespace AftkRender
{
	class C_TextureManager
	{
	public:
			
		C_TextureManager();
		~C_TextureManager();

		bool Initialise();
		void Clear();

		C_ResourcePtr<Display::C_RefTexture2D> LoadTexture2DFromFile(const char *pTexturePath, U32 clampSize = 0, U32 levels = 0);
		C_ResourcePtr<Display::C_RefTexture3D> LoadTexture3DFromFile(const char *pTexturePath, U32 clampSize = 0);
		C_ResourcePtr<Display::C_RefTextureCube> LoadTextureCubeFromFile(const char *pTexturePath, U32 clampSize = 0);

		C_ResourcePtr<Display::C_RefTexture2D> AddTexture2D(const char* pIdentifier, Display::C_RefTexture2D *pTexture);
		C_ResourcePtr<Display::C_RefTexture3D> AddTexture3D(const char* pIdentifier, Display::C_RefTexture3D *pTexture);
		C_ResourcePtr<Display::C_RefTextureCube> AddTextureCube(const char* pIdentifier, Display::C_RefTextureCube *pTexture);

		C_ResourcePtr<Display::C_RefTexture2D> GetDefault2DTexture();
		C_ResourcePtr<Display::C_RefTexture3D> GetDefault3DTexture();
		C_ResourcePtr<Display::C_RefTextureCube> GetDefaultCubeTexture();

		void FlushUnusedTextures();

	private:
		bool CreateDefaultTextures();

		// can't have single array of C_Texture pointers due to lack of virtual destructor...
		Ut::UtVector<Ut::DynamicVector< C_ResourcePtr<Display::C_RefTexture2D> > >m_Texture2DResourceArray;
		Ut::UtVector<Ut::DynamicVector< C_ResourcePtr<Display::C_RefTexture3D> > >m_Texture3DResourceArray;
		Ut::UtVector<Ut::DynamicVector< C_ResourcePtr<Display::C_RefTextureCube> > >m_TextureCubeResourceArray;

		Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTexture2D>, Ut::HashKeyOpSmall<U32> > m_Texture2DResourceMap;
		Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTexture3D>, Ut::HashKeyOpSmall<U32> > m_Texture3DResourceMap;
		Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTextureCube>, Ut::HashKeyOpSmall<U32> > m_TextureCubeResourceMap;

		C_ResourcePtr<Display::C_RefTexture2D>		m_Default2DResource;
		C_ResourcePtr<Display::C_RefTexture3D>		m_Default3DResource;
		C_ResourcePtr<Display::C_RefTextureCube>	m_DefaultCubeResource;
		
		bool		m_Initialised;

		DECLARE_SYNCHRONISED();

	};
}
#endif