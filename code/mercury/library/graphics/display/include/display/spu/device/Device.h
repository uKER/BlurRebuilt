#ifndef DISPLAY_SPU_DEVICE_H
#define DISPLAY_SPU_DEVICE_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



class C_CommandBuffer;
class C_VertexBuffer;
class C_IndexBuffer;
class C_MemoryLocation;



struct C_DeviceParams
{
	U32		m_CommandBufferAddress;
	U32		m_CommandBufferSize;
	U32		m_MemoryBufferAddress;
	U8 *	m_pScratchBuffer;
};



class C_Device
{

public:

	C_Device();
	~C_Device();

	bool	Create(const C_DeviceParams &deviceParams);

	bool	CreateVertexBuffer(const U32 size, const U32 alignment, C_VertexBuffer * pVertexBuffer);
	bool	CreateIndexBuffer(const U32 size, const U32 alignment, C_IndexBuffer * pIndexBuffer);

	void	SetInvalidateVertexCache();
	void	SetVertexDataArray(const U32 attributeIndex, const E_VertexElementType vertexElementType, const U32 vertexElementCount, const U32 vertexStride, const U32 vertexLocation, const U32 vertexOffset);
	void	SetDrawIndexArray(const E_PrimitiveType primitiveType, const U32 indexCount, const E_Format indexFormat, const U32 indexLocation, const U32 indexOffset);

	void	Finish();

	void	WaitForVideoMemory(const U32 address);

private:

	bool	AllocateMemory(const U32 size, const U32 alignment, C_MemoryLocation * pLocation);

private:

	CellGcmContextData	m_gcmContext;

	U32					m_CommandBufferAddress;
	U32					m_CommandBufferSize;
	U32					m_MemoryBufferAddress;
	U8 *				m_pScratchBuffer;

};



extern C_Device g_Device;

C_Device * Device();



}; // namespace Spu
}; // namespace Display



#include "Display/Spu/Device/Device.inl"



#endif // DISPLAY_SPU_DEVICE_H
