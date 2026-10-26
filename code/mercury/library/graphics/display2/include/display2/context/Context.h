#ifndef DISPLAY2_CONTEXT_H
#define DISPLAY2_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Display2Defines.h"

#if DEVICE_D3D11
#include "Display2/Context/D3D11Context.h" // [Blur1PCDX11] Native context declarations.
#elif DEVICE_GCM
#include "Display2/Context/GCMContext.h"
#endif

namespace Display2
{

//------------------------------------------------------------------------------
// Viewport

//! Gets the viewport x
U32 GetViewportX(const C_Viewport* pViewport);

//! Gets the viewport y
U32 GetViewportY(const C_Viewport* pViewport);

//! Gets the viewport width
U32 GetViewportWidth(const C_Viewport* pViewport);

//! Gets the viewport height
U32 GetViewportHeight(const C_Viewport* pViewport);

//! Gets the viewport min z
F32 GetViewportMinZ(const C_Viewport* pViewport);

//! Gets the viewport max z
F32 GetViewportMaxZ(const C_Viewport* pViewport);

//! Sets the viewport x
void SetViewportX(C_Viewport* pViewport, const U32 x);

//! Sets the viewport y
void SetViewportY(C_Viewport* pViewport, const U32 y);

//! Sets the viewport width
void SetViewportWidth(C_Viewport* pViewport, const U32 width);

//! Sets the viewport height
void SetViewportHeight(C_Viewport* pViewport, const U32 height);

//! Sets the viewport min z
void SetViewportMinZ(C_Viewport* pViewport, const F32 minZ);

//! Sets the viewport max z
void SetViewportMaxZ(C_Viewport* pViewport, const F32 maxZ);

//------------------------------------------------------------------------------
// Vertex element

//! Gets the vertex element stream
U16 GetVertexElementStream(const C_VertexElement* pVertexElement);

//! Gets the vertex element offset
U32 GetVertexElementOffset(const C_VertexElement* pVertexElement);

//! Gets the vertex element type
U32 GetVertexElementType(const C_VertexElement* pVertexElement);

//! Gets the vertex element usage
U8 GetVertexElementUsage(const C_VertexElement* pVertexElement);

//! Gets the vertex element usage index
U8 GetVertexElementUsageIndex(const C_VertexElement* pVertexElement);

//! Sets the vertex element stream
void SetVertexElementStream(C_VertexElement* pVertexElement, const U16 stream);

//! Sets the vertex element offset
void SetVertexElementOffset(C_VertexElement* pVertexElement, const U32 offset);

//! Sets the vertex element type
void SetVertexElementType(C_VertexElement* pVertexElement, const U32 type);

//! Sets the vertex element usage
void SetVertexElementUsage(C_VertexElement* pVertexElement, const U8 usage);

//! Sets the vertex element usage index
void SetVertexElementUsageIndex(C_VertexElement* pVertexElement, const U8 usageIndex);

//! Gets the vertex element size
U32 GetVertexElementSize(const C_VertexElement* pVertexElement);

//! Gets the vertex element count
U32 GetVertexElementCount(const C_VertexElement* pVertexElement);

//------------------------------------------------------------------------------
// Vertex declaration

//! Is the vertex declaration created
bool IsVertexDeclarationCreated(const C_VertexDeclaration* pVertexDeclaration);

//! Gets the vertex declaration element count
S32 GetVertexDeclarationElementCount(const C_VertexDeclaration* pVertexDeclaration);

// TODO: Need a cross-platform get vertex declaration elements if possible!

//! Gets the vertex declaration stride
U32 GetVertexDeclarationStride(const C_VertexDeclaration* pVertexDeclaration, const U32 stream = 0);

//------------------------------------------------------------------------------
// Vertex buffer

//! Is the vertex buffer created
bool IsVertexBufferCreated(const C_VertexBuffer* pVertexBuffer);

//! Gets the vertex buffer size
U32 GetVertexBufferSize(const C_VertexBuffer* pVertexBuffer);

//! Gets the vertex buffer flags
U32 GetVertexBufferFlags(const C_VertexBuffer* pVertexBuffer);

//! Locks the vertex buffer
void* LockVertexBuffer(C_VertexBuffer* pVertexBuffer, const U32 offset = 0, const U32 size = 0, const U32 flags = 0);

//! Unlocks the vertex buffer
void UnlockVertexBuffer(C_VertexBuffer* pVertexBuffer);

//------------------------------------------------------------------------------
// Index buffer

//! Is the index buffer created
bool IsIndexBufferCreated(const C_IndexBuffer* pIndexBuffer);

//! Gets the index buffer size
U32 GetIndexBufferSize(const C_IndexBuffer* pIndexBuffer);

//! Gets the index buffer flags
U32 GetIndexBufferFlags(const C_IndexBuffer* pIndexBuffer);

//! Gets the index buffer format
E_IndexFormat GetIndexBufferFormat(const C_IndexBuffer* pIndexBuffer);

//! Locks the index buffer
void* LockIndexBuffer(C_IndexBuffer* pIndexBuffer, const U32 offset = 0, const U32 size = 0, const U32 flags = 0);

//! Unlocks the index buffer
void UnlockIndexBuffer(C_IndexBuffer* pIndexBuffer);

//------------------------------------------------------------------------------
// Vertex shader

//! Is the vertex shader created
bool IsVertexShaderCreated(const C_VertexShader* pVertexShader);

//! Gets the vertex shader size
U32 GetVertexShaderSize(const C_VertexShader* pVertexShader);

//! Gets the vertex shader flags
U32 GetVertexShaderFlags(const C_VertexShader* pVertexShader);

//! Gets the vertex shader parameter given its name
C_VertexShaderParam GetVertexShaderParamFromName(C_VertexShader* pVertexShader, const char* name);

//------------------------------------------------------------------------------
// Pixel shader

//! Is the pixel shader created
bool IsPixelShaderCreated(const C_PixelShader* pPixelShader);

//! Gets the pixel shader size
U32 GetPixelShaderSize(const C_PixelShader* pPixelShader);

//! Gets the pixel shader flags
U32 GetPixelShaderFlags(const C_PixelShader* pPixelShader);

//! Gets the pixel shader parameter given its name
C_PixelShaderParam GetPixelShaderParamFromName(C_PixelShader* pPixelShader, const char* name);

//------------------------------------------------------------------------------
// Surface

//! Gets the surface pitch from width
U32 GetSurfacePitchFromWidth(U32 width, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample);

//! Gets the surface size from height and pitch
U32 GetSurfaceSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample);

//! Gets the surface size
U32 GetSurfaceSize(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample);

//! Gets the surface size and pitch
void GetSurfaceSizeAndPitch(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample, U32& size, U32& pitch);

//! Is the surface created
bool IsSurfaceCreated(const C_Surface* pSurface);

//! Gets the surface size
U32 GetSurfaceSize(const C_Surface* pSurface);

//! Gets the surface width
U32 GetSurfaceWidth(const C_Surface* pSurface);

//! Gets the surface height
U32 GetSurfaceHeight(const C_Surface* pSurface);

//! Gets the surface format
E_SurfaceFormat GetSurfaceFormat(const C_Surface* pSurface);

//! Gets the surface multisample
E_Multisample GetSurfaceMultisample(const C_Surface* pSurface);

//! Locks the surface rect
C_LockedRect LockSurfaceRect(C_Surface* pSurface, const U32 flags = 0);

//! Unlocks the surface rect
void UnlockSurfaceRect(C_Surface* pSurface);

//------------------------------------------------------------------------------
// Texture

//! Gets the texture pitch from width
U32 GetTexturePitchFromWidth(U32 width, const U32 flags, const E_TextureFormat format);

//! Gets the texture size from height and pitch
U32 GetTextureSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_TextureFormat format);

//! Gets the texture pitch
U32 GetTexturePitch(U32 width, const U32 flags, const E_TextureFormat format);

//! Gets the texture row pitch
U32 GetTextureRowPitch(U32 width, const U32 flags, const E_TextureFormat format);

//! Gets the texture slice pitch
U32 GetTextureSlicePitch(U32 width, U32 height, U32 levels, const U32 flags, const E_TextureFormat format);

//! Gets the texture 2d size
U32 GetTexture2DSize(const U32 width, const U32 height, const U32 flags, const E_TextureFormat format);

//! Gets the texture 2d size
U32 GetTexture2DSize(U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format);

//! Gets the texture 2d size and pitch
void GetTexture2DSizeAndPitch(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch);

//! Gets the texture 3d size
U32 GetTexture3DSize(const U32 width, const U32 height, const U32 depth, const U32 flags, const E_TextureFormat format);

//! Gets the texture 3d size
U32 GetTexture3DSize(U32 width, U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format);

//! Gets the texture 3d size and pitch
void GetTexture3DSizeAndPitch(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch);

//! Gets the texture cube size
U32 GetTextureCubeSize(const U32 width, const U32 flags, const E_TextureFormat format);

//! Gets the texture cube size
U32 GetTextureCubeSize(U32 width, const U32 levels, const U32 flags, const E_TextureFormat format);

//! Gets the texture cube size and pitch
void GetTextureCubeSizeAndPitch(const U32 width, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch);

//! Gets the texture 2d level offset
U32 GetTexture2DLevelOffset(const U32 level, U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format);

//! Gets the texture 3d level offset
U32 GetTexture3DLevelOffset(const U32 level, U32 width, U32 height, U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format);

//! Gets the texture cube level offset
U32 GetTextureCubeLevelOffset(const E_CubeMapFace face, const U32 level, U32 width, const U32 levels, const U32 flags, const E_TextureFormat format);

#if PLATFORM_PS3

U32 SetTexture2DHeader(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture2D* pTexture);

U32 SetTexture3DHeader(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture3D* pTexture);

U32 SetTextureCubeHeader(const U32 width, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTextureCube* pTexture);

#endif

//! Is the texture created
bool IsTextureCreated(const C_Texture* pTexture);

//! Gets the texture type
E_TextureType GetTextureType(const C_Texture* pTexture);

//! Gets the texture size
U32 GetTextureSize(const C_Texture* pTexture);

//! Gets the texture flags
U32 GetTextureFlags(const C_Texture* pTexture);

//! Gets the texture width
U32 GetTextureWidth(const C_Texture* pTexture, const U32 level = 0);

//! Gets the texture height
U32 GetTextureHeight(const C_Texture* pTexture, const U32 level = 0);

//! Gets the texture depth
U32 GetTextureDepth(const C_Texture* pTexture, const U32 level = 0);

//! Gets the texture levels
U32 GetTextureLevels(const C_Texture* pTexture);

//! Gets the texture format
E_TextureFormat GetTextureFormat(const C_Texture* pTexture);

//! Locks the texture 2d
C_LockedRect LockTextureRect2D(C_Texture* pTexture, const U32 level, const U32 flags = 0);

//! Unlocks the texture 2d
void UnlockTextureRect2D(C_Texture* pTexture, const U32 level);

//! Locks the texture 3d
C_LockedBox LockTextureBox3D(C_Texture* pTexture, const U32 level, const U32 flags = 0);

//! Unlocks the texture 3d
void UnlockTextureBox3D(C_Texture* pTexture, const U32 level);

//! Locks the texture cube
C_LockedRect LockTextureRectCube(C_Texture* pTexture, const E_CubeMapFace face, const U32 level, const U32 flags = 0);

//! Unlocks the texture cube
void UnlockTextureRectCube(C_Texture* pTexture, const E_CubeMapFace face, const U32 level);

//------------------------------------------------------------------------------
// Vertex format

//! C_VertexFormat
class C_VertexFormat
{
public:

	C_VertexFormat();
	~C_VertexFormat();
	
	U32 GetStride(const U32 stream = UINT_MAX) const;

	void AddElement(const U16 stream, const U32 type, const U8 usage, const U8 usageIndex);
	void AddElement(const U16 stream, const U32 offset, const U32 type, const U8 usage, const U8 usageIndex);

public:

	S32					m_ElementCount;
	static const S32	m_MaxElementCount = C_VertexDeclaration::m_MaxElementCount;
	C_VertexElement		m_Elements[m_MaxElementCount];
};

} // namespace Display2

#include "Display2/Context/Context.inl"

#endif // DISPLAY2_CONTEXT_H
