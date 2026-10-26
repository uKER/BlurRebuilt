#ifndef DISPLAY_SPU_DEVICE_INL
#define DISPLAY_SPU_DEVICE_INL

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{


	
inline void C_Device::SetInvalidateVertexCache()
{
	cellGcmSetInvalidateVertexCacheUnsafeInline(&m_gcmContext);
}

inline void C_Device::SetVertexDataArray(const U32 attributeIndex, const E_VertexElementType vertexElementType, const U32 vertexElementCount, const U32 vertexStride, const U32 vertexLocation, const U32 vertexOffset)
{
	cellGcmSetVertexDataArrayUnsafeInline(&m_gcmContext, attributeIndex, 0, vertexStride, vertexElementCount, vertexElementType, vertexLocation, vertexOffset);
}

inline void C_Device::SetDrawIndexArray(const E_PrimitiveType primitiveType, const U32 indexCount, const E_Format indexFormat, const U32 indexLocation, const U32 indexOffset)
{
	cellGcmSetDrawIndexArrayUnsafeInline(&m_gcmContext, primitiveType, indexCount, indexFormat, indexLocation, indexOffset);
}



inline C_Device * Device()
{
	return &g_Device;
}



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_DEVICE_INL
