#include <nexus_render/stdafx.h>

#include <nexus_render/D3DShaderManager.h>
#include <nexus_render/D3DImageLoader.h>

using namespace NexusRender;

D3DShaderManager::D3DShaderManager()
{
	m_pLastShaderPointerReturned = 0;
}

D3DShaderManager::~D3DShaderManager()
{
}

void D3DShaderManager::Create(IDirect3DDevice9 *pD3DDevice)
{
	m_pD3DDevice = pD3DDevice;
}


void D3DShaderManager::Destroy()
{
	// force deletion of remaining shaders
	Clear();	
}

Tu::Status D3DShaderManager::OnPreResetDevice()
{
	std::map<ID3DXEffect*, std::string>::iterator it;
	
	for (
		it = m_shaderResourcesByShaderPointer.begin(); 
		it != m_shaderResourcesByShaderPointer.end(); 
		it++)
	{
		ID3DXEffect* pEffect = it->first;
		if (!pEffect)
		{
			return Tu::Status::FAILURE;
		}

		if (FAILED(pEffect->OnLostDevice()))
		{
			return Tu::Status::FAILURE;
		}

		if (FAILED(pEffect->OnResetDevice()))
		{
			return Tu::Status::FAILURE;
		}

		if (!Load(it->second))
		{
			return Tu::Status::FAILURE;
		}
	}
	
	return Tu::Status::SUCCESS;;
}

Tu::Status D3DShaderManager::OnPostResetDevice()
{
	return Tu::Status::SUCCESS;;
}

ID3DXEffect* D3DShaderManager::Load(const std::string& fileName)
{
	if ((U32)m_shaderResources.size() > 0)
	{
		if (m_pLastShaderPointerReturned != 0)
		{
			if (m_lastShaderFileName == fileName)
			{
				//Use caching to speed up shader processing.
				return m_pLastShaderPointerReturned;
			}
		}
	}
	else
	{
		m_pLastShaderPointerReturned = 0;
	}

	std::map<std::string, D3DShaderResource>::iterator it;
	it = m_shaderResources.find(fileName);
	if (it != m_shaderResources.end()) 
	{
		D3DShaderResource* pIteratorContents = &(it->second);
		pIteratorContents->m_referenceCount++;
		return pIteratorContents->m_pShader;
	}
	else
	{
		//Load a new resource
		D3DShaderResource newResource;
		newResource.m_fileName = fileName;
		newResource.m_referenceCount = 1;
		newResource.m_pShader = LoadFromDisk(fileName);
		if (newResource.m_pShader != 0)
		{
			//Add this resource to our resource array.
			m_shaderResources[fileName] = newResource;
			m_shaderResourcesByShaderPointer[newResource.m_pShader] = fileName;
			return newResource.m_pShader;
		}
		else
		{
			//We failed to load the shader for some reason.
			return 0;
		}
	}
}

void D3DShaderManager::Unload(ID3DXEffect* pShader)
{
	std::map<ID3DXEffect*, std::string>::iterator it;
	it = m_shaderResourcesByShaderPointer.find(pShader);

	if (it != m_shaderResourcesByShaderPointer.end()) 
	{
		std::string fileName = it->second;

		std::map<std::string, D3DShaderResource>::iterator it2;
		it2 = m_shaderResources.find(fileName);
		if (it2 != m_shaderResources.end())
		{
			D3DShaderResource* pIteratorContents = &(it2->second);

			if (pIteratorContents->m_referenceCount <= 1)
			{
				pIteratorContents->m_pShader->Release();

				m_shaderResources.erase(it2);
				m_shaderResourcesByShaderPointer.erase(it);

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

U32 D3DShaderManager::NumShaders() const
{
	return (U32)m_shaderResources.size();
}

void D3DShaderManager::Clear()
{
	std::map<std::string, D3DShaderResource>::iterator it;
	for (it = m_shaderResources.begin(); it != m_shaderResources.end(); it++) 
	{
		D3DShaderResource* pShaderResource = &(it->second);
		if (pShaderResource->m_pShader != 0) 
		{
			pShaderResource->m_pShader->Release();
		}
	}
	m_shaderResources.clear();
	m_shaderResourcesByShaderPointer.clear();
}

ID3DXEffect* D3DShaderManager::LoadFromDisk(const std::string fileName)
{
	ID3DXEffect*	pShader = 0;

#ifdef D3D_USE_REF_DEVICE
	DWORD dwShaderFlags = D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
#else
	DWORD dwShaderFlags = 0;
#endif

#ifndef LOAD_SHADERS_FROM_FILE
	HRESULT	hr;
	// determine whether it's a resource or a filename by looking at the string
	if (!strrchr(fileName.c_str(),'.')) // if it doesn't have a file-extension maybe it's a resource
	{
		// load it from a resource
		hr = D3DXCreateEffectFromResource(
			m_pD3DDevice, 
			0, 
			fileName.c_str(),
			0, 
			0, 
			dwShaderFlags,
			0,
			&pShader, 
			0);

		if (SUCCEEDED(hr))
		{
			return pShader;
		}
	}
	else
#endif
	{
		// if we didn't manage to load it from a resource try from a file
		if (FAILED( D3DXCreateEffectFromFile(
			m_pD3DDevice, 
			fileName.c_str(),
			0, 
			0, 
			dwShaderFlags,
			0, 
			&pShader, 
			0)))
		{
			return 0;
		}
	}

	return pShader;
}
