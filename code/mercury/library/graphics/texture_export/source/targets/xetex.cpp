#include "stdafx.h"

#define NGL_GPU_D3D_XENON 1

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include "d3d9.h"  // Make sure this is the Xenon version.
#include "d3dx9.h"
#include "Xgraphics.h"

#include <core/core.h>
#include <core/containers/string.h>
#include <system/scheduler/core/tl_defs.h>
#include <system/serialise/I_BinaryStream.h>
#include <system/serialise/BinarySerialise.h>
#include <core/utility/endian.h>
#include "ngl/defs.h"

//#include "vmath.h"

#define NGL_GPU_D3D 1
#define NGL_GPU_D3D9 1
#define NGL_GPU_D3D_XENON 1
#define NGL_API
#include "ngl/dx/ngl_dx_gpu_types.h"	// output texture structures
#include "texture_export/ngl/gpu/ngl_gpu_texture.h"

#define NGL_TOOL 1

#include "texture_export/texture_export.h"
#include "texture_export/Texture.h"

using namespace ActTools;

#include "texture_export/targets/xetex.h"

int NglXeTex::Flags;

struct XenonTexelFormatDesc
{
	const char* name;
	D3DFORMAT format;
};

XenonTexelFormatDesc XenonTexelFormats[] =
{
	// DXT:
	{ "DXT1", D3DFMT_DXT1 },
	{ "LIN_DXT1", D3DFMT_LIN_DXT1 },
	{ "DXT2", D3DFMT_DXT2 },
	{ "LIN_DXT2", D3DFMT_LIN_DXT2 },
	{ "DXT3", D3DFMT_DXT3 },
	{ "LIN_DXT3", D3DFMT_LIN_DXT3 },
	{ "DXT3A", D3DFMT_DXT3A },
	{ "LIN_DXT3A", D3DFMT_LIN_DXT3A },
	{ "DXT3A_1111", D3DFMT_DXT3A_1111 },
	{ "LIN_DXT3A_1111", D3DFMT_LIN_DXT3A_1111 },
	{ "DXT4", D3DFMT_DXT4 },
	{ "LIN_DXT4", D3DFMT_LIN_DXT4 },
	{ "DXT5", D3DFMT_DXT5 },
	{ "LIN_DXT5", D3DFMT_LIN_DXT5 },
	{ "DXT5A", D3DFMT_DXT5A },
	{ "LIN_DXT5A", D3DFMT_LIN_DXT5A },
	{ "DXN", D3DFMT_DXN },
	{ "LIN_DXN", D3DFMT_LIN_DXN },
	{ "CTX1", D3DFMT_CTX1 },
	{ "LIN_CTX1", D3DFMT_LIN_CTX1 },

	//8bpp:
	{ "A8", D3DFMT_A8 },
	{ "LIN_A8", D3DFMT_LIN_A8 },
	{ "L8", D3DFMT_L8 },
	{ "LIN_L8", D3DFMT_LIN_L8 },

	//16bpp:
	{ "R5G6B5", D3DFMT_R5G6B5 },
	{ "LIN_R5G6B5", D3DFMT_LIN_R5G6B5 },
	{ "R6G5B5", D3DFMT_R6G5B5 },
	{ "LIN_R6G5B5", D3DFMT_LIN_R6G5B5 },
	{ "L6V5U5", D3DFMT_L6V5U5 },
	{ "LIN_L6V5U5", D3DFMT_LIN_L6V5U5 },
	{ "X1R5G5B5", D3DFMT_X1R5G5B5 },
	{ "LIN_X1R5G5B5", D3DFMT_LIN_X1R5G5B5 },
	{ "A1R5G5B5", D3DFMT_A1R5G5B5 },
	{ "LIN_A1R5G5B5", D3DFMT_LIN_A1R5G5B5 },
	{ "A4R4G4B4", D3DFMT_A4R4G4B4 },
	{ "LIN_A4R4G4B4", D3DFMT_LIN_A4R4G4B4 },
	{ "X4R4G4B4", D3DFMT_X4R4G4B4 },
	{ "LIN_X4R4G4B4", D3DFMT_LIN_X4R4G4B4 },
	{ "Q4W4V4U4", D3DFMT_Q4W4V4U4 },
	{ "LIN_Q4W4V4U4", D3DFMT_LIN_Q4W4V4U4 },
	{ "A8L8", D3DFMT_A8L8 },
	{ "LIN_A8L8", D3DFMT_LIN_A8L8 },
	{ "G8R8", D3DFMT_G8R8 },
	{ "LIN_G8R8", D3DFMT_LIN_G8R8 },
	{ "V8U8", D3DFMT_V8U8 },
	{ "LIN_V8U8", D3DFMT_LIN_V8U8 },
	{ "D16", D3DFMT_D16 },
	{ "LIN_D16", D3DFMT_LIN_D16 },
	{ "L16", D3DFMT_L16 },
	{ "LIN_L16", D3DFMT_LIN_L16 },
	{ "R16F", D3DFMT_R16F },
	{ "LIN_R16F", D3DFMT_LIN_R16F },
	{ "R16F_EXPAND", D3DFMT_R16F_EXPAND },
	{ "LIN_R16F_EXPAND", D3DFMT_LIN_R16F_EXPAND },
	{ "UYVY", D3DFMT_UYVY },
	{ "LIN_UYVY", D3DFMT_LIN_UYVY },
	{ "LE_UYVY", D3DFMT_LE_UYVY },
	{ "LE_LIN_UYVY", D3DFMT_LE_LIN_UYVY },
	{ "G8R8_G8B8", D3DFMT_G8R8_G8B8 },
	{ "LIN_G8R8_G8B8", D3DFMT_LIN_G8R8_G8B8 },
	{ "R8G8_B8G8", D3DFMT_R8G8_B8G8 },
	{ "LIN_R8G8_B8G8", D3DFMT_LIN_R8G8_B8G8 },
	{ "YUY2", D3DFMT_YUY2 },
	{ "LIN_YUY2", D3DFMT_LIN_YUY2 },
	{ "LE_YUY2", D3DFMT_LE_YUY2 },
	{ "LE_LIN_YUY2", D3DFMT_LE_LIN_YUY2 },

	//32bpp:
	{ "A8R8G8B8", D3DFMT_A8R8G8B8 },
	{ "LIN_A8R8G8B8", D3DFMT_LIN_A8R8G8B8 },
	{ "X8R8G8B8", D3DFMT_X8R8G8B8 },
	{ "LIN_X8R8G8B8", D3DFMT_LIN_X8R8G8B8 },
	{ "A8B8G8R8", D3DFMT_A8B8G8R8 },
	{ "LIN_A8B8G8R8", D3DFMT_LIN_A8B8G8R8 },
	{ "X8B8G8R8", D3DFMT_X8B8G8R8 },
	{ "LIN_X8B8G8R8", D3DFMT_LIN_X8B8G8R8 },
	{ "X8L8V8U8", D3DFMT_X8L8V8U8 },
	{ "LIN_X8L8V8U8", D3DFMT_LIN_X8L8V8U8 },
	{ "Q8W8V8U8", D3DFMT_Q8W8V8U8 },
	{ "LIN_Q8W8V8U8", D3DFMT_LIN_Q8W8V8U8 },
	{ "A2R10G10B10", D3DFMT_A2R10G10B10 },
	{ "LIN_A2R10G10B10", D3DFMT_LIN_A2R10G10B10 },
	{ "X2R10G10B10", D3DFMT_X2R10G10B10 },
	{ "LIN_X2R10G10B10", D3DFMT_LIN_X2R10G10B10 },
	{ "A2B10G10R10", D3DFMT_A2B10G10R10 },
	{ "LIN_A2B10G10R10", D3DFMT_LIN_A2B10G10R10 },
	{ "A2W10V10U10", D3DFMT_A2W10V10U10 },
	{ "LIN_A2W10V10U10", D3DFMT_LIN_A2W10V10U10 },
	{ "A16L16", D3DFMT_A16L16 },
	{ "LIN_A16L16", D3DFMT_LIN_A16L16 },
	{ "G16R16", D3DFMT_G16R16 },
	{ "LIN_G16R16", D3DFMT_LIN_G16R16 },
	{ "V16U16", D3DFMT_V16U16 },
	{ "LIN_V16U16", D3DFMT_LIN_V16U16 },
	{ "R10G11B11", D3DFMT_R10G11B11 },
	{ "LIN_R10G11B11", D3DFMT_LIN_R10G11B11 },
	{ "R11G11B10", D3DFMT_R11G11B10 },
	{ "LIN_R11G11B10", D3DFMT_LIN_R11G11B10 },
	{ "W10V11U11", D3DFMT_W10V11U11 },
	{ "LIN_W10V11U11", D3DFMT_LIN_W10V11U11 },
	{ "W11V11U10", D3DFMT_W11V11U10 },
	{ "LIN_W11V11U10", D3DFMT_LIN_W11V11U10 },
	{ "G16R16F", D3DFMT_G16R16F },
	{ "LIN_G16R16F", D3DFMT_LIN_G16R16F },
	{ "G16R16F_EXPAND", D3DFMT_G16R16F_EXPAND },
	{ "LIN_G16R16F_EXPAND", D3DFMT_LIN_G16R16F_EXPAND },
	{ "L32", D3DFMT_L32 },
	{ "LIN_L32", D3DFMT_LIN_L32 },
	{ "R32F", D3DFMT_R32F },
	{ "LIN_R32F", D3DFMT_LIN_R32F },
	{ "D24S8", D3DFMT_D24S8 },
	{ "LIN_D24S8", D3DFMT_LIN_D24S8 },
	{ "D24X8", D3DFMT_D24X8 },
	{ "LIN_D24X8", D3DFMT_LIN_D24X8 },
	{ "D24FS8", D3DFMT_D24FS8 },
	{ "LIN_D24FS8", D3DFMT_LIN_D24FS8 },
	{ "D32", D3DFMT_D32 },
	{ "LIN_D32", D3DFMT_LIN_D32 },

	//64bpp:
	{ "A16B16G16R16", D3DFMT_A16B16G16R16 },
	{ "LIN_A16B16G16R16", D3DFMT_LIN_A16B16G16R16 },
	{ "Q16W16V16U16", D3DFMT_Q16W16V16U16 },
	{ "LIN_Q16W16V16U16", D3DFMT_LIN_Q16W16V16U16 },
	{ "A16B16G16R16F", D3DFMT_A16B16G16R16F },
	{ "LIN_A16B16G16R16F", D3DFMT_LIN_A16B16G16R16F },
	{ "A16B16G16R16F_EXPAND", D3DFMT_A16B16G16R16F_EXPAND },
	{ "LIN_A16B16G16R16F_EXPAND", D3DFMT_LIN_A16B16G16R16F_EXPAND },
	{ "A32L32", D3DFMT_A32L32 },
	{ "LIN_A32L32", D3DFMT_LIN_A32L32 },
	{ "G32R32", D3DFMT_G32R32 },
	{ "LIN_G32R32", D3DFMT_LIN_G32R32 },
	{ "V32U32", D3DFMT_V32U32 },
	{ "LIN_V32U32", D3DFMT_LIN_V32U32 },
	{ "G32R32F", D3DFMT_G32R32F },
	{ "LIN_G32R32F", D3DFMT_LIN_G32R32F },

	//128bpp:
	{ "A32B32G32R32", D3DFMT_A32B32G32R32 },
	{ "LIN_A32B32G32R32", D3DFMT_LIN_A32B32G32R32 },
	{ "Q32W32V32U32", D3DFMT_Q32W32V32U32 },
	{ "LIN_Q32W32V32U32", D3DFMT_LIN_Q32W32V32U32 },
	{ "A32B32G32R32F", D3DFMT_A32B32G32R32F },
	{ "LIN_A32B32G32R32F", D3DFMT_LIN_A32B32G32R32F },
};

// Calculates the amount of memory used by a surface.
size_t NglXeTex::GetSurfaceBytes(int format, size_t w, size_t h)
{
	return XGSetTextureHeader((UINT)w, (UINT)h, 1, 0, (D3DFORMAT)format, 0, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0, NULL, NULL, NULL);
}

int NglXeTex::GetD3DFormat(const char* id)
{
	for (size_t i = 0; i < tl_elementsof(XenonTexelFormats); i++)
		if (_stricmp(id, XenonTexelFormats[i].name) == 0)
			return XenonTexelFormats[i].format;
	assert(false);
	return 0;
}

const char* NglXeTex::GetD3DFormatName(int format)
{
	for (size_t i = 0; i < tl_elementsof(XenonTexelFormats); i++)
		if (XenonTexelFormats[i].format == format)
			return XenonTexelFormats[i].name;
	assert(false);
	return "invalid";
}

class XenonTexelFormat : public TexelFormat
{
public:
	D3DFORMAT format;
	const char* id;
	NglXeTex* driver;

	XenonTexelFormat(NglXeTex* _driver, D3DFORMAT _format, const char* _id) 
		: driver(_driver), format(_format), id(_id) {}

	void Copy(Surface* aDstSurface, const Surface* aSrcSurface, D3DFORMAT Format, int BPP) const
	{
		IDirect3DTexture9 TempTex;
		UINT Size;

		// Create a temp texture with the xenon's pitch.
		XGSetTextureHeader(aDstSurface->GetWidth(), aDstSurface->GetHeight(), 1, 0, format, 0, 0, 0, 0, &TempTex, &Size, NULL);

		XGTEXTURE_DESC desc;
		XGGetTextureDesc(&TempTex, 0, &desc);

		// Make the destination little endian and linear, it will get fixed in SerialOutTexture.
		D3DFORMAT fmt = (D3DFORMAT)(format & ~(D3DFORMAT_ENDIAN_MASK|D3DFORMAT_TILED_MASK));
		void* Buf = malloc(Size);

		D3DFORMAT SrcFmt = (D3DFORMAT)(Format & ~(D3DFORMAT_ENDIAN_MASK|D3DFORMAT_TILED_MASK));

		if (XGCopySurface(Buf, desc.RowPitch, desc.Width, desc.Height, fmt, NULL, aSrcSurface->GetTexels(), BPP*desc.Width, SrcFmt, NULL, 0, 0.5f) != S_OK)
		{
			// Currently only LIN_DXT3A_1111 fails here.
			memset(Buf, 0, Size);
		}

		// Copy from the temp texture to the destination surface.
		u32 SrcPitch = desc.RowPitch;
		u32 DstPitch = desc.WidthInBlocks * desc.BytesPerBlock;

		for (uint y = 0; y < desc.HeightInBlocks; y++)
			memcpy(aDstSurface->GetTexels() + y * DstPitch, (char*)Buf + y * SrcPitch, SrcPitch);

		free(Buf);

		// hack up dest surface based on the actual # of bytes
		assert(aDstSurface->GetSize() == driver->GetSurfaceBytes(format, aDstSurface->GetWidth(), aDstSurface->GetHeight()));
		//aDstSurface->theSize = driver->GetSurfaceBytes(format, aDstSurface->GetWidth(), aDstSurface->GetHeight());
	}

	void CopyFromA8R8G8B8(Surface* aDstSurface, const Surface* aSrcSurface) const
	{
		Copy(aDstSurface, aSrcSurface, D3DFMT_A8R8G8B8, 4);
	}

	void CopyFromA32R32G32B32F(Surface* aDstSurface, const Surface* aSrcSurface) const
	{
		D3DFORMAT Fmt = (D3DFORMAT)MAKED3DFMT2(GPUTEXTUREFORMAT_32_32_32_32_FLOAT, GPUENDIAN_NONE, FALSE, 
		                                       GPUSIGN_SIGNED, GPUSIGN_SIGNED, GPUSIGN_SIGNED, GPUSIGN_SIGNED,
		                                       GPUNUMFORMAT_INTEGER, GPUSWIZZLE_Y, GPUSWIZZLE_Z, GPUSWIZZLE_W, GPUSWIZZLE_X);

		Copy(aDstSurface, aSrcSurface, Fmt, 16);
	}

	void CopyToA8R8G8B8(Surface* aDstSurface, const Surface* aSrcSurface) const
	{
		assert(false);
	}

	size_t Bytes(size_t w, size_t h) const { return driver->GetSurfaceBytes(format, w, h); }
	const char* GetId() const { return id; }
	TexelFormat* Clone() const { return new XenonTexelFormat(driver, format, id); }

	Class GetClass() const { return CLASS_INTEGER; }
};

TexelFormat* NglXeTex::GetTexelFormat(int format)
{
	for (size_t i = 0; i < tl_elementsof(XenonTexelFormats); i++)
		if (XenonTexelFormats[i].format == format)
			return new XenonTexelFormat(this, XenonTexelFormats[i].format, XenonTexelFormats[i].name);
	assert(false);
	return NULL;
}

TexelFormat* NglXeTex::GetTexelFormat(const char* format)
{
	for (size_t i = 0; i < tl_elementsof(XenonTexelFormats); i++)
		if (_stricmp(XenonTexelFormats[i].name, format) == 0)
			return new XenonTexelFormat(this, XenonTexelFormats[i].format, XenonTexelFormats[i].name);
	assert(false);
	return NULL;
}

void WritePhysicalData(void* DestBuffer, D3DBaseTexture* D3DTexture, const Texture* aTexture, 
                       int ArraySize, int NumLevels, int BaseSize, int MipSize)
{
	void* data_start = DestBuffer;

	// If we have no mips then we do not need to perform mip tail packing at all,
	// regardless of the size of the texture.
	unsigned tail_level = 255;
	if ( 1 != NumLevels )
		tail_level = XGGetMipTailBaseLevel( aTexture->GetWidth(), aTexture->GetHeight(), FALSE );

	XGTEXTURE_DESC base_desc;
	XGGetTextureDesc( D3DTexture, 0, &base_desc );
	DWORD gpu_format = (base_desc.Format & D3DFORMAT_TEXTUREFORMAT_MASK) >> D3DFORMAT_TEXTUREFORMAT_SHIFT;
	D3DFORMAT D3DFormat = base_desc.Format;

	u8* buffer = new u8[ base_desc.SlicePitch ]; // buffer to hold the linear surfaces.
	for ( int i_face = 0; i_face < ArraySize; ++i_face )
	{
		u8* dest_ptr = (u8*)data_start;

		for ( int i_mip = 0; i_mip < NumLevels; ++i_mip )
		{
			const u8* src_ptr = const_cast<Texture*>(aTexture)->GetMipmap( i_face, i_mip )->GetTexels();

			XGTEXTURE_DESC desc;
			XGGetTextureDesc( D3DTexture, i_mip, &desc );

			// Determine the flags for XGCopySurface.
			DWORD flags = 0;

			if ( desc.Format == D3DFMT_UYVY || desc.Format == D3DFMT_LIN_UYVY ||
			     desc.Format == D3DFMT_YUY2 || desc.Format == D3DFMT_LIN_YUY2 )
			{
				flags = XGCOMPRESS_YUV_DESTINATION;
			}

			// Intermediate textures are linear and little endian.
			D3DFORMAT src_format = (D3DFORMAT)(D3DFormat & ~(D3DFORMAT_ENDIAN_MASK|D3DFORMAT_TILED_MASK));

			// Make sure the destination format is linear to keep XGCopySurface happy.
			D3DFORMAT dst_format = (D3DFORMAT)(desc.Format & ~D3DFORMAT_TILED_MASK);

			uint offset = 0;
			if ( (int)tail_level <= i_mip )
			{
				offset = XGGetMipTailLevelOffset( base_desc.Width, base_desc.Height, base_desc.Depth,
				                                  i_mip, gpu_format, (D3DFormat & D3DFORMAT_TILED_MASK) != 0, FALSE );
			}
			// YUV textures don't actually require an endian swap, despite what the D3DFORMAT claims.
			if ( XGCOMPRESS_YUV_DESTINATION == flags )
			{
				dst_format = (D3DFORMAT)((dst_format & ~D3DFORMAT_ENDIAN_MASK));
			}

#if 0
			// Do the format conversion.
			if (src_format == D3DFMT_LIN_A8 && dst_format == D3DFMT_LIN_A8)
			{
				// Bug in XGCopySurface (apparently will be fixed in November XDK)
				// Copy manually for now.
				for (uint y=0;y<desc.Height;y++)
				{
					memcpy(
						buffer + offset + y*desc.WidthInBlocks,
						src_ptr + y*desc.WidthInBlocks, 
						desc.WidthInBlocks);
				}
			} else {
				if (XGGetGpuFormat(src_format) == XGGetGpuFormat(dst_format) &&
				    (D3DFMT_LIN_DXT1 == dst_format || D3DFMT_LIN_DXT5 == dst_format ))
				{
					// XGCopySurface will recompress the texture even if the source and destination formats are the same,
					// so manually copy it and endian swap if the GPU format is the same (currently always true).
					u32 SrcPitch = desc.WidthInBlocks * desc.BytesPerBlock;
					u32 DstPitch = desc.RowPitch;

					for (uint y = 0; y < desc.HeightInBlocks; y++)
					{
						memcpy(buffer + offset + y * DstPitch, src_ptr + y * SrcPitch, SrcPitch);
					}

					if ((dst_format & D3DFORMAT_ENDIAN_MASK) >> D3DFORMAT_ENDIAN_SHIFT != GPUENDIAN_NONE)
					{
						XGEndianSwapSurface(buffer + offset, desc.RowPitch, buffer + offset, desc.RowPitch,
						                    desc.Width, desc.Height, dst_format);
					}
				}
				else
				{
					XGCopySurface( buffer + offset, desc.RowPitch, desc.Width, desc.Height, dst_format, NULL, 
					               src_ptr, desc.WidthInBlocks * desc.BytesPerBlock, src_format, 
					               NULL, flags, 0.5f );
				}
			}
#else
			{
				// XGCopySurface will recompress the texture even if the source and destination formats are the same,
				// so manually copy it and endian swap if the GPU format is the same (currently always true).
				u32 SrcPitch = desc.WidthInBlocks * desc.BytesPerBlock;
				u32 DstPitch = desc.RowPitch;

				for (uint y = 0; y < desc.HeightInBlocks; y++)
				{
					memcpy(buffer + offset + y * DstPitch, src_ptr + y * SrcPitch, SrcPitch);
				}

				if ((dst_format & D3DFORMAT_ENDIAN_MASK) >> D3DFORMAT_ENDIAN_SHIFT != GPUENDIAN_NONE)
				{
					XGEndianSwapSurface(buffer + offset, desc.RowPitch, buffer + offset, desc.RowPitch,
					                    desc.Width, desc.Height, dst_format);
				}
			}

#endif

			if ( i_mip < (int)tail_level )
			{
				dest_ptr += desc.SlicePitch * i_face;

				// Tile the destination pixels, if necessary.
				if ( D3DFormat & D3DFORMAT_TILED_MASK )
				{
					if ( D3DTexture->GetType() == D3DRTYPE_VOLUMETEXTURE )
					{
						// Set volume to be tiled
						D3DBOX box = { 0, 0, desc.WidthInBlocks, desc.HeightInBlocks, 0, desc.DepthInBlocks };

						XGTileVolume( dest_ptr, desc.WidthInBlocks, desc.HeightInBlocks, desc.DepthInBlocks, NULL,
						              buffer, desc.RowPitch, desc.SlicePitch, &box, desc.BytesPerBlock );
					}
					else
					{
						// Set area to be tiled
						POINT point = { 0, 0 };
						RECT rect  = { 0, 0, desc.WidthInBlocks, desc.HeightInBlocks };
						::SetRect( &rect, 0, 0, desc.WidthInBlocks, desc.HeightInBlocks);

						XGTileSurface( dest_ptr, desc.RowPitch / desc.BytesPerBlock, desc.HeightInBlocks, &point,
						               buffer, desc.RowPitch, &rect, desc.BytesPerBlock );
					}
				}
				else
				{
					memcpy( dest_ptr, buffer, desc.SlicePitch );
				}

				// Make sure we don't overrun the buffer.
//				asserte( (desc.SlicePitch <= size, "Buffer overrun.") );

				dest_ptr += desc.SlicePitch * ( ArraySize - i_face );
//				size -= desc.SlicePitch;
			}

//			src_ptr += desc.WidthInBlocks * desc.HeightInBlocks * desc.BytesPerBlock;
		}

		// Tile the mip tail, if necessary.
		if ( (int)tail_level < NumLevels )
		{
			XGTEXTURE_DESC desc;
			XGGetTextureDesc( D3DTexture, tail_level, &desc );

			if ( D3DFormat & D3DFORMAT_TILED_MASK )
			{
				u32 block_width, block_height;
				XGGetBlockDimensions( gpu_format, &block_width, &block_height );

				u32 tile_width = GPU_TEXTURE_TILE_DIMENSION * block_width;
				u32 tile_height = GPU_TEXTURE_TILE_DIMENSION * block_height;
				u32 tail_width_in_blocks  = ((desc.Width  + tile_width  - 1) & ~(tile_width  - 1)) / block_width;
				u32 tail_height_in_blocks = ((desc.Height + tile_height - 1) & ~(tile_height - 1)) / block_height;

				// Set area to be tiled
				POINT point = { 0, 0 };
				RECT rect = { 0, 0, tail_width_in_blocks, tail_height_in_blocks };
				::SetRect( &rect, 0, 0, tail_width_in_blocks, tail_height_in_blocks );

				dest_ptr += desc.SlicePitch * i_face;
				XGTileSurface( dest_ptr, tail_width_in_blocks, tail_height_in_blocks, &point,
				               buffer, desc.RowPitch, &rect, desc.BytesPerBlock);
				dest_ptr += desc.SlicePitch * ( ArraySize - i_face );
			}
			else
			{
				memcpy( dest_ptr, buffer, desc.SlicePitch );
			}
//			size -= desc.SlicePitch;
		}
	}

	delete buffer;
}

ACTRESULT NglXeTex::SerialOutTexture(Sys::I_BinaryStream* stream, const Texture* aTexture, OutputType outputType)
{
	Sys::BinarySerialiser<Ut::NativeToBigEndian> ar(*stream);
	Texture tmpTexture;

	/*if (Texture::ANIMATED == aTexture->GetType())
	{
		assert(0);
//		return SerialOutAnimTexture(aPackFile, aTexture, NGL_APK_TEXTURE_VERSION, IFL_PLAT_XENON | IFL_ENDIANSWAP);
		return ACT_ERROR;
	}*/

	gpuTexture tex;
	int flags = 0;

	D3DFORMAT D3DFormat;
	if (!stricmp("R8G8B8", aTexture->GetFormat()->GetId()))
	{
		// 2006.10.18 KGT: a 24bit R8G8B8 is not supported on Xenon.
		//                 to handle properly DDS or other 24bpp RGB texture we will convert the surfaces to A8R8G8B8 and
		//                 serailize them as a 32bpp D3DFMT_X8R8G8B8 Xenon format
		tmpTexture.CopyFrom(*aTexture);
		ActTools::A8R8G8B8 a8r8g8b8;
		tmpTexture.ConvertTo(&a8r8g8b8);

		aTexture = &tmpTexture;
		D3DFormat = D3DFMT_X8R8G8B8;
	}
	else
	{
		D3DFormat = (D3DFORMAT)GetD3DFormat(aTexture->GetFormat()->GetId());
	}

	if (XGIsTiledFormat(D3DFormat))
		flags |= NGLTEX_SWIZZLED;

	int NMipMaps = aTexture->GetNumMipMaps();
	u32 BaseSize, MipSize;

	u32 Usage = (Flags & NGL_XETEX_CACHED_MEMORY) ? D3DUSAGE_CPU_CACHED_MEMORY : 0;

	// Fill in the texture header.
	int ArraySize=0;
	u32 size=0;
	u32 sizeUsed=0;
	if (aTexture->GetType() == Texture::TEXTURE)
	{
		size = sizeUsed = XGSetTextureHeader(aTexture->GetWidth(), aTexture->GetHeight(), NMipMaps, Usage, D3DFormat, (D3DPOOL)0, 0,
		                          XGHEADER_CONTIGUOUS_MIP_OFFSET, 0, (D3DTexture*)&tex, &BaseSize, &MipSize);

		S32 base = XGGetMipTailBaseLevel(aTexture->GetWidth(), aTexture->GetHeight(), 0);

		if (NMipMaps > base && aTexture->GetHeight() < aTexture->GetWidth() * 4)
		{
			if (D3DFormat == D3DFMT_DXT1)
				sizeUsed -= 4096;
			else if (D3DFormat == D3DFMT_DXT5)
				sizeUsed -= 8192;
		}

		ArraySize = 1;
	}
	else if (aTexture->GetType() == Texture::CUBETEXTURE)
	{
		size = sizeUsed = XGSetCubeTextureHeader(aTexture->GetWidth(), NMipMaps, Usage, D3DFormat, (D3DPOOL)0, 0,
		                              XGHEADER_CONTIGUOUS_MIP_OFFSET, (D3DCubeTexture*)&tex, &BaseSize, &MipSize);

		ArraySize = 6;
		flags |= NGLTEX_CUBE;
	}
	else if (aTexture->GetType() == Texture::VOLUMETEXTURE)
	{
		if (NMipMaps != 1)
		{
			// Volume texture mipmaps aren't supported yet.
			NMipMaps = 1;
		}

		D3DFormat = (D3DFORMAT)(D3DFormat & ~D3DFORMAT_TILED_MASK); // XGTileVolume isn't working right now.

		size = sizeUsed = XGSetVolumeTextureHeader(aTexture->GetWidth(), aTexture->GetHeight(), aTexture->GetDepth(), NMipMaps, Usage, D3DFormat, (D3DPOOL)0, 0,
		                                XGHEADER_CONTIGUOUS_MIP_OFFSET, (D3DVolumeTexture*)&tex, &BaseSize, &MipSize);
		ArraySize = aTexture->GetDepth();
		flags |= NGLTEX_VOLUME;
	}
	else
	{
		LOGERROR( TextureExport::g_Log, "Unknown texture type!");
		return ACT_ERROR;
	}

	// Allocate memory for the texture data.
	int iAlignment;
	if (Flags & NGL_XETEX_CACHED_MEMORY)
		iAlignment = GPU_TEXTURE_ALIGNMENT;
	else
	{
		if (Flags & NGL_XETEX_LARGE_PAGES)
			iAlignment = 64*1024;
		else
			iAlignment = 4*1024;
	}

	if (aTexture->GetSRGBFlag())
	{
		tex.tex.base.Format.SignX = 3;
		tex.tex.base.Format.SignY = 3;
		tex.tex.base.Format.SignZ = 3;
	}

	if (outputType != JUST_DATA)
	{
		// endian-swap the header (gpuTexture) and write it out
		for (U32 *p=(U32*)&tex; p<(U32*)((&tex)+1); p++)
			ar << *p;

		ar << sizeUsed << iAlignment << flags;
	}

	if (aTexture->GetSRGBFlag())
	{
		tex.tex.base.Format.SignX = 0;
		tex.tex.base.Format.SignY = 0;
		tex.tex.base.Format.SignZ = 0;
	}

	if (outputType != JUST_HEADER)
	{
		// Write out the actual data
		U8 *Data = new U8[size];
		memset(Data, 0xcd, size);
		WritePhysicalData(Data, &tex.tex.base, aTexture, ArraySize, NMipMaps, BaseSize, MipSize);

		//for (int i = sizeUsed; i < size; i++)
		//	assert(Data[i] == 0xcd);

		ar.Write(Data, sizeUsed);
		delete Data;
	}

	return ACT_OK;
}

ACTRESULT NglXeTex::SerialInTexture(Texture* aTexture, Sys::I_BinaryStream* stream) 
{
	Sys::BinaryDeserialiser<Ut::NativeToBigEndian> ar(*stream);
	ACTRESULT err = ACT_OK;

	gpuTexture tex;
	// read the header (gpuTexture)
	for (U32 *p=(U32*)&tex; p<(U32*)((&tex)+1); p++)
		ar >> *p;

	int size, alignment, flags;
	ar >> size >> alignment >> flags;

	// Second is the D3DTexture
	IDirect3DBaseTexture9* gpu_texture = &tex.tex.base;
	XGTEXTURE_DESC base_desc;
	XGGetTextureDesc(gpu_texture, 0, &base_desc);
	DWORD gpu_format = XGGetGpuFormat(base_desc.Format);
	DWORD num_faces = base_desc.Depth;
	DWORD num_mipmaps = gpu_texture->GetLevelCount();

	u8* data_buffer = new u8[size];
	u8* data_start = data_buffer;
	ar.Read(data_buffer, size);

	ActTools::A8B8G8R8 a8r8g8b8;
	if ( D3DRTYPE_TEXTURE == base_desc.ResourceType )
	{		
		assert( 1 == base_desc.Depth );
		aTexture->CreateTexture(&a8r8g8b8, base_desc.Width, base_desc.Height, num_mipmaps);
	}
	else if ( D3DRTYPE_CUBETEXTURE == base_desc.ResourceType )
		aTexture->CreateCubeTexture(&a8r8g8b8, base_desc.Width, num_mipmaps);
	else if ( D3DRTYPE_VOLUMETEXTURE == base_desc.ResourceType )
		aTexture->CreateVolumeTexture(&a8r8g8b8, base_desc.Width, base_desc.Height, base_desc.Depth, num_mipmaps);
	else
	{
		assert(0);
		return ACT_ERROR;
	}

	// If we have no mips then we do not need to perform mip tail packing at all,
	// regardless of the size of the texture.
	unsigned tail_level = 255;
	if (1 != num_mipmaps)
		tail_level = XGGetMipTailBaseLevel(base_desc.Width, base_desc.Height, FALSE);

	// Untile mips levels
	u8* src_ptr;
	u8* dst_ptr;
	for (unsigned i_face = 0; i_face < num_faces; ++i_face)
	{
		dst_ptr = (u8*)data_buffer;
		src_ptr = (u8*)data_start;
		for( unsigned i_mip = 0; i_mip < num_mipmaps; ++i_mip )
		{
			XGTEXTURE_DESC desc;
			XGGetTextureDesc(gpu_texture, i_mip, &desc);
			if (i_mip < tail_level)
			{
				src_ptr += desc.SlicePitch * i_face;
				dst_ptr += desc.SlicePitch * i_face;
				// Tile the destination pixels, if necessary.
				if ( desc.Format & D3DFORMAT_TILED_MASK )
				{
					if (flags & NGLTEX_VOLUME)
					{
						// Set volume to be tiled
						D3DBOX box = { 0, 0, desc.WidthInBlocks, desc.HeightInBlocks, 0, desc.DepthInBlocks };

	//						XGUntileVolume(buffer, dest_ptr, desc.WidthInBlocks, desc.HeightInBlocks, desc.DepthInBlocks, NULL,
	//						             buffer, desc.RowPitch, desc.SlicePitch, &box, desc.BytesPerBlock);
	//						XGTileVolume(dest_ptr, desc.WidthInBlocks, desc.HeightInBlocks, desc.DepthInBlocks, NULL,
	//						             buffer, desc.RowPitch, desc.SlicePitch, &box, desc.BytesPerBlock);
					}
					else
					{
						// Set area to be tiled
						POINT point = { 0, 0 };
						RECT rect  = { 0, 0, desc.WidthInBlocks, desc.HeightInBlocks };
						::SetRect(&rect, 0, 0, desc.WidthInBlocks, desc.HeightInBlocks);

						XGUntileSurface( dst_ptr, desc.RowPitch, &point,
						                 src_ptr, desc.RowPitch / desc.BytesPerBlock, desc.HeightInBlocks, &rect, desc.BytesPerBlock );
					}
				}
				else
				{
					memcpy( dst_ptr, src_ptr, desc.SlicePitch );
				}

				// Make sure we don't overrun the buffer.
				assert(((int)desc.SlicePitch <= size && "Buffer overrun."));

				src_ptr += desc.SlicePitch * (num_faces - i_face);
				dst_ptr += desc.SlicePitch * (num_faces - i_face);
				size -= desc.SlicePitch;
			}
		} // end of loop i_mip:=[0..num_mipmaps)

		// Untile the mip tail, if necessary.
		if ( tail_level < num_mipmaps )
		{
			XGTEXTURE_DESC desc;
			XGGetTextureDesc( gpu_texture, tail_level, &desc );
			DWORD gpu_format = XGGetGpuFormat(desc.Format);
			if ( base_desc.Format & D3DFORMAT_TILED_MASK )
			{
				u32 block_width, block_height;
				XGGetBlockDimensions( gpu_format, &block_width, &block_height );

				u32 tile_width = GPU_TEXTURE_TILE_DIMENSION * block_width;
				u32 tile_height = GPU_TEXTURE_TILE_DIMENSION * block_height;
				u32 tail_width_in_blocks  = ((desc.Width  + tile_width  - 1) & ~(tile_width  - 1)) / block_width;
				u32 tail_height_in_blocks = ((desc.Height + tile_height - 1) & ~(tile_height - 1)) / block_height;

				// Set area to be tiled
				POINT point = { 0, 0 };
				RECT rect = { 0, 0, tail_width_in_blocks, tail_height_in_blocks };
				::SetRect( &rect, 0, 0, tail_width_in_blocks, tail_height_in_blocks );

				dst_ptr += desc.SlicePitch * i_face;
				src_ptr += desc.SlicePitch * i_face;
				XGUntileSurface( dst_ptr, desc.RowPitch, &point, 
				                 src_ptr, tail_width_in_blocks, tail_height_in_blocks, &rect, desc.BytesPerBlock);
			}
			else
			{
				memcpy( dst_ptr, src_ptr, desc.SlicePitch );
			}
		}
	} // end of loop i_face:=[0..num_faces)

	for (unsigned i_face = 0; i_face < num_faces; ++i_face)
	{
		u8* src_ptr = (u8*)data_buffer;

		for(unsigned i_mip = 0; i_mip < num_mipmaps; ++i_mip)
		{
			XGTEXTURE_DESC desc;
			XGGetTextureDesc(gpu_texture, i_mip, &desc);

			u32 offset = 0;
			if (tail_level <= i_mip)
			{
				offset = XGGetMipTailLevelOffset( base_desc.Width, base_desc.Height, base_desc.Depth,
				                                  i_mip, gpu_format, 0 != (desc.Format & D3DFORMAT_TILED_MASK), FALSE );
			}


			ActTools::Surface* dst_sruface = aTexture->GetMipmap( i_face, i_mip );
			dst_sruface->Create( desc.Width, desc.Height );
			u8* dst_ptr = dst_sruface->GetTexels();
			// Do the format conversion.
			D3DFORMAT dst_format = (D3DFORMAT)(D3DFMT_LIN_A8B8G8R8 & ~(D3DFORMAT_ENDIAN_MASK|D3DFORMAT_TILED_MASK));
			D3DFORMAT src_format = (D3DFORMAT)(desc.Format & ~D3DFORMAT_TILED_MASK);
			XGCopySurface( dst_ptr, 4 * desc.Width, desc.Width, desc.Height, dst_format, NULL, 
			               src_ptr + offset, desc.RowPitch, src_format, NULL, 0, 0.5f);

			if ( i_mip < tail_level )
			{
				src_ptr += desc.SlicePitch * (num_faces - i_face);
			}
		}
	}

	delete data_buffer;

	return err;
}

