// [Blur1PCDX11] Native Display2 device declarations.
#ifndef DISPLAY2_PCD3D11_DEVICE_H
#define DISPLAY2_PCD3D11_DEVICE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Device/D3D11Device.h"
#include "Display2/Memory/Memory.h"

namespace Display2
{

//! C_DeviceParams

class C_DeviceParams : public C_D3D11DeviceParams
{
public:

	C_DeviceParams();
	~C_DeviceParams();

public:
};

//! C_Device

class C_Device : public C_D3D11Device
{
public:

	C_Device();
	~C_Device();

	void GetParams(C_DeviceParams& params);
	void SetParams(const C_DeviceParams& params);

	U32 IsCreated() const;
	bool Create();
	bool CreateVolatile();
	void DestroyVolatile();
	void Destroy();

	C_Context& GetContext();
	const C_Context& GetContext() const;
	
	C_ContextNoCheck& GetContextNoCheck();
	const C_ContextNoCheck& GetContextNoCheck() const;
	
	C_ContextNoCheckNoTrack& GetContextNoCheckNoTrack();
	const C_ContextNoCheckNoTrack& GetContextNoCheckNoTrack() const;

	void Destroy(C_VertexDeclaration* pVertexDeclaration);
	
	bool CreateVertexBuffer(U32 size, U32 flags, C_VertexBuffer* pVertexBuffer, void* pMemory = NULL);
	void DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer);
	void Destroy(C_VertexBuffer* pVertexBuffer);

	bool CreateIndexBuffer(U32 size, U32 flags, E_IndexFormat format, C_IndexBuffer* pIndexBuffer, void* pMemory = NULL);
	void DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer);
	void Destroy(C_IndexBuffer* pIndexBuffer);

	bool CreateVertexShader(U32 size, const U8* pData, U32 flags, C_VertexShader* pVertexShader, void* pMemory = NULL);
	void DestroyVertexShader(C_VertexShader* pVertexShader);
	void Destroy(C_VertexShader* pVertexShader);

	bool CreatePixelShader(U32 size, const U8* pData, U32 flags, C_PixelShader* pPixelShader, void* pMemory = NULL);
	void DestroyPixelShader(C_PixelShader* pPixelShader);
	void Destroy(C_PixelShader* pPixelShader);

	bool CreateColourSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pMemory = NULL, const U32 memorySize = 0);
	bool CreateDepthStencilSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pMemory = NULL, const U32 memorySize = 0);
	void DestroySurface(C_Surface* pSurface);
	void Destroy(C_Surface* pSurface);

	bool CreateTexture2D(U32 width, U32 height, U32 levels, U32 flags, E_TextureFormat format, C_Texture* pTexture, void* pMemory = NULL, const U32 memorySize = 0);
	bool CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pMemory = NULL, const U32 memorySize = 0);
	bool CreateTextureCube(const U32 width, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pMemory = NULL, const U32 memorySize = 0);
	void DestroyTexture(C_Texture* pTexture);
	void Destroy(C_Texture* pTexture);

	const C_AlphaTestState& GetAlphaTestState() const;
	const C_BlendState& GetBlendState() const;
	const C_ColourState& GetColourState() const;
	const C_CullState& GetCullState() const;
	const C_DepthState& GetDepthState() const;
	const C_FillState& GetFillState() const;
	const C_SamplerState& GetSamplerState(const U32 index) const;
	const C_ScissorState& GetScissorState() const;
	const C_StencilState& GetStencilState() const;
	const C_VertexSamplerState& GetVertexSamplerState(const U32 index) const;

	void SetAlphaTestState(const C_AlphaTestState& state);
	void SetBlendState(const C_BlendState& state);
	void SetColourState(const C_ColourState& state);
	void SetCullState(const C_CullState& state);
	void SetDepthState(const C_DepthState& state);
	void SetFillState(const C_FillState& state);
	void SetSamplerState(const U32 index, const C_SamplerState& state);
	void SetScissorState(const C_ScissorState& state);
	void SetStencilState(const C_StencilState& state);
	void SetVertexSamplerState(const U32 index, const C_VertexSamplerState& state);

	void SetCullMode(const E_CullMode cullMode);
	void SetFillMode(const E_FillMode fillMode);
	void SetIndexBuffer(const C_IndexBuffer* pIndexBuffer);
	void SetPixelShader(const C_PixelShader* pPixelShader);
	void SetPixelShaderParamF(const C_PixelShaderParam& shaderParam, const F32* pData, const U32 registerCount = 1);
	void SetTexture(const U32 index, const C_Texture* pTexture);
	void SetVertexBuffer(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride);
	void SetVertexDeclaration(const C_VertexDeclaration* pVertexDeclaration);
	void SetVertexShader(const C_VertexShader* pVertexShader);
	void SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);
	void SetVertexShaderParamF(const C_VertexShaderParam& shaderParam, const F32* pData);
	
	void DrawPrimitive(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount);
	void DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount);

	void BlockUntilIdle();

private:

	bool CreateSurface(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pMemory, const U32 memorySize = 0);

private:

	C_Context								m_Context;
	C_ContextNoCheck						m_ContextNoCheck;
	C_ContextNoCheckNoTrack					m_ContextNoCheckNoTrack;
	C_ContextState							m_ContextState;
};

//! C_DeviceManagerParams

class C_DeviceManagerParams : public C_D3D11DeviceManagerParams
{
public:
	
	C_DeviceManagerParams();
	~C_DeviceManagerParams();
};

//! C_DeviceManager

class C_DeviceManager : public C_D3D11DeviceManager
{
public:

	C_DeviceManager();
	~C_DeviceManager();

	void GetParams(C_DeviceManagerParams& params);
	void SetParams(const C_DeviceManagerParams& params);

	bool Create(const C_DeviceManagerParams& params);
	void Destroy();

	void SetupDefaultDeviceParams(C_DeviceParams& params);
};

} // namespace Display2

// [Blur1PCDX11] Native methods are defined in the D3D11 backend.

#endif // DISPLAY2_PCD3D11_DEVICE_H
