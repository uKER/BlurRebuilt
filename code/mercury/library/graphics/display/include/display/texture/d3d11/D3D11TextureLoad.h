#ifndef DISPLAY_TEXTURE_D3D11_TEXTURE_LOAD_H
#define DISPLAY_TEXTURE_D3D11_TEXTURE_LOAD_H

// [Blur1PCDX11] PC texture stream upload directly into D3D11 resources.
#include "Display/Device/D3D11/D3D11Resources.h"
namespace Sys { class I_BinaryStream; }
namespace Display
{
void LoadTexture2D(C_Texture2D* texture, Sys::I_BinaryStream* stream);
void PCLoadTexture(C_Texture2D* texture, Sys::I_BinaryStream* stream);
S32 CreateTexture2D(C_Texture2D* texture, Sys::I_BinaryStream* stream);
void LoadTextureData2D(C_Texture2D* texture, Sys::I_BinaryStream* stream, int size);
void AllocateTextureMemory(C_Texture2D* texture);
void CopyTextureData2D(C_Texture2D* destination, const C_Texture2D* source);
void CloneTexture2D(C_Texture2D* destination, const C_Texture2D* source,
                    int width, int height, int levels, int size);
}
#endif
