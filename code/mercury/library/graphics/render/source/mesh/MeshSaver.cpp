#include "stdafx.h"
#include "Render/Mesh/MeshSaver.h"
#include "Render/Mesh/MeshBuilder.h"
#include "Render/Mesh/MeshTypes.h"
#include <System/Serialise.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Render
{
namespace Mesh
{

void SaveMesh(const Util::C_String fileName, const C_MeshBuilder& meshBuilder)
{
#if 0
	Sys::C_FilePath filePath = Sys::GetDefaultFileSystem()->GetWorkingDirectory() + "/" + fileName;

	#if defined(PLATFORM_PC)
	filePath += ".pcm";
	#elif defined(PLATFORM_PS3)
	filePath += ".p3m";

	#endif
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(filePath, Sys::E_FileAccess_Write);
	if (!pFile)
	{
		LOGWARN(g_Log, "Failed to open file '%s'\n", fileName.Chars());
		return;
	}

	// file version...

	E_FileVersion fileVersion = E_FileVersion_Current;
	pFile->Write(&fileVersion, sizeof(E_FileVersion));

	// vertex declarations...

	S32 vertexDeclarationCount = meshBuilder.m_VertexDeclarations.Size();
	pFile->Write(&vertexDeclarationCount, sizeof(S32));
	for (S32 vertexDeclarationIndex=0; vertexDeclarationIndex<vertexDeclarationCount; vertexDeclarationIndex++)
	{
		const Display::C_VertexDeclaration* pVertexDeclaration = meshBuilder.m_VertexDeclarations[vertexDeclarationIndex];

		S32 vertexElementCount = pVertexDeclaration->m_ElementCount;
		pFile->Write(&vertexElementCount, sizeof(S32));
		for (S32 vertexElementIndex=0; vertexElementIndex<vertexElementCount; vertexElementIndex++)
		{
			const Display::C_VertexElement& vertexElement = pVertexDeclaration->m_ElementArray[vertexElementIndex];

			pFile->Write(&vertexElement.m_Stream, sizeof(U32));
			pFile->Write(&vertexElement.m_Offset, sizeof(U32));
			
			E_VertexElementType vertexElementType = E_VertexElementType_Unknown;
			switch (vertexElement.m_Type)
			{
			case Display::E_VertexElementType_F32_1:
				vertexElementType = E_VertexElementType_F32_1;
				break;
			case Display::E_VertexElementType_F32_2:
				vertexElementType = E_VertexElementType_F32_2;
				break;
			case Display::E_VertexElementType_F32_3:
				vertexElementType = E_VertexElementType_F32_3;
				break;
			case Display::E_VertexElementType_F32_4:
				vertexElementType = E_VertexElementType_F32_4;
				break;
			case Display::E_VertexElementType_F16_2:
				vertexElementType = E_VertexElementType_F16_2;
				break;
			case Display::E_VertexElementType_U8_4N:
				vertexElementType = E_VertexElementType_U8_4N;
				break;
			case Display::E_VertexElementType_U10_3N:
				vertexElementType = E_VertexElementType_U10_3N;
				break;
			case Display::E_VertexElementType_U10_4N:
				vertexElementType = E_VertexElementType_U10_4N;
				break;
			default:
				LOGERROR(g_Log, "Unsupported vertex element type %i\n", vertexElement.m_Type);
				break;
			}
			pFile->Write(&vertexElementType, sizeof(E_VertexElementType));
			
			E_VertexElementUsage vertexElementUsage = E_VertexElementUsage_Unknown;
			switch (vertexElement.m_Usage)
			{
			case Display::E_VertexElementUsage_Position:
				vertexElementUsage = E_VertexElementUsage_Position;
				break;
			case Display::E_VertexElementUsage_Normal:
				vertexElementUsage = E_VertexElementUsage_Normal;
				break;
			case Display::E_VertexElementUsage_TexCoord:
				vertexElementUsage = E_VertexElementUsage_TexCoord;
				break;
			case Display::E_VertexElementUsage_Tangent:
				vertexElementUsage = E_VertexElementUsage_Tangent;
				break;
			case Display::E_VertexElementUsage_Binormal:
				vertexElementUsage = E_VertexElementUsage_Binormal;
				break;
			case Display::E_VertexElementUsage_Colour:
				vertexElementUsage = E_VertexElementUsage_Colour;
				break;
			default:
				LOGERROR(g_Log, "Unsupported vertex element usage %i\n", vertexElement.m_Usage);
				break;
			}
			pFile->Write(&vertexElementUsage, sizeof(E_VertexElementUsage));
			
			pFile->Write(&vertexElement.m_UsageIndex, sizeof(U32));
		}
	}

	// vertex buffers...

	S32 vertexBufferCount = meshBuilder.m_VertexBuffers.Size();
	pFile->Write(&vertexBufferCount, sizeof(S32));
	for (S32 vertexBufferIndex=0; vertexBufferIndex<vertexBufferCount; vertexBufferIndex++)
	{
		Display::C_VertexBuffer* pVertexBuffer = meshBuilder.m_VertexBuffers[vertexBufferIndex];

		U32 size = pVertexBuffer->GetSize();
		assert(size > 0);
		pFile->Write(&size, sizeof(U32));

		void* pVertex = pVertexBuffer->Lock();
		assert(pVertex);
		pFile->Write(pVertex, size);
		pVertexBuffer->Unlock();
	}

	// index buffers...

	S32 indexBufferCount = meshBuilder.m_IndexBuffers.Size();
	pFile->Write(&indexBufferCount, sizeof(S32));
	for (S32 indexBufferIndex=0; indexBufferIndex<indexBufferCount; indexBufferIndex++)
	{
		Display::C_IndexBuffer* pIndexBuffer = meshBuilder.m_IndexBuffers[indexBufferIndex];

		U32 size = pIndexBuffer->GetSize();
		assert(size > 0);
		pFile->Write(&size, sizeof(U32));

		Display::E_Format format = pIndexBuffer->GetFormat();
		pFile->Write(&format, sizeof(Display::E_Format));

		void* pIndex = pIndexBuffer->Lock();
		assert(pIndex);
		pFile->Write(pIndex, size);
		pIndexBuffer->Unlock();
	}

	// materials...

	S32 materialCount = meshBuilder.m_Materials.Size();
	pFile->Write(&materialCount, sizeof(S32));
	for (S32 materialIndex=0; materialIndex<materialCount; materialIndex++)
	{
		const C_MeshBuilderMaterial& material = meshBuilder.m_Materials[materialIndex];

		pFile->Write(&material.m_VertexStride, sizeof(U32));
		pFile->Write(&material.m_VertexDeclaration, sizeof(S32));
	}

	// batches...

	S32 batchCount = meshBuilder.m_Batches.Size();
	pFile->Write(&batchCount, sizeof(S32));
	for (S32 batchIndex=0; batchIndex<batchCount; batchIndex++)
	{
		const C_MeshBuilderBatch& batch = meshBuilder.m_Batches[batchIndex];

		pFile->Write(&batch.m_Material, sizeof(S32));
		pFile->Write(&batch.m_VertexBuffer, sizeof(S32));
		pFile->Write(&batch.m_IndexBuffer, sizeof(S32));

		E_PrimitiveType primitiveType = E_PrimitiveType_Unknown;
		switch (batch.m_PrimitiveType)
		{
		case Display::E_PrimitiveType_TriangleList:
			primitiveType = E_PrimitiveType_TriangleList;
			break;
		case Display::E_PrimitiveType_TriangleStrip:
			primitiveType = E_PrimitiveType_TriangleStrip;
			break;
		}
		pFile->Write(&primitiveType, sizeof(E_PrimitiveType));

		pFile->Write(&batch.m_BaseVertex, sizeof(S32));
		pFile->Write(&batch.m_StartVertex, sizeof(S32));
		pFile->Write(&batch.m_VertexCount, sizeof(S32));
		pFile->Write(&batch.m_StartIndex, sizeof(S32));
		pFile->Write(&batch.m_IndexCount, sizeof(S32));
	}

	SAFE_DELETE_PTR(pFile);
#endif
}

}; // namespace Mesh
}; // namespace Render
