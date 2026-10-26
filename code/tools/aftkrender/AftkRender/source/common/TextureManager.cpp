#include "AftkRenderPch.h"
#include "common/TextureManager.h"

#include "attk/Attk.h"

D3DFORMAT D3DTextureFormat(Attk::E_PixelFormat format)
{
	switch(format)
	{
	case Attk::E_PixelFormat_R8G8B8U:
	case Attk::E_PixelFormat_B8G8R8U:
		return D3DFMT_X8R8G8B8;

	case Attk::E_PixelFormat_R8G8B8A8U:
	case Attk::E_PixelFormat_B8G8R8A8U:
	case Attk::E_PixelFormat_A8R8G8B8U:
	case Attk::E_PixelFormat_A8B8G8R8U:
		return D3DFMT_A8R8G8B8;

	case Attk::E_PixelFormat_Compressed_DXT1:
		return D3DFMT_DXT1;
	case Attk::E_PixelFormat_Compressed_DXT2:
		return D3DFMT_DXT2;
	case Attk::E_PixelFormat_Compressed_DXT3:
		return D3DFMT_DXT3;
	case Attk::E_PixelFormat_Compressed_DXT4:
		return D3DFMT_DXT4;
	case Attk::E_PixelFormat_Compressed_DXT5:
		return D3DFMT_DXT5;

	default:
		return D3DFMT_UNKNOWN;
	}
}


namespace AftkRender
{
	extern Debug::Logger& g_Log;
}

using namespace AftkRender;

IMPLEMENT_SYNCHRONISED(C_TextureManager);

C_TextureManager::C_TextureManager()
: m_Initialised(false)
{
}

C_TextureManager::~C_TextureManager()
{
	if (m_Initialised)
		Clear();
}

bool C_TextureManager::Initialise()
{
	if (!m_Initialised)
		m_Initialised = CreateDefaultTextures();

	return m_Initialised;
}

void C_TextureManager::Clear()
{
	Synchronise<C_TextureManager> sync(this);

	m_Texture2DResourceMap.Clear();
	m_Texture3DResourceMap.Clear();
	m_TextureCubeResourceMap.Clear();

	for (S32 i=0; i<m_Texture2DResourceArray.Size(); ++i)
		Display::Device()->Destroy(m_Texture2DResourceArray[i]);
	m_Texture2DResourceArray.Clear();

	for (S32 i=0; i<m_Texture3DResourceArray.Size(); ++i)
		Display::Device()->Destroy(m_Texture3DResourceArray[i]);
	m_Texture3DResourceArray.Clear();

	for (S32 i=0; i<m_TextureCubeResourceArray.Size(); ++i)
		Display::Device()->Destroy(m_TextureCubeResourceArray[i]);
	m_TextureCubeResourceArray.Clear();

	if (m_Default2DResource != 0)
		Display::Device()->Destroy(m_Default2DResource);
	if (m_Default3DResource != 0)
		Display::Device()->Destroy(m_Default3DResource);
	if (m_DefaultCubeResource != 0)
		Display::Device()->Destroy(m_DefaultCubeResource);

	m_Default2DResource = 0;
	m_Default3DResource = 0;
	m_DefaultCubeResource = 0;

	m_Initialised = false;

}

C_ResourcePtr<Display::C_RefTexture2D> C_TextureManager::LoadTexture2DFromFile(
	const char *pTexturePath, 
	U32 clampSize,
	U32 levels)
{
	Synchronise<C_TextureManager> sync(this);

	U32 pathHash = Ut::HashKeyOpString::GetHash(pTexturePath);
	Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTexture2D>, Ut::HashKeyOpSmall<U32>>::Iterator it 
		= m_Texture2DResourceMap.Find(pathHash);
	if (it != m_Texture2DResourceMap.End())
		return it->GetValue();

	/*if (clampSize == 0)
	{
		C_ResourcePtr<Display::C_RefTexture2D> ptr = new Display::C_RefTexture2D();
		if (Display::CreateTexture2DFromFile(pTexturePath, ptr))
		{
			m_Texture2DResourceArray.PushBack(ptr);
			m_Texture2DResourceMap[pathHash] = m_Texture2DResourceArray[m_Texture2DResourceArray.Size()-1];
			return ptr;
		}
	}
	else*/
	{
		/*
		C_ResourcePtr<Display::C_Texture2D> ptr = new Display::C_Texture2D();
		if (!Display::CreateTexture2DFromFile(pTexturePath, ptr))
		{
			delete ptr;
			return 0;
		}
		else
		{
			m_Texture2DResourceArray.PushBack(ptr);
			m_Texture2DResourceMap[pathHash] = m_Texture2DResourceArray[m_Texture2DResourceArray.Size()-1];
			return ptr;
		}
		*/
		
		Attk::C_Image image;
		if (image.Read(pTexturePath))
		{
			D3DFORMAT format = D3DTextureFormat(image.GetPixelFormat());
			if (format == D3DFMT_UNKNOWN)
			{				
				LOGERROR(g_Log, "%s : Unsupported pixel format", pTexturePath);
				return 0;
			}

			if (image.GetBitsPerPixel() == 24 || image.GetBitsPerPixel() == 32)
				image.ChangePixelFormat(Attk::E_PixelFormat_B8G8R8A8U);

			if (clampSize != 0)
				AttkUt::C_ImageResize::Resize(image, Maths::Min<U32>(image.GetWidth(), clampSize), Maths::Min<U32>(image.GetHeight(), clampSize), 1);

			image.ChangePixelFormat(Attk::E_PixelFormat_B8G8R8A8U);

			Attk::C_MipmapPixelDataChain mips;
			AttkUt::C_GenerateMipmaps::Generate(mips, image, levels);

			C_ResourcePtr<Display::C_RefTexture2D> ptr = new Display::C_RefTexture2D();

			if (format!=D3DFMT_UNKNOWN && Display::Device()->CreateTexture2D(image.GetWidth(), image.GetHeight(), mips.GetNumLevels(), 0, (Display::E_Format)format, ptr))
			{
				ptr->SetFileName(pTexturePath);

				Display::C_TextureParams uploadParams;
				uploadParams.m_Width = image.GetWidth();
				uploadParams.m_Height = image.GetHeight();
				uploadParams.m_Levels = mips.GetNumLevels();
				uploadParams.m_Format = (Display::E_Format)format;
				uploadParams.m_MipCopy = true;
				uploadParams.m_MipSize = mips.GetPixelDataSize();
				uploadParams.m_pMipData = mips.GetPixelDataPtr();
				uploadParams.m_MipHead = 0;
				uploadParams.m_MipTail = mips.GetNumLevels()-1;
				
				bool success = Display::UploadTexture2D(ptr, &uploadParams);

				//is Attk::C_MipmapPixelDataChain supposed to assume ownership of it's pixel data ptr?
				delete mips.GetPixelDataPtr();
				mips.SetPixelDataPtr(0);

				if (success)
				{
					m_Texture2DResourceArray.PushBack(ptr);
					m_Texture2DResourceMap[pathHash] = m_Texture2DResourceArray[m_Texture2DResourceArray.Size()-1];
					return ptr;
				}
			}
		}
	}

	return 0;
}

C_ResourcePtr<Display::C_RefTexture3D> C_TextureManager::LoadTexture3DFromFile(const char *pTexturePath, U32 clampSize)
{
	Synchronise<C_TextureManager> sync(this);

	U32 pathHash = Ut::HashKeyOpString::GetHash(pTexturePath);
	Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTexture3D>, Ut::HashKeyOpSmall<U32>>::Iterator it 
		= m_Texture3DResourceMap.Find(pathHash);
	if (it != m_Texture3DResourceMap.End())
		return it->GetValue();

	C_ResourcePtr<Display::C_RefTexture3D> ptr = new Display::C_RefTexture3D();
	if (Display::CreateTexture3DFromFile(pTexturePath, ptr))
	{
		m_Texture3DResourceArray.PushBack(ptr);
		m_Texture3DResourceMap[pathHash] = m_Texture3DResourceArray[m_Texture3DResourceArray.Size()-1];
		return ptr;
	}
	else
		return 0;
}

C_ResourcePtr<Display::C_RefTextureCube> C_TextureManager::LoadTextureCubeFromFile(const char *pTexturePath, U32 clampSize)
{
	Synchronise<C_TextureManager> sync(this);

	U32 pathHash = Ut::HashKeyOpString::GetHash(pTexturePath);
	Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTextureCube>, Ut::HashKeyOpSmall<U32>>::Iterator it 
		= m_TextureCubeResourceMap.Find(pathHash);
	if (it != m_TextureCubeResourceMap.End())
		return it->GetValue();

	C_ResourcePtr<Display::C_RefTextureCube> ptr = new Display::C_RefTextureCube();
	if (Display::CreateTextureCubeFromFile(pTexturePath, ptr))
	{
		m_TextureCubeResourceArray.PushBack(ptr);
		m_TextureCubeResourceMap[pathHash] = m_TextureCubeResourceArray[m_TextureCubeResourceArray.Size()-1];
		return ptr;
	}
	else
		return 0;
}

C_ResourcePtr<Display::C_RefTexture2D> C_TextureManager::AddTexture2D(const char* pIdentifier, Display::C_RefTexture2D *pTexture)
{
	Synchronise<C_TextureManager> sync(this);

	U32 pathHash = Ut::HashKeyOpString::GetHash(pIdentifier);
	Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTexture2D>, Ut::HashKeyOpSmall<U32>>::Iterator it 
		= m_Texture2DResourceMap.Find(pathHash);
	if (it != m_Texture2DResourceMap.End())
		return 0;

	m_Texture2DResourceArray.PushBack(pTexture);
	m_Texture2DResourceMap[pathHash] = m_Texture2DResourceArray[m_Texture2DResourceArray.Size()-1];
	return m_Texture2DResourceArray.Back();
}

C_ResourcePtr<Display::C_RefTexture3D> C_TextureManager::AddTexture3D(const char* pIdentifier, Display::C_RefTexture3D *pTexture)
{
	Synchronise<C_TextureManager> sync(this);

	U32 pathHash = Ut::HashKeyOpString::GetHash(pIdentifier);
	Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTexture3D>, Ut::HashKeyOpSmall<U32>>::Iterator it 
		= m_Texture3DResourceMap.Find(pathHash);
	if (it != m_Texture3DResourceMap.End())
		return 0;

	m_Texture3DResourceArray.PushBack(pTexture);
	m_Texture3DResourceMap[pathHash] = m_Texture3DResourceArray[m_Texture3DResourceArray.Size()-1];
	return m_Texture3DResourceArray.Back();

}

C_ResourcePtr<Display::C_RefTextureCube> C_TextureManager::AddTextureCube(const char* pIdentifier, Display::C_RefTextureCube *pTexture)
{
	Synchronise<C_TextureManager> sync(this);

	U32 pathHash = Ut::HashKeyOpString::GetHash(pIdentifier);
	Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTextureCube>, Ut::HashKeyOpSmall<U32>>::Iterator it 
		= m_TextureCubeResourceMap.Find(pathHash);
	if (it != m_TextureCubeResourceMap.End())
		return 0;

	m_TextureCubeResourceArray.PushBack(pTexture);
	m_TextureCubeResourceMap[pathHash] = m_TextureCubeResourceArray[m_TextureCubeResourceArray.Size()-1];
	return m_TextureCubeResourceArray.Back();
}

void C_TextureManager::FlushUnusedTextures()
{
	Synchronise<C_TextureManager> sync(this);

	//2D textures
	{
		Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTexture2D>, Ut::HashKeyOpSmall<U32>>::Iterator it
			= m_Texture2DResourceMap.Begin();
		while (it != m_Texture2DResourceMap.End())
		{
			if (it->GetValue().Count() == 2)
			{
				Display::Device()->Destroy(it->GetValue());
				m_Texture2DResourceMap.Erase(it);
			}
			it++;
		}

		for (S32 i=0; i<m_Texture2DResourceArray.Size(); ++i)
			if (m_Texture2DResourceArray[i].Count() == 1)
				m_Texture2DResourceArray.EraseNonOrdered(i);
	}

	//3D textures
	{
		Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTexture3D>, Ut::HashKeyOpSmall<U32>>::Iterator it
			= m_Texture3DResourceMap.Begin();
		while (it != m_Texture3DResourceMap.End())
		{
			if (it->GetValue().Count() == 2)
			{
				Display::Device()->Destroy(it->GetValue());
				m_Texture3DResourceMap.Erase(it);
			}
			it++;
		}

		for (S32 i=0; i<m_Texture3DResourceArray.Size(); ++i)
			if (m_Texture3DResourceArray[i].Count() == 1)
				m_Texture3DResourceArray.EraseNonOrdered(i);
	}

	//cubemap textures
	{
		Ut::OpenHashMap<U32, C_ResourcePtr<Display::C_RefTextureCube>, Ut::HashKeyOpSmall<U32>>::Iterator it
			= m_TextureCubeResourceMap.Begin();
		while (it != m_TextureCubeResourceMap.End())
		{
			if (it->GetValue().Count() == 2)
			{
				Display::Device()->Destroy(it->GetValue());
				m_TextureCubeResourceMap.Erase(it);
			}
			it++;
		}

		for (S32 i=0; i<m_TextureCubeResourceArray.Size(); ++i)
			if (m_TextureCubeResourceArray[i].Count() == 1)
				m_TextureCubeResourceArray.EraseNonOrdered(i);
	}
}

bool C_TextureManager::CreateDefaultTextures()
{
	Synchronise<C_TextureManager> sync(this);

	m_Default2DResource = new Display::C_RefTexture2D();
	if (!Display::Device()->CreateTexture2D(1,1,1,Display::E_Usage_Managed,Display::E_Format_A8R8G8B8,m_Default2DResource))
		LOGERROR(g_Log, "Failed to create default 2D texture");

	m_Default3DResource = new Display::C_RefTexture3D();
	if (!Display::Device()->CreateTexture3D(1,1,1,1,Display::E_Usage_Managed,Display::E_Format_A8R8G8B8,m_Default3DResource))
		LOGERROR(g_Log, "Failed to create default 3D texture");

	m_DefaultCubeResource = new Display::C_RefTextureCube();
	if (!Display::Device()->CreateTextureCube(1,1,Display::E_Usage_Managed,Display::E_Format_A8R8G8B8,m_DefaultCubeResource))
		LOGERROR(g_Log, "Failed to create default cube texture");


	if (m_Default2DResource)
	{
		Display::C_LockedRect rect = m_Default2DResource->Lock(0);
		if (rect.pBits)
			rect.pBits[0] = 255; rect.pBits[1] = 255; rect.pBits[2] = 255; rect.pBits[3] = 255;
		m_Default2DResource->Unlock(0);
	}

	if (m_Default3DResource)
	{
		IDirect3DVolume9 *pVolume;
		HRESULT hr;

		hr = m_Default3DResource->GetGPUTexture3D()->GetVolumeLevel(0, &pVolume);
		assert(SUCCEEDED(hr) && "Failed to lock the default d3d 3D texture");
		D3DLOCKED_BOX box;
		hr = pVolume->LockBox(&box, NULL, D3DLOCK_NOSYSLOCK);
		assert(SUCCEEDED(hr) && "Failed to lock the default d3d 3D texture");
		assert(box.pBits);
		U8* pBits = (U8*)box.pBits;
		pBits[0] = 255; pBits[1] = 255; pBits[2] = 255; pBits[3] = 255;
		hr = pVolume->UnlockBox();
		assert(SUCCEEDED(hr) && "Failed to unlock the default d3d 3D texture");
		//getting a volume slice implicitly addrefs the resource ... we need to release it
		assert(m_Default3DResource->GetGPUTexture3D()->Release() == 1);
	}

	if (m_DefaultCubeResource)
	{
		for (U32 face=0; face<6; ++face)
		{
			D3DLOCKED_RECT rect;
			HRESULT hr = m_DefaultCubeResource->GetGPUTextureCube()->LockRect((D3DCUBEMAP_FACES)face, 0, &rect, NULL, D3DLOCK_NOSYSLOCK);
			assert(SUCCEEDED(hr) && "Failed to lock the default cube texture");
			assert(rect.pBits);
			U8* pBits = (U8*)rect.pBits;
			pBits[0] = 255; pBits[1] = 255; pBits[2] = 255; pBits[3] = 255;
			hr = m_DefaultCubeResource->GetGPUTextureCube()->UnlockRect((D3DCUBEMAP_FACES)face, 0);
			assert(SUCCEEDED(hr) && "Failed to unlock the default cube texture");
		}
	}

	return true;
}

C_ResourcePtr<Display::C_RefTexture2D> C_TextureManager::GetDefault2DTexture()
{
	Synchronise<C_TextureManager> sync(this);
	return m_Default2DResource;
}


C_ResourcePtr<Display::C_RefTexture3D> C_TextureManager::GetDefault3DTexture()
{
	Synchronise<C_TextureManager> sync(this);
	return m_Default3DResource;
}

C_ResourcePtr<Display::C_RefTextureCube> C_TextureManager::GetDefaultCubeTexture()
{
	Synchronise<C_TextureManager> sync(this);
	return m_DefaultCubeResource;
}