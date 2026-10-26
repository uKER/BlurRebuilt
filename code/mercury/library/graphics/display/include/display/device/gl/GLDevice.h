#ifndef DISPLAY_GLDEVICE_H
#define DISPLAY_GLDEVICE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceBase.h"
#include "Display/Device/GL/GL.h"
#include "Display/Device/GL/GLDeviceParams.h"
#include "Display/Device/GL/GLDeviceBuffers.h"
#include "Display/Surface/GL/GLSurface.h"
#include "Display/Texture/GL/GLTexture2D.h"
#include "Display/Texture/GL/GLTexture3D.h"
#include "Display/Texture/GL/GLTextureCube.h"
#include "Display/Texture/GL/GLRenderTexture2D.h"
#include "Display/Buffer/GL/GLVertexDeclaration.h"
#include "Display/Buffer/GL/GLVertexBuffer.h"
#include "Display/Buffer/GL/GLIndexBuffer.h"
#include "Display/Shader/GL/GLVertexShader.h"
#include "Display/Shader/GL/GLPixelShader.h"
#include "Display/Buffer/DynamicVertexBuffer.h"
#include "Display/Memory/MemoryTracker.h"
#include "btl/containers/set.h"
#include "btl/allocators/pool.h"

namespace NewDisplay
{
namespace GL
{

class C_RenderTexture2D;

class C_Sampler : public C_SamplerBase
{
public:

	C_Sampler();
	~C_Sampler();

	void InvalidateState();

public:

    bool				m_Enabled;
	const C_Texture*	m_pTexture;

	// Border colour
	U32					m_BorderColour;
	U32					m_BorderColourDefault;
};

class C_Stream
{
public:

	C_Stream();
	~C_Stream();

public:

	const C_VertexBuffer*	m_pVertexBuffer;
	U32						m_VertexStride;
	U32						m_VertexOffset;
};

class C_GLDevice : public C_DeviceBase, public C_MemoryTracker
{
public:

	C_GLDevice();
	~C_GLDevice();

	/**************************************************************************************************
	- Public cross-platform functions.
	- Safe to use by game as these functions are guaranteed to be cross-platform.
	- Basically anything here must be the interface.
	**************************************************************************************************/
	
	void DefaultStates();
	bool BeginScene();
	void EndScene();
	void Reset();
	void Clear(const U32 flags);
	void Clear(const U32 flags, const Maths::RGBA colour);
	void Clear(const U32 flags, const Maths::RGBA colour, const F32 depth, const U8 stencil);
	
	// Primitive drawing
	void DrawPrimitive(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount);
	void DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 startIndex, const U32 indexCount);
	void DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount);

	// State setting
	void SetAlphaEnable(const E_Bool alphaEnable);
	void SetAlphaFunc(const E_CmpFunc alphaFunc);
	void SetAlphaRef(const U32 alphaRef);
	void SetBlendDst(const E_Blend blendDst);
    void SetBlendEnable(const E_Bool blendEnable);
    void SetBlendMode(const E_BlendMode blendMode);
	void SetBlendOp(const E_BlendOp blendOp);
	void SetBlendSrc(const E_Blend blendSrc);
	void SetColourArray(const bool enable);
	void SetColourWriteEnable(const U32 colourWriteEnable);
    void SetCullMode(E_CullMode cullMode);
	void SetDepthBias(const F32 depthBias);
	void SetDepthEnable(E_Bool depthEnable);
	void SetDepthFlip(const bool depthCmpFlip, const bool depthViewportFlip = true);
	void SetDepthFunc(const E_CmpFunc depthFunc);
	void SetDepthWriteEnable(const E_Bool depthWriteEnable);
	void SetFillMode(E_FillMode fillMode);
	void SetIndexBuffer(const C_IndexBuffer* pIndexBuffer);
    void SetNormalArray(const bool enable);
	void SetPixelShader(const C_PixelShader* pPixelShader);
	void SetPresentInterval(E_PresentInterval presentInterval);
	void SetSlopeScaledBias(const F32 slopeScaledBias);
	void SetTexcoordArray(const U32 sampler, const bool enable);
	void SetTexture(const U32 sampler, const C_Texture* pTexture);
	void SetTextureAddressU(const U32 sampler, const E_TextureAddress addressU);
	void SetTextureAddressV(const U32 sampler, const E_TextureAddress addressV);
	void SetTextureAddressW(const U32 sampler, const E_TextureAddress addressW);
	void SetTextureAddressUVW(const U32 sampler, const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW);
	void SetTextureFilterMag(const U32 sampler, const E_TextureFilter filterMag);
	void SetTextureFilterMin(const U32 sampler, const E_TextureFilter filterMin);
	void SetTextureFilterMip(const U32 sampler, const E_TextureFilter filterMip);
	void SetTextureFilterMagMinMip(const U32 sampler, const E_TextureFilter filterMag, E_TextureFilter filterMin, E_TextureFilter filterMip);
	void SetTextureGamma(const U32 sampler, const E_Bool gamma);
	void SetTextureBorderColour(const U32 sampler, const U32 borderColour);
	void SetVertexArray(const bool enable);
    void SetVertexBuffer(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride);
	void SetVertexDeclaration(const C_VertexDeclaration* pVertexDeclaration);
	void SetVertexShader(const C_VertexShader* pVertexShader);
	void SetViewport(const C_Viewport& viewport);
	
	// Dynmaic vertex buffer management
	void* BeginVertices(const E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, C_VertexDeclaration* pVertexDeclaration);
	void EndVertices();

	// Surface management
	void SetColourSurface();
	void SetDepthStencilSurface();
	void SetColourSurface(const C_Surface* pSurface);
	void SetColourSurface(const U32 index, const C_Surface* pSurface);
	void SetDepthStencilSurface(const C_Surface* pSurface);
	bool CreateColourSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface);
	bool CreateDepthStencilSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface);
	void DestroyColourSurface(C_Surface* pSurface);
	void DestroyDepthStencilSurface(C_Surface* pSurface);
	void DestroySurface(C_Surface* pSurface);

	// Texture 2d management
	bool CreateTexture2D(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_Format format, C_Texture2D* pTexture);
	void DestroyTexture2D(C_Texture2D* pTexture);

	// Texture 3d management
	bool CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_Format format, C_Texture3D* pTexture);
	void DestroyTexture3D(C_Texture3D* pTexture);

	// Texture cube management
	bool CreateTextureCube(const U32 width, const U32 levels, const U32 flags, const E_Format format, C_TextureCube* pCubeTexture);
	void DestroyTextureCube(C_TextureCube* pTexture);

	// Render texture 2d management
	bool CreateRenderTexture2D(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTexture2D* pTexture);
	void DestroyRenderTexture2D(C_RenderTexture2D* pTexture);

	// Vertex buffer management
	bool CreateVertexBuffer(const U32 size, U32 flags, C_VertexBuffer* pVertexBuffer, bool allowFail = false);
	void DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer);

	// Index buffer management
	bool CreateIndexBuffer(const U32 size, const U32 flags, const E_Format format, C_IndexBuffer* pIndexBuffer, bool allowFail = false);
	void DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer);

	// Vertex shader management
	bool CreateVertexShader(const U32 size, const S32* pData, const U32 flags, C_VertexShader* pVertexShader);
	void DestroyVertexShader(C_VertexShader* pVertexShader);

	// Pixel shader management
	bool CreatePixelShader(const U32 size, const S32* pData, const U32 flags, C_PixelShader* pPixelShader);
	void DestroyPixelShader(C_PixelShader* pPixelShader);

	// View management
	void SetupView(const S32 id, const C_ViewSetup& viewSetup);
	void SetView(const S32 id);

	// Memory management
	U32 GetFreeResourceMemory();

	/**************************************************************************************************
	- Public platform specific functions.
	- Not safe to use by game as these functions are not guaranteed to be cross-platform.
	- Wrap functions calls with define GRAPHICS_GL.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	const C_BackBuffer& GetBackBuffer() const;
	const C_RenderTarget& GetRenderTarget() const;

	// Cg management
	CGprofile GetCGProfileVS();
	CGprofile GetCGProfilePS();
	CGcontext GetCGContext();
	CGparameter GetShaderParameter(CGprogram cgProgram, const char* name);
	void SetVertexShaderParameter(CGparameter cgParameter, F32* pValue);
	void SetPixelShaderParameter(const C_PixelShader* pPixelShader, CGparameter cgParameter, F32* pValue);

protected:

	/**************************************************************************************************
	- Protected cross-platform functions.
	- Basically anything here must be the interface.
	**************************************************************************************************/

	// Platform specific device management
	bool CreateDevice(C_DeviceParams& deviceParams);
	bool CreateDeviceVolatile(C_DeviceParams& deviceParams);
	void DestroyDeviceVolatile();
	void DestroyDevice();

	// Vertex declaration management
	C_VertexDeclaration* InitVertexDeclaration(const S32 numVertexElements, const C_VertexElement* vertexElements);
	
	/**************************************************************************************************
	- Protected platform specific functions.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	// TODO: Add functions here.
	
private:

	/**************************************************************************************************
	- Private cross-platform functions.
	- Basically anything here must be the interface.
	**************************************************************************************************/

	// TODO: Add functions here.

	/**************************************************************************************************
	- Private platform specific functions.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	// CG management
	void DestroyCG();

	// Called before every draw call - is there a better way of doing this?
	void SetVertexDeclaration();

public:

	Btl::Set<C_Surface* Btl::DynamicPool<> >				m_Surfaces;
	Btl::Set<C_Texture2D* Btl::DynamicPool<> >				m_Texture2Ds;
	Btl::Set<C_Texture3D* Btl::DynamicPool<> >				m_Texture3Ds;
	Btl::Set<C_TextureCube* Btl::DynamicPool<> >			m_TextureCubes;
	Btl::Set<C_VertexBuffer* Btl::DynamicPool<> >			m_VertexBuffers;
	Btl::Set<C_IndexBuffer* Btl::DynamicPool<> >			m_IndexBuffers;
	Btl::Set<C_VertexShader* Btl::DynamicPool<> >			m_VertexShaders;
	Btl::Set<C_PixelShader* Btl::DynamicPool<> >			m_PixelShaders;
	Btl::Set<C_RenderTexture2D* Btl::DynamicPool<> >		m_RenderTexture2Ds;

	template<typename T>
	struct SetType
	{
		typedef typename Btl::Set<T, Btl::DynamicPool<> >::Iterator Iterator;
	};

protected:

	bool								m_ColourArrayEnable;
	bool								m_NormalArrayEnable;
	F32									m_SlopeScaledBias;
	bool								m_TexcoordArrayEnable[E_Sampler_Count];
	bool								m_VertexArrayEnable;
	E_PresentInterval					m_PresentInterval;
	E_PresentInterval					m_PresentIntervalDefault;
	
	const C_VertexShader*				m_pVertexShader;
	const C_PixelShader*				m_pPixelShader;
	C_Sampler							m_Sampler[E_Sampler_Count];
	const C_VertexDeclaration*			m_pVertexDeclaration;
	C_Stream							m_Stream[E_Stream_Count];
	const C_IndexBuffer*				m_pIndexBuffer;
	C_DynamicVertexBuffer				m_DynamicVertexBuffer;
	U32									m_CurrentProfileVS;
	U32									m_CurrentProfilePS;
	U32									m_LatestProfileVS;
	U32									m_LatestProfilePS;
	CGprofile							m_cgLatestProfileVS;
	CGprofile							m_cgLatestProfilePS;
	CGcontext							m_cgContext;
	
	C_BackBuffer						m_BackBuffer;
	C_RenderTarget						m_RenderTarget;

	Ut::UtVector<Ut::DynamicVector<C_VertexDeclaration*> > m_VertexDeclarations;

private:

	bool								m_SetVertexDeclaration;
};

#define BUFFER_OFFSET(i) ((char*)NULL + (i)) 

#include "GLDevice.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLDEVICE_H
