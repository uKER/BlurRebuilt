#ifndef D3D_H
#define D3D_H

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

extern IDirect3DDevice9* g_pd3dDevice;

bool D3DInitialise();
void D3DShutdown();

bool D3DCreateTexture2DFromFile(const Util::C_String& filePath, IDirect3DTexture9** ppd3dTexture, DWORD& textureSize);
void D3DDestroyTexture2D(IDirect3DTexture9*& pd3dTexture);

bool D3DCreateTexture3DFromFile(const Util::C_String& filePath, IDirect3DVolumeTexture9** ppd3dTexture, DWORD& textureSize);
void D3DDestroyTexture3D(IDirect3DVolumeTexture9*& pd3dTexture);

bool D3DCreateTexture3DFromFile(const Util::C_String& filePath, IDirect3DVolumeTexture9** ppd3dTexture, DWORD& textureSize);
void D3DDestroyTexture3D(IDirect3DVolumeTexture9*& pd3dTexture);

bool D3DCreateTextureCubeFromFile(const Util::C_String& filePath, IDirect3DCubeTexture9** ppd3dTexture, DWORD& textureSize);
void D3DDestroyTextureCube(IDirect3DCubeTexture9*& pd3dTexture);

U32 D3DGetTexture2DSize(const D3DSURFACE_DESC& desc);
U32 D3DGetTexture3DSize(const D3DVOLUME_DESC& desc);

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(_ptr) if(_ptr) { _ptr->Release(); _ptr = NULL; }
#endif

#endif // D3D_H
