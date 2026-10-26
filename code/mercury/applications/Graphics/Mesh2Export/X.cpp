#include "StdAfx.h"
#include "X.h"

C_XMesh::C_XMesh() :
	m_Options(D3DXMESH_SYSTEMMEM),
	m_MaterialCount(0),
	m_pMaterials(NULL),
	m_pAdjacency(NULL),
	m_pMesh(NULL)
{
}

C_XMesh::~C_XMesh()
{
	Destroy();
}

void C_XMesh::Destroy()
{
	SAFE_DELETE_ARY(m_pMaterials);
	SAFE_DELETE_ARY(m_pAdjacency);
	SAFE_RELEASE(m_pMesh);
}

bool C_XMesh::Load(const Util::C_String& filePath)
{
	Destroy();

	ID3DXBuffer* pAdjacency = NULL;
	ID3DXBuffer* pMaterials = NULL;
	ID3DXBuffer* pEffectInstances = NULL;

	const Util::C_String newFilePath = filePath;
	
	HRESULT hr = D3DXLoadMeshFromX(
		newFilePath.Chars(),
		m_Options,
		g_pd3dDevice,
		&pAdjacency,
		&pMaterials,
		&pEffectInstances,
		&m_MaterialCount,
		&m_pMesh);
	if(SUCCEEDED(hr))
	{
		if(m_MaterialCount > 0)
		{
			m_pMaterials = new C_XMaterial[m_MaterialCount];

			D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pMaterials->GetBufferPointer();

			for(DWORD i=0; i<m_MaterialCount; i++)
			{
				if(d3dxMaterials[i].pTextureFilename)
					m_pMaterials[i].m_TextureFileName = d3dxMaterials[i].pTextureFilename;
			}
		}

		m_pAdjacency = new DWORD[m_pMesh->GetNumFaces()*3];
		memcpy(m_pAdjacency, pAdjacency->GetBufferPointer(), sizeof(DWORD)*m_pMesh->GetNumFaces()*3);
		
		SAFE_RELEASE(pAdjacency);
		SAFE_RELEASE(pMaterials);
		SAFE_RELEASE(pEffectInstances);

		if(!Clean(D3DXCLEAN_BACKFACING))
			return false;
		if(!Clean(D3DXCLEAN_BOWTIES))
			return false;
		if(!Clean(D3DXCLEAN_SKINNING))
			return false;
		if(!Clean(D3DXCLEAN_OPTIMIZATION))
			return false;
		if(!Clean(D3DXCLEAN_SIMPLIFICATION))
			return false;
		if(!Optimize(D3DXMESHOPT_ATTRSORT))
			return false;

		return true;
	}
	else
	{
		LOGERROR(g_Log, "ERROR: Failed to load mesh from file %s\n", newFilePath.Chars());
		return false;
	}
}

bool C_XMesh::Optimize(const DWORD flags)
{
	ID3DXBuffer* pVertexRemap = NULL;

	ID3DXMesh* pNewMesh = NULL;

	DWORD* pNewAdjacency = new DWORD[3*m_pMesh->GetNumFaces()];

	HRESULT hr = m_pMesh->Optimize(
		flags,
		m_pAdjacency,
		pNewAdjacency,
		NULL,
		&pVertexRemap,
		&pNewMesh);
	if(SUCCEEDED(hr))
	{
		m_pAdjacency = pNewAdjacency;

		SAFE_DELETE_ARY(m_pAdjacency);

		SAFE_RELEASE(pVertexRemap);

		SAFE_RELEASE(m_pMesh);

		m_pMesh = pNewMesh;

		return true;
	}
	else
	{
		SAFE_DELETE_ARY(pNewAdjacency);

		return false;
	}
}

bool C_XMesh::Clean(const D3DXCLEANTYPE cleanType)
{
	ID3DXMesh* pNewMesh = NULL;

	DWORD* pNewAdjacency = new DWORD[3*m_pMesh->GetNumFaces()];

	HRESULT hr = D3DXCleanMesh(
		cleanType,
		m_pMesh,
		m_pAdjacency,
		&pNewMesh,
		pNewAdjacency,
		NULL);
	if(SUCCEEDED(hr))
	{
		SAFE_DELETE_ARY(m_pAdjacency);
		m_pAdjacency = pNewAdjacency;

		const ULONG refCount = m_pMesh->Release();
		//BIZ_ASSERT(refCount == 0);
		m_pMesh = pNewMesh;
		
		return true;
	}
	else
	{
		SAFE_DELETE_ARY(pNewAdjacency);

		return false;
	}
}

bool C_XMesh::ComputeTangentFrame()
{
	HRESULT hr;

	U32 vertexStride = m_pMesh->GetNumBytesPerVertex();

	D3DVERTEXELEMENT9 oldDeclarations[MAX_FVF_DECL_SIZE];
	hr = m_pMesh->GetDeclaration(oldDeclarations);
	BIZ_ASSERT(SUCCEEDED(hr));

	D3DVERTEXELEMENT9 newDeclarations[MAX_FVF_DECL_SIZE];
	memcpy(newDeclarations, oldDeclarations, sizeof(D3DVERTEXELEMENT9)*MAX_FVF_DECL_SIZE);
	BIZ_ASSERT(SUCCEEDED(hr));

	int i=0;
	while(newDeclarations[i].Stream != 255)
	{
		i++;
	}

	newDeclarations[i].Stream = 0;
	newDeclarations[i].Offset = vertexStride;
	newDeclarations[i].Type = D3DDECLTYPE_FLOAT3;
	newDeclarations[i].Method = 0;
	newDeclarations[i].Usage = D3DDECLUSAGE_TANGENT;
	newDeclarations[i].UsageIndex = 0;
	vertexStride += 12;
	i++;

	/*newDeclarations[i].Stream = 0;
	newDeclarations[i].Offset = vertexStride;
	newDeclarations[i].Type = D3DDECLTYPE_FLOAT3;
	newDeclarations[i].Method = 0;
	newDeclarations[i].Usage = D3DDECLUSAGE_BINORMAL;
	newDeclarations[i].UsageIndex = 0;
	vertexStride += 12;
	i++;*/

	newDeclarations[i].Stream = 0xFF;
	newDeclarations[i].Offset = 0;
	newDeclarations[i].Type = D3DDECLTYPE_UNUSED;
	newDeclarations[i].Method = 0;
	newDeclarations[i].Usage = 0;
	newDeclarations[i].UsageIndex = 0;

	ID3DXMesh* pNewMesh = NULL;
	hr = m_pMesh->CloneMesh(
		m_Options,
		newDeclarations,
		g_pd3dDevice,
		&pNewMesh);
	if(SUCCEEDED(hr))
	{
		SAFE_DELETE_ARY(m_pAdjacency);
		m_pAdjacency = new DWORD[3*m_pMesh->GetNumFaces()];

		hr = m_pMesh->GenerateAdjacency(0.001f, m_pAdjacency);
		BIZ_ASSERT(SUCCEEDED(hr));

		const ULONG refCount = m_pMesh->Release();
		BIZ_ASSERT(refCount == 0);
		m_pMesh = pNewMesh;

#if 1
		hr = D3DXComputeTangent(
			m_pMesh,
			0,
			0,
			D3DX_DEFAULT,
			0,
			m_pAdjacency);
		if(SUCCEEDED(hr))
		{
			return true;
		}
		else
		{
			return false;
		}
#else
		hr = D3DXComputeTangentFrameEx(m_pMesh,
			D3DDECLUSAGE_TEXCOORD, 0,   
			D3DDECLUSAGE_BINORMAL, 0,
			D3DDECLUSAGE_TANGENT, 0, 
			D3DDECLUSAGE_NORMAL, 0, 
			m_Options | D3DXTANGENT_GENERATE_IN_PLACE,
			NULL, 0.01f, 0.25f, 0.01f, NULL, NULL);
		if(SUCCEEDED(hr))
		{
			return true;
		}
		else
		{
			return false;
		}
#endif

		return true;
	}
	else
	{
		return false;
	}
}
