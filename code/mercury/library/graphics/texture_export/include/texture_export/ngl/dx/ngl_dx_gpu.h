/** \file ngl_dx_gpu.h
	DirectX GPU module.
	\see NGL_GPU
*/

#ifndef NGL_DX_GPU_H
#define NGL_DX_GPU_H


#if NGL_GPU_D3D8
// Compatibility layer to wrap some Xbox APIs into Xenon API format.

inline BOOL XGIsTiledFormat(D3DFORMAT Format)
{
	return XGIsSwizzledFormat(Format);
}

template<typename T>
inline VOID XGOffsetResourceAddress(T *Buf, void *Address)
{
	Buf->Data = (Buf->Data + (u32)Address) & 0x7fffffff;
}

inline VOID XGSetVertexBufferHeader(
  UINT Length,
  DWORD Usage,
  D3DPOOL Pool,
  UINT Offset,
  IDirect3DVertexBuffer8 *pVertexBuffer
)
{
	XGSetVertexBufferHeader(Length, Usage, 0, Pool, pVertexBuffer, Offset);
}

inline VOID WINAPI XGSetIndexBufferHeader(
  UINT Length,
  DWORD Usage,
  D3DFORMAT Format,
  D3DPOOL Pool,
  UINT Data,
  IDirect3DIndexBuffer8* pBuffer
)
{
	XGSetIndexBufferHeader(Length, Usage, Format, Pool, pBuffer, Data);
}


class gpuD3DDevice : public IDirect3DDevice8
{
public:
	inline HRESULT CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVertexBuffer, HANDLE* pSharedHandle)
	{
		return IDirect3DDevice8::CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer);
	}

	inline HRESULT CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer, HANDLE* pSharedHandle)
	{
		return IDirect3DDevice8::CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer);
	}

	inline HRESULT CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture, HANDLE* pSharedHandle)
	{
		return IDirect3DDevice8::CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture);
	}

	inline HRESULT CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8 **ppCubeTexture, HANDLE* pSharedHandle)
	{
		return IDirect3DDevice8::CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture);
	}

	inline HRESULT CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8** ppVolumeTexture, HANDLE* pSharedHandle)
	{
		return IDirect3DDevice8::CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture);
	}

	inline HRESULT CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface8** ppSurface, HANDLE* pSharedHandle)
	{
		return IDirect3DDevice8::CreateRenderTarget(Width, Height, Format, MultiSample, Lockable, ppSurface);
	}

	inline HRESULT CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface8** ppSurface, HANDLE* pSharedHandle)
	{
		return IDirect3DDevice8::CreateDepthStencilSurface(Width, Height, Format, MultiSample, ppSurface);
	}

	inline HRESULT SetVertexShaderConstantF(UINT StartRegister, CONST float *pConstantData, UINT Vector4fCount)
	{
		return IDirect3DDevice8::SetVertexShaderConstantFast(StartRegister, pConstantData, Vector4fCount);
	}

	inline HRESULT SetPixelShaderConstantF(UINT StartRegister, CONST float *pConstantData, UINT Vector4fCount)
	{
		return IDirect3DDevice8::SetPixelShaderConstant(StartRegister, pConstantData, Vector4fCount);
	}

	inline HRESULT SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
	{
		return SetTextureStageState(Sampler, (D3DTEXTURESTAGESTATETYPE)Type, Value);
	}

	inline HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration *pDecl)
	{
		return SelectVertexShaderDirect(pDecl, 0);
	}

	inline HRESULT SetPixelShader(DWORD* pShader)
	{
		return IDirect3DDevice8::SetPixelShaderProgram((D3DPIXELSHADERDEF*)pShader);
	}

	inline HRESULT SetIndices(IDirect3DIndexBuffer *pIndexData, UINT BaseVertexIndex = 0)
	{
		return IDirect3DDevice8::SetIndices(pIndexData, BaseVertexIndex);
	}

	inline HRESULT SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer *pStreamData, UINT OffsetInBytes, UINT Stride)
	{
		return IDirect3DDevice8::SetStreamSource(StreamNumber, pStreamData, Stride);
	}

	inline HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount)
	{
		return IDirect3DDevice8::DrawIndexedPrimitive(Type, MinIndex, NumVertices, StartIndex, PrimitiveCount);
	}

	inline HRESULT SetScissorRect(CONST RECT *pRect)
	{
		return SetScissors(1, FALSE, (D3DRECT*)pRect);
	}
};

#endif // NGL_GPU_D3D8


#if NGL_GPU_D3D_PC
#define SetRenderState_Inline SetRenderState
#define SetSamplerState_Inline SetSamplerState
#endif

// note- elements *must* be in persistent memory (global var or heap), the pointer is stored!
inline void gpuCreateVertexFormat(gpuVertexFormat& format, uint size, const gpuVertexElement* elements)
{
	format.VertexSize = size;
	format.Elements = elements;
#if NGL_GPU_D3D8
	format.VertexDeclaration = (IDirect3DVertexDeclaration*)tlMemAlloc(sizeof(D3DVERTEXATTRIBUTEFORMAT));
	gpuVertexElement *dst = &format.VertexDeclaration->Input[0];
	uint n = 0;

	// Copy elements across.
	while( elements[n].Format != D3DVSDT_NONE )
	{
		TL_ASSERT( n < 16, "Too many vertex elements." );
		dst[n] = elements[n];
		n++;
	}

	// Pad unused entries.
	while( n < 16 )
	{
		dst[n].StreamIndex = 0;
		dst[n].Offset = 0;
		dst[n].Format = D3DVSDT_NONE;
		dst[n].TessType = 0;
		dst[n].TessSource = 0;
		n++;
	}

#endif
#if NGL_GPU_D3D9
	format.VertexDeclaration = NULL;
	gpuDev->CreateVertexDeclaration(elements, &format.VertexDeclaration);
#endif
}

inline void gpuDestroyVertexFormat(gpuVertexFormat& format)
{
#if NGL_GPU_D3D8
	tlMemFree(format.VertexDeclaration);
#endif
#if NGL_GPU_D3D9
	format.VertexDeclaration->Release();
#endif
}

inline int gpuGetVertexElementSize(gpuVertexElementFormat Element)
{

#if NGL_GPU_D3D_PC
  switch (Element)
  {
  case D3DDECLTYPE_FLOAT1:
    return 4;
  case D3DDECLTYPE_FLOAT2:
    return 8;
  case D3DDECLTYPE_FLOAT3:
    return 12;
  case D3DDECLTYPE_FLOAT4:
    return 16;
  case D3DDECLTYPE_D3DCOLOR:
    return 4;
  case D3DDECLTYPE_UBYTE4:
    return 4;
  case D3DDECLTYPE_SHORT2:
    return 4;
  case D3DDECLTYPE_SHORT4:
    return 8;
  case D3DDECLTYPE_UBYTE4N:
    return 4;
  case D3DDECLTYPE_SHORT2N:
    return 4;
  case D3DDECLTYPE_SHORT4N:
    return 8;
  case D3DDECLTYPE_USHORT2N:
    return 4;
  case D3DDECLTYPE_USHORT4N:
    return 8;
  case D3DDECLTYPE_UDEC3:
    return 4;
  case D3DDECLTYPE_DEC3N:
    return 4;
  case D3DDECLTYPE_FLOAT16_2:
    return 4;
  case D3DDECLTYPE_FLOAT16_4:
    return 8;
  }
#endif

	return 0;
}

inline const gpuVertexElement* gpuFindVertexElement(gpuVertexFormat* fmt, gpuVertexElementUsage usage, uint usageindex)
{
#if NGL_GPU_D3D8
	return &fmt->Elements[usage];
#endif
#if NGL_GPU_D3D9
	const gpuVertexElement* elem;
	for (elem = fmt->Elements; GPU_VERTEX_ELEMENT_TYPE(elem) != GPU_VERTEX_UNUSED; elem++)
	{
		if (elem->Usage == usage && elem->UsageIndex == usageindex)
			return elem;
	}

	assert(0 && "Could not find requested vertex component.");
	return NULL;
#endif
}

inline void gpuCreateVertexBuffer(gpuVertexBuffer& buf, uint bytes, u32 flags, void* address)
{
#if NGL_GPU_D3D_PC
	address;
	gpuDev->CreateVertexBuffer(bytes, (flags & GPU_BUFFER_MAIN) ? /*D3DUSAGE_DYNAMIC*/0 : 0, 0, GPU_D3D_POOL, &buf.vb, NULL);
#endif

}

inline void gpuCreateVertexBuffer(gpuVertexBuffer& buf, uint nverts, const gpuVertexFormat* fmt, u32 flags, void* address)
{
	uint bytes = nverts*fmt->VertexSize;
	gpuCreateVertexBuffer(buf, bytes, flags, address);
}

inline void gpuDestroyVertexBuffer(gpuVertexBuffer& buf)
{
#if NGL_GPU_D3D_PC
	if( buf.vb )
	{
		buf.vb->Release();
		buf.vb = NULL;
	}
#endif

}

inline void* gpuMapVertexBuffer(gpuVertexBuffer& buf, uint offset, uint size)
{

#if NGL_GPU_D3D_PC
	IDirect3DLockType* data;
	buf.GetD3DVertexBuffer()->Lock(offset, size, &data, GPU_D3D_LOCKTYPE);
#endif

	return data;
}

inline void gpuUnmapVertexBuffer(gpuVertexBuffer& buf)
{
	buf.GetD3DVertexBuffer()->Unlock();
}

inline void* gpuVertexBuffer::GetAddress() const
{
#if NGL_GPU_D3D_PC
	return GetD3DVertexBuffer();
#endif

}
inline uint gpuVertexBuffer::GetSize() const
{ 
#if NGL_GPU_D3D_PC
	D3DVERTEXBUFFER_DESC desc;
	GetD3DVertexBuffer()->GetDesc(&desc);
	return desc.Size;
#endif

}

#if NGL_GPU_D3D_PC
inline D3DVertexBuffer* gpuVertexBuffer::GetD3DVertexBuffer() const { return vb; }
#endif


inline bool operator==(const gpuVertexBuffer& a, const gpuVertexBuffer& b)
{
	return a.GetD3DVertexBuffer() == b.GetD3DVertexBuffer();
}

inline void gpuCreateIndexBuffer(gpuIndexBuffer& buf, uint nindices, gpuIndexType indextype, u32 flags, void* address)
{
	uint indexsize = (indextype == GPU_INDEX_32) ? 4 : 2;

	address;
#if NGL_GPU_D3D_PC
	gpuDev->CreateIndexBuffer(nindices*indexsize, (flags & GPU_BUFFER_MAIN) ? /*D3DUSAGE_DYNAMIC*/0 : 0, (D3DFORMAT)indextype, GPU_D3D_POOL, &buf.ib, NULL);
	address;
#endif

}

inline void gpuDestroyIndexBuffer(gpuIndexBuffer& buf)
{
#if NGL_GPU_D3D_PC
	if( buf.ib )
	{
		buf.ib->Release();
		buf.ib = NULL;
	}
#endif

}

inline void* gpuIndexBuffer::GetAddress() const
{
#if NGL_GPU_D3D_PC
	return GetD3DIndexBuffer();
#endif

}
inline uint gpuIndexBuffer::GetSize() const
{ 
#if NGL_GPU_D3D_PC
	D3DINDEXBUFFER_DESC desc;
	GetD3DIndexBuffer()->GetDesc(&desc);
	return desc.Size;
#endif

}

inline void* gpuMapIndexBuffer(gpuIndexBuffer& buf, uint offset, uint size)
{

#if NGL_GPU_D3D_PC
	IDirect3DLockType* data;
	buf.GetD3DIndexBuffer()->Lock(offset, size, &data, GPU_D3D_LOCKTYPE);
#endif

	return data;
}

inline void gpuUnmapIndexBuffer(gpuIndexBuffer& buf)
{
	buf.GetD3DIndexBuffer()->Unlock();
}

#if NGL_GPU_D3D_PC
inline D3DIndexBuffer* gpuIndexBuffer::GetD3DIndexBuffer() const { return ib; }
#endif


// gpuSurface
#if NGL_GPU_D3D_PC
inline D3DSurface* gpuSurface::GetD3DSurface() { return surf; }
inline const D3DSurface* gpuSurface::GetD3DSurface() const { return surf; }
#endif


// gpuTexture
#if NGL_GPU_D3D_PC

inline D3DBaseTexture* gpuTexture::GetD3DBaseTexture() { return tex; }
inline D3DTexture* gpuTexture::GetD3DTexture() { return (D3DTexture*)tex; }
inline D3DCubeTexture* gpuTexture::GetD3DCubeTexture() { return (D3DCubeTexture*)tex; }
inline D3DVolumeTexture* gpuTexture::GetD3DVolumeTexture() { return (D3DVolumeTexture*)tex; }
inline const D3DBaseTexture* gpuTexture::GetD3DBaseTexture() const { return tex; }
inline const D3DTexture* gpuTexture::GetD3DTexture() const { return (D3DTexture*)tex; }
inline const D3DCubeTexture* gpuTexture::GetD3DCubeTexture() const { return (D3DCubeTexture*)tex; }
inline const D3DVolumeTexture* gpuTexture::GetD3DVolumeTexture() const { return (D3DVolumeTexture*)tex; }

inline int gpuTexture::GetWidth() const { return desc.Width; }
inline int gpuTexture::GetHeight() const { return desc.Height; }
inline int gpuTexture::GetDepth() const { return desc.Depth; }
inline int gpuTexture::GetLevels() const { return desc.Levels; }
inline gpuTextureFormat gpuTexture::GetFormat() const { return (gpuTextureFormat)desc.Format; }
inline gpuTextureType gpuTexture::GetType() const { return (gpuTextureType)desc.Type; }

#endif


inline bool gpuIsTextureFormatLinear(gpuTextureFormat format)
{

#if NGL_GPU_D3D_PC
	format;
	return true;
#endif
}

inline bool gpuIsTextureFormatSwizzled(gpuTextureFormat format)
{

#if NGL_GPU_D3D_PC
	format;
	return false;
#endif
}

inline bool gpuIsTextureFormatDXT(gpuTextureFormat format)
{

#if NGL_GPU_D3D_PC
	switch (format)
	{
	case D3DFMT_DXT1:
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		return true;

	default:
		return false;
	}
#endif
}

inline uint gpuGetTextureFormatBytesPerPixel(gpuTextureFormat format)
{

#if NGL_GPU_D3D_PC
	switch (format)
	{
	case D3DFMT_DXT1:
		return 8;
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		return 16;

	case D3DFMT_A8:
	case D3DFMT_P8:
	case D3DFMT_R3G3B2:
	case D3DFMT_L8:
	case D3DFMT_A4L4:
		return 1;

	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
	case D3DFMT_A8R3G3B2:
	case D3DFMT_X4R4G4B4:
	case D3DFMT_A8P8:
	case D3DFMT_A8L8:
	case D3DFMT_V8U8:
	case D3DFMT_L6V5U5:
	case D3DFMT_UYVY:
	case D3DFMT_R8G8_B8G8:
	case D3DFMT_YUY2:
	case D3DFMT_G8R8_G8B8:
	case D3DFMT_D16_LOCKABLE:
	case D3DFMT_D15S1:
	case D3DFMT_D16:
	case D3DFMT_L16:
	case D3DFMT_INDEX16:
	case D3DFMT_R16F:
	case D3DFMT_CxV8U8:
	case D3DFMT_MULTI2_ARGB8:
		return 2;

	case D3DFMT_R8G8B8:
		return 3;

	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A2B10G10R10:
	case D3DFMT_A8B8G8R8:
	case D3DFMT_X8B8G8R8:
	case D3DFMT_G16R16:
	case D3DFMT_A2R10G10B10:
	case D3DFMT_X8L8V8U8:
	case D3DFMT_Q8W8V8U8:
	case D3DFMT_V16U16:
	case D3DFMT_A2W10V10U10:
	case D3DFMT_D32:
	case D3DFMT_D24S8:
	case D3DFMT_D24X8:
	case D3DFMT_D24X4S4:
	case D3DFMT_D32F_LOCKABLE:
	case D3DFMT_D24FS8:
	case D3DFMT_INDEX32:
	case D3DFMT_G16R16F:
	case D3DFMT_R32F:
		return 4;

	case D3DFMT_A16B16G16R16:
	case D3DFMT_Q16W16V16U16:
	case D3DFMT_A16B16G16R16F:
	case D3DFMT_G32R32F:
		return 8;

	case D3DFMT_A32B32G32R32F:
		return 16;

	default:
		assert(0 && "Unknown surface format.");
		return 0;
	};
#endif
}

inline void gpuCreateTexture(gpuTexture& tex, uint width, uint height, uint levels, gpuTextureFormat format, u32 flags, void* address)
{
#if NGL_GPU_D3D_PC
	address;
	gpuSetTextureHeader(tex, width, height, levels, format, flags);
	gpuAllocTextureData(tex);
	address;
#endif

}

inline void gpuCreateCubeTexture(gpuTexture& tex, uint size, uint levels, gpuTextureFormat format, u32 flags, void* address )
{
#if NGL_GPU_D3D_PC
	address;
	gpuSetCubeTextureHeader(tex, size, levels, format, flags);
	gpuAllocTextureData(tex);
	address;
#endif

}

inline void gpuCreateVolumeTexture(gpuTexture& tex, uint width, uint height, uint depth, uint levels, gpuTextureFormat format, u32 flags, void* address)
{
#if NGL_GPU_D3D_PC
	address;
	gpuSetVolumeTextureHeader(tex, width, height, depth, levels, format, flags);
	gpuAllocTextureData(tex);
	address;
#endif

}

inline void gpuSetTextureHeader(gpuTexture& tex, uint width, uint height, uint levels, gpuTextureFormat format, u32 flags)
{
#if NGL_GPU_D3D_PC
	uint usage = (flags & GPU_TEXTURE_MAIN) ? /*D3DUSAGE_DYNAMIC*/0 : 0;

	if (flags & GPU_TEXTURE_RENDERTARGET)
		usage |= D3DUSAGE_RENDERTARGET;

	tex.desc.Width = width;
	tex.desc.Height = height;
	tex.desc.Depth = 1;
	tex.desc.Levels = levels;
	tex.desc.Format = format;
	tex.desc.Usage = usage;
	tex.desc.Type = GPU_TEXTURE_2D;
	tex.tex = NULL;
#endif

}

inline void gpuSetCubeTextureHeader(gpuTexture& tex, uint size, uint levels, gpuTextureFormat format, u32 flags)
{
#if NGL_GPU_D3D_PC
	uint usage = (flags & GPU_TEXTURE_MAIN) ? /*D3DUSAGE_DYNAMIC*/0 : 0;

	if (flags & GPU_TEXTURE_RENDERTARGET)
		usage |= D3DUSAGE_RENDERTARGET;

	tex.desc.Width = size;
	tex.desc.Height = size;
	tex.desc.Depth = 6;
	tex.desc.Levels = levels;
	tex.desc.Format = format;
	tex.desc.Usage = usage;
	tex.desc.Type = GPU_TEXTURE_CUBE;
	tex.tex = NULL;
#endif

}

inline void gpuSetVolumeTextureHeader(gpuTexture& tex, uint width, uint height, uint depth, uint levels, gpuTextureFormat format, u32 flags)
{
#if NGL_GPU_D3D_PC
	uint usage = (flags & GPU_TEXTURE_MAIN) ? /*D3DUSAGE_DYNAMIC*/0 : 0;

	if (flags & GPU_TEXTURE_RENDERTARGET)
		usage |= D3DUSAGE_RENDERTARGET;

	tex.desc.Width = width;
	tex.desc.Height = height;
	tex.desc.Depth = depth;
	tex.desc.Levels = levels;
	tex.desc.Format = format;
	tex.desc.Usage = usage;
	tex.desc.Type = GPU_TEXTURE_VOLUME;
	tex.tex = NULL;
#endif

}

inline void gpuAllocTextureData(gpuTexture& tex)
{
#if NGL_GPU_D3D_PC
	nglGpuAcquireDevice();

	D3DPOOL pool = (tex.desc.Usage & D3DUSAGE_RENDERTARGET) ? D3DPOOL_DEFAULT : GPU_D3D_POOL;
	gpuTextureType type = tex.GetType();

	if (type == GPU_TEXTURE_2D)
		gpuDev->CreateTexture(tex.desc.Width, tex.desc.Height, tex.desc.Levels, tex.desc.Usage, (D3DFORMAT)tex.desc.Format, pool, (IDirect3DTexture**)&tex.tex, NULL);
	else if (type == GPU_TEXTURE_CUBE)
		gpuDev->CreateCubeTexture(tex.desc.Width, tex.desc.Levels, tex.desc.Usage, (D3DFORMAT)tex.desc.Format, pool, (IDirect3DCubeTexture**)&tex.tex, NULL);
	else if (type == GPU_TEXTURE_VOLUME)
		gpuDev->CreateVolumeTexture(tex.desc.Width, tex.desc.Height, tex.desc.Depth, tex.desc.Levels, tex.desc.Usage, (D3DFORMAT)tex.desc.Format, pool, (IDirect3DVolumeTexture**)&tex.tex, NULL);

	nglGpuReleaseDevice();
#endif

}

inline void gpuFreeTextureData(gpuTexture& tex)
{
#if NGL_GPU_D3D_PC
	if (tex.tex)
		tex.tex->Release();
#endif

}

inline void gpuDestroyTexture(gpuTexture& tex)
{
	gpuFreeTextureData(tex);
}

inline void* gpuMapTexture(gpuTexture& tex, int face, int level)
{

#if NGL_GPU_D3D_PC
	D3DLOCKED_RECT Rect;
	D3DLOCKED_BOX Box;
	gpuTextureType type = tex.GetType();
	if (type == GPU_TEXTURE_2D)
	{
		tex.GetD3DTexture()->LockRect(level, &Rect, NULL, 0);
		return Rect.pBits;
	}
	else if (type == GPU_TEXTURE_CUBE)
	{
		tex.GetD3DCubeTexture()->LockRect((D3DCUBEMAP_FACES)face, level, &Rect, NULL, 0);
		return Rect.pBits;
	}
	else if (type == GPU_TEXTURE_VOLUME)
	{
		tex.GetD3DVolumeTexture()->LockBox(level, &Box, NULL, 0);
		return Box.pBits;
	}
	return NULL;
#endif
}

inline void gpuUnmapTexture(gpuTexture& tex, int face, int level)
{
#if NGL_GPU_D3D_PC
	gpuTextureType type = tex.GetType();
	if (type == GPU_TEXTURE_2D)
		tex.GetD3DTexture()->UnlockRect(level);
	else if (type == GPU_TEXTURE_CUBE)
		tex.GetD3DCubeTexture()->UnlockRect((D3DCUBEMAP_FACES)face, level);
	else if (type == GPU_TEXTURE_VOLUME)
		tex.GetD3DVolumeTexture()->UnlockBox(level);
#endif
}

inline void gpuSetSurfaceHeader(gpuSurface& surf, uint w, uint h, gpuSurfaceFormat format, gpuMultiSampleType multisample, u32 flags, void* address, void* hizaddress)
{
#if NGL_GPU_D3D_PC
	surf; w; h; format; multisample; flags; address; hizaddress;
#endif

	surf; w; h; format; multisample; flags; address; hizaddress;
}

inline bool gpuIsSurfaceFormatDepth(gpuSurfaceFormat format)
{
#if NGL_GPU_D3D_PC
	switch (format)
	{
	case GPU_SURFACE_D16_LOCKABLE:
	case GPU_SURFACE_D32:
	case GPU_SURFACE_D15S1:
	case GPU_SURFACE_D24S8:
	case GPU_SURFACE_D24X8:
	case GPU_SURFACE_D24X4S4:
	case GPU_SURFACE_D16:
	case GPU_SURFACE_D32F_LOCKABLE:
	case GPU_SURFACE_D24FS8:
		return true;
	}
#endif


	return false;
}

inline gpuSurfaceFormat gpuTextureFormatToSurfaceFormat(gpuTextureFormat format)
{
#if NGL_GPU_D3D_PC
	return (gpuSurfaceFormat)format;
#endif

}

inline gpuTextureFormat gpuSurfaceFormatToTextureFormat(gpuSurfaceFormat format)
{
	return (gpuTextureFormat)format;
}

inline void gpuCreateSurface(gpuSurface& surf, uint w, uint h, gpuSurfaceFormat format, gpuMultiSampleType multisample, u32 flags, void* address, void* hizaddress)
{
#if NGL_GPU_D3D_PC
	flags; address; hizaddress;
	if (gpuIsSurfaceFormatDepth(format))
	{
		nglGpuAcquireDevice();
		gpuDev->CreateDepthStencilSurface(w, h, (D3DFORMAT)format, (D3DMULTISAMPLE_TYPE)multisample, 0, 0, &surf.surf, NULL);
		nglGpuReleaseDevice();
	}
	else
	{
		nglGpuAcquireDevice();
		gpuDev->CreateRenderTarget(w, h, (D3DFORMAT)format, (D3DMULTISAMPLE_TYPE)multisample, 0, 0, &surf.surf, NULL);
		nglGpuReleaseDevice();
	}
#endif


}

inline void gpuResolveRenderTarget(uint idx, gpuTexture& dst, uint miplevel)
{
#if NGL_GPU_D3D_PC
	idx; dst; miplevel;
#endif

}

#if NGL_GPU_D3D
inline void gpuResolveDepthStencilTarget(gpuTexture& dst, uint miplevel)
{
#if NGL_GPU_D3D_PC
	dst; miplevel;
#endif

}
#endif

inline void gpuDestroySurface(gpuSurface& surf)
{
#if NGL_GPU_D3D_PC
	if (surf.GetD3DSurface())
		surf.GetD3DSurface()->Release();
#endif
}

inline void gpuCreateVertexShader(gpuVertexShader& shader, const void* microcode, const void* decl)
{
#if NGL_GPU_D3D_PC
	decl;
#endif

	nglGpuAcquireDevice();
#if NGL_GPU_D3D8
	gpuDev->CreateVertexShader((const DWORD*)decl, (const DWORD*)microcode, &shader, 0);
#endif
#if NGL_GPU_D3D9
	decl;
	gpuDev->CreateVertexShader((const DWORD*)microcode, &shader);
#endif
	nglGpuReleaseDevice();
}

inline void gpuDestroyVertexShader(gpuVertexShader& shader)
{
#if NGL_GPU_D3D8
	gpuDev->DeleteVertexShader(shader);
#endif
#if NGL_GPU_D3D9
	shader->Release();
#endif
}

inline void gpuCreatePixelShader(gpuPixelShader& shader, const void* microcode)
{
#if NGL_GPU_D3D8
	shader = (gpuPixelShader)microcode;
#endif
#if NGL_GPU_D3D9
	nglGpuAcquireDevice();
	gpuDev->CreatePixelShader((const DWORD*)microcode, &shader);
	nglGpuReleaseDevice();
#endif
}

inline void gpuDestroyPixelShader(gpuPixelShader& shader)
{
#if NGL_GPU_D3D9
	shader->Release();
#endif
}

// Shaders & shader parameters

#if NGL_GPU_D3D9
inline gpuParameter gpuGetParameterByName(void* function, const char* name)
{
	gpuParameter param = -1;
	ID3DXConstantTable* table;
	D3DXGetShaderConstantTable((const DWORD*)function, &table);
	D3DXHANDLE handle = table->GetConstantByName(NULL, name);
	if (handle)
	{
		D3DXCONSTANT_DESC desc;
		UINT Count = 1;
		table->GetConstantDesc(handle, &desc, &Count);
		param = desc.RegisterIndex;
	}
	else // Try as a sampler
	{
		UINT count = 0;
		D3DXGetShaderSamplers((const DWORD*)function, NULL, &count);
		const char** names = (const char**)LocalAlloc(0, count * sizeof(char*));
		D3DXGetShaderSamplers((const DWORD*)function, names, &count);
		for (UINT i = 0; i < count; i++)
			if (stricmp(names[i], name) == 0)
			{
				param = i;
				break;
			}
		LocalFree(names);
	}
	table->Release();
	return param;
}
#endif

// note- Not to be used in performance sensitive code, especially on D3D platforms.
#if NGL_GPU_D3D9
inline gpuParameter gpuGetParameterByName(gpuVertexShader& shader, const char* name)
{
	UINT size = 0;
	shader->GetFunction(NULL, &size);
	void* function = LocalAlloc(0, size);
	shader->GetFunction(function, &size);
	gpuParameter param = gpuGetParameterByName(function, name);
	LocalFree(function);
	return param;
}
#endif

// note- Not to be used in performance sensitive code, especially on D3D platforms.
inline gpuParameter gpuGetParameterByName(gpuPixelShader& shader, const char* name)
{
#if NGL_GPU_D3D8
	return GPU_INVALID_PARAM;
#endif
#if NGL_GPU_D3D9
	UINT size = 0;
	shader->GetFunction(NULL, &size);
	void* function = LocalAlloc(0, size);
	shader->GetFunction(function, &size);
	gpuParameter param = gpuGetParameterByName(function, name);
	LocalFree(function);
	return param;
#endif
}

// Vertex and Pixel shader render states
inline void gpuSetVertexShader(gpuVertexShader& shader)
{
	size_t hash = (size_t)shader;
	if (hash == gpuStateCache.VertexShader)
		return;

	gpuStateCache.VertexShader = hash;

#if NGL_GPU_D3D8
	gpuDev->LoadVertexShaderProgram((const DWORD *)shader, 0);

	// Load up some dummy streams just to initialize the programmable pipeline.
	static D3DVERTEXATTRIBUTEFORMAT Inputs =
	{{
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
		{ 0, 0, D3DVSDT_NONE, 0, 0 },
	}};

	gpuDev->SelectVertexShaderDirect(&Inputs, 0);
#endif
#if NGL_GPU_D3D9
	gpuDev->SetVertexShader(shader);
#endif
}

inline void gpuClearVertexShader()
{
	gpuStateCache.VertexShader = 0;
#if NGL_GPU_D3D8
#endif
#if NGL_GPU_D3D9
	gpuDev->SetVertexShader(NULL);
#endif
}

inline void gpuSetVertexConstant(gpuParameter idx, bool val)
{
#if NGL_GPU_D3D9
	BOOL b = (val != 0);
	gpuDev->SetVertexShaderConstantB(idx, &b, 1);
#endif
}

inline void gpuSetVertexConstant(gpuParameter idx, int val)
{
#if NGL_GPU_D3D9
	int vec[4] = { val, 0, 0, 0 };
	gpuDev->SetVertexShaderConstantI(idx, vec, 1);
#endif
}

inline void gpuSetVertexConstant(gpuParameter idx, float val)
{
#if NGL_GPU_D3D9
	float vec[4] = { val, 0.0f, 0.0f, 0.0f };
	gpuDev->SetVertexShaderConstantF(idx, vec, 1);
#endif
}

inline void gpuSetVertexConstant(gpuParameter idx, const gpuFloat4& val)
{
	gpuDev->SetVertexShaderConstantF(idx, (float*)&val, 1);
}

inline void gpuSetVertexConstant(gpuParameter idx, const gpuFloat4* data, uint nelements)
{
	gpuDev->SetVertexShaderConstantF(idx, (float*)data, nelements);
}

inline void gpuSetVertexConstant(gpuParameter idx, const float* data, uint nelements)
{
	gpuDev->SetVertexShaderConstantF(idx, data, nelements);
}

#if VMATHD

inline void gpuSetVertexConstant(gpuParameter idx, math::Vector4Arg val)
{
	gpuDev->SetVertexShaderConstantF(idx, (float*)&val, 1);
}

inline void gpuSetVertexConstant(gpuParameter idx, const math::Mat44& val)
{
	gpuDev->SetVertexShaderConstantF(idx, (float*)&val, 4);
}

inline void gpuSetVertexConstant(gpuParameter idx, const math::Vector4* data, uint nelements)
{
	gpuDev->SetVertexShaderConstantF(idx, (float*)const_cast<math::Vector4*>(data), nelements);
}

inline void gpuSetVertexConstant(gpuParameter idx, const math::Mat44* data, uint nelements)
{
	gpuDev->SetVertexShaderConstantF(idx, (float*)const_cast<math::Mat44*>(data), nelements*4);
}

#endif

#if NGL_GPU_D3D_PC
GPU_EXTERN gpuFloat4 gpuVertexConstantCache[512];
#endif

inline void* gpuBeginVertexConstant(gpuParameter idx, uint nelements)
{
#if NGL_GPU_D3D_PC
	nelements;
	return &gpuVertexConstantCache[idx];
#endif
}

inline void gpuEndVertexConstant(gpuParameter idx, uint nelements)
{
#if NGL_GPU_D3D_PC
	gpuSetVertexConstant(idx, &gpuVertexConstantCache[idx], nelements);
#endif
}

inline void gpuOwnVertexConstant(gpuParameter idx, uint nelements)
{
#if NGL_GPU_D3D_PC
	idx; nelements;
#endif

}

inline void gpuDisownVertexConstant(gpuParameter idx, uint nelements)
{
#if NGL_GPU_D3D_PC
	idx; nelements;
#endif

}

inline void gpuSetPixelShader(gpuPixelShader& shader)
{
	size_t hash = (size_t)shader;
	if (hash == gpuStateCache.PixelShader)
		return;

	gpuStateCache.PixelShader = hash;

	gpuDev->SetPixelShader(shader);
}

inline void gpuClearPixelShader()
{
	gpuStateCache.PixelShader = 0;
	gpuDev->SetPixelShader(NULL);
}

inline void gpuSetPixelConstant(gpuParameter idx, bool val)
{
#if NGL_GPU_D3D9
	BOOL b = (val != 0);
	gpuDev->SetPixelShaderConstantB(idx, &b, 1);
#endif
}

inline void gpuSetPixelConstant(gpuParameter idx, int val)
{
#if NGL_GPU_D3D9
	int vec[4] = { val, 0, 0, 0 };
	gpuDev->SetPixelShaderConstantI(idx, vec, 1);
#endif
}

inline void gpuSetPixelConstant(gpuParameter idx, float val)
{
	float vec[4] = { val, 0.0f, 0.0f, 0.0f };
	gpuDev->SetPixelShaderConstantF(idx, vec, 1);
}

inline void gpuSetPixelConstant(gpuParameter idx, const gpuFloat4& val)
{
	gpuDev->SetPixelShaderConstantF(idx, (float*)&val, 1);
}

inline void gpuSetPixelConstant(gpuParameter idx, const gpuFloat4* data, uint nelements)
{
	gpuDev->SetPixelShaderConstantF(idx, (float*)data, nelements);
}

inline void gpuSetPixelConstant(gpuParameter idx, const float* data, uint nelements)
{
	gpuDev->SetPixelShaderConstantF(idx, data, nelements);
}

#if VMATHD

inline void gpuSetPixelConstant(gpuParameter idx, math::Vector4Arg val)
{
	gpuDev->SetPixelShaderConstantF(idx, (float*)&val, 1);
}

inline void gpuSetPixelConstant(gpuParameter idx, const math::Mat44& val)
{
	gpuDev->SetPixelShaderConstantF(idx, (float*)&val, 4);
}

inline void gpuSetPixelConstant(gpuParameter idx, const math::Vector4* data, uint nelements)
{
	gpuDev->SetPixelShaderConstantF(idx, (float*)const_cast<math::Vector4*>(data), nelements);
}

inline void gpuSetPixelConstant(gpuParameter idx, const math::Mat44* data, uint nelements)
{
	gpuDev->SetPixelShaderConstantF(idx, (float*)const_cast<math::Mat44*>(data), nelements);
}

#endif

#if NGL_GPU_D3D_PC
GPU_EXTERN gpuFloat4 gpuPixelConstantCache[1024];
#endif

inline void* gpuBeginPixelConstant(gpuParameter idx, uint nelements)
{
#if NGL_GPU_D3D_PC
	nelements;
	return &gpuPixelConstantCache[idx];
#endif
}

inline void gpuEndPixelConstant(gpuParameter idx, uint nelements)
{
#if NGL_GPU_D3D_PC
	gpuSetPixelConstant(idx, &gpuPixelConstantCache[idx], nelements);
#endif
}

inline void gpuOwnPixelConstant(gpuParameter idx, uint nelements)
{
#if NGL_GPU_D3D_PC
	idx; nelements;
#endif

}

inline void gpuDisownPixelConstant(gpuParameter idx, uint nelements)
{
#if NGL_GPU_D3D_PC
	idx; nelements;
#endif

}

// Sampler functions

inline void gpuSetSamplerTexture(gpuSampler idx, gpuTexture& tex)
{
	D3DBaseTexture *base = tex.GetD3DBaseTexture();
	if (gpuStateCache.Texture[idx] == (size_t)base)
		return;

	gpuDev->SetTexture(idx, base);
	gpuStateCache.Texture[idx] = (size_t)base;
}

inline void gpuClearSamplerTexture(gpuSampler idx)
{
	gpuDev->SetTexture(idx, NULL);
	gpuStateCache.Texture[idx] = 0;
}

// note- Must be called after gpuSetTexture.
inline void gpuSetSamplerFilter(gpuSampler idx, gpuFilterMode min, gpuFilterMode mag, gpuFilterMode mip, uint maxanisotropy)
{
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_MINFILTER, min);
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_MAGFILTER, mag);
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_MIPFILTER, mip);
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_MAXANISOTROPY, maxanisotropy);
}

inline void gpuSetSamplerBorderColor(gpuSampler idx, uint color)
{
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_BORDERCOLOR, color);
}

// note- Must be called after gpuSetTexture.
inline void gpuSetSamplerWrap(gpuSampler idx, gpuAddressMode uwrap, gpuAddressMode vwrap)
{
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_ADDRESSU, uwrap);
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_ADDRESSV, vwrap);
}

// Functions for 3D samplers.
// note- Must be called after gpuSetTexture.
inline void gpuSetSamplerWrap3D(gpuSampler idx, gpuAddressMode uwrap, gpuAddressMode vwrap, gpuAddressMode wwrap)
{
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_ADDRESSU, uwrap);
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_ADDRESSV, vwrap);
	gpuDev->SetSamplerState_Inline(idx, D3DSAMP_ADDRESSW, wwrap);
}

inline void gpuSetVertexBuffer(gpuVertexBuffer& vtx, gpuVertexFormat* vertexformat, 
                               uint vtxoffset, uint streamidx)
{
	size_t hashbuf = (size_t)&vtx + (size_t)vtxoffset + (size_t)(streamidx << 16);
	size_t hashfmt = (size_t)(vertexformat->VertexDeclaration);

	if (hashbuf == gpuStateCache.VertexBuffer[streamidx] && hashfmt == gpuStateCache.VertexFormat)
		return;

#if NGL_GPU_D3D8
	D3DSTREAM_INPUT stream;
	stream.Offset = vtxoffset;
	stream.Stride = vertexformat->VertexSize;
	stream.VertexBuffer = vtx.GetD3DVertexBuffer();

	gpuDev->SetVertexShaderInputDirect(vertexformat->VertexDeclaration, 1, &stream);
#else
	if (hashfmt != gpuStateCache.VertexFormat)
		gpuDev->SetVertexDeclaration(vertexformat->VertexDeclaration);
	gpuDev->SetStreamSource(streamidx, vtx.GetD3DVertexBuffer(), vtxoffset, vertexformat->VertexSize);
#endif

	gpuStateCache.VertexBuffer[streamidx] = hashbuf;
	gpuStateCache.VertexFormat = hashfmt;
}

inline void gpuClearVertexBuffer(uint streamidx)
{
	gpuStateCache.VertexBuffer[streamidx] = 0;
	gpuStateCache.VertexFormat = 0;

#if NGL_GPU_D3D8
	gpuDev->SetVertexShaderInputDirect(NULL, 0, NULL);
#else
	gpuDev->SetStreamSource(streamidx, NULL, 0, 0);
	gpuDev->SetVertexDeclaration(NULL);
#endif
}

inline void gpuSetIndexBuffer(gpuIndexBuffer& idx)
{
	size_t hash = (size_t)&idx;
	if (hash == gpuStateCache.IndexBuffer)
		return;

	gpuStateCache.IndexBuffer = hash;

	gpuDev->SetIndices(idx.GetD3DIndexBuffer());
}

inline void gpuClearIndexBuffer()
{
	gpuStateCache.IndexBuffer = 0;

	gpuDev->SetIndices(NULL);
}

// Renders primitives from index and vertex buffers.
inline void gpuDrawIndexedPrimitive(
	gpuPrimType prim, uint nindices, uint idxoffset, gpuIndexBuffer& idx, gpuIndexType indexformat, 
	uint nverts, uint vtxoffset, gpuVertexBuffer& vtx, gpuVertexFormat* vertexformat)
{
	gpuSetVertexBuffer(vtx, vertexformat, vtxoffset);

	gpuSetIndexBuffer(idx);
	gpuDev->DrawIndexedPrimitive((D3DPRIMITIVETYPE)prim, 0, 0, nverts, idxoffset / (indexformat == GPU_INDEX_16 ? 2 : 4), gpuGetPrimCount(prim, nindices));
}

// Same as gpuDrawIndexedPrimitive, except draws inputs from memory without using vertex/index buffers.
inline void gpuDrawIndexedPrimitivePtr(
	gpuPrimType prim, uint nindices, void* idx, gpuIndexType indexformat, 
	uint nverts, void* vtx, gpuVertexFormat* vertexformat)
{
	gpuStateCache.IndexBuffer = 0;
	gpuStateCache.VertexBuffer[0] = 0;
	gpuStateCache.VertexFormat = 0;
	gpuDev->SetVertexDeclaration(vertexformat->VertexDeclaration);
	gpuDev->DrawIndexedPrimitiveUP(
		(D3DPRIMITIVETYPE)prim, 0, nverts, gpuGetPrimCount(prim, nindices), idx, (D3DFORMAT)indexformat, 
		vtx, vertexformat->VertexSize);
}

// Renders primitives from a vertex buffer.
inline void gpuDrawPrimitive(gpuPrimType prim, uint nverts, uint vtxoffset, gpuVertexBuffer& vtx, gpuVertexFormat* vertexformat)
{
	gpuSetVertexBuffer(vtx, vertexformat, vtxoffset);

	gpuDev->DrawPrimitive((D3DPRIMITIVETYPE)prim, 0, gpuGetPrimCount(prim, nverts));
}

// Renders primitives from a vertex buffer.
inline void gpuDrawPrimitivePtr(gpuPrimType prim, uint nverts, void* vtx, gpuVertexFormat* vertexformat)
{
	gpuStateCache.VertexBuffer[0] = 0;
	gpuStateCache.VertexFormat = 0;
	gpuDev->SetVertexDeclaration(vertexformat->VertexDeclaration);
	gpuDev->DrawPrimitiveUP((D3DPRIMITIVETYPE)prim, gpuGetPrimCount(prim, nverts), vtx, vertexformat->VertexSize);
}

inline void gpuSetPrimitiveResetIndex(uint index)
{
#if NGL_GPU_D3D_PC
	index;
#endif

}

inline void gpuDrawIndexedPrimitive(gpuPrimType prim, uint nindices, gpuIndexType indexformat, uint nverts)
{
#if NGL_GPU_D3D_PC
	indexformat;
#endif

	gpuDev->DrawIndexedPrimitive((D3DPRIMITIVETYPE)prim, 0, 0, nverts, 0, gpuGetPrimCount(prim, nindices));
}

inline void gpuDrawPrimitive(gpuPrimType prim, uint nverts)
{
	gpuDev->DrawPrimitive((D3DPRIMITIVETYPE)prim, 0, gpuGetPrimCount(prim, nverts));
}


inline void gpuSetCullMode(gpuCullMode cull)
{
	gpuDev->SetRenderState_Inline(D3DRS_CULLMODE, cull);
}

inline void gpuSetFillMode(gpuFillMode fill)
{
	gpuDev->SetRenderState_Inline(D3DRS_FILLMODE, fill);
}

inline void gpuSetShadeMode(gpuShadeMode shade)
{
	gpuDev->SetRenderState_Inline(D3DRS_SHADEMODE, shade);
}

// Z buffer related render states.
inline void gpuDisableZTest()
{
	gpuDev->SetRenderState_Inline(D3DRS_ZENABLE, false);
}

inline void gpuSetZTest(gpuCmpMode cmp)
{
	gpuDev->SetRenderState_Inline(D3DRS_ZENABLE, true);
#if NGL_ZINVERT
	if (cmp == GPU_CMP_GREATER)
		cmp = GPU_CMP_LESS;
	else if (cmp == GPU_CMP_GREATEREQUAL)
		cmp = GPU_CMP_LESSEQUAL;
	else if (cmp == GPU_CMP_LESSEQUAL)
		cmp = GPU_CMP_GREATEREQUAL;
	else if (cmp == GPU_CMP_LESS)
		cmp = GPU_CMP_GREATER;
#endif

	gpuDev->SetRenderState_Inline(D3DRS_ZFUNC, cmp);
}

inline void gpuEnableZWrite(bool enable)
{
	gpuDev->SetRenderState_Inline(D3DRS_ZWRITEENABLE, enable);
}

// Alpha blending related render states.
inline void gpuDisableAlpha()
{
	gpuDev->SetRenderState_Inline(D3DRS_ALPHABLENDENABLE, false);
}

// todo- State by state settings.
inline void gpuSetAlpha(gpuBlendMode src, gpuBlendMode dest, uint constant, gpuBlendOp op)
{
	gpuDev->SetRenderState_Inline(D3DRS_ALPHABLENDENABLE, true);
	gpuDev->SetRenderState_Inline(D3DRS_SRCBLEND, src);
	gpuDev->SetRenderState_Inline(D3DRS_DESTBLEND, dest);
	gpuDev->SetRenderState_Inline(D3DRS_BLENDFACTOR, constant);
	gpuDev->SetRenderState_Inline(D3DRS_BLENDOP, op);
}

// Separate alpha blend render states (affecting alpha only).
// note- Disabling does nothing in Cg, you must call gpuSetAlpha to switch back to regular blending.
inline void gpuDisableSeparateAlpha()
{
#if NGL_GPU_D3D9
	gpuDev->SetRenderState_Inline(D3DRS_SEPARATEALPHABLENDENABLE, false);
#endif
}

// todo- State by state settings.
inline void gpuSetSeparateAlpha(
	gpuBlendMode colorsrc, gpuBlendMode colordest, uint colorconstant, 
	gpuBlendMode alphasrc, gpuBlendMode alphadest, uint alphaconstant, 
	gpuBlendOp colorop, gpuBlendOp alphaop )
{
#if NGL_GPU_D3D_PC
	colorsrc; colordest; colorconstant; alphasrc; alphadest; alphaconstant; colorop; alphaop;
#endif

}

// Alpha test render states
inline void gpuDisableAlphaTest()
{
	gpuDev->SetRenderState_Inline(D3DRS_ALPHATESTENABLE, false);
}

inline void gpuSetAlphaTest(gpuCmpMode cmp, uint ref)
{
	gpuDev->SetRenderState_Inline(D3DRS_ALPHATESTENABLE, true);
	gpuDev->SetRenderState_Inline(D3DRS_ALPHAFUNC, cmp);
	gpuDev->SetRenderState_Inline(D3DRS_ALPHAREF, ref);
}

// Stencil related render states

inline void gpuDisableStencil()
{
	gpuDev->SetRenderState_Inline(D3DRS_STENCILENABLE, false);
}

inline void gpuSetStencil(gpuCmpMode op, uint ref, gpuStencilOp pass, gpuStencilOp fail, gpuStencilOp zfail,
                          uint mask, uint writemask)
{
	gpuDev->SetRenderState_Inline(D3DRS_STENCILENABLE, true);
	gpuDev->SetRenderState_Inline(D3DRS_STENCILPASS, pass);
	gpuDev->SetRenderState_Inline(D3DRS_STENCILFAIL, fail);
	gpuDev->SetRenderState_Inline(D3DRS_STENCILZFAIL, zfail);
	gpuDev->SetRenderState_Inline(D3DRS_STENCILFUNC, op);
	gpuDev->SetRenderState_Inline(D3DRS_STENCILREF, ref);
	gpuDev->SetRenderState_Inline(D3DRS_STENCILMASK, mask);
	gpuDev->SetRenderState_Inline(D3DRS_STENCILWRITEMASK, writemask);
}

inline void gpuDisableTwoSidedStencil()
{
#if NGL_GPU_D3D9
	gpuDev->SetRenderState_Inline(D3DRS_TWOSIDEDSTENCILMODE, false);
#endif
}

// note- Don't forget to disable culling when using two sided stencil mode!
// note- ref is ignored by D3D
// note- mask & writemask ignored on PC.
inline void gpuSetTwoSidedStencil(gpuStencilOp pass, gpuStencilOp fail, gpuStencilOp zfail, 
                                  gpuCmpMode op, uint ref, uint mask, uint writemask)
{
#if NGL_GPU_D3D_PC
	writemask; mask; ref;
#endif

#if NGL_GPU_D3D9
	gpuDev->SetRenderState_Inline(D3DRS_TWOSIDEDSTENCILMODE, true);
	gpuDev->SetRenderState_Inline(D3DRS_CCW_STENCILPASS, pass);
	gpuDev->SetRenderState_Inline(D3DRS_CCW_STENCILFAIL, fail);
	gpuDev->SetRenderState_Inline(D3DRS_CCW_STENCILZFAIL, zfail);
	gpuDev->SetRenderState_Inline(D3DRS_CCW_STENCILFUNC, op);
#endif
}

// Render target binding.

inline void gpuSetRenderTarget(uint idx, gpuTexture& tex, uint miplevel)
{
	D3DSurface* surf;
	tex.GetD3DTexture()->GetSurfaceLevel(miplevel, &surf);
	gpuDev->SetRenderTarget(idx, surf);
	surf->Release();
}

inline void gpuSetCubeRenderTarget(uint idx, gpuTexture& tex, gpuCubeMapFace face, uint miplevel)
{
	IDirect3DSurface* surf;
	tex.GetD3DCubeTexture()->GetCubeMapSurface((D3DCUBEMAP_FACES)face, miplevel, &surf);
	gpuDev->SetRenderTarget(idx, surf);
	surf->Release();
}


inline void gpuSetRenderTarget(uint idx, gpuSurface &surf)
{
	gpuDev->SetRenderTarget(idx, surf.GetD3DSurface());
}

#if NGL_GPU_D3D9
inline void gpuSetDepthStencilTarget(gpuSurface &surf)
{
	gpuDev->SetDepthStencilSurface(surf.GetD3DSurface());
}
#endif

inline void gpuClearRenderTarget(uint idx)
{
	gpuDev->SetRenderTarget(idx, NULL);
}

inline void gpuClearDepthStencilTarget()
{
	gpuDev->SetDepthStencilSurface(NULL);
}

inline void gpuSetColorWrite(gpuWriteMask mask)
{
	gpuDev->SetRenderState_Inline(D3DRS_COLORWRITEENABLE, mask);
}

inline void gpuSetColorWrite(uint target, gpuWriteMask mask)
{
	switch (target)
	{
	default: case 0: gpuDev->SetRenderState_Inline(D3DRS_COLORWRITEENABLE, mask);
#if NGL_GPU_D3D9
	case 1: gpuDev->SetRenderState_Inline(D3DRS_COLORWRITEENABLE1, mask);
	case 2: gpuDev->SetRenderState_Inline(D3DRS_COLORWRITEENABLE2, mask);
	case 3: gpuDev->SetRenderState_Inline(D3DRS_COLORWRITEENABLE3, mask);
#endif
	}
}

inline void gpuEnableGammaCorrect(bool enable)
{
#if NGL_GPU_D3D9
	gpuDev->SetRenderState_Inline(D3DRS_SRGBWRITEENABLE, enable);
#endif
}

inline void gpuDisableScissor()
{
#if NGL_GPU_D3D8
	gpuDev->SetScissors(0, FALSE, NULL);
#endif
#if NGL_GPU_D3D9
	gpuDev->SetRenderState_Inline(D3DRS_SCISSORTESTENABLE, false);
#endif
}

inline void gpuSetScissor(int x1, int y1, int x2, int y2)
{
#if NGL_GPU_D3D9
	gpuDev->SetRenderState_Inline(D3DRS_SCISSORTESTENABLE, true);
#endif
	RECT r = { x1, y1, x2, y2 };
	gpuDev->SetScissorRect(&r);
}

inline void gpuSetViewport(int x, int y, int w, int h, float minZ, float maxZ)
{
#if NGL_GPU_D3D8
	D3DVIEWPORT8 vp;
#else
	D3DVIEWPORT9 vp;
#endif
	vp.X = x;
	vp.Y = y;
	vp.Width  = (DWORD)w;
	vp.Height = (DWORD)h;
	vp.MinZ = minZ;
	vp.MaxZ = maxZ;
	gpuDev->SetViewport(&vp);
}

inline void gpuEnableDither(bool enable)
{
	//note- unsupported on Xenon
	gpuDev->SetRenderState_Inline(D3DRS_DITHERENABLE, enable);
}

inline void gpuClear(gpuClearMask flags, uint color, float depth, uint stencil)
{
	gpuDev->Clear(0, NULL, flags, color, depth, stencil);
}

inline void gpuSetDepthBias(float slopebias, float depthbias)
{
	gpuDev->SetRenderState_Inline(D3DRS_SLOPESCALEDEPTHBIAS, (unsigned&)slopebias);
	gpuDev->SetRenderState_Inline(D3DRS_DEPTHBIAS, (unsigned&)depthbias);
}

inline void gpuSetShaderBalance(float Balance)
{
#if NGL_GPU_D3D_PC
	Balance;
#endif

	Balance;
}

// Fence related functions.
typedef DWORD gpuFenceHandle;

inline gpuFenceHandle gpuInsertFence()
{
#if NGL_GPU_D3D_PC
	return 0;
#endif
}

inline bool gpuIsFencePending(gpuFenceHandle fence)
{
#if NGL_GPU_D3D_PC
	fence;
	return false;
#endif
}

inline void gpuBlockOnFence(gpuFenceHandle fence)
{
#if NGL_GPU_D3D_PC
	fence;
#endif
}

// Callback related functions.
#if NGL_GPU_D3D_PC
typedef void (__cdecl *gpuCallback)(DWORD);
#define GPU_CALLBACK(name) void name(DWORD data)
#endif

enum gpuCallbackType
{
#if NGL_GPU_D3D_PC
	GPU_CALLBACK_ASYNC = 0,
	GPU_CALLBACK_SYNC = 1,
#endif
};

inline void gpuInsertCallback(gpuCallbackType type, gpuCallback callback, void* data)
{
#if NGL_GPU_D3D_PC
	type; callback; data;
#endif
}

// VBlank callback.
#if NGL_GPU_D3D_PC
typedef TIMECALLBACK gpuVBlankCallback;
#define GPU_VBLANK_CALLBACK(name) void CALLBACK name(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
#endif

inline void gpuSetVBlankCallback(gpuVBlankCallback callback)
{
#if NGL_GPU_D3D_PC
	static MMRESULT timer = 0;
	if (timer) timeKillEvent(timer);
	timer = timeSetEvent(17, 0, callback, NULL, TIME_PERIODIC);
#endif
}

// todo
typedef int gpuWaitHandle;

inline gpuWaitHandle gpuInsertWait()
{
	return 0;
}

inline void gpuReleaseWait(gpuWaitHandle w)
{
#if NGL_GPU_D3D_PC
	w;
#endif
}

// Initialization / presentation functions
inline void gpuPresent()
{
#if NGL_GPU_D3D_PC
	gpuDev->EndScene();
	gpuDev->Present(NULL, NULL, NULL, NULL);
	gpuDev->BeginScene();
#else
	gpuDev->Present(NULL, NULL, NULL, NULL);
#endif
}

#endif // NGL_DX_GPU_H
