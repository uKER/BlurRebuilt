#include "stdafx.h"

#include <scene_renderer/D3dShaderManager.h>
#include <scene_renderer/D3dImageLoader.h>

using namespace SRD;

D3dShaderManager *D3dShaderManager::m_pSingleton = 0;

D3dShaderManager::D3dShaderManager()
{
}

D3dShaderManager::~D3dShaderManager()
{
}

D3dShaderManager &D3dShaderManager::GetSingleton()
{
	if (!m_pSingleton) 
	{
		m_pSingleton = new D3dShaderManager;
		m_pSingleton->m_pLastShaderPointerReturned = 0;
	}

	return *m_pSingleton;
}


void D3dShaderManager::Create(IDirect3DDevice9 *pD3DDevice)
{
	m_pSingleton->m_pD3DDevice = pD3DDevice;
}


void D3dShaderManager::Destroy()
{
	// force deletion of remaining shaders
	m_pSingleton->Clear();	

	delete m_pSingleton;
}

ID3DXEffect* D3dShaderManager::Load(const std::string& fileName)
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

	std::map<std::string, D3dShaderResource>::iterator it;
	it = m_shaderResources.find(fileName);
	if (it != m_shaderResources.end()) 
	{
		D3dShaderResource* pIteratorContents = &(it->second);
		pIteratorContents->m_referenceCount++;
		return pIteratorContents->m_pShader;
	}
	else
	{
		//Load a new resource
		D3dShaderResource newResource;
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

void D3dShaderManager::Unload(ID3DXEffect* pShader)
{
	std::map<ID3DXEffect*, std::string>::iterator it;
	it = m_shaderResourcesByShaderPointer.find(pShader);

	if (it != m_shaderResourcesByShaderPointer.end()) 
	{
		std::string fileName = it->second;

		std::map<std::string, D3dShaderResource>::iterator it2;
		it2 = m_shaderResources.find(fileName);
		if (it2 != m_shaderResources.end())
		{
			D3dShaderResource* pIteratorContents = &(it2->second);

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

U32 D3dShaderManager::GetNumShaders() const
{
	return (U32)m_shaderResources.size();
}

void D3dShaderManager::Clear()
{
	std::map<std::string, D3dShaderResource>::iterator it;
	for (it = m_shaderResources.begin(); it != m_shaderResources.end(); it++) 
	{
		D3dShaderResource* pShaderResource = &(it->second);
		if (pShaderResource->m_pShader != 0) 
		{
			pShaderResource->m_pShader->Release();
		}
	}
	m_shaderResources.clear();
	m_shaderResourcesByShaderPointer.clear();
}

ID3DXEffect* D3dShaderManager::LoadFromDisk(const std::string fileName)
{
	ID3DXEffect*	pShader = 0;
	HRESULT			hr;

	DWORD dwShaderFlags = 0;

	// determine whether it's a resource or a filename by looking at the string
	if (!strrchr(fileName.c_str(),'.')) // if it doesn't have a file-extension maybe it's a resource
	{
		// load it from a resource
		hr = D3DXCreateEffectFromResource(m_pD3DDevice, NULL, fileName.c_str(), NULL, NULL, dwShaderFlags, NULL, &pShader, NULL);

		if (SUCCEEDED(hr))
		{
			return pShader;
		}
	}

	// if we didn't manage to load it from a resource try from a file
	if (FAILED( D3DXCreateEffectFromFile(
		m_pD3DDevice, 
		fileName.c_str(),
		0, 
		0, 
		dwShaderFlags|D3DXSHADER_USE_LEGACY_D3DX9_31_DLL,
		0, 
		&pShader, 
		0)))
	{
		return 0;
	}
	
	return pShader;
}
