#ifndef DISPLAY2_WII_DEVICE_INL
#define DISPLAY2_WII_DEVICE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

// C_Device

INLINE U32 C_Device::IsCreated() const
{
	return false;
}

INLINE C_Context& C_Device::GetContext()
{
	return m_Context;
}

INLINE const C_Context& C_Device::GetContext() const
{
	return m_Context;
}

INLINE C_ContextNoCheck& C_Device::GetContextNoCheck()
{
	return m_ContextNoCheck;
}

INLINE const C_ContextNoCheck& C_Device::GetContextNoCheck() const
{
	return m_ContextNoCheck;
}

INLINE C_ContextNoCheckNoTrack& C_Device::GetContextNoCheckNoTrack()
{
	return m_ContextNoCheckNoTrack;
}

INLINE const C_ContextNoCheckNoTrack& C_Device::GetContextNoCheckNoTrack() const
{
	return m_ContextNoCheckNoTrack;
}

INLINE S32 C_Device::GetVertexDeclarationCount() const
{
	return 0;
}

INLINE C_VertexDeclaration* C_Device::GetVertexDeclaration(const S32 index)
{
	return NULL;
}

INLINE void C_Device::Destroy(C_VertexDeclaration* pVertexDeclaration)
{
	DestroyVertexDeclaration(pVertexDeclaration);
}

INLINE void C_Device::Destroy(C_VertexBuffer* pVertexBuffer)
{
	DestroyVertexBuffer(pVertexBuffer);
}

INLINE void C_Device::Destroy(C_IndexBuffer* pIndexBuffer)
{
	DestroyIndexBuffer(pIndexBuffer);
}

INLINE void C_Device::Destroy(C_VertexShader* pVertexShader)
{
	DestroyVertexShader(pVertexShader);
}

INLINE void C_Device::Destroy(C_PixelShader* pPixelShader)
{
	DestroyPixelShader(pPixelShader);
}

INLINE void C_Device::Destroy(C_Surface* pSurface)
{
	DestroySurface(pSurface);
}

INLINE void C_Device::Destroy(C_Texture* pTexture)
{
	DestroyTexture(pTexture);
}

// C_DeviceManager

INLINE bool C_DeviceManager::IsCreated()
{
	return false;
}

} // namespace Display2

#endif // DISPLAY2_WII_DEVICE_INL
