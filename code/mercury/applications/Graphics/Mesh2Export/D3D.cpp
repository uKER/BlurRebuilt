#include "StdAfx.h"
#include "D3D.h"

HWND g_pWnd = NULL;
IDirect3D9* g_pd3dObject = NULL;
IDirect3DDevice9* g_pd3dDevice = NULL;

bool D3DInitialise()
{
	g_pWnd = CreateWindow("BUTTON", "Window", WS_OVERLAPPED, 100, 100, 32, 32, 0, 0, GetModuleHandle(0), 0);
	if(!g_pWnd)
	{
		LOGERROR(g_Log, "Failed to create the window\n");
		return false;
	}
	
	BIZ_ASSERT(!g_pd3dObject);
	g_pd3dObject = Direct3DCreate9(D3D_SDK_VERSION);
	if(!g_pd3dObject)
	{
		LOGERROR(g_Log, "Failed to create d3d\n");
		return false;
	}

	D3DPRESENT_PARAMETERS d3dPresentParams; 
	ZeroMemory(&d3dPresentParams, sizeof(d3dPresentParams));
	d3dPresentParams.Windowed                   = TRUE;
	d3dPresentParams.hDeviceWindow              = g_pWnd;
	d3dPresentParams.BackBufferWidth            = 32;
	d3dPresentParams.BackBufferHeight           = 32;
	d3dPresentParams.BackBufferFormat           = D3DFMT_A8R8G8B8;
	d3dPresentParams.BackBufferCount            = 1;
	d3dPresentParams.EnableAutoDepthStencil     = TRUE;
	d3dPresentParams.AutoDepthStencilFormat     = D3DFMT_D24S8;
	d3dPresentParams.Flags                      = 0;
	d3dPresentParams.SwapEffect                 = D3DSWAPEFFECT_DISCARD;

	const HRESULT hr = g_pd3dObject->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_NULLREF,
		g_pWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dPresentParams,
		&g_pd3dDevice);
	if(FAILED(hr))
	{
		LOGERROR(g_Log, "Failed to create the d3d device\n");
		return false;
	}

	return true;
}

void D3DShutdown()
{
	if(g_pd3dDevice)
	{
		const ULONG refCount = g_pd3dDevice->Release();
		BIZ_ASSERT(refCount == 0);
		g_pd3dDevice = NULL;
	}

	if(g_pd3dObject)
	{
		const ULONG refCount = g_pd3dObject->Release();
		BIZ_ASSERT(refCount == 0);
		g_pd3dObject = NULL;
	}
}

bool D3DCreateTexture2DFromFile(const Util::C_String& filePath, IDirect3DTexture9** ppd3dTexture, DWORD& textureSize)
{
	bool success = false;

	textureSize = 0;

	BIZ_ASSERT(g_pd3dDevice);
	HRESULT hr = D3DXCreateTextureFromFile(g_pd3dDevice, filePath.Chars(), ppd3dTexture);
	if(SUCCEEDED(hr))
	{
		DWORD levelCount = (*ppd3dTexture)->GetLevelCount();
		for(DWORD level=0; level<levelCount; level++)
		{
			D3DSURFACE_DESC desc;
			hr = (*ppd3dTexture)->GetLevelDesc(level, &desc);
			BIZ_ASSERT(SUCCEEDED(hr));

			DWORD surfaceSize = 0;
			switch(desc.Format)
			{
			case D3DFMT_DXT1:
				surfaceSize = ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * 8;
				break;
			case D3DFMT_DXT3:
			case D3DFMT_DXT5:
				surfaceSize = ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * 16;
				break;
			case D3DFMT_X8R8G8B8:
			case D3DFMT_A8R8G8B8:
				surfaceSize = desc.Width * desc.Height * 4;
				break;
			default:
				BIZ_ASSERT(0);
				break;
			}
			textureSize += surfaceSize;
		}

		success = true;
	}
	else
	{
		switch(hr)
		{
		case D3DERR_NOTAVAILABLE:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DERR_NOTAVAILABLE\n", filePath.Chars());
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DERR_OUTOFVIDEOMEMORY\n", filePath.Chars());
			break;
		case D3DERR_INVALIDCALL:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DERR_INVALIDCALL\n", filePath.Chars());
			break;
		case D3DXERR_INVALIDDATA:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DXERR_INVALIDDATA\n", filePath.Chars());
			break;
		case E_OUTOFMEMORY:
			LOGERROR(g_Log, "Failed to create texture from file %s - E_OUTOFMEMORY\n", filePath.Chars());
			break;
		default:
			LOGERROR(g_Log, "Failed to create texture from file %s\n", filePath.Chars());
			break;
		}
	}
	
	return success;
}

void D3DDestroyTexture2D(IDirect3DTexture9*& pd3dTexture)
{
	if(pd3dTexture)
	{
		const ULONG refCount = pd3dTexture->Release();
		BIZ_ASSERT(refCount == 0);
		pd3dTexture = NULL;
	}
}

bool D3DCreateTexture3DFromFile(const Util::C_String& filePath, IDirect3DVolumeTexture9** ppd3dTexture, DWORD& textureSize)
{
	bool success = false;
	
	textureSize = 0;

	HRESULT hr = D3DXCreateVolumeTextureFromFile(g_pd3dDevice, filePath.Chars(), ppd3dTexture);
	if(SUCCEEDED(hr))
	{
		DWORD levelCount = (*ppd3dTexture)->GetLevelCount();
		for(DWORD level=0; level<levelCount; level++)
		{
			D3DVOLUME_DESC desc;
			hr = (*ppd3dTexture)->GetLevelDesc(level, &desc);
			BIZ_ASSERT(SUCCEEDED(hr));

			DWORD surfaceSize = 0;
			switch(desc.Format)
			{
			case D3DFMT_DXT1:
				surfaceSize = ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * desc.Depth * 8;
				break;
			case D3DFMT_DXT3:
			case D3DFMT_DXT5:
				surfaceSize = ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * desc.Depth * 16;
				break;
			case D3DFMT_A8R8G8B8:
				surfaceSize = desc.Width * desc.Height * desc.Depth * 4;
				break;
			default:
				BIZ_ASSERT(0);
				break;
			}
			textureSize += surfaceSize;
		}

		success = true;
	}
	else
	{
		switch(hr)
		{
		case D3DERR_NOTAVAILABLE:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DERR_NOTAVAILABLE\n", filePath.Chars());
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DERR_OUTOFVIDEOMEMORY\n", filePath.Chars());
			break;
		case D3DERR_INVALIDCALL:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DERR_INVALIDCALL\n", filePath.Chars());
			break;
		case D3DXERR_INVALIDDATA:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DXERR_INVALIDDATA\n", filePath.Chars());
			break;
		case E_OUTOFMEMORY:
			LOGERROR(g_Log, "Failed to create texture from file %s - E_OUTOFMEMORY\n", filePath.Chars());
			break;
		default:
			LOGERROR(g_Log, "Failed to create texture from file %s\n", filePath.Chars());
			break;
		}
	}

	return success;
}

void D3DDestroyTexture3D(IDirect3DVolumeTexture9*& pd3dTexture)
{
	if(pd3dTexture)
	{
		const ULONG refCount = pd3dTexture->Release();
		BIZ_ASSERT(refCount == 0);
		pd3dTexture = NULL;
	}
}

bool D3DCreateTextureCubeFromFile(const Util::C_String& filePath, IDirect3DCubeTexture9** ppd3dTexture, DWORD& textureSize)
{
	bool success = false;
	
	textureSize = 0;

	HRESULT hr = D3DXCreateCubeTextureFromFile(g_pd3dDevice, filePath.Chars(), ppd3dTexture);
	if(SUCCEEDED(hr))
	{
		DWORD levelCount = (*ppd3dTexture)->GetLevelCount();
		for(DWORD level=0; level<levelCount; level++)
		{
			D3DSURFACE_DESC desc;
			hr = (*ppd3dTexture)->GetLevelDesc(level, &desc);
			BIZ_ASSERT(SUCCEEDED(hr));

			DWORD surfaceSize = 0;
			switch(desc.Format)
			{
			case D3DFMT_DXT1:
				surfaceSize = ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * 8 *6;
				break;
			case D3DFMT_DXT3:
			case D3DFMT_DXT5:
				surfaceSize = ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * 16 *6;
				break;
			case D3DFMT_A8R8G8B8:
				surfaceSize = desc.Width * desc.Height * 4 *6;
				break;
			default:
				BIZ_ASSERT(0);
				break;
			}
			textureSize += surfaceSize;
		}

		success = true;
	}
	else
	{
		switch(hr)
		{
		case D3DERR_NOTAVAILABLE:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DERR_NOTAVAILABLE\n", filePath.Chars());
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DERR_OUTOFVIDEOMEMORY\n", filePath.Chars());
			break;
		case D3DERR_INVALIDCALL:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DERR_INVALIDCALL\n", filePath.Chars());
			break;
		case D3DXERR_INVALIDDATA:
			LOGERROR(g_Log, "Failed to create texture from file %s - D3DXERR_INVALIDDATA\n", filePath.Chars());
			break;
		case E_OUTOFMEMORY:
			LOGERROR(g_Log, "Failed to create texture from file %s - E_OUTOFMEMORY\n", filePath.Chars());
			break;
		default:
			LOGERROR(g_Log, "Failed to create texture from file %s\n", filePath.Chars());
			break;
		}
	}

	return success;
}

void D3DDestroyTextureCube(IDirect3DCubeTexture9*& pd3dTexture)
{
	if(pd3dTexture)
	{
		const ULONG refCount = pd3dTexture->Release();
		BIZ_ASSERT(refCount == 0);
		pd3dTexture = NULL;
	}
}

U32 D3DGetTexture2DSize(const D3DSURFACE_DESC& desc)
{
	switch(desc.Format)
	{
	case D3DFMT_DXT1:
		return ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * 8;
	case D3DFMT_DXT3:
	case D3DFMT_DXT5:
		return ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * 16;
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A8R8G8B8:
		return desc.Width * desc.Height * 4;
	default:
		BIZ_ASSERT(0);
		return -1;
	}
}

U32 D3DGetTexture3DSize(const D3DVOLUME_DESC& desc)
{
	switch(desc.Format)
	{
	case D3DFMT_DXT1:
		return ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * desc.Depth * 8;
	case D3DFMT_DXT3:
	case D3DFMT_DXT5:
		return ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * desc.Depth * 16;
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A8R8G8B8:
		return desc.Width * desc.Height * desc.Depth * 4;
	default:
		BIZ_ASSERT(0);
		return -1;
	}
}
