#ifndef DISPLAY_SPU_VERTEX_TABLE_H
#define DISPLAY_SPU_VERTEX_TABLE_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



class C_VertexTable
{
public:

	bool	Create(const U32 count, const U32 stride);
	void	Destroy();

	void	Load(const U32 address, const U32 memoryId = 0);
	void	LoadSync(const U32 address, const U32 memoryId = 0);

public:

	void *	m_pBuffer;
	U32		m_Stride;
	U32		m_Count;

};



class C_VertexBlock : public C_VertexTable
{
public:

	C_VertexBlock();

	bool	Create(const U32 stride, const U32 count);
	bool	Create(const U32 size);

public:

	U32		m_Size;

};


/*
class C_VertexStream
{
public:

	C_VertexStream(const C_VertexBuffer &buffer, const U32 streamAmount = 32);
	~C_VertexStream();

	bool			Prefetch();
	C_VertexBlock *	FetchLast();
	C_VertexBlock *	FetchNext();

public:

	U32				m_MemoryId;

private:

	C_VertexBuffer	m_Buffer;
	U32				m_Offset;

	C_VertexBlock	m_Blocks[2];
	U32				m_Index;

};
*/


}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_VERTEX_TABLE_H
