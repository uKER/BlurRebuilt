#include "stdafx.h"

#include <scene_renderer/D3dTextureManager.h>
#include <scene_renderer/D3dImageLoader.h>

using namespace SRD;

D3dTextureManager *D3dTextureManager::m_pSingleton = 0;

D3dTextureManager::D3dTextureManager()
{
}

D3dTextureManager::~D3dTextureManager()
{
}

D3dTextureManager &D3dTextureManager::GetSingleton()
{
	if (!m_pSingleton) 
	{
		m_pSingleton = new D3dTextureManager;
		m_pSingleton->m_pLastTexturePointerReturned = 0;
	}

	return *m_pSingleton;
}


void D3dTextureManager::Create(IDirect3DDevice9 *pD3DDevice)
{
	m_pSingleton->m_pD3DDevice = pD3DDevice;
}


void D3dTextureManager::Destroy()
{
	// force deletion of remaining textures
	m_pSingleton->Clear();	

	delete m_pSingleton;
}

IDirect3DTexture9* D3dTextureManager::Load(const std::string& fileName)
{
	if ((U32)m_textureResources.size() > 0)
	{
		if (m_pLastTexturePointerReturned != 0)
		{
			if (m_lastTextureFileName == fileName)
			{
				//Use caching to speed up texture processing.
				return m_pLastTexturePointerReturned;
			}
		}
	}
	else
	{
		m_pLastTexturePointerReturned = 0;
	}

	std::map<std::string, D3dTextureResource>::iterator it;
	it = m_textureResources.find(fileName);
	if (it != m_textureResources.end()) 
	{
		D3dTextureResource* pIteratorContents = &(it->second);
		pIteratorContents->m_referenceCount++;
		return pIteratorContents->m_pTexture;
	}
	else
	{
		//Load a new resource
		D3dTextureResource newResource;
		newResource.m_fileName = fileName;
		newResource.m_referenceCount = 1;
		newResource.m_pTexture = LoadFromDisk(fileName);
		if (newResource.m_pTexture != 0)
		{
			//Add this resource to our resource array.
			m_textureResources[fileName] = newResource;
			m_textureResourcesByTexturePointer[newResource.m_pTexture] = fileName;
			return newResource.m_pTexture;
		}
		else
		{
			//We failed to load the texture for some reason.
			return 0;
		}
	}
}

void D3dTextureManager::Unload(IDirect3DTexture9* pTexture)
{
	std::map<IDirect3DTexture9*, std::string>::iterator it;
	it = m_textureResourcesByTexturePointer.find(pTexture);

	if (it != m_textureResourcesByTexturePointer.end()) 
	{
		std::string fileName = it->second;

		std::map<std::string, D3dTextureResource>::iterator it2;
		it2 = m_textureResources.find(fileName);
		if (it2 != m_textureResources.end())
		{
			D3dTextureResource* pIteratorContents = &(it2->second);

			if (pIteratorContents->m_referenceCount <= 1)
			{
				pIteratorContents->m_pTexture->Release();

				m_textureResources.erase(it2);
				m_textureResourcesByTexturePointer.erase(it);

				return;
			}
			else
			{
				//decrease reference count
				pIteratorContents->m_referenceCount--;
				return;
			}
		}
	}
}

U32 D3dTextureManager::GetNumTextures() const
{
	return (U32)m_textureResources.size();
}

void D3dTextureManager::Clear()
{
	std::map<std::string, D3dTextureResource>::iterator it;
	for (it = m_textureResources.begin(); it != m_textureResources.end(); it++) 
	{
		D3dTextureResource* pTextureResource = &(it->second);
		if (pTextureResource->m_pTexture != 0) 
		{
			pTextureResource->m_pTexture->Release();
		}
	}
	m_textureResources.clear();
	m_textureResourcesByTexturePointer.clear();
}

IDirect3DTexture9* D3dTextureManager::LoadFromDisk(const std::string fileName)
{
	D3dImageLoader		imageLoader;
	IDirect3DTexture9*	pTexture = 0;
	_D3DFORMAT			format;
	imageLoader.LoadToTexture(fileName, m_pD3DDevice, format, &pTexture);
	
	return pTexture;
}

