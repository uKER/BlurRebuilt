#ifndef MSH_FILE_H
#define MSH_FILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Mesh2/Mesh/Mesh2Types.h>

struct C_VertexElement
{
	C_VertexElement();

	bool Save(Sys::I_File* pFile);

	U8										m_Stream;
	U32										m_Offset;
	U8										m_Type;
	U8										m_Usage;
	U8										m_UsageIndex;
};

struct C_VertexDeclaration
{
	C_VertexDeclaration();

	void AddElement(const U8 stream, const U32 offset, const U8 type, const U8 usage, const U8 usageIndex);

	bool Save(Sys::I_File* pFile);

	S32										m_ElementCount;
	static const S32						m_MaxElements = 12;
	C_VertexElement							m_Elements[m_MaxElements];
};

struct C_VertexBuffer
{
	~C_VertexBuffer();

	bool Save(Sys::I_File* pFile);

	Btl::Vector<U8>							m_Vertices;
};

struct C_IndexBuffer
{
	C_IndexBuffer();
	~C_IndexBuffer();

	bool Save(Sys::I_File* pFile);

	U32										m_Format;
	Btl::Vector<U8>							m_Indices;
};

struct C_Texture
{
	C_Texture(const Util::C_String& name);

	bool Save(Sys::I_File* pFile);

	Util::C_String							m_Name;
};

struct C_Sampler
{
	C_Sampler();

	bool Save(Sys::I_File* pFile);

	U32										m_Number;
	S32										m_Texture;
};

struct C_Material
{
	C_Material(const Util::C_String& name);
	~C_Material();

	bool Save(Sys::I_File* pFile);

	C_Sampler* AddSampler();

	Util::C_String							m_Name;
	Btl::Vector<C_Sampler*>					m_SamplerList;
};

struct C_Batch
{
	C_Batch();

	bool Save(Sys::I_File* pFile);

	S32										m_VertexDeclaration;
	S32										m_VertexBuffer;
	S32										m_IndexBuffer;
	S32										m_Material;

	U32										m_PrimitiveType;
	U32										m_VertexOffset;
	U32										m_VertexStart;
	U32										m_VertexCount;
	U32										m_IndexStart;
	U32										m_IndexCount;
};

struct C_Mesh
{
	~C_Mesh();

	S32 FindTexture(const Util::C_String& name);
	S32 FindMaterial(const Util::C_String& name);

	void AddVertexDeclaration(S32& iVertexDeclaration, C_VertexDeclaration** ppVertexDeclaration);
	void AddVertexBuffer(S32& iVertexBuffer, C_VertexBuffer** ppVertexBuffer);
	void AddIndexBuffer(S32& iIndexBuffer, C_IndexBuffer** ppIndexBuffer);
	bool AddTexture(const Util::C_String& name, S32& iTexture, C_Texture** ppTexture);
	bool AddMaterial(const Util::C_String& name, S32& iMaterial, C_Material** ppMaterial);

	C_Batch* AddBatch();

	bool Save(Sys::I_File* pFile);
	bool Save(const Util::C_String& filePath, const Render::E_Mesh2FilePlatform filePlatform);
		
	Btl::Vector<C_VertexDeclaration*>		m_VertexDeclarationList;
	Btl::Vector<C_VertexBuffer*>			m_VertexBufferList;
	Btl::Vector<C_IndexBuffer*>				m_IndexBufferList;
	Btl::Vector<C_Texture*>					m_TextureList;
	Btl::Vector<C_Material*>				m_MaterialList;
	Btl::Vector<C_Batch*>					m_BatchList;
};

#endif // MSH_FILE_H
