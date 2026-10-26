#include "stdafx.h"
#include "Display/Utils/Screenshot.h"
#include "Display/Device/Device.h"
#include <system/debug/Blur2PCAutomation.h>
#include <d3d11sdklayers.h>
#include <vector>
#include <string>
#include <algorithm>
#include <DirectXPackedVector.h>

enum E_DisplayScreenShot
{
	E_DisplayScreenShot_ColourRGB,
	E_DisplayScreenShot_ColourA,
	E_DisplayScreenShot_Depth,
};

DEBUG_ENUM(E_DisplayScreenShot)
	DE_OPTION(E_DisplayScreenShot_ColourRGB,	"ColourRGB")
	DE_OPTION(E_DisplayScreenShot_ColourA,		"ColourA")
	DE_OPTION(E_DisplayScreenShot_Depth,		"Depth")
END_DEBUG_ENUM

Debug::Enum<E_DisplayScreenShot> g_debugDisplayScreenshotMode("Graphics.Render.ScreenShotMode", E_DisplayScreenShot_ColourRGB);

namespace Display
{

// [Blur1PCDX11] Copy the current D3D11 target to CPU memory and write TGA.
static void SaveNativeTargetAsTGA(const Sys::C_FilePath& path, ID3D11Texture2D* source, bool scRGB = false, F32 paperWhiteScale = 1.0f)
{
	if (!source) return;
	ID3D11Device* device = Display::Device()->GetD3DDevice();
	if (!device) return;
	ID3D11DeviceContext* context = nullptr;
	device->GetImmediateContext(&context);
	if (!context) return;
	D3D11_TEXTURE2D_DESC desc = {};
	source->GetDesc(&desc);
	Blur2PCAutomation::LogF("DX11 shot %s %ux%u fmt=%u samples=%u", path.Chars(),
		desc.Width, desc.Height, static_cast<unsigned>(desc.Format), desc.SampleDesc.Count);
	// [Blur1PCDX11] Typeless colour storage has the same readback bytes as its
	// UNORM view. Resolve MSAA using the typed view format required by DX11.
	DXGI_FORMAT resolveFormat = desc.Format;
	if (resolveFormat == DXGI_FORMAT_B8G8R8A8_TYPELESS) resolveFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	if (resolveFormat == DXGI_FORMAT_R8G8B8A8_TYPELESS) resolveFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (resolveFormat != DXGI_FORMAT_B8G8R8A8_UNORM &&
		desc.Format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB &&
		resolveFormat != DXGI_FORMAT_R8G8B8A8_UNORM &&
		desc.Format != DXGI_FORMAT_R16G16B16A16_FLOAT &&
		desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB &&
		desc.Format != DXGI_FORMAT_R32_FLOAT)
	{
		context->Release();
		return;
	}
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
			Blur2PCAutomation::LogF("DX11 shot resolve texture failed");
			context->Release();
			return;
		}
		context->ResolveSubresource(resolved, 0, source, 0, resolveFormat);
	}
	D3D11_TEXTURE2D_DESC stagingDesc = desc;
	stagingDesc.SampleDesc.Count = 1;
	stagingDesc.SampleDesc.Quality = 0;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.MiscFlags = 0;
	ID3D11Texture2D* staging = nullptr;
	const HRESULT stagingResult = device->CreateTexture2D(&stagingDesc, nullptr, &staging);
	if (SUCCEEDED(stagingResult))
	{
		context->CopyResource(staging, resolved ? resolved : source);
		D3D11_MAPPED_SUBRESOURCE mapped = {};
		if (SUCCEEDED(context->Map(staging, 0, D3D11_MAP_READ, 0, &mapped)))
		{
			// [Blur2PC][HDR] Preserve floating-point evidence alongside the SDR preview.
			if (desc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT && scRGB)
			{
				std::string floatPath(path.Chars()); floatPath += ".pfm";
				FILE* pfm = nullptr; fopen_s(&pfm, floatPath.c_str(), "wb");
				float peak = 0; U32 highlights = 0;
				if (pfm) fprintf(pfm, "PF\n%u %u\n-1.0\n", desc.Width, desc.Height);
				for (U32 y = desc.Height; y-- > 0;)
					for (U32 x = 0; x < desc.Width; ++x)
					{
						const U16* pixel = reinterpret_cast<const U16*>(static_cast<const U8*>(mapped.pData) + y * mapped.RowPitch) + x * 4;
						float rgb[3]; for (int c = 0; c < 3; ++c) rgb[c] = DirectX::PackedVector::XMConvertHalfToFloat(pixel[c]);
						const float maximum = (std::max)(rgb[0], (std::max)(rgb[1], rgb[2]));
						peak = (std::max)(peak, maximum); highlights += maximum > paperWhiteScale;
						if (pfm) fwrite(rgb, sizeof(float), 3, pfm);
					}
				if (pfm) fclose(pfm);
				Blur2PCAutomation::LogF("HDR capture maxChannelNits=%.2f abovePaperWhitePixels=%u pfm=%s", peak * 80, highlights, floatPath.c_str());
			}
			FILE* file = nullptr;
			fopen_s(&file, path.Chars(), "wb");
			if (!file) Blur2PCAutomation::LogF("DX11 shot file open failed");
			if (file)
			{
				U8 header[18] = {};
				header[2] = 2;
				header[12] = static_cast<U8>(desc.Width);
				header[13] = static_cast<U8>(desc.Width >> 8);
				header[14] = static_cast<U8>(desc.Height);
				header[15] = static_cast<U8>(desc.Height >> 8);
				header[16] = 32;
				header[17] = 0x28; // Top-left origin, eight alpha bits.
				fwrite(header, 1, sizeof(header), file);
				const bool rgba = resolveFormat == DXGI_FORMAT_R8G8B8A8_UNORM ||
					desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
				for (U32 y = 0; y < desc.Height; ++y)
				{
					const U8* row = static_cast<const U8*>(mapped.pData) + y * mapped.RowPitch;
					for (U32 x = 0; x < desc.Width; ++x)
					{
						const U8* pixel = row + x * 4;
						U8 bgra[4] = { pixel[rgba ? 2 : 0], pixel[1],
							pixel[rgba ? 0 : 2], pixel[3] };
						if (desc.Format == DXGI_FORMAT_R32_FLOAT)
						{
							const F32 value = *reinterpret_cast<const F32*>(pixel);
							const U8 grey = static_cast<U8>(Maths::Clamp(value, 0.0f, 1.0f) * 255.0f);
							bgra[0] = bgra[1] = bgra[2] = grey;
							bgra[3] = 255;
						}
						if (desc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT)
						{
							const U16* fp = reinterpret_cast<const U16*>(row) + x * 4;
							for (int c = 0; c < 3; ++c)
							{
								float value = DirectX::PackedVector::XMConvertHalfToFloat(fp[c]);
								if (scRGB) { value = (std::max)(0.0f, value / paperWhiteScale); value = value <= 0.0031308f ? 12.92f * value : 1.055f * powf(value, 1.0f / 2.4f) - 0.055f; }
								bgra[2 - c] = static_cast<U8>(Maths::Clamp(value, 0.0f, 1.0f) * 255 + 0.5f);
							}
							bgra[3] = 255;
						}
						fwrite(bgra, 1, 4, file);
					}
				}
				fclose(file);
				Blur2PCAutomation::LogF("DX11 shot written");
			}
			context->Unmap(staging, 0);
		}
		staging->Release();
	}
	else
	{
		Blur2PCAutomation::LogF("DX11 shot staging failed %08x, removed reason %08x",
			static_cast<unsigned>(stagingResult), static_cast<unsigned>(device->GetDeviceRemovedReason()));
		ID3D11InfoQueue* queue = nullptr;
		if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&queue))))
		{
			const UINT64 count = queue->GetNumStoredMessages();
			Blur2PCAutomation::LogF("DX11 validation messages: %llu", count);
			for (UINT64 i = count > 25 ? count - 25 : 0; i < count; ++i)
			{
				SIZE_T size = 0;
				queue->GetMessage(i, nullptr, &size);
				std::vector<U8> bytes(size);
				D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(bytes.data());
				if (SUCCEEDED(queue->GetMessage(i, message, &size)))
					Blur2PCAutomation::LogF("DX11 %u: %s", static_cast<unsigned>(message->Severity),
						message->pDescription);
			}
			queue->Release();
		}
	}
	if (resolved) resolved->Release();
	context->Release();
}

void TakeD3D11BackBufferScreenshot(const char* fileName, ID3D11Texture2D* source, bool scRGB, F32 paperWhiteScale)
{
	if (fileName && source)
		SaveNativeTargetAsTGA(Sys::C_FilePath(fileName), source, scRGB, paperWhiteScale);
}

void SaveAsTGA(const Sys::C_FilePath& fileName, const S32 width, const S32 height, const U8* pData)
{
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(fileName.Chars(), Sys::E_FileAccess_Write);
	if(!pFile)
	{
		LOGWARN(g_Log, "Failed to open file '%s'\n", fileName.Chars());
		return;
	}

	U8 tgaHeader[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
	pFile->Write(tgaHeader, sizeof(U8)*12);

	U8 imgHeader[6] = {width%256,width/256,height%256,height/256,32,0};
	pFile->Write(imgHeader, sizeof(U8)*6);

	#ifdef PLATFORM_PS3
	U32 alignedWidth = GCM_ALIGN_SIZE(width, 64);
	U32 alignedHeight = height;
	#else
	U32 alignedWidth = width;
	U32 alignedHeight = height;
	#endif
	
	for(S32 y=0; y<height; y++)
	{
		for(S32 x=0; x<width; x++)
		{
			U8 r=0, g=0, b=0, a=0;
#if 1
			r = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+3];
			g = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+2];
			b = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+1];
			a = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+0];
#else
			switch(g_debugDisplayScreenshotMode)
			{
			case E_DisplayScreenShot_ColourRGB:
				{
					r = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+3];
					g = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+2];
					b = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+1];
				}
				break;
			case E_DisplayScreenShot_ColourA:
				{
					r = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+0];
					g = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+0];
					b = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+0];
				}
				break;
			case E_DisplayScreenShot_Depth:
				{
					r = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+0];
					g = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+1];
					b = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+2];
					a = pData[(((alignedHeight-y)*alignedWidth*4)+(x*4))+3];
				}
				break;
			default:
				assert(0 && "Unsupported screenshot mode");
				break;
			}
#endif
			pFile->Write(&r, sizeof(U8));
			pFile->Write(&g, sizeof(U8));
			pFile->Write(&b, sizeof(U8));
			pFile->Write(&a, sizeof(U8));
		}
	}

	SAFE_DELETE_PTR(pFile);
}

void TakeScreenshot(const Sys::C_FilePath& fileName, const C_Surface* pSurface)
{
	if(!pSurface || !pSurface->IsCreated())
	{
		LOGWARN(g_Log, "Failed to save screenshot '%s'\n", fileName.Chars());
		return;
	}

	const Sys::C_FilePath filePath = 
		Sys::GetDefaultFileSystem()->GetWorkingDirectory() + "/" + fileName;
	
	SaveNativeTargetAsTGA(filePath, pSurface->m_pd3dSurface);
}

void TakeScreenshot(const Sys::C_FilePath fileName)
{
	if(!DeviceExists())
		return;
	
	#if defined(GRAPHICS_D3D11) // [Blur1PCDX11] Capture the native back buffer.
	{
		const Sys::C_FilePath path = Sys::GetDefaultFileSystem()->GetWorkingDirectory() + "/" + fileName;
		ID3D11Device* device = Display::Device()->GetD3DDevice();
		ID3D11DeviceContext* context = nullptr;
		device->GetImmediateContext(&context);
		ID3D11RenderTargetView* target = nullptr;
		context->OMGetRenderTargets(1, &target, nullptr);
		if (target)
		{
			ID3D11Resource* resource = nullptr;
			target->GetResource(&resource);
			ID3D11Texture2D* texture = nullptr;
			if (resource) resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);
			SaveNativeTargetAsTGA(path, texture);
			if (texture) texture->Release();
			if (resource) resource->Release();
			target->Release();
		}
		context->Release();
	}
	#elif defined(GRAPHICS_GCM)
	{
		const C_Surface* pSurface = NULL;
#if 1
		switch(g_debugDisplayScreenshotMode)
		{
		case E_DisplayScreenShot_Depth:
			{
				const C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
				pSurface = backBuffer.GetDepthStencilSurface();
			}
			break;
		default:
			{
				const C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
				pSurface = backBuffer.GetColourSurface();
			}
			break;
		}
#else
		switch(g_debugDisplayScreenshotMode)
		{
		case E_DisplayScreenShot_Depth:
			{
				const C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
				pSurface = backBuffer.GetDepthStencilSurface();
			}
			break;
		default:
			{
				const C_FrontBuffer& frontBuffer = Display::Device()->GetSwappedFrontBuffer();
				pSurface = frontBuffer.GetSurface();
			}
			break;
		}
#endif
		TakeScreenshot(fileName, pSurface);
	}
	#endif
}

}; // namespace Display
