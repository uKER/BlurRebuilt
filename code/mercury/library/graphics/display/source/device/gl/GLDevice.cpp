#include "stdafx.h"
#include "Display/Device/GL/GLDevice.h"
#include "Display/Texture/Texture2DUtils.h"
#include "Display/Shaders/Constants.h"

namespace NewDisplay
{
namespace GL
{

void CGError()
{
	const char* errorString = cgGetErrorString(cgGetError());
	LOGERROR(g_Log, "%s\n", errorString);
	DebugBreak();
}

C_Sampler::C_Sampler()
{
	m_Enabled				= false;
	m_pTexture				= NULL;
	// Border colour
	m_BorderColour			= 0x00000000;
	m_BorderColourDefault	= 0x00000000;
}

C_Sampler::~C_Sampler()
{
}

void C_Sampler::InvalidateState()
{
	C_SamplerBase::InvalidateState();

	m_pTexture		= NULL;
	// Border colour
	m_BorderColour	= 0x00000000;
}

C_Stream::C_Stream() :
	m_pVertexBuffer(NULL),
	m_VertexStride(0),
	m_VertexOffset(0)
{
}

C_Stream::~C_Stream()
{
}

C_GLDevice::C_GLDevice() : m_SetVertexDeclaration(false)
{
	m_CullModeDefault					= E_CullMode_Cw;
	m_SlopeScaledBias					= 0.f;
	m_PresentInterval					= E_PresentInterval_Unknown;
	m_PresentIntervalDefault			= E_PresentInterval_One;
	m_pVertexDeclaration				= NULL;
	m_pIndexBuffer						= NULL;
	m_pVertexShader						= NULL;
	m_pPixelShader						= NULL;
	m_CurrentProfileVS					= 0;
	m_CurrentProfilePS					= 0;
	m_LatestProfileVS					= 0;
	m_LatestProfilePS					= 0;
	m_cgContext							= NULL;
}

C_GLDevice::~C_GLDevice()
{
	BIZ_ASSERT_MSG(!m_cgContext, "The cg context has not been destroyed");
}

//glViewport(0, 0, m_Width, m_Height);
//glScissor(0, 0, m_Width, m_Height);
//glEnable(GL_CULL_FACE);
//glCullFace(GL_BACK);
//glFrontFace(GL_CCW);
//glDepthMask(true);
//glEnable(GL_DEPTH_TEST);
//glDepthFunc(GL_LEQUAL);
//glDisable(GL_ALPHA_TEST);
//glAlphaFunc(GL_ALWAYS, 0);
//glDisable(GL_BLEND);
//glBlendFunc(GL_ONE, GL_ZERO);
//glBlendEquation(GL_FUNC_ADD);

void C_GLDevice::DefaultStates()
{
	// alpha
	SetAlphaEnable(m_AlphaEnableDefault);
	SetAlphaFunc(m_AlphaFuncDefault);
	SetAlphaRef(m_AlphaRefDefault);
	// blend
	SetBlendEnable(m_BlendEnableDefault);
	SetBlendSrc(m_BlendSrcDefault);
	SetBlendDst(m_BlendDstDefault);
	SetBlendOp(m_BlendOpDefault);
	SetBlendMode(m_BlendModeDefault);
	// colour
	SetColourWriteEnable(m_ColourWriteEnable);
	// cull
	SetCullMode(m_CullModeDefault);
	// depth
	SetDepthEnable(m_DepthEnableDefault);
	SetDepthFunc(m_DepthFuncDefault);
	SetDepthWriteEnable(m_DepthWriteEnableDefault);
	// point sprite
	SetPointSpriteEnable(m_PointSpriteEnableDefault);
	// fill
	SetFillMode(m_FillModeDefault);
}

bool C_GLDevice::CreateDevice(C_DeviceParams& deviceParams)
{
	m_BackBuffer.m_Width = deviceParams.m_Width;
	m_BackBuffer.m_Height = deviceParams.m_Height;
	m_BackBuffer.m_Multisample = deviceParams.m_Multisample;
	m_BackBuffer.m_ColourFormat = deviceParams.m_ColourFormat;
	m_BackBuffer.m_DepthStencilFormat = deviceParams.m_DepthStencilFormat;

	if (!m_DynamicVertexBuffer.Create((U32)(0.5f * 1024.f * 1024.f)))
		return false;

	cgSetErrorCallback(CGError);
	
	return true;
}

bool C_GLDevice::CreateDeviceVolatile(C_DeviceParams& deviceParams)
{
	m_BackBuffer.m_Width = deviceParams.m_Width;
	m_BackBuffer.m_Height = deviceParams.m_Height;
	m_BackBuffer.m_Multisample = deviceParams.m_Multisample;
	m_BackBuffer.m_ColourFormat = deviceParams.m_ColourFormat;
	m_BackBuffer.m_DepthStencilFormat = deviceParams.m_DepthStencilFormat;

	return true;
}

void C_GLDevice::DestroyDeviceVolatile()
{
}

void C_GLDevice::DestroyDevice()
{
	m_DynamicVertexBuffer.Destroy();

	DestroyCG();
}

void C_GLDevice::DestroyCG()
{
	if (m_cgContext)
	{
		cgDestroyContext(m_cgContext);
		m_cgContext = NULL;
	}
}

bool C_GLDevice::BeginScene()
{
	BIZ_ASSERT(!m_InScene);
	m_InScene = true;
	Reset();
	return true;
}

void C_GLDevice::EndScene()
{
	BIZ_ASSERT(m_InScene);
	m_InScene = false;
	Reset();
}

void C_GLDevice::Reset()
{
	SetVertexShader(NULL);
	SetPixelShader(NULL);
	SetVertexDeclaration(NULL);
	SetIndexBuffer(NULL);
	for (U32 sampler = 0; sampler < E_Sampler_Count; sampler++)
		SetTexture(sampler, NULL);
	for (U32 stream = 0; stream < E_Stream_Count; stream++)
		SetVertexBuffer(stream, NULL, 0, 0);
}


void C_GLDevice::Clear(const U32 flags)
{
	Clear(flags, m_ColourClear, m_DepthClear, m_StencilClear);
}

void C_GLDevice::Clear(const U32 flags, const Maths::RGBA colour)
{
	Clear(flags, colour, m_DepthClear, m_StencilClear);
}

void C_GLDevice::Clear(const U32 flags, const Maths::RGBA colour, const F32 depth, const U8 stencil)
{
	if (colour != m_ColourClear)
	{
	#ifdef PLATFORM_PS3
		const F32 a = F32((colour.A() & 0x000000FF)) / 255.f;
		const F32 b = F32((colour.B() & 0x0000FF00) >> 8) / 255.f;
		const F32 g = F32((colour.G() & 0x00FF0000) >> 16) / 255.f;
		const F32 r = F32((colour.R() & 0xFF000000) >> 24) / 255.f;
	#else
		const F32 b = F32((colour.B() & 0x000000FF)) / 255.f;
		const F32 g = F32((colour.G() & 0x0000FF00) >> 8) / 255.f;
		const F32 r = F32((colour.R() & 0x00FF0000) >> 16) / 255.f;
		const F32 a = F32((colour.A() & 0xFF000000) >> 24) / 255.f;
	#endif
		glClearColor(r, g, b, a);
		m_ColourClear = colour;
	}

	if (depth != m_DepthClear)
	{
	#ifdef PLATFORM_PS3
		glClearDepthf(depth);
	#endif
		m_DepthClear = depth;
	}

	if (stencil != m_StencilClear)
	{
		glClearStencil(stencil);
		m_StencilClear = stencil;
	}

	U32 glMask = 0;
	if (flags & E_Clear_Depth)
		glMask |= GL_DEPTH_BUFFER_BIT;
	if (flags & E_Clear_Colour)
		glMask |= GL_COLOR_BUFFER_BIT;
	if (flags & E_Clear_Stencil)
		glMask |= GL_STENCIL_BUFFER_BIT;
	if (glMask)
		glClear(glMask);
}

void* C_GLDevice::BeginVertices(const E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, C_VertexDeclaration* pVertexDeclaration)
{
#ifdef GRAPHICS_GL
	return m_DynamicVertexBuffer.Begin(primitiveType, vertexCount, vertexStride, pVertexDeclaration);
#else
	return NULL;
#endif
}

void C_GLDevice::EndVertices()
{
	m_DynamicVertexBuffer.End();

#ifdef GRAPHICS_GL
	SetVertexBuffer(0, m_DynamicVertexBuffer.GetVertexBuffer(), 0, m_DynamicVertexBuffer.GetVertexStride());
	SetVertexDeclaration(m_DynamicVertexBuffer.GetVertexDeclaration());
#endif

	DrawPrimitive(m_DynamicVertexBuffer.GetPrimitiveType(), 0, m_DynamicVertexBuffer.GetVertexCount());
}

void C_GLDevice::SetColourSurface()
{
	m_RenderTarget.m_pColourSurface = &m_BackBuffer.m_ColourSurface;
#ifdef PLATFORM_PS3
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
#endif
	BIZ_ASSERT(m_pView);
	SetViewport(m_pView->GetViewport());
}

void C_GLDevice::SetDepthStencilSurface()
{
	m_RenderTarget.m_pDepthStencilSurface = &m_BackBuffer.m_DepthStencilSurface;
#ifdef PLATFORM_PS3
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, 0);
#endif
}

void C_GLDevice::SetColourSurface(const U32 index, const C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	m_RenderTarget.m_pColourSurface = pSurface;
#ifdef PLATFORM_PS3
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, pSurface->m_glID);
#endif
	BIZ_ASSERT(m_pView);
	SetViewport(m_pView->GetViewport());
}

void C_GLDevice::SetDepthStencilSurface(const C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	m_RenderTarget.m_pDepthStencilSurface = pSurface;
#ifdef PLATFORM_PS3
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, pSurface->m_glID);
#endif
}

bool C_GLDevice::CreateColourSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	BIZ_ASSERT(!pSurface->IsCreated());
#ifdef PLATFORM_PS3
	glGenFramebuffersOES(1, &pSurface->m_glID);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, pSurface->m_glID);
#endif
	pSurface->SetFlags(flags);
	pSurface->SetSystemID(MemoryGetSystemID());
	MemoryIncSystem(pSurface->GetSystemID(), pSurface->GetSize(), E_MemoryType_Surface);

	pSurface->Acquire();

	return true;
}

void C_GLDevice::DestroyColourSurface(C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	if (pSurface->IsCreated() && pSurface->Release() == 0)
	{
		m_Surfaces.Erase(pSurface);
	
		MemoryDecSystem(pSurface->GetSystemID(), pSurface->GetSize(), E_MemoryType_Surface);
		pSurface->SetSize(0);
		pSurface->SetFlags(0);
		pSurface->SetSystemID(-1);
		
#ifdef PLATFORM_PS3
		glDeleteFramebuffersOES(1, &pSurface->m_glID);
		pSurface->m_glID = 0;
#endif
	}
}

bool C_GLDevice::CreateDepthStencilSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	BIZ_ASSERT(!pSurface->IsCreated());
#ifdef PLATFORM_PS3
	glGenRenderbuffersOES(1, &pSurface->m_glID);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, pSurface->m_glID);
#endif
	pSurface->SetFlags(flags);
	pSurface->SetSystemID(MemoryGetSystemID());
	MemoryIncSystem(pSurface->GetSystemID(), pSurface->GetSize(), E_MemoryType_Surface);

	pSurface->Acquire();

	return true;
}

void C_GLDevice::DestroyDepthStencilSurface(C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	if (pSurface->IsCreated() && pSurface->Release() == 0)
	{
		m_Surfaces.Erase(pSurface);
	
		MemoryDecSystem(pSurface->GetSystemID(), pSurface->GetSize(), E_MemoryType_Surface);
		pSurface->SetSize(0);
		pSurface->SetFlags(0);
		pSurface->SetSystemID(-1);
#ifdef PLATFORM_PS3		
		glDeleteRenderbuffersOES(1, &pSurface->m_glID);
		pSurface->m_glID = 0;
#endif
	}
}

void C_GLDevice::DestroySurface(C_Surface* pSurface)
{
	BIZ_ASSERT_FAIL("Functionality not yet supported in gl");
}

bool C_GLDevice::CreateTexture2D(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_Format format, C_Texture2D* pTexture)
{
	BIZ_ASSERT(pTexture);
	BIZ_ASSERT(!pTexture->IsCreated());

	glGenTextures (1, &pTexture->m_glID);
	GLError();
			
	glBindTexture(GL_TEXTURE_2D, pTexture->m_glID);
	GLError();

	pTexture->SetSize(GetTexture2DSize(width, height, levels, format));
	pTexture->SetSystemID(MemoryGetSystemID());
	MemoryIncSystem(pTexture->GetSystemID(), pTexture->GetSize(), E_MemoryType_Texture2D);
	
	m_Texture2Ds.Insert(pTexture);

	pTexture->Acquire();

	return true;
}

void C_GLDevice::DestroyTexture2D(C_Texture2D* pTexture)
{
	BIZ_ASSERT(pTexture);
	if (pTexture->IsCreated() && pTexture->Release() == 0)
	{
		MemoryDecSystem(pTexture->GetSystemID(), pTexture->GetSize(), E_MemoryType_Texture2D);
		pTexture->SetSize(0);
		pTexture->SetSystemID(-1);
		
		m_Texture2Ds.Erase(pTexture);
		
		glDeleteTextures(1, &pTexture->m_glID);
		GLError();
		pTexture->m_glID = 0;
	}
}

bool C_GLDevice::CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_Format format, C_Texture3D* pTexture)
{
	BIZ_ASSERT_FAIL("Functionality not yet supported in gl");

	return false;
}

void C_GLDevice::DestroyTexture3D(C_Texture3D* pTexture)
{
	BIZ_ASSERT_FAIL("Functionality not yet supported in gl");
}

bool C_GLDevice::CreateTextureCube(const U32 width, const U32 levels, const U32 flags, const E_Format format, C_TextureCube* pTexture)
{
	BIZ_ASSERT(pTexture);
	BIZ_ASSERT(!pTexture->IsCreated());

	glGenTextures (1, &pTexture->m_glID);
	GLError();
			
	glBindTexture(GL_TEXTURE_2D, pTexture->m_glID);
	GLError();

	pTexture->SetSize(0);
	pTexture->SetSystemID(MemoryGetSystemID());
	MemoryIncSystem(pTexture->GetSystemID(), pTexture->GetSize(), E_MemoryType_TextureCube);
	
	m_TextureCubes.Insert(pTexture);

	pTexture->Acquire();

	return true;
}

void C_GLDevice::DestroyTextureCube(C_TextureCube* pTexture)
{
	BIZ_ASSERT(pTexture);
	if (pTexture->IsCreated() && pTexture->Release() == 0)
	{
		MemoryDecSystem(pTexture->GetSystemID(), pTexture->GetSize(), E_MemoryType_TextureCube);
		pTexture->SetSize(0);
		pTexture->SetSystemID(-1);
		
		m_TextureCubes.Erase(pTexture);
		
		glDeleteTextures(1, &pTexture->m_glID);
		GLError();
		pTexture->m_glID = 0;
	}
}

bool C_GLDevice::CreateRenderTexture2D(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTexture2D* pTexture)
{
	// TODO: Proper ref counting for render texture 2d render buffers.

	BIZ_ASSERT(pTexture);
	if (CreateTexture2D(width, height, levels, flags, format, &pTexture->m_Texture))
	{
#ifdef PLATFORM_PS3
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_ALLOCATION_HINT_SCE, GL_TEXTURE_TILED_GPU_SCE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ARGB_SCE, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		glGenFramebuffersOES(1, &pTexture->m_Surface.m_glID);
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, pTexture->m_Surface.m_glID);
		glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pTexture->m_Texture.m_glID, 0);
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0); // Resore default frame buffer as new one is bound at this point.
#endif
		return true;
	}
	else
	{
		return false;
	}
}

void C_GLDevice::DestroyRenderTexture2D(C_RenderTexture2D* pTexture)
{
	// TODO: Proper ref counting for render texture 2d render buffers.

	BIZ_ASSERT(pTexture);
	if (pTexture->m_Surface.m_glID)
	{
#ifdef PLATFORM_PS3
		glDeleteRenderbuffersOES(1, &pTexture->m_Surface.m_glID);
		pTexture->m_Surface.m_glID = 0;
#endif
	}
	DestroyTexture2D(&pTexture->m_Texture);
}

C_VertexDeclaration* C_GLDevice::InitVertexDeclaration(const S32 numVertexElements, const C_VertexElement* vertexElements)
{
	BIZ_ASSERT(numVertexElements > 0);

	for (S32 vertexDeclaration = 0; vertexDeclaration < m_VertexDeclarations.Size(); vertexDeclaration++)
	{
		C_VertexDeclaration* pVertexDeclaration = m_VertexDeclarations[vertexDeclaration];

		if (pVertexDeclaration->m_Elements.Size() == numVertexElements && 
			memcmp(&pVertexDeclaration->m_Elements[0], vertexElements, sizeof(C_VertexElement) * numVertexElements) == 0)
		{
			return pVertexDeclaration;
		}
	}

	C_VertexDeclaration* pVertexDeclaration = new C_VertexDeclaration();
	pVertexDeclaration->m_Elements.Resize(numVertexElements);
	memcpy(&pVertexDeclaration->m_Elements[0], vertexElements, sizeof(C_VertexElement) * numVertexElements);
	m_VertexDeclarations.PushBack(pVertexDeclaration);

	return pVertexDeclaration;
}

bool C_GLDevice::CreateVertexBuffer(const U32 size, const U32 flags, C_VertexBuffer* pVertexBuffer, bool allowFail)
{
	BIZ_ASSERT(pVertexBuffer);
	BIZ_ASSERT(!pVertexBuffer->IsCreated());
	
	glGenBuffers(1, &pVertexBuffer->m_Name);
	GLError();

	glBindBuffer(GL_ARRAY_BUFFER, pVertexBuffer->m_Name);
	GLError();

	BIZ_ASSERT(size > 0);
	glBufferData(GL_ARRAY_BUFFER, size, 0, GLUsage(flags));
	GLError();

	pVertexBuffer->SetSize(size);
	pVertexBuffer->SetSystemID(MemoryGetSystemID());
	MemoryIncSystem(pVertexBuffer->GetSystemID(), pVertexBuffer->GetSize(), E_MemoryType_VertexBuffer);

	m_VertexBuffers.Insert(pVertexBuffer);

	pVertexBuffer->Acquire();

	return true;
}

void C_GLDevice::DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer)
{
	BIZ_ASSERT(pVertexBuffer);
	if (pVertexBuffer->IsCreated() && pVertexBuffer->Release() == 0)
	{
		MemoryDecSystem(pVertexBuffer->GetSystemID(), pVertexBuffer->GetSize(), E_MemoryType_VertexBuffer);
		pVertexBuffer->SetSize(0);
		pVertexBuffer->SetSystemID(-1);

		m_VertexBuffers.Erase(pVertexBuffer);

		glDeleteBuffers(1, &pVertexBuffer->m_Name);
		GLError();
		pVertexBuffer->m_Name = 0;
	}
}

bool C_GLDevice::CreateIndexBuffer(const U32 size, const U32 flags, const E_Format format, C_IndexBuffer* pIndexBuffer, bool allowFail)
{
	BIZ_ASSERT(pIndexBuffer);
	BIZ_ASSERT(!pIndexBuffer->IsCreated());

	glGenBuffers(1, &pIndexBuffer->m_Name);
	GLError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer->m_Name);
	GLError();

	BIZ_ASSERT(size > 0);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, 0, GLUsage(flags));
	GLError();

	pIndexBuffer->m_Format = format;

	pIndexBuffer->SetSize(size);
	pIndexBuffer->SetSystemID(MemoryGetSystemID());
	MemoryIncSystem(pIndexBuffer->GetSystemID(), pIndexBuffer->GetSize(), E_MemoryType_IndexBuffer);

	m_IndexBuffers.Insert(pIndexBuffer);

	pIndexBuffer->Acquire();

	return true;
}

void C_GLDevice::DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer)
{
	BIZ_ASSERT(pIndexBuffer);
	if (pIndexBuffer->IsCreated() && pIndexBuffer->Release() == 0)
	{
		MemoryDecSystem(pIndexBuffer->GetSystemID(), pIndexBuffer->GetSize(), E_MemoryType_IndexBuffer);
		pIndexBuffer->SetSize(0);
		pIndexBuffer->SetSystemID(-1);

		m_IndexBuffers.Erase(pIndexBuffer);

		glDeleteBuffers(1, &pIndexBuffer->m_Name);
		GLError();
		pIndexBuffer->m_Name = 0;
	}
}

bool C_GLDevice::CreateVertexShader(const U32 size, const S32* pData, const U32 flags, C_VertexShader* pVertexShader)
{
#if 1
	return true;
#else
	BIZ_ASSERT_FAIL("Functionality not yet supported in gl");

	return false;
#endif
}

void C_GLDevice::DestroyVertexShader(C_VertexShader* pVertexShader)
{
	BIZ_ASSERT(pVertexShader);
	if (pVertexShader->IsCreated() && pVertexShader->Release() == 0)
	{
		MemoryDecSystem(pVertexShader->GetSystemID(), pVertexShader->GetSize(), E_MemoryType_VertexShader);
		pVertexShader->SetSize(0);
		pVertexShader->SetSystemID(-1);

		m_VertexShaders.Erase(pVertexShader);
		
		cgDestroyProgram(pVertexShader->m_cgProgram);
		//BIZ_ASSERT(cgGetError() == 0);
		pVertexShader->m_cgProgram = NULL;
	}
}

bool C_GLDevice::CreatePixelShader(const U32 size, const S32* pData, const U32 flags, C_PixelShader* pPixelShader)
{
#if 1
	return true;
#else
	BIZ_ASSERT_FAIL("Functionality not yet supported in gl");

	return false;
#endif
}

void C_GLDevice::DestroyPixelShader(C_PixelShader* pPixelShader)
{
	BIZ_ASSERT(pPixelShader);
	if (pPixelShader->IsCreated() && pPixelShader->Release() == 0)
	{
		MemoryDecSystem(pPixelShader->GetSystemID(), pPixelShader->GetSize(), E_MemoryType_PixelShader);
		pPixelShader->SetSize(0);
		pPixelShader->SetSystemID(-1);

		m_PixelShaders.Erase(pPixelShader);
		
		cgDestroyProgram(pPixelShader->m_cgProgram);
		BIZ_ASSERT(cgGetError() == 0);
		pPixelShader->m_cgProgram = NULL;
	}
}

void C_GLDevice::SetupView(const S32 id, const C_ViewSetup& viewSetup)
{
	BIZ_ASSERT(id >= 0 && id < m_Views.Size());
	m_Views[id]->Setup(viewSetup, m_BackBuffer.GetWidth(), m_BackBuffer.GetHeight());
}

void C_GLDevice::SetView(const S32 id)
{
	BIZ_ASSERT(id >= 0 && id < m_Views.Size());
	m_pView = m_Views[id];
	
	SetViewport(m_pView->GetViewport());
	
	SetDepthFunc(m_DepthFlip ? E_CmpFunc_GreaterEqual : E_CmpFunc_LessEqual);

	const C_ViewSettings2D& viewSettings2D = m_pView->GetViewSettings2D();
	
	const C_ViewSettings3D& viewSettings3D = m_pView->GetViewSettings3D();
}

void C_GLDevice::SetVertexDeclaration()
{
	if (m_SetVertexDeclaration)
	{
		SetVertexArray(false);
		SetNormalArray(false);
		SetColourArray(false);
		for (U32 sampler = 0; sampler < E_Sampler_Count; sampler++)
			SetTexcoordArray(sampler, false);
		
		if (m_pVertexDeclaration)
		{
			for (S32 vertexElementIndex = 0; vertexElementIndex < m_pVertexDeclaration->m_Elements.Size(); vertexElementIndex++)
			{
				const C_VertexElement& vertexElement = m_pVertexDeclaration->m_Elements[vertexElementIndex];

				switch (vertexElement.m_Usage)
				{
					// POSITIONS

					case(E_VertexElementUsage_Position):
					{
						BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
						const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];
						BIZ_ASSERT(vertexElement.m_UsageIndex == 0);

						if (vertexStream.m_pVertexBuffer)
						{
							SetVertexArray(true);
							switch (vertexElement.m_Type)
							{
							case E_VertexElementType_F32_1:
							case E_VertexElementType_F32_2:
							case E_VertexElementType_F32_3:
							case E_VertexElementType_F32_4:
								glVertexPointer(vertexElement.GetCount(), GL_FLOAT, vertexStream.m_VertexStride, BUFFER_OFFSET(vertexElement.m_Offset));
								break;
							}
						}
					}
					break;

					// NORMALS

					case E_VertexElementUsage_Normal:
					{
						BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
						const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];

						if (vertexStream.m_pVertexBuffer)
						{
							SetNormalArray(true);
							switch (vertexElement.m_Type)
							{
							case E_VertexElementType_F32_3:
								glNormalPointer(GL_FLOAT, vertexStream.m_VertexStride, BUFFER_OFFSET(vertexElement.m_Offset));
								break;
							#ifdef PLATFORM_PS3
							case E_VertexElementType_U10_3N:
								glNormalPointer(GL_FIXED_11_11_10_SCE, vertexStream.m_VertexStride, BUFFER_OFFSET(vertexElement.m_Offset));
								break;
							#endif
							}
						}
					}
					break;

					// COLOURS

					case E_VertexElementUsage_Colour:
					{
						BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
						const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];

						if (vertexStream.m_pVertexBuffer)
						{
							SetColourArray(true);
							switch (vertexElement.m_Type)
							{
							case E_VertexElementType_F32_1:
							case E_VertexElementType_F32_2:
							case E_VertexElementType_F32_3:
							case E_VertexElementType_F32_4:
								glColorPointer(vertexElement.GetCount(), GL_FLOAT, vertexStream.m_VertexStride, BUFFER_OFFSET(vertexElement.m_Offset));
								break;
							case E_VertexElementType_Colour:
								glColorPointer(4, GL_UNSIGNED_BYTE, vertexStream.m_VertexStride, BUFFER_OFFSET(vertexElement.m_Offset));
								break;
							}
						}
					}
					break;

					// TEXCOORDS

					case E_VertexElementUsage_TexCoord:
					{
						BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
						const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];

						if (vertexStream.m_pVertexBuffer)
						{
							SetTexcoordArray(vertexElement.m_UsageIndex, true);
							switch (vertexElement.m_Type)
							{
							case E_VertexElementType_F32_1:
							case E_VertexElementType_F32_2:
							case E_VertexElementType_F32_3:
							case E_VertexElementType_F32_4:
								glTexCoordPointer(vertexElement.GetCount(), GL_FLOAT, vertexStream.m_VertexStride, BUFFER_OFFSET(vertexElement.m_Offset));
								break;
							case E_VertexElementType_F16_2:
							case E_VertexElementType_F16_4:
								glTexCoordPointer(vertexElement.GetCount(), GL_HALF_FLOAT_ARB, vertexStream.m_VertexStride, BUFFER_OFFSET(vertexElement.m_Offset));
								break;
							}
						}
					}
					break;

					// BLENDINDICES

					case E_VertexElementUsage_BlendIndices:
					{
						BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
						const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];

						if (vertexStream.m_pVertexBuffer)
						{
							SetTexcoordArray(3, true);
							glTexCoordPointer(4, GL_SHORT, vertexStream.m_VertexStride, BUFFER_OFFSET(vertexElement.m_Offset));
						}
					}
					break;

					// BLENDWEIGHTS

					case E_VertexElementUsage_BlendWeight:
					{
						BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
						const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];

						if (vertexStream.m_pVertexBuffer)
						{
							SetTexcoordArray(4, true);
							glTexCoordPointer(4, GL_UNSIGNED_BYTE, vertexStream.m_VertexStride, BUFFER_OFFSET(vertexElement.m_Offset));
						}
					}
					break;
				}
			}
		}

		m_SetVertexDeclaration = false;
	}
}

}; // namespace GL
}; // namespace NewDisplay
