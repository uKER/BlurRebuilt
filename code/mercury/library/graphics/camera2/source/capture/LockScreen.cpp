
#include "stdafx.h"
#pragma warning (push, 3)
#include <Display.h>
#pragma warning (pop)
#include "camera2/capture/LockScreen.h"

namespace Cam2
{



// [Blur1PCDX11] Read the current back buffer through a D3D11 staging texture.
static ID3D11Texture2D* Surface = nullptr;
static ID3D11DeviceContext* s_LockContext = nullptr;

static ID3D11Texture2D* GetScreenSurface()
{
	ID3D11Device* device = Display::Device()->GetD3DDevice();
	const Display::C_Surface* colour = Display::Device()->GetBackBuffer().GetColourSurface();
	if (!device || !colour || !colour->m_pd3dSurface) return nullptr;
	ID3D11DeviceContext* context = nullptr;
	device->GetImmediateContext(&context);
	if (!context) return nullptr;
	D3D11_TEXTURE2D_DESC desc = {};
	colour->m_pd3dSurface->GetDesc(&desc);
	ID3D11Texture2D* resolved = nullptr;
	if (desc.SampleDesc.Count > 1)
	{
		D3D11_TEXTURE2D_DESC resolveDesc = desc;
		resolveDesc.SampleDesc.Count = 1;
		resolveDesc.SampleDesc.Quality = 0;
		resolveDesc.BindFlags = 0;
		resolveDesc.MiscFlags = 0;
		if (FAILED(device->CreateTexture2D(&resolveDesc, nullptr, &resolved)))
		{
			context->Release();
			return nullptr;
		}
		context->ResolveSubresource(resolved, 0, colour->m_pd3dSurface, 0, desc.Format);
	}
	D3D11_TEXTURE2D_DESC stagingDesc = desc;
	stagingDesc.SampleDesc.Count = 1;
	stagingDesc.SampleDesc.Quality = 0;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.MiscFlags = 0;
	ID3D11Texture2D* staging = nullptr;
	if (SUCCEEDED(device->CreateTexture2D(&stagingDesc, nullptr, &staging)))
		context->CopyResource(staging, resolved ? resolved : colour->m_pd3dSurface);
	if (resolved) resolved->Release();
	context->Release();
	return staging;
}


S32 LockScreen(C_LockScreenInfo& info)
{
	Surface = GetScreenSurface();
	if (!Surface) return 1;
	Display::Device()->GetD3DDevice()->GetImmediateContext(&s_LockContext);
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	if (!s_LockContext || FAILED(s_LockContext->Map(Surface, 0, D3D11_MAP_READ, 0, &mapped)))
	{
		if (s_LockContext) { s_LockContext->Release(); s_LockContext = nullptr; }
		Surface->Release(); Surface = nullptr;
		return 1;
	}
	D3D11_TEXTURE2D_DESC desc = {};
	Surface->GetDesc(&desc);
	info.bits = mapped.pData;
	info.pitch = mapped.RowPitch;
	info.format = D3DFMT_A8R8G8B8;
	info.width = desc.Width;
	info.height = desc.Height;
	return 0;
#if defined(PLATFORM_PC)
#endif

#ifdef PLATFORM_PS3
	const Display::C_FrontBuffer& frontBuffer = Display::Device()->GetSwappedFrontBuffer();
	const Display::C_Surface* pSurface = frontBuffer.GetSurface();
	if (!pSurface->IsCreated())
		return 1;

	info.bits = pSurface->GetAddress();
	info.pitch = GCM_ALIGN_SIZE(pSurface->GetWidth(), 64) << 2;

	if (pSurface->GetWidth() == 1920)
		info.pitch = 8192;

	info.format = 3;
	info.width = pSurface->GetWidth();
	info.height = pSurface->GetHeight();
	
	return 0;
#endif
}

void UnlockScreen()
{
	if (s_LockContext && Surface)
	{
		s_LockContext->Unmap(Surface, 0);
		s_LockContext->Release();
		Surface->Release();
		s_LockContext = nullptr;
		Surface = nullptr;
	}
}

} // End of namespace
