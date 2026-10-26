#include "StdAfx.h"
#include "MSH.h"
#include <Mesh2/Mesh/Mesh2Types.h>

C_VertexElement::C_VertexElement() :
	m_Stream(0),
	m_Offset(0),
	m_Type(0),
	m_Usage(0),
	m_UsageIndex(0)
{
}

bool C_VertexElement::Save(Sys::I_File* pFile)
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_Stream;
	ar << m_Offset;
	ar << m_Type;
	ar << m_Usage;
	ar << m_UsageIndex;

	return true;
}

C_VertexDeclaration::C_VertexDeclaration() :
	m_ElementCount(0)
{
}

void C_VertexDeclaration::AddElement(const U8 stream, const U32 offset, const U8 type, const U8 usage, const U8 usageIndex)
{
	assert(m_ElementCount < m_MaxElements);

	m_Elements[m_ElementCount].m_Stream			= stream;
	m_Elements[m_ElementCount].m_Offset			= offset;
	m_Elements[m_ElementCount].m_Type			= type;
	m_Elements[m_ElementCount].m_Usage			= usage;
	m_Elements[m_ElementCount].m_UsageIndex		= usageIndex;

	m_ElementCount++;
}

bool C_VertexDeclaration::Save(Sys::I_File* pFile)
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_ElementCount;
	for(S32 i=0; i<m_ElementCount; i++)
		m_Elements[i].Save(pFile);
	
	return true;
}

C_VertexBuffer::~C_VertexBuffer()
{
	m_Vertices.Clear();
}

bool C_VertexBuffer::Save(Sys::I_File* pFile)
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_Vertices.Size();
	for(S32 i=0; i<m_Vertices.Size(); i++)
		ar << m_Vertices[i];

	return true;
}

C_IndexBuffer::C_IndexBuffer() :
	m_Format(Render::E_Mesh2IndexFormat_16)
{
}

C_IndexBuffer::~C_IndexBuffer()
{
	m_Indices.Clear();
}

bool C_IndexBuffer::Save(Sys::I_File* pFile)
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_Format;

	ar << m_Indices.Size();
	for(S32 i=0; i<m_Indices.Size(); i++)
		ar << m_Indices[i];
	
	return true;
}

C_Texture::C_Texture(const Util::C_String& name) :
	m_Name(name)
{
}

bool C_Texture::Save(Sys::I_File* pFile)
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_Name;

	return true;
}

C_Sampler::C_Sampler() :
	m_Number(0),
	m_Texture(-1)
{
}

bool C_Sampler::Save(Sys::I_File* pFile)
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);
	
	ar << m_Number;
	ar << m_Texture;

	return true;
}

C_Material::C_Material(const Util::C_String& name) :
	m_Name(name)
{
}

C_Material::~C_Material()
{
	for(S32 i=0; i<m_SamplerList.Size(); i++)
		SAFE_DELETE_PTR(m_SamplerList[i]);
	m_SamplerList.Clear();
}

C_Sampler* C_Material::AddSampler()
{
	C_Sampler* pSampler = new C_Sampler();
	m_SamplerList.PushBack(pSampler);
	return pSampler;
}

bool C_Material::Save(Sys::I_File* pFile)
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_Name;

	ar << m_SamplerList.Size();
	for(S32 i=0; i<m_SamplerList.Size(); i++)
		m_SamplerList[i]->Save(pFile);

	return true;
}

C_Batch::C_Batch() :
	m_VertexDeclaration(-1),
	m_VertexBuffer(-1),
	m_IndexBuffer(-1),
	m_Material(-1),
	m_PrimitiveType(Render::E_Mesh2PrimitiveType_TriangleList),
	m_VertexOffset(0),
	m_VertexStart(0),
	m_VertexCount(0),
	m_IndexStart(0),
	m_IndexCount(0)
{
}

bool C_Batch::Save(Sys::I_File* pFile)
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_VertexDeclaration;
	ar << m_VertexBuffer;
	ar << m_IndexBuffer;
	ar << m_Material;
	ar << m_PrimitiveType;
	ar << m_VertexOffset;
	ar << m_VertexStart;
	ar << m_VertexCount;
	ar << m_IndexStart;
	ar << m_IndexCount;

	return true;
}

C_Mesh::~C_Mesh()
{
	for(S32 i=0; i<m_VertexDeclarationList.Size(); i++)
		SAFE_DELETE_PTR(m_VertexDeclarationList[i]);
	m_VertexDeclarationList.Clear();

	for(S32 i=0; i<m_VertexBufferList.Size(); i++)
		SAFE_DELETE_PTR(m_VertexBufferList[i]);
	m_VertexBufferList.Clear();

	for(S32 i=0; i<m_IndexBufferList.Size(); i++)
		SAFE_DELETE_PTR(m_IndexBufferList[i]);
	m_IndexBufferList.Clear();

	for(S32 i=0; i<m_TextureList.Size(); i++)
		SAFE_DELETE_PTR(m_TextureList[i]);
	m_TextureList.Clear();

	for(S32 i=0; i<m_MaterialList.Size(); i++)
		SAFE_DELETE_PTR(m_MaterialList[i]);
	m_MaterialList.Clear();

	for(S32 i=0; i<m_BatchList.Size(); i++)
		SAFE_DELETE_PTR(m_BatchList[i]);
	m_BatchList.Clear();
}

S32 C_Mesh::FindTexture(const Util::C_String& name)
{
	for(S32 i=0; i<m_TextureList.Size(); i++)
		if(name == m_TextureList[i]->m_Name)
			return i;
	return -1;
}

S32 C_Mesh::FindMaterial(const Util::C_String& name)
{
	for(S32 i=0; i<m_MaterialList.Size(); i++)
		if(name == m_MaterialList[i]->m_Name)
			return i;
	return -1;
}

void C_Mesh::AddVertexDeclaration(S32& iVertexDeclaration, C_VertexDeclaration** ppVertexDeclaration)
{
	iVertexDeclaration = m_VertexDeclarationList.Size();
	*ppVertexDeclaration = new C_VertexDeclaration();
	m_VertexDeclarationList.PushBack(*ppVertexDeclaration);
}

void C_Mesh::AddVertexBuffer(S32& iVertexBuffer, C_VertexBuffer** ppVertexBuffer)
{
	iVertexBuffer = m_VertexBufferList.Size();
	*ppVertexBuffer = new C_VertexBuffer();
	m_VertexBufferList.PushBack(*ppVertexBuffer);
}

void C_Mesh::AddIndexBuffer(S32& iIndexBuffer, C_IndexBuffer** ppIndexBuffer)
{
	iIndexBuffer = m_IndexBufferList.Size();
	*ppIndexBuffer = new C_IndexBuffer();
	m_IndexBufferList.PushBack(*ppIndexBuffer);
}

bool C_Mesh::AddTexture(const Util::C_String& name, S32& iTexture, C_Texture** ppTexture)
{
	iTexture = FindTexture(name);
	if(iTexture != -1)
	{
		*ppTexture = m_TextureList[iTexture];
		return true;
	}
	else
	{
		iTexture = m_TextureList.Size();
		*ppTexture = new C_Texture(name);
		m_TextureList.PushBack(*ppTexture);
		return false;
	}
}

bool C_Mesh::AddMaterial(const Util::C_String& name, S32& iMaterial, C_Material** ppMaterial)
{
	iMaterial = FindMaterial(name);
	if(iMaterial != -1)
	{
		*ppMaterial = m_MaterialList[iMaterial];
		return true;
	}
	else
	{
		iMaterial = m_MaterialList.Size();
		*ppMaterial = new C_Material(name);
		m_MaterialList.PushBack(*ppMaterial);
		return false;
	}
}

C_Batch* C_Mesh::AddBatch()
{
	C_Batch* pBatch = new C_Batch();
	m_BatchList.PushBack(pBatch);
	return pBatch;
}

bool C_Mesh::Save(Sys::I_File* pFile)
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_VertexDeclarationList.Size();
	for(S32 i=0; i<m_VertexDeclarationList.Size(); i++)
		m_VertexDeclarationList[i]->Save(pFile);

	ar << m_VertexBufferList.Size();
	for(S32 i=0; i<m_VertexBufferList.Size(); i++)
		m_VertexBufferList[i]->Save(pFile);
	
	ar << m_IndexBufferList.Size();
	for(S32 i=0; i<m_IndexBufferList.Size(); i++)
		m_IndexBufferList[i]->Save(pFile);

	ar << m_TextureList.Size();
	for(S32 i=0; i<m_TextureList.Size(); i++)
		m_TextureList[i]->Save(pFile);
	
	ar << m_MaterialList.Size();
	for(S32 i=0; i<m_MaterialList.Size(); i++)
		m_MaterialList[i]->Save(pFile);
	
	ar << m_BatchList.Size();
	for(S32 i=0; i<m_BatchList.Size(); i++)
		m_BatchList[i]->Save(pFile);

	return true;
}

bool C_Mesh::Save(const Util::C_String& filePath, const Render::E_Mesh2FilePlatform filePlatform)
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(filePath, Sys::E_FileAccess_Write);
	if(pFile)
	{
		Sys::BinarySerialiserAutoEndian ar(*pFile);

		ar << (U32)filePlatform;

		ar << (U32)Render::E_Mesh2FileVersion_Latest;

		if(Save(pFile))
		{
			SAFE_DELETE_PTR(pFile);
			return true;
		}
		else
		{
			SAFE_DELETE_PTR(pFile);
			LOGERROR(g_Log, "ERROR: Failed to save file '%s'\n",	filePath.Chars());
			return false;
		}

		return true;

	}
	else
	{
		LOGERROR(g_Log, "ERROR: Failed to open file '%s'\n", filePath.Chars());
		return false;
	}
}
