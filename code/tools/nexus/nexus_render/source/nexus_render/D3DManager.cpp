#include <nexus_render/stdafx.h>
#include <nexus_render/D3DManager.h>

namespace NexusRender
{
	char* g_vertexDeclAsString[D3DManager::VERTEX_DECL_COUNT] = {
		"pos", 
		"posCol",

		"posNormTex1",
		"posNormTex2",
		"posNormTex1Weight4Idx4",
		"posNormTex2Weight4Idx4",

		"posNormColTex1",
		"posNormColTex2",
		"posNormColTex1Weight4Idx4",
		"posNormColTex2Weight4Idx4"};
}


using namespace NexusRender;
//Initialise singleton static.
D3DManager *D3DManager::m_pSingleton = 0;
char** D3DManager::m_ppVertexDeclAsString = g_vertexDeclAsString;

D3DVERTEXELEMENT9 g_vertexPosDeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	D3DDECL_END()
};

D3DVERTEXELEMENT9 g_vertexPosColDeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	D3DDECL_END()
};

D3DVERTEXELEMENT9 g_vertexPosNormColTex1DeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{0, 28, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};

D3DVERTEXELEMENT9 g_vertexPosNormTex1DeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};

D3DVERTEXELEMENT9 g_vertexPosNormTex2DeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0, 32, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
	D3DDECL_END()
};

D3DVERTEXELEMENT9 g_vertexPosNormColTex2DeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{0, 28, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0, 36, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
	D3DDECL_END()
};

D3DVERTEXELEMENT9 g_vertexPosNormTex1Weight4Idx4DeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0, 32, D3DDECLTYPE_FLOAT4,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
	{0, 48, D3DDECLTYPE_UBYTE4,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},

	D3DDECL_END()
};

D3DVERTEXELEMENT9 g_vertexPosNormTex2Weight4Idx4DeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0, 32, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
	{0, 40, D3DDECLTYPE_FLOAT4,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
	{0, 56, D3DDECLTYPE_UBYTE4,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},

	D3DDECL_END()
};


D3DVERTEXELEMENT9 g_vertexPosNormColTex1Weight4Idx4DeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{0, 28, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0, 36, D3DDECLTYPE_FLOAT4,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
	{0, 52, D3DDECLTYPE_UBYTE4,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},

	D3DDECL_END()
};

D3DVERTEXELEMENT9 g_vertexPosNormColTex2Weight4Idx4DeclS0[]=
{
	{0,  0, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0, 24, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{0, 28, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0, 36, D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
	{0, 44, D3DDECLTYPE_FLOAT4,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
	{0, 60, D3DDECLTYPE_UBYTE4,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},

	D3DDECL_END()
};

D3DManager::D3DManager()
{
}

D3DManager::~D3DManager()
{
}

Tu::Status D3DManager::Create(IDirect3DDevice9* pDevice)
{
	if (m_pSingleton)
	{
		m_pSingleton->m_referenceCount++;
		return Tu::Status::SUCCESS;
	}

	m_pSingleton = new D3DManager();

	m_pSingleton->m_referenceCount = 0;
	m_pSingleton->m_pDevice = pDevice;

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosDeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosColDeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS_COL])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosNormTex1DeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS_NORM_TEX1])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosNormColTex1DeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS_NORM_COL_TEX1])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosNormTex2DeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS_NORM_TEX2])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosNormColTex2DeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS_NORM_COL_TEX2])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosNormTex1Weight4Idx4DeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS_NORM_TEX1_WEIGHT4_IDX4])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosNormTex2Weight4Idx4DeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS_NORM_TEX2_WEIGHT4_IDX4])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosNormColTex1Weight4Idx4DeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS_NORM_COL_TEX1_WEIGHT4_IDX4])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");

	if(FAILED(pDevice->CreateVertexDeclaration(
		g_vertexPosNormColTex2Weight4Idx4DeclS0,
		&m_pSingleton->m_pVertexDeclList[VERTEX_DECL_POS_NORM_COL_TEX2_WEIGHT4_IDX4])))
		return Tu::Status(Tu::Status::FAILURE, "Failed to create vertex format.");


	m_pSingleton->m_referenceCount++;
	return Tu::Status::SUCCESS;
}


void D3DManager::Destroy()
{
	if (m_pSingleton)
	{
		m_pSingleton->m_referenceCount--;
		if (m_pSingleton->m_referenceCount)
			return;

		//Release vertex declarations.
		for (U32 iDecl = 0; iDecl < VERTEX_DECL_COUNT; iDecl++)
		{
			m_pSingleton->m_pVertexDeclList[iDecl]->Release();
		}

		delete m_pSingleton;
		m_pSingleton = 0;
	}
}

Tu::Status D3DManager::OnPreResetDevice()
{
	return Tu::Status::SUCCESS;
}

Tu::Status D3DManager::OnPostResetDevice()
{
	return Tu::Status::SUCCESS;
}

IDirect3DVertexDeclaration9* D3DManager::GetVertexDeclPtr(EVertexDecl vertexDecl)
{
	return m_pSingleton->m_pVertexDeclList[vertexDecl];
}

Tu::Status D3DManager::SetVertexDeclaration(EVertexDecl vertexDecl)
{
	if (FAILED(m_pSingleton->m_pDevice->SetVertexDeclaration(
		m_pSingleton->m_pVertexDeclList[vertexDecl])))
	{
		return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_VERTEX_DECL);
	}
	return Tu::Status::SUCCESS;
}

IDirect3DDevice9* D3DManager::GetDevicePtr()
{
	return m_pSingleton->m_pDevice;
}

