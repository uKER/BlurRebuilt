#include "stdafx.h"
#include "Mesh2/Mesh/Mesh2Utils.h"
#include "Mesh2/Mesh/Mesh2Types.h"
#include <Display2/Utils/Utils.h>
#include <System/Serialise.h>

namespace Render
{

void* EndianSwap16(U16* data, S32 count)
{
	for(S32 i=0; i<count; i++)
	{
		unsigned char* pElement = (unsigned char*)data;
		Ut::Swap(pElement[0], pElement[1]);
		data++;
	}

	return data;
}

void* EndianSwap32(U32* data, S32 count)
{
	for(S32 i=0; i<count; i++ )
	{
		unsigned char* pElement = (unsigned char*)data;
		Ut::Swap(pElement[0], pElement[3]);
		Ut::Swap(pElement[1], pElement[2]);
		data++;
	}

	return data;
}

void* EndianSwap64(U64* data, S32 count)
{
	for(S32 i=0; i<count; i++ )
	{
		unsigned char* pElement = (unsigned char*)data;
		Ut::Swap(pElement[0], pElement[7]);
		Ut::Swap(pElement[1], pElement[6]);
		Ut::Swap(pElement[2], pElement[5]);
		Ut::Swap(pElement[3], pElement[4]);
		data++;
	}

	return data;
}

Display2::E_VertexElementType ToDisplayVertexElementType(const U32 vertexElementType)
{
	switch(vertexElementType)
	{
	case E_Mesh2VertexElementType_F32_1:			return Display2::E_VertexElementType_F32_1;
	case E_Mesh2VertexElementType_F32_2:			return Display2::E_VertexElementType_F32_2;
	case E_Mesh2VertexElementType_F32_3:			return Display2::E_VertexElementType_F32_3;
	case E_Mesh2VertexElementType_F32_4:			return Display2::E_VertexElementType_F32_4;
	case E_Mesh2VertexElementType_Colour:			return Display2::E_VertexElementType_Colour;
	case E_Mesh2VertexElementType_U8_4:			return Display2::E_VertexElementType_U8_4;
	case E_Mesh2VertexElementType_U8_4N:			return Display2::E_VertexElementType_U8_4N;
	case E_Mesh2VertexElementType_F16_2:			return Display2::E_VertexElementType_F16_2;
	case E_Mesh2VertexElementType_F16_4:			return Display2::E_VertexElementType_F16_4;
	default:
		assert(0 && "Unsupported vertex element type");
		return Display2::E_VertexElementType_Unknown;
	}
}

Display2::E_VertexElementUsage ToDisplayVertexElementUsage(const U32 vertexElementUsage)
{
	switch(vertexElementUsage)
	{
	case E_Mesh2VertexElementUsage_Position:		return Display2::E_VertexElementUsage_Position;
	case E_Mesh2VertexElementUsage_BlendWeight:	return Display2::E_VertexElementUsage_BlendWeight;
	case E_Mesh2VertexElementUsage_BlendIndices:	return Display2::E_VertexElementUsage_BlendIndices;
	case E_Mesh2VertexElementUsage_Normal:			return Display2::E_VertexElementUsage_Normal;
	case E_Mesh2VertexElementUsage_PSize:			return Display2::E_VertexElementUsage_PSize;
	case E_Mesh2VertexElementUsage_TexCoord:		return Display2::E_VertexElementUsage_TexCoord;
	case E_Mesh2VertexElementUsage_Tangent:		return Display2::E_VertexElementUsage_Tangent;
	case E_Mesh2VertexElementUsage_Binormal:		return Display2::E_VertexElementUsage_Binormal;
	case E_Mesh2VertexElementUsage_Colour:			return Display2::E_VertexElementUsage_Colour;
	case E_Mesh2VertexElementUsage_Fog:			return Display2::E_VertexElementUsage_Fog;
	default:
		assert(0 && "Unsupported vertex element usage");
		return Display2::E_VertexElementUsage_Unknown;
	}
}

Display2::E_IndexFormat ToDisplayIndexFormat(const U32 indexFormat)
{
	switch(indexFormat)
	{
	case E_Mesh2IndexFormat_16:					return Display2::E_IndexFormat_16;
	case E_Mesh2IndexFormat_32:					return Display2::E_IndexFormat_32;
	default:
		assert(0 && "Unsupported index format");
		return Display2::E_IndexFormat_Unknown;
	}
}

Display2::E_PrimitiveType ToDisplayPrimitiveType(const U32 primitiveType)
{
	switch(primitiveType)
	{
	case E_Mesh2PrimitiveType_TriangleList:		return Display2::E_PrimitiveType_TriangleList;
	default:
		assert(0 && "Unsupported primitive type");
		return Display2::E_PrimitiveType_Unknown;
	}
}

bool LoadMesh2(C_Mesh2* pMesh, const Util::C_String& filePath, const Util::C_String& fileRoot, Display2::C_Device* pDevice)
{
	BIZ_ASSERT(pMesh);
	BIZ_ASSERT(pDevice);

	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(filePath);
	if(!pFile)
	{
		LOGWARN(Display2::g_Log, "Failed to open file '%s'\n", filePath.Chars());
		return false;
	}

	#if PLATFORM_PS3
	Sys::BinaryDeserialiser<Ut::DoEndianSwap> ar(*pFile);
	#else
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);
	#endif

	U32 filePlatform;
	ar >> filePlatform;

	U32 fileVersion;
	ar >> fileVersion;

	// Load vertex declarations

	S32 vertexDeclarationCount = 0;
	ar >> vertexDeclarationCount;
	pMesh->m_VertexDeclarationList.Resize(vertexDeclarationCount);
	for(S32 iVertexDeclaration=0; iVertexDeclaration<vertexDeclarationCount; iVertexDeclaration++)
	{
		Display2::C_VertexDeclaration& vertexDeclaration = pMesh->m_VertexDeclarationList[iVertexDeclaration];

		Display2::C_VertexFormat vertexFormat;

		S32 vertexElementCount = 0;
		ar >> vertexElementCount;
		for(S32 iVertexElement=0; iVertexElement<vertexElementCount; iVertexElement++)
		{
			U8 stream = 0;
			ar >> stream;

			U32 offset = 0;
			ar >> offset;

			U8 type = 0;
			ar >> type;
			Display2::E_VertexElementType eType = ToDisplayVertexElementType(type);

			U8 usage = 0;
			ar >> usage;
			Display2::E_VertexElementUsage eUsage = ToDisplayVertexElementUsage(usage);

			U8 usageIndex = 0;
			ar >> usageIndex;

			vertexFormat.AddElement(stream, offset, eType, eUsage, usageIndex);
		}

		if(!pDevice->CreateVertexDeclaration(vertexFormat, &vertexDeclaration))
			return false;
	}
	
	// Load vertex buffers

	S32 vertexBufferCount = 0;
	ar >> vertexBufferCount;
	pMesh->m_VertexBufferList.Resize(vertexBufferCount);
	for(S32 iVertexBuffer=0; iVertexBuffer<vertexBufferCount; iVertexBuffer++)
	{
		Display2::C_VertexBuffer& vertexBuffer = pMesh->m_VertexBufferList[iVertexBuffer];

		U32 size;
		ar >> size;
		if(size > 0)
		{
			if(!pDevice->CreateVertexBuffer(size, 0, &vertexBuffer))
				return false;

			void* pVertices = vertexBuffer.Lock();
			assert(pVertices);
			ar.Read(pVertices, sizeof(U8)*size);
			#if PLATFORM_PS3
			EndianSwap32((U32*)pVertices, size/sizeof(F32));
			#endif
			vertexBuffer.Unlock();
		}
		else
		{
			LOGWARN(Display2::g_Log, "Vertex buffer %i size is 0\n", iVertexBuffer);
		}
	}

	// Load index buffers

	S32 indexBufferCount = 0;
	ar >> indexBufferCount;
	pMesh->m_IndexBufferList.Resize(indexBufferCount);
	for(S32 iIndexBuffer=0; iIndexBuffer<indexBufferCount; iIndexBuffer++)
	{
		Display2::C_IndexBuffer& indexBuffer = pMesh->m_IndexBufferList[iIndexBuffer];

		U32 format = 0;
		ar >> format;
		Display2::E_IndexFormat eFormat = ToDisplayIndexFormat(format);

		U32 size = 0;
		ar >> size;
		if(size > 0)
		{
			if(!pDevice->CreateIndexBuffer(size, 0, eFormat, &indexBuffer))
				return false;

			void* pIndices = indexBuffer.Lock();
			assert(pIndices);
			ar.Read(pIndices, sizeof(U8)*size);
			#if PLATFORM_PS3
			EndianSwap16((U16*)pIndices, size/sizeof(U16));
			#endif
			indexBuffer.Unlock();
		}
		else
		{
			LOGWARN(Display2::g_Log, "Index buffer %i size is 0\n", iIndexBuffer);
		}
	}

	// Load textures

	S32 textureCount = 0;
	ar >> textureCount;
	pMesh->m_TextureList.Resize(textureCount);
	for(S32 iTexture=0; iTexture<textureCount; iTexture++)
	{
		Display2::C_Texture& texture = pMesh->m_TextureList[iTexture];

		S32 strSize = 0;
		ar >> strSize;
		char* strData = new char[strSize+1];
		ar.Read(strData, sizeof(char)*strSize);
		strData[strSize] = '\0';

#if PLATFORM_PS3
		U32 width = 512;
		U32 height = 512;
		U32 levels = Display2::GetTexture2DLevels(width, height);
		U32 flags = 0;
		Display2::E_TextureFormat format = Display2::E_TextureFormat_A8R8G8B8;
		if(!Display2::Device()->CreateTexture2D(width, height, levels, flags, format, &texture))
			return false;
		for(U32 level=0; level<levels; level++)
		{
			Display2::C_LockedRect lockedRect = texture.LockRect2D(level);
			if(lockedRect.GetBits())
			{
				U32 size = Display2::GetTextureSizeFromHeightAndPitch(height, lockedRect.GetPitch(), flags, format);

				U32 colour;
				switch(level)
				{
				case 0: colour = 0xFFFF0000; break;
				case 1: colour = 0xFF00FF00; break;
				case 2: colour = 0xFF0000FF; break;
				case 3: colour = 0xFFFFFF00; break;
				case 4: colour = 0xFF00FFFF; break;
				case 5: colour = 0xFF000000; break;
				default: colour = 0xFFFFFFFF; break;
				}

				U32* pTemp = (U32*)lockedRect.GetBits();
				for(U32 i=0; i<(size/sizeof(U32)); i++)
					pTemp[i] = colour;
			}
			texture.UnlockRect2D(level);

			if(width > 1)
				width /= 2;
			if(height > 1)
				height /= 2;
		}
#else
		const Util::C_String textureFileName = strData;
		const Util::C_String textureFilePath = fileRoot + textureFileName;
		if(!Display2::CreateTexture2DFromFile(textureFilePath, &texture))
			return false;
#endif
	}

	// Load materials

	S32 materialCount = 0;
	ar >> materialCount;
	pMesh->m_MaterialList.Resize(materialCount);
	for(S32 iMaterial=0; iMaterial<materialCount; iMaterial++)
	{
		C_Mesh2Material& material = pMesh->m_MaterialList[iMaterial];

		S32 strSize = 0;
		ar  >> strSize;
		char* strData = new char[strSize+1];
		ar.Read(strData, sizeof(char)*strSize);
		strData[strSize] = '\0';
		Util::C_String name = strData;
		
		S32 samplerCount = 0;
		ar >> samplerCount;
		material.m_SamplerList.Resize(samplerCount);
		for(S32 iSampler=0; iSampler<samplerCount; iSampler++)
		{
			C_Mesh2Sampler& sampler = material.m_SamplerList[iSampler];

			ar >> sampler.m_Number;
			ar >> sampler.m_Texture;
		}
	}

	// Load batches

	S32 batchCount = 0;
	ar >> batchCount;
	pMesh->m_BatchList.Resize(batchCount);
	for(S32 iBatch=0; iBatch<batchCount; iBatch++)
	{
		C_Mesh2Batch& batch = pMesh->m_BatchList[iBatch];

		ar >> batch.m_VertexDeclaration;
		ar >> batch.m_VertexBuffer;
		ar >> batch.m_IndexBuffer;
		ar >> batch.m_Material;

		//batch.m_Material = -1; // HACK: Til we load the textures!

		U32 primitiveType = 0;
		ar >> primitiveType;
		batch.m_PrimitiveType = ToDisplayPrimitiveType(primitiveType);

		ar >> batch.m_VertexOffset;
		ar >> batch.m_VertexStart;
		ar >> batch.m_VertexCount;
		ar >> batch.m_IndexStart;
		ar >> batch.m_IndexCount;
	}
	
	SAFE_DELETE_PTR(pFile);

	return true;
}

void DestroyMesh2(C_Mesh2* pMesh, Display2::C_Device* pDevice)
{
	BIZ_ASSERT(pMesh);
	BIZ_ASSERT(pDevice);

	for(S32 i=0; i<pMesh->m_VertexDeclarationList.Size(); i++)
		pDevice->Destroy(&pMesh->m_VertexDeclarationList[i]);
	pMesh->m_VertexDeclarationList.Clear();

	for(S32 i=0; i<pMesh->m_VertexBufferList.Size(); i++)
		pDevice->Destroy(&pMesh->m_VertexBufferList[i]);
	pMesh->m_VertexBufferList.Clear();

	for(S32 i=0; i<pMesh->m_IndexBufferList.Size(); i++)
		pDevice->Destroy(&pMesh->m_IndexBufferList[i]);
	pMesh->m_IndexBufferList.Clear();

	for(S32 i=0; i<pMesh->m_TextureList.Size(); i++)
		pDevice->Destroy(&pMesh->m_TextureList[i]);
	pMesh->m_TextureList.Clear();
}

Util::C_String GetMesh2FilePath(const Util::C_String& fileName)
{
	#if PLATFORM_PS3
	const Util::C_String filePath = "Meshes/" + fileName + ".p3msh";
	#elif PLATFORM_PC
	const Util::C_String filePath = "Meshes/" + fileName + ".pcmsh";
	#else
	const Util::C_String filePath = "";
	#endif

	return filePath;

}

} // namespace Render
