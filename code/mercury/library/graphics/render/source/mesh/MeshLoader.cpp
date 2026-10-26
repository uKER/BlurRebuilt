#include "stdafx.h"
#include "Render/Mesh/MeshLoader.h"
#include "Render/Mesh/MeshTypes.h"
#include "Render/Mesh/Mesh.h"
#include <System/Serialise.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Render
{
namespace Mesh
{
	
bool LoadMesh(const Util::C_String fileName, C_Mesh* pMesh, const E_VertexShader vertexShader, const E_PixelShader pixelShader)
{
#if 0
	assert(pMesh);

	Sys::C_FilePath filePath = Sys::GetDefaultFileSystem()->GetWorkingDirectory() + "/" + fileName;

	#if defined(PLATFORM_PC)
	filePath += ".pcm";
	#elif defined(PLATFORM_PS3)
	filePath += ".p3m";
	#endif
	
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(filePath, Sys::E_FileAccess_Read);
	if (!pFile)
	{
		LOGWARN(g_Log, "Failed to open file '%s'\n", fileName.Chars());
		return false;
	}

	// file version...

	E_FileVersion fileVersion;
	pFile->Read(&fileVersion, sizeof(E_FileVersion));

	// vertex declarations...

	S32 vertexDeclarationCount;
	pFile->Read(&vertexDeclarationCount, sizeof(S32));
	pMesh->SetVertexDeclarationCount(vertexDeclarationCount);
	for (S32 vertexDeclarationIndex=0; vertexDeclarationIndex<vertexDeclarationCount; vertexDeclarationIndex++)
	{
		Display::C_VertexFormat vertexFormat;

		S32 vertexElementCount;
		pFile->Read(&vertexElementCount, sizeof(S32));
		for (S32 vertexElementIndex=0; vertexElementIndex<vertexElementCount; vertexElementIndex++)
		{
			U32 stream;
			pFile->Read(&stream, sizeof(U32));

			U32 offset;
			pFile->Read(&offset, sizeof(U32));
			
			E_VertexElementType vertexElementType;
			pFile->Read(&vertexElementType, sizeof(E_VertexElementType));
			Display::E_VertexElementType type = Display::E_VertexElementType_Unknown;
			switch (vertexElementType)
			{
			case E_VertexElementType_F32_1:
				type = Display::E_VertexElementType_F32_1;
				break;
			case E_VertexElementType_F32_2:
				type = Display::E_VertexElementType_F32_2;
				break;
			case E_VertexElementType_F32_3:
				type = Display::E_VertexElementType_F32_3;
				break;
			case E_VertexElementType_F32_4:
				type = Display::E_VertexElementType_F32_4;
				break;
			case E_VertexElementType_F16_2:
				type = Display::E_VertexElementType_F16_2;
				break;
			case E_VertexElementType_U8_4N:
				type = Display::E_VertexElementType_U8_4N;
				break;
			case E_VertexElementType_U10_3N:
				type = Display::E_VertexElementType_U10_3N;
				break;
			case E_VertexElementType_U10_4N:
				type = Display::E_VertexElementType_U10_4N;
				break;
			}
			
			E_VertexElementUsage vertexElementUsage;
			pFile->Read(&vertexElementUsage, sizeof(E_VertexElementUsage));
			Display::E_VertexElementUsage usage = Display::E_VertexElementUsage_Unknown;
			switch (vertexElementUsage)
			{
			case E_VertexElementUsage_Position:
				usage = Display::E_VertexElementUsage_Position;
				break;
			case E_VertexElementUsage_Normal:
				usage = Display::E_VertexElementUsage_Normal;
				break;
			case E_VertexElementUsage_TexCoord:
				usage = Display::E_VertexElementUsage_TexCoord;
				break;
			case E_VertexElementUsage_Tangent:
				usage = Display::E_VertexElementUsage_Tangent;
				break;
			case E_VertexElementUsage_Binormal:
				usage = Display::E_VertexElementUsage_Binormal;
				break;
			case E_VertexElementUsage_Colour:
				usage = Display::E_VertexElementUsage_Colour;
				break;
			}
			
			U32 usageIndex;
			pFile->Read(&usageIndex, sizeof(U32));

			vertexFormat.AddElement(stream, offset, type, usage, usageIndex);
		}

		pMesh->SetVertexDeclaration(vertexDeclarationIndex, vertexFormat);
	}

	// vertex buffers...

	S32 vertexBufferCount;
	pFile->Read(&vertexBufferCount, sizeof(S32));
	pMesh->SetVertexBufferCount(vertexBufferCount);
	for (S32 vertexBufferIndex=0; vertexBufferIndex<vertexBufferCount; vertexBufferIndex++)
	{
		Display::C_VertexBuffer* pVertexBuffer = pMesh->GetVertexBuffer(vertexBufferIndex);

		U32 size;
		pFile->Read(&size, sizeof(U32));
		assert(size > 0);

		if (!Display::Device()->CreateVertexBuffer(size, 0, pVertexBuffer))
		{
			SAFE_DELETE_PTR(pFile);
			LOGFATAL(g_Log, "Failed to create the mesh vertex buffer of size '%i'\n", size);
			return false;
		}

		void* pVertex = pVertexBuffer->Lock();
		assert(pVertex);
		pFile->Read(pVertex, size);
		pVertexBuffer->Unlock();
	}

	// index buffers...

	S32 indexBufferCount;
	pFile->Read(&indexBufferCount, sizeof(S32));
	pMesh->SetIndexBufferCount(indexBufferCount);
	for (S32 indexBufferIndex=0; indexBufferIndex<indexBufferCount; indexBufferIndex++)
	{
		Display::C_IndexBuffer* pIndexBuffer = pMesh->GetIndexBuffer(indexBufferIndex);

		U32 size;
		pFile->Read(&size, sizeof(U32));
		assert(size > 0);

		Display::E_Format format;
		pFile->Read(&format, sizeof(Display::E_Format));

		if (!Display::Device()->CreateIndexBuffer(size, 0, Display::E_Format_Index32, pIndexBuffer)) // TODO: [LloydW] Save index format away properly.
		//if (!Display::Device()->CreateIndexBuffer(size, 0, format, pIndexBuffer))
		{
			SAFE_DELETE_PTR(pFile);
			LOGFATAL(g_Log, "Failed to create the mesh index buffer of size '%i'\n", size);
			return false;
		}

		void* pIndex = pIndexBuffer->Lock();
		assert(pIndex);
		pFile->Read(pIndex, size);
		pIndexBuffer->Unlock();
	}

	// materials...

	S32 materialCount;
	pFile->Read(&materialCount, sizeof(S32));
	pMesh->SetMaterialCount(materialCount);
	for (S32 materialIndex=0; materialIndex<materialCount; materialIndex++)
	{
		C_Material* pMaterial = pMesh->GetMaterial(materialIndex);

		pFile->Read(&pMaterial->m_VertexStride, sizeof(U32));
		pFile->Read(&pMaterial->m_VertexDeclaration, sizeof(S32));

		pMaterial->m_pVertexShader = &pMesh->m_Shaders.m_VertexShader[vertexShader];
		pMaterial->m_pPixelShader = &pMesh->m_Shaders.m_PixelShader[pixelShader];
	}

	// batches...

	S32 batchCount;
	pFile->Read(&batchCount, sizeof(S32));
	pMesh->SetBatchCount(batchCount);
	for (S32 batchIndex=0; batchIndex<batchCount; batchIndex++)
	{
		C_Batch* pBatch = pMesh->GetBatch(batchIndex);

		pFile->Read(&pBatch->m_Material, sizeof(S32));
		pFile->Read(&pBatch->m_VertexBuffer, sizeof(S32));
		pFile->Read(&pBatch->m_IndexBuffer, sizeof(S32));

		E_PrimitiveType primitiveType;
		pFile->Read(&primitiveType, sizeof(E_PrimitiveType));
		switch (primitiveType)
		{
		case E_PrimitiveType_TriangleList:
			pBatch->m_PrimitiveType = Display::E_PrimitiveType_TriangleList;
			break;
		case E_PrimitiveType_TriangleStrip:
			pBatch->m_PrimitiveType = Display::E_PrimitiveType_TriangleStrip;
			break;
		}

		pFile->Read(&pBatch->m_BaseVertex, sizeof(S32));
		pFile->Read(&pBatch->m_StartVertex, sizeof(S32));
		pFile->Read(&pBatch->m_VertexCount, sizeof(S32));
		pFile->Read(&pBatch->m_StartIndex, sizeof(S32));
		pFile->Read(&pBatch->m_IndexCount, sizeof(S32));
		int g = 0;
	}

	SAFE_DELETE_PTR(pFile);
#endif

	return true;
}

}; // namespace Mesh
}; // namespace Render
