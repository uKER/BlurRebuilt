#ifndef DISPLAY_TEXTURE_GCM_GCM_TEXTURE_LOAD_H
#define DISPLAY_TEXTURE_GCM_GCM_TEXTURE_LOAD_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/GCM/GCMTexture.h"
#include "Display/Texture/GCM/GCMTexture2D.h"

namespace Sys
{
	class I_BinaryStream;
};

namespace Display
{
namespace GCM
{

// Load a texture from a stream, including GPU data.
void LoadTexture2D(C_Texture2D *pTexture, Sys::I_BinaryStream *pStream);

// Create a texture from a stream, but don't allocate GPU memory or load the data.
S32 CreateTexture2D(C_Texture2D *pTexture, Sys::I_BinaryStream *pStream);

void CloneTexture2D(C_Texture2D *pDst, const C_Texture2D *pSrc, int newWidth, int newHeight, int newLevels, int newSize);

// Allocate GPU resources for a texture.
void AllocateTextureMemory(C_Texture2D *pTexture);

// Transfer image data using the GPU (both pointers must be to local memory).
void GPUImageTransfer(void* pDstData, const void* pSrcData, U32 srcSize);

// Copy GPU data with the CPU.
void CopyGPUData(void* pDst, const void* pSrc, S32 size);

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_TEXTURE_GCM_GCM_TEXTURE_LOAD_H
