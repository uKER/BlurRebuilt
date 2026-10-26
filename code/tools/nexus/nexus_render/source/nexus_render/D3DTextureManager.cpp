#include <nexus_render/stdafx.h>

#include <nexus_render/D3DTextureManager.h>
#include <nexus_render/D3DImageLoader.h>

using namespace NexusRender;

D3DTextureManager::D3DTextureManager()
{
	m_pLastTexturePointerReturned = 0;
	m_pBlankTexture = 0;
	m_referenceCount = 0;
}

D3DTextureManager::~D3DTextureManager()
{
}

void D3DTextureManager::Create(IDirect3DDevice9 *pD3DDevice)
{
	m_referenceCount++;

	m_pD3DDevice = pD3DDevice;
	m_pBlankTexture = LoadBlankFromMemory();
}


void D3DTextureManager::Destroy()
{
	m_referenceCount--;
	if (m_referenceCount)
		return;

	// force deletion of remaining textures
	Clear();

	if (m_pBlankTexture)
	{
		m_pBlankTexture->Release();
		m_pBlankTexture = 0;
	}
}

Tu::Status D3DTextureManager::OnPreResetDevice()
{
	return Tu::Status::SUCCESS;
}

Tu::Status D3DTextureManager::OnPostResetDevice()
{
	return Tu::Status::SUCCESS;
}

IDirect3DTexture9* D3DTextureManager::Blank()
{
	return m_pBlankTexture;
}

IDirect3DTexture9* D3DTextureManager::Find(const std::string &fileName)
{
	std::map<std::string, D3DTextureResource*>::iterator it;
	it = m_textureResources.find(fileName);
	if (it != m_textureResources.end()) 
	{
		D3DTextureResource* pIteratorContents = it->second;
		return pIteratorContents->m_pTexture;
	}

	return 0;
}

S32 D3DTextureManager::TextureIndex(const std::string& fileName) const
{
	std::map<std::string, D3DTextureResource*>::const_iterator it;
	it = m_textureResources.find(fileName);
	if (it != m_textureResources.end()) 
	{
		D3DTextureResource* pIteratorContents = it->second;
		return pIteratorContents->m_index;
	}

	return -1;
}

IDirect3DTexture9* D3DTextureManager::Load(
	const std::string& fileName,
	S32 sizeCap,
	U32* pWidth,
	U32* pHeight)
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

	std::map<std::string, D3DTextureResource*>::iterator it;
	it = m_textureResources.find(fileName);
	if (it != m_textureResources.end()) 
	{
		D3DTextureResource* pIteratorContents = it->second;
		pIteratorContents->m_referenceCount++;
		if (pWidth)
			*pWidth = pIteratorContents->m_width;
		if (pHeight)
			*pHeight = pIteratorContents->m_height;
		return pIteratorContents->m_pTexture;
	}
	else
	{
		//Load a new resource
		D3DTextureResource* pNewResource = new D3DTextureResource;
		pNewResource->m_fileName = fileName;
		pNewResource->m_referenceCount = 1;
		pNewResource->m_pTexture = LoadFromDisk(
			pNewResource->m_width,
			pNewResource->m_height,
			fileName, 
			sizeCap);
		if (pWidth)
			*pWidth = pNewResource->m_width;
		if (pHeight)
			*pHeight = pNewResource->m_height;
		pNewResource->m_index = (U32)m_textureResourceArray.Size();
		m_textureResourceArray.PushBack(pNewResource);

		//Add this resource to our resource array.
		m_textureResources[fileName] = pNewResource;
		m_textureResourcesByTexturePointer[pNewResource->m_pTexture] = fileName;
		return pNewResource->m_pTexture;
	}
}

IDirect3DTexture9* D3DTextureManager::LoadBlank(const std::string& fileName)
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

	std::map<std::string, D3DTextureResource*>::iterator it;
	it = m_textureResources.find(fileName);
	if (it != m_textureResources.end()) 
	{
		D3DTextureResource* pIteratorContents = it->second;
		pIteratorContents->m_referenceCount++;
		return pIteratorContents->m_pTexture;
	}
	else
	{
		//Load a new resource
		D3DTextureResource* pNewResource = new D3DTextureResource;
		pNewResource->m_fileName = fileName;
		pNewResource->m_referenceCount = 1;
		pNewResource->m_pTexture = LoadBlankFromMemory();
		if (pNewResource->m_pTexture != 0)
		{
			//Add this resource to our resource array.
			m_textureResources[fileName] = pNewResource;
			m_textureResourcesByTexturePointer[pNewResource->m_pTexture] = fileName;
			return pNewResource->m_pTexture;
		}
		else
		{
			//We failed to load the texture for some reason.
			return 0;
		}
	}
}

void D3DTextureManager::Unload(IDirect3DTexture9* pTexture)
{
	std::map<IDirect3DTexture9*, std::string>::iterator it;
	it = m_textureResourcesByTexturePointer.find(pTexture);

	if (it != m_textureResourcesByTexturePointer.end()) 
	{
		std::string fileName = it->second;

		std::map<std::string, D3DTextureResource*>::iterator it2;
		it2 = m_textureResources.find(fileName);
		if (it2 != m_textureResources.end())
		{
			D3DTextureResource* pIteratorContents = it2->second;

			if (pIteratorContents->m_referenceCount <= 1)
			{
				pIteratorContents->m_pTexture->Release();

				m_textureResources.erase(it2);
				m_textureResourcesByTexturePointer.erase(it);
				delete pIteratorContents;
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

U32 D3DTextureManager::GetNumTextures() const
{
	return (U32)m_textureResources.size();
}

void D3DTextureManager::Clear()
{
	std::map<std::string, D3DTextureResource*>::iterator it;
	for (it = m_textureResources.begin(); it != m_textureResources.end(); it++) 
	{
		D3DTextureResource* pTextureResource = it->second;
		if (pTextureResource->m_pTexture != 0) 
		{
			pTextureResource->m_pTexture->Release();
		}
		delete pTextureResource;
	}
	m_textureResources.clear();
	m_textureResourcesByTexturePointer.clear();
}

IDirect3DTexture9* D3DTextureManager::LoadFromDisk(
	U32& width,
	U32& height,
	const std::string fileName, 
	S32 sizeCap)
{
	D3DImageLoader		imageLoader;
	IDirect3DTexture9*	pTexture = 0;
	_D3DFORMAT			format;

	imageLoader.LoadToTextureFromDisk(
		&pTexture,
		format, 
		width,
		height,
		fileName, 
		sizeCap,
		m_pD3DDevice);
	
	return pTexture;
}

IDirect3DTexture9* D3DTextureManager::LoadBlankFromMemory()
{
	D3DImageLoader		imageLoader;
	IDirect3DTexture9*	pTexture = 0;
	_D3DFORMAT			format;

	U8 rawImageData[] = {255,255,255,255};

	imageLoader.LoadToTextureFromMemory(
		rawImageData,
		1,
		1,
		4,
		m_pD3DDevice, 
		format, 
		&pTexture);
	
	return pTexture;
}


