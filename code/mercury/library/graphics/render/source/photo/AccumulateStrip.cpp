#include "stdafx.h"
#include "Render/Photo/AccumulateStrip.h"
#include "Render/Shaders/Constants.h"
#include <Display/Buffer/VertexDeclarations.h>

// Photo Accumulate buffer
// This can average several frames of video together to make a 'Photo' higher quality version of the screen

// The idea is that by jittering the projection matrix or other things (e.g. through time to create motion blur)
// and gathering extra pixels you get a higher quality version

// This is a strip-based version, it copies each line of width x 16 pixels out of the Photo texture first,
// and then merges them back in

EXTERN_SHADER(psPhoto);

namespace Render
{
namespace Photo
{

using namespace Maths;
using namespace Display;

C_AccumulateStrip::C_AccumulateStrip() : I_Renderer("PhotoAccumulate")
{
	m_PhotoTex = m_StripTex = m_ScreenStrip = 0;
	m_Width = m_Height = 0;
	m_SampleNum = m_SampleCount = 0;
	memset(m_Jitter, 0, sizeof(m_Jitter));
	m_Viewport = new C_Viewport;
	m_PlainSurface = 0;
	m_LockContext = 0;
	m_CurrentJitter = 0.f;
}

C_AccumulateStrip::~C_AccumulateStrip()
{
	SAFE_DELETE_PTR(m_Viewport)
	DestroyRenderer();
}

bool C_AccumulateStrip::OnCreate()
{
	RESOURCE_MEMORY_ID("Photo");

	bool ret = CreatePixelShaderFromFunc("Shader/psPhoto", 0, &m_PixelShader, REF_SHADER(psPhoto));
	if (!ret)
		return false;
	
	m_AdjustParam = m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_AdjustParam");

	return true;
}

void C_AccumulateStrip::OnDestroy()
{
	Device()->Destroy(&m_PixelShader);
}

bool C_AccumulateStrip::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("Photo");

	MEMORY_TRACKER_PUSH("graphics.renderers.photomode");

	m_Width = Device()->GetBackBuffer().GetWidth();
	m_Height = Device()->GetBackBuffer().GetHeight();

	// Create Render textures to accumulate into:
	m_PhotoTex = new C_RenderTexture2D;
	m_StripTex = new C_RenderTexture2D;
	if (!m_PhotoTex || !m_StripTex)
	{
		MEMORY_TRACKER_POP("graphics.renderers.photomode");
		return false;
	}

	bool ret = Device()->CreateRenderTexture2D(m_Width, m_Height, 1, E_Usage_Colour, E_Format_A8R8G8B8, E_Multisample_None, 0, m_PhotoTex);
	if (!ret)
	{
		MEMORY_TRACKER_POP("graphics.renderers.photomode");
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(m_PhotoTex, "_AccumulateStripPhotoRT");
	#endif

	ret = Device()->CreateRenderTexture2D(m_Width, 16, 1, E_Usage_Colour, E_Format_A8R8G8B8, E_Multisample_None, 0, m_StripTex);
	if (!ret)
	{
		MEMORY_TRACKER_POP("graphics.renderers.photomode");
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(m_StripTex, "_AccumulateStripStripRT");
	#endif

#ifdef PLATFORM_PC
	m_ScreenStrip = new C_RenderTexture2D;
	if (!m_ScreenStrip)
	{
		MEMORY_TRACKER_POP("graphics.renderers.photomode");
		return false;
	}
	ret = Device()->CreateRenderTexture2D(m_Width, 16, 1, E_Usage_Colour, E_Format_A8R8G8B8, E_Multisample_None, 0, m_ScreenStrip);
	if (!ret)
	{
		MEMORY_TRACKER_POP("graphics.renderers.photomode");
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(m_ScreenStrip, "_AccumulateStripScreenStripRT");
	#endif
#endif

	ClearTextures();

	MEMORY_TRACKER_POP("graphics.renderers.photomode");

	return true;
}

void C_AccumulateStrip::OnDestroyVolatile()
{
	// Destroy Render texture:
	if (m_ScreenStrip)
		Device()->Destroy(m_ScreenStrip);
	if (m_PhotoTex)
		Device()->Destroy(m_PhotoTex);
	if (m_StripTex)
		Device()->Destroy(m_StripTex);

	m_PhotoTex = m_StripTex = m_ScreenStrip = 0;
	
	m_Width = m_Height = 0;
}

// Start rendering into a rectangle in a texture
void C_AccumulateStrip::SetRenderTarget(C_RenderTexture2D* tex, S32 x, S32 y, S32 w, S32 h)
{
	if (!tex || !m_Viewport)
		return;

	C_Surface* surface = &tex->GetSurface();
	Device()->SetColourDepthStencilSurface(surface, 0);

	if (w <= 0)
		w = surface->GetWidth();
	if (h <= 0)
		h = surface->GetHeight();

	m_Viewport->m_X = x;
	m_Viewport->m_Y = y;
	m_Viewport->m_W = w;
	m_Viewport->m_H = h;
	Device()->SetViewport(*m_Viewport);
}

// End rendering and resolve into a rectangle on a texture
void C_AccumulateStrip::ResolveRenderTarget(C_RenderTexture2D* tex)
{
	if (!tex || !m_Viewport)
		return;

}

void C_AccumulateStrip::ClearTextures()
{
	// Clear texture:
	SetRenderTarget(m_PhotoTex);
	Device()->Clear(E_Clear_Colour, RGBA(0xff000000));
	ResolveRenderTarget(m_PhotoTex);

	SetRenderTarget(m_StripTex);
	Device()->Clear(E_Clear_Colour, RGBA(0xff000000));
	ResolveRenderTarget(m_StripTex);

	if (m_ScreenStrip)
	{
		SetRenderTarget(m_ScreenStrip);
		Device()->Clear(E_Clear_Colour, RGBA(0xff000000));
		ResolveRenderTarget(m_ScreenStrip);
	}
}

C_Texture2D* C_AccumulateStrip::GetTexture()
{
	C_RenderTexture2D* tex = m_PhotoTex;
	if (!tex)
		return 0;

	return &tex->GetTexture();
}

C_Texture2D* C_AccumulateStrip::StripTexture()
{
	C_RenderTexture2D* tex = m_StripTex;
	if (!tex)
		return 0;

	return &tex->GetTexture();
}

void C_AccumulateStrip::Start(S32 sampleCount)
{
	bool makeJitter = sampleCount != m_SampleCount;

	m_SampleNum = 0;
	m_SampleCount = sampleCount;

	ClearTextures();

	if (makeJitter)
	{
		// Make jitter table:
		S32 max = sampleCount;
		if (max < 1)
			max = 1;
		if (max > 256)
			max = 256;
		// Make values evenly between -1 and +1
		S32 i = 0;
		for (i = 0; i < max; i++)
		{
			m_Jitter[i] = ((F32)i + 0.5f) * 2.f / (F32)max - 1.f;
		}

		// Randomly swap them around:
		for (i = 0; i < (max << 6); i++)
		{
			S32 a = rand() % max;
			S32 b = rand() % max;
			F32 tmp = m_Jitter[a];
			m_Jitter[a] = m_Jitter[b];
			m_Jitter[b] = tmp;
		}
	}
}

// Add one sample of the screen to the Photo texture:
void C_AccumulateStrip::AddSample(const C_Texture2D* texture)
{
	if (m_SampleCount < 1)
		return;

#ifdef PLATFORM_PS3
	if (texture == 0)
		texture = Device()->GetBackBuffer().GetColourTexture();
#endif

	S32 sh = m_StripTex->GetTexture().GetHeight();
	S32 height = m_PhotoTex->GetTexture().GetHeight();

	m_CurrentJitter = m_Jitter[m_SampleNum & 255];

	// Go down the photo:
	for (S32 y = 0; y < height; y += sh)
	{
		// On PC we need to copy each row of the screen to a Strip texture:
		ScreenToStrip(y, sh);

		// Copy a row of the photo texture to a strip texture:
		PhotoToStrip(y, sh);

		// Then Strip + Screen -> back into the Photo texture:
		StripToPhoto(y, sh, texture);
	}
	
	m_SampleNum++;
}

// On PC, copy a row of the screen to a Strip texture:
void C_AccumulateStrip::ScreenToStrip(S32 top, S32 height)
{
#ifdef PLATFORM_PC
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	S32 width  = backBuffer.GetWidth();
	ID3D11Texture2D* source = backBuffer.GetColourSurface()->m_pd3dSurface;
	ID3D11Texture2D* destination = m_ScreenStrip->GetSurface().m_pd3dSurface;
	if (!source || !destination) return;
	ID3D11Device* device = Display::Device()->GetD3DDevice();
	ID3D11DeviceContext* context = nullptr;
	device->GetImmediateContext(&context);
	if (!context) return;
	D3D11_TEXTURE2D_DESC sourceDesc = {};
	source->GetDesc(&sourceDesc);
	ID3D11Texture2D* resolved = nullptr;
	if (sourceDesc.SampleDesc.Count > 1)
	{
		D3D11_TEXTURE2D_DESC desc = sourceDesc;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		if (SUCCEEDED(device->CreateTexture2D(&desc, nullptr, &resolved)))
		{
			context->ResolveSubresource(resolved, 0, source, 0, sourceDesc.Format);
			source = resolved;
		}
	}
	if (sourceDesc.SampleDesc.Count == 1 || resolved)
	{
		const D3D11_BOX region = { 0, static_cast<UINT>(top), 0,
			static_cast<UINT>(width), static_cast<UINT>(top + height), 1 };
		context->CopySubresourceRegion(destination, 0, 0, 0, 0, source, 0, &region);
		Display2::D3D11ResourceWritten(destination, context); // [Blur1PCDX11]
	}
	if (resolved) resolved->Release();
	context->Release();
#endif
}

// Copy a row of the photo to a Strip texture:
void C_AccumulateStrip::PhotoToStrip(S32 top, S32 height)
{
	SetRenderTarget(m_StripTex);

	Device()->SetPixelShader(&PixelShader::m_PixelShaderTexture);
	Device()->SetTexture(0, &m_PhotoTex->GetTexture());

	// Half pixel offset:
	F32 y1 = (F32)top / m_Height;
	F32 y2 = (F32)(top + height) / m_Height;
	F32 xoff = 1.f / m_Viewport->m_W;
	F32 yoff = 1.f / m_Viewport->m_H;
	DrawQuad(y1, y2, xoff, yoff);

	ResolveRenderTarget(m_StripTex);
}

// Copy strip with new frame of screen back into Photo texture:
void C_AccumulateStrip::StripToPhoto(S32 top, S32 height, const C_Texture2D* screenTexture)
{
	SetRenderTarget(m_PhotoTex, 0, top, m_Width, height);

	Device()->SetColourWriteEnable(E_ColourWriteEnable_All);

	F32 adjust[4] = {0.f, 0.f, 0.f, 1.f};
	adjust[0] = m_CurrentJitter;
	adjust[1] = (F32)m_SampleCount;
	
	Device()->SetTexture(0, &m_StripTex->GetTexture());

#ifdef PLATFORM_PC
	// Read screen strip texture:
	Device()->SetTexture(1, &m_ScreenStrip->GetTexture());
#else
	// Read direct from screen:
	Device()->SetTexture(1, screenTexture);
	adjust[2] = (F32)top / m_Height;
	adjust[3] = (F32)height / m_Height;
#endif

	Device()->SetPixelShaderParameterF(m_AdjustParam, adjust);
	Device()->SetPixelShaderParameters(&m_PixelShader);
	Device()->SetPixelShader(&m_PixelShader);

	// Half pixel offset:
	F32 xoff = 1.f / m_Viewport->m_W;
	F32 yoff = 1.f / m_Viewport->m_H;
	DrawQuad(0.f, 1.f, xoff, yoff);

	Device()->SetTexture(0, 0);
	Device()->SetTexture(1, 0);

	ResolveRenderTarget(m_PhotoTex);
}

void C_AccumulateStrip::DrawQuad(F32 srcTop, F32 srcBottom, F32 xoff, F32 yoff)
{
#ifdef PLATFORM_PS3
	xoff = yoff = 0.f; // Half pixel offset not needed on PS3
#endif

	Device()->SetDepthEnable(E_Bool_False);

	Device()->SetVertexShader(&VertexShader_XY::m_VertexShader);
	Device()->SetTextureFilter(0, E_TextureFilter_Point);
	Device()->SetTextureFilter(1, E_TextureFilter_Point);
	
	Vertex_XY_UV* vertex = (Vertex_XY_UV*)Device()->BeginVertices(E_PrimitiveType_TriangleStrip, 4, sizeof(Vertex_XY_UV), Vertex_XY_UV::m_pDeclaration, false);
	if (vertex)
	{
		vertex->x = -1.f - xoff; vertex->y = 1.f + yoff;
		vertex->u = 0.f; vertex->v = srcTop;
		vertex++;
		
		vertex->x = 1.f - xoff; vertex->y = 1.f + yoff;
		vertex->u = 1.f; vertex->v = srcTop;
		vertex++;
		
		vertex->x = -1.f - xoff; vertex->y = -1.f + yoff;
		vertex->u = 0.f; vertex->v = srcBottom;
		vertex++;
		
		vertex->x = 1.f - xoff; vertex->y = -1.f + yoff;
		vertex->u = 1.f; vertex->v = srcBottom;
		vertex++;
		
		Device()->EndVertices(vertex);
	}

	Device()->SetDepthEnable(E_Bool_True);
}

bool C_AccumulateStrip::LockBitmap(unsigned char*& retBits, S32& retPitch, S32& retWidth, S32& retHeight, S32& retFormat)
{
	Display::C_Texture2D* texture = GetTexture();
	if (!texture)
		return false;

	retWidth  = texture->GetWidth(0);
	retHeight = texture->GetHeight(0);

#ifdef PLATFORM_PC
	ID3D11Texture2D* source = texture->GetGPUTexture2D();
	if (!source) return false;
	D3D11_TEXTURE2D_DESC desc = {};
	source->GetDesc(&desc);
	if (desc.Format != DXGI_FORMAT_B8G8R8A8_UNORM || desc.SampleDesc.Count != 1)
		return false;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	ID3D11Device* device = Device()->GetD3DDevice();
	if (FAILED(device->CreateTexture2D(&desc, nullptr, &m_PlainSurface)))
		return false;
	device->GetImmediateContext(&m_LockContext);
	if (!m_LockContext) { m_PlainSurface->Release(); m_PlainSurface = 0; return false; }
	m_LockContext->CopySubresourceRegion(m_PlainSurface, 0, 0, 0, 0, source, 0, nullptr);
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	if (FAILED(m_LockContext->Map(m_PlainSurface, 0, D3D11_MAP_READ_WRITE, 0, &mapped)))
	{
		m_LockContext->Release(); m_LockContext = 0;
		m_PlainSurface->Release(); m_PlainSurface = 0;
		return false;
	}
	retBits = static_cast<unsigned char*>(mapped.pData);
	retPitch = mapped.RowPitch;
	retFormat = D3DFMT_A8R8G8B8;

#else
	Display::C_LockedRect rect = texture->Lock(0);
	retBits = rect.pBits;
	retPitch = rect.pitch;
	retFormat = 0;
#endif


	return true;
}

void C_AccumulateStrip::UnlockBitmap()
{
#ifdef PLATFORM_PC
	if (m_PlainSurface && m_LockContext)
	{
		m_LockContext->Unmap(m_PlainSurface, 0);
		Display::C_Texture2D* texture = GetTexture();
		if (texture && texture->GetGPUTexture2D())
		{
			m_LockContext->CopySubresourceRegion(texture->GetGPUTexture2D(), 0,
				0, 0, 0, m_PlainSurface, 0, nullptr);
			Display2::D3D11ResourceWritten(texture->GetGPUTexture2D(), m_LockContext); // [Blur1PCDX11]
		}
		m_LockContext->Release();
		m_PlainSurface->Release();
		m_LockContext = 0;
		m_PlainSurface = 0;
	}
#else
	Display::C_Texture2D* texture = GetTexture();
	if (texture)
		texture->Unlock(0);

#endif
}

} // namespace Photo

} // namespace Render
