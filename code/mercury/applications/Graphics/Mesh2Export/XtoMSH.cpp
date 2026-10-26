#include "StdAfx.h"
#include "XtoMSH.h"

void XtoMSH(const Util::C_String& srcFileDir, const Util::C_String& srcFileName, C_XMesh* pSrcModel, C_Mesh* pDstModel)
{
	HRESULT hr;

	const F32 vertexScale = 5.0f;

	const U32 vertexCount = pSrcModel->m_pMesh->GetNumVertices();
	const U32 vertexStride = pSrcModel->m_pMesh->GetNumBytesPerVertex();
	
	const U32 faceCount = pSrcModel->m_pMesh->GetNumFaces();
	const U32 indexCount = faceCount*3;
	const U32 indexStride = sizeof(U16);

	// Add vertex declaration

	S32 iDstVertexDeclaration = -1;
	C_VertexDeclaration* pDstVertexDeclaration = NULL;
	pDstModel->AddVertexDeclaration(iDstVertexDeclaration, &pDstVertexDeclaration);

	D3DVERTEXELEMENT9 srcDeclarations[MAX_FVF_DECL_SIZE];
	hr = pSrcModel->m_pMesh->GetDeclaration(srcDeclarations);
	BIZ_ASSERT(SUCCEEDED(hr));

	D3DVERTEXELEMENT9* pSrcElement = srcDeclarations;
	while(pSrcElement->Stream != 255)
	{
		U32 dstElementType = Render::E_Mesh2VertexElementType_Unknown;
		switch(pSrcElement->Type)
		{
		case D3DDECLTYPE_FLOAT2:
			dstElementType = Render::E_Mesh2VertexElementType_F32_2;
			break;
		case D3DDECLTYPE_FLOAT3:
			dstElementType = Render::E_Mesh2VertexElementType_F32_3;
			break;
		case D3DDECLTYPE_D3DCOLOR:
			dstElementType = Render::E_Mesh2VertexElementType_Colour;
			break;
		default:
			BIZ_ASSERT(0 && "Unsupported element type");
			break;
		}

		U32 dstElementUsage = Render::E_Mesh2VertexElementUsage_Unknown;
		switch(pSrcElement->Usage)
		{
		case D3DDECLUSAGE_POSITION:
			dstElementUsage = Render::E_Mesh2VertexElementUsage_Position;
			break;
		case D3DDECLUSAGE_NORMAL:
			dstElementUsage = Render::E_Mesh2VertexElementUsage_Normal;
			break;
		case D3DDECLUSAGE_COLOR:
			dstElementUsage = Render::E_Mesh2VertexElementUsage_Colour;
			break;
		case D3DDECLUSAGE_TEXCOORD:
			dstElementUsage = Render::E_Mesh2VertexElementUsage_TexCoord;
			break;
		case D3DDECLUSAGE_TANGENT:
			dstElementUsage = Render::E_Mesh2VertexElementUsage_Tangent;
			break;
		case D3DDECLUSAGE_BINORMAL:
			dstElementUsage = Render::E_Mesh2VertexElementUsage_Binormal;
			break;
		default:
			BIZ_ASSERT(0 && "Unsupported element usage");
			break;
		}

		pDstVertexDeclaration->AddElement((U8)pSrcElement->Stream, pSrcElement->Offset, (U8)dstElementType, (U8)dstElementUsage, (U8)pSrcElement->UsageIndex);

		pSrcElement++;
	}

	// Add vertex buffer

	S32 iDstVertexBuffer = -1;
	C_VertexBuffer* pDstVertexBuffer = NULL;
	pDstModel->AddVertexBuffer(iDstVertexBuffer, &pDstVertexBuffer);

	void* pVertices = NULL;
	hr = pSrcModel->m_pMesh->LockVertexBuffer(0, &pVertices);
	BIZ_ASSERT(SUCCEEDED(hr));

	U8* pTemp;

	F32 vMin[3] = { FLT_MAX,  FLT_MAX,  FLT_MAX};
	F32 vMax[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

	pTemp = (U8*)pVertices;
	for(U32 i=0; i<vertexCount; i++)
	{
		F32* vPos = (F32*)pTemp;

		if(vPos[0] < vMin[0])
			vMin[0] = vPos[0];
		if(vPos[1] < vMin[1])
			vMin[1] = vPos[1];
		if(vPos[2] < vMin[2])
			vMin[2] = vPos[2];

		if(vPos[0] > vMax[0])
			vMax[0] = vPos[0];
		if(vPos[1] > vMax[1])
			vMax[1] = vPos[1];
		if(vPos[2] > vMax[2])
			vMax[2] = vPos[2];

		pTemp += vertexStride;
	}

	F32 vCen[3] = {(vMin[0]+vMax[0])/2.0f, (vMin[1]+vMax[1])/2.0f, (vMin[2]+vMax[2])/2.0f};

	pTemp = (U8*)pVertices;
	for(U32 i=0; i<vertexCount; i++)
	{
		F32* vPos = (F32*)pTemp;

		vPos[0] -= vCen[0];
		vPos[1] -= vCen[1];
		vPos[2] -= vCen[2];
		
		vPos[1] += (vMin[1]+vMax[1])/2.0f;

		vPos[0] *= vertexScale;
		vPos[1] *= vertexScale;
		vPos[2] *= vertexScale;

		pTemp += vertexStride;
	}

	F32 fRad = 0.0f;
	if(fRad < vMax[0]-vMin[0])
		fRad = vMax[0]-vMin[0];
	if(fRad < vMax[1]-vMin[1])
		fRad = vMax[1]-vMin[1];
	if(fRad < vMax[2]-vMin[2])
		fRad = vMax[2]-vMin[2];

	fRad /= 2.0f;

	pDstVertexBuffer->m_Vertices.Resize(vertexStride*vertexCount);
	memcpy(&pDstVertexBuffer->m_Vertices.Front(), pVertices, vertexStride*vertexCount);

	hr = pSrcModel->m_pMesh->UnlockVertexBuffer();
	BIZ_ASSERT(SUCCEEDED(hr));

	// Add index buffer

	S32 iDstIndexBuffer = -1;
	C_IndexBuffer* pDstIndexBuffer = NULL;
	pDstModel->AddIndexBuffer(iDstIndexBuffer, &pDstIndexBuffer);

	void* pIndices = NULL;
	hr = pSrcModel->m_pMesh->LockIndexBuffer(0, &pIndices);
	BIZ_ASSERT(SUCCEEDED(hr));

	pDstIndexBuffer->m_Indices.Resize(indexStride*indexCount);
	memcpy(&pDstIndexBuffer->m_Indices.Front(), pIndices, indexStride*indexCount);

	hr = pSrcModel->m_pMesh->UnlockIndexBuffer();
	BIZ_ASSERT(SUCCEEDED(hr));

	// Add batches

	DWORD srcBatchCount;
	hr = pSrcModel->m_pMesh->GetAttributeTable(NULL, &srcBatchCount);
	BIZ_ASSERT(SUCCEEDED(hr));

	D3DXATTRIBUTERANGE* pSrcBatches = new D3DXATTRIBUTERANGE[srcBatchCount];

	hr = pSrcModel->m_pMesh->GetAttributeTable(pSrcBatches, &srcBatchCount);
	BIZ_ASSERT(SUCCEEDED(hr));

	for(DWORD i=0; i<srcBatchCount; i++)
	{
		D3DXATTRIBUTERANGE& srcBatch = pSrcBatches[i];

		C_XMaterial& srcMaterial = pSrcModel->m_pMaterials[srcBatch.AttribId];
		Util::C_String srcTextureName = srcFileDir + srcMaterial.m_TextureFileName;

		// Add texture
		
		S32 iDstTexture = -1;
		C_Texture* pDstTexture = NULL;
		pDstModel->AddTexture(srcTextureName, iDstTexture, &pDstTexture);

		// Add material

		S32 iDstMaterial = -1;
		C_Material* pDstMaterial = NULL;
		if(!pDstModel->AddMaterial(srcTextureName, iDstMaterial, &pDstMaterial))
		{
			C_Sampler* pDstSampler = pDstMaterial->AddSampler();
			
			pDstSampler->m_Texture = iDstTexture;
		}

		// Add batch

		C_Batch* pDstBatch = pDstModel->AddBatch();
		
		pDstBatch->m_VertexDeclaration	= iDstVertexDeclaration;
		pDstBatch->m_VertexBuffer		= iDstVertexBuffer;
		pDstBatch->m_IndexBuffer		= iDstIndexBuffer;
		pDstBatch->m_Material			= iDstMaterial;
		pDstBatch->m_PrimitiveType		= Render::E_Mesh2PrimitiveType_TriangleList;
		pDstBatch->m_VertexOffset		= 0;
		pDstBatch->m_VertexStart		= srcBatch.VertexStart;
		pDstBatch->m_VertexCount		= srcBatch.VertexCount;
		pDstBatch->m_IndexStart			= srcBatch.FaceStart*3;
		pDstBatch->m_IndexCount			= srcBatch.FaceCount*3;

		int g = 0;
	}

	SAFE_DELETE_ARY(pSrcBatches);
}
