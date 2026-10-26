/** \file ngl_gpu.h
	NGL GPU module.
	\see NGL_GPU
*/

/**
	\defgroup NGL_GPU NGL GPU API

	This module implements a low level graphics device abstraction which is 
	used by higher level portions of NGL, and also by custom graphics code 
	written by clients.

	It is a lightly wrapped bridge between Direct3D and GCM and is intended 
	to fully interoperate with those APIs on their respective platforms.
	For that reason, it does not encompass 100% of either API, just the parts 
	where there is a clear parallel.  If you need some functionality that is 
	not covered by the NGL GPU module, don't hesitate to drop back to the native API.

	@{
*/

#ifndef NGL_GPU_H
#define NGL_GPU_H


// Platform detection
#if NGL_GPU_D3D+NGL_GPU_GCM+NGL_GPU_REV == 0

// Automatically detect D3D versions.
#if defined(_WIN32)
 #define NGL_GPU_D3D 1
  #define NGL_GPU_D3D9 1
  #define NGL_GPU_D3D_PC 1
#endif

// Automatically detect GCM versions.
#if __CELLOS_LV2__
 #define NGL_GPU_GCM 1
#endif

#if defined(RVL_OS)
 #define NGL_GPU_REV 1
#endif

#endif

// DLL linkage inherited from NGL or else not used.
#if defined(NGL_API) && !NGL_TOOL
#define GPU_EXTERN NGL_EXTERN
#else
#define GPU_EXTERN extern
#define NGL_API
#define NGL_EXTERN extern
#endif

//___________________________________________________________________________

// Other library dependencies.
#include "system/scheduler/core/tl_defs.h"
//#include "tl_system.h"

// Per-platform data types.
#include "texture_export/ngl/ngl_gpu_types.h"

extern gpuDevice* gpuDev;

// State cache data.
struct gpuStateCacheClass
{
	size_t Texture[GPU_NUM_SAMPLERS];
	size_t VertexBuffer[GPU_NUM_STREAMS];
	size_t IndexBuffer;
	size_t PixelShader;
	size_t VertexShader;
	size_t VertexFormat;
	float  ShaderBalance;
};

GPU_EXTERN gpuStateCacheClass gpuStateCache;

//___________________________________________________________________________

/** \struct gpuVertexElementFormat

	GPU_VERTEX_FLOAT3 et al. See the platform's definition for a full list.

	\code  enum gpuVertexElementFormat;  \endcode
*/

/** \enum gpuVertexElementUsage

	GPU_VERTEX_POSITION et al. See the platform's definition for a full list.

	\code  enum gpuVertexElementUsage;  \endcode
*/

//___________________________________________________________________________

/** \defgroup NGL_GPU_VERTEX_FORMAT Vertex stream formats
	Stream format for declaring vertex formats.

	\code
	static gpuVertexElement MyVertexElements[] =
	{
		GPU_VERTEX_ELEMENT(GPU_VERTEX_POSITION, 0,GPU_VERTEX_FLOAT3, 0),
		GPU_VERTEX_ELEMENT(GPU_VERTEX_NORMAL,   0,GPU_VERTEX_DEC3N,  12),
		GPU_VERTEX_ELEMENT(GPU_VERTEX_TEXCOORD, 0,GPU_VERTEX_FLOAT2, 16),
		GPU_VERTEX_ELEMENT_END()
	};
	\endcode

	@{
*/

/** \struct gpuVertexElement
	Storage class for vertex elements.
*/
/** \def GPU_VERTEX_ELEMENT
	Declares a vertex element.

	\code  GPU_VERTEX_ELEMENT(usage,index,format,offset) \endcode
	\see NGL_GPU_VERTEX_FORMAT
*/
/** \def GPU_VERTEX_ELEMENT_END
	Terminates a list of vertex elements.

	\code  GPU_VERTEX_ELEMENT_END() \endcode
	\see NGL_GPU_VERTEX_FORMAT
*/
/** \def GPU_VERTEX_ELEMENT_TYPE
	Gets the type of a vertex element.

	\code  gpuVertexElementFormat type = GPU_VERTEX_ELEMENT_TYPE(elem); \endcode
	\see NGL_GPU_VERTEX_FORMAT
*/

//___________________________________________________________________________

/// Vertex format structure.
struct gpuVertexFormat;

/** Creates a vertex format from a list of vertex elements.

	\remarks  Elements *must* be in persistent memory (global var or heap), as the pointer is stored!
*/
//@{
inline void gpuCreateVertexFormat(gpuVertexFormat& format, uint size, const gpuVertexElement* elements);
inline gpuVertexFormat gpuCreateVertexFormat(uint size, const gpuVertexElement* elements)
{
	gpuVertexFormat tmp;
	gpuCreateVertexFormat(tmp, size, elements);
	return tmp;
}
//@}

/// Destroys a vertex format.
inline void gpuDestroyVertexFormat(const gpuVertexFormat& format);

/** Find a vertex element inside a format.
	\param fmt         The vertex format.
	\param usage       The element usage to look for.
	\param usageindex  The element usage index to look for.
	\returns  A pointer to the element. It will assert if not found.
*/
inline const gpuVertexElement* gpuFindVertexElement(gpuVertexFormat* fmt, gpuVertexElementUsage usage, uint usageindex);

inline int gpuGetVertexElementSize(gpuVertexElementFormat Element);

// NGL_GPU_VERTEX_FORMAT
//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_VERTEX_BUFFER Vertex buffers
	Vertex data storage.
	@{
*/

/** \typedef gpuVertexBuffer
	Portable representation of a vertex buffer.
*/

/** Creates a vertex buffer.
	\param nverts  Number of verts to hold.
	\param fmt     Format for the vertex buffer.
*/
//@{
inline void gpuCreateVertexBuffer(gpuVertexBuffer& buf, uint nverts, const gpuVertexFormat* fmt, u32 flags=0, void* address=NULL);
inline gpuVertexBuffer gpuCreateVertexBuffer(uint nverts, const gpuVertexFormat* fmt, u32 flags=0, void* address=NULL)
{
	gpuVertexBuffer tmp;
	gpuCreateVertexBuffer(tmp, nverts, fmt, flags, address);
	return tmp;
}
//@}

/** Creates a vertex buffer.
	\param bytes  Size of vertex buffer in bytes.
*/
//@{
inline void gpuCreateVertexBuffer(gpuVertexBuffer& buf, uint bytes, u32 flags=0, void* address=NULL);
inline gpuVertexBuffer gpuCreateVertexBuffer(uint bytes, u32 flags=0, void* address=NULL)
{
	gpuVertexBuffer tmp;
	gpuCreateVertexBuffer(tmp, bytes, flags, address);
	return tmp;
}
//@}

/// Destroys a vertex buffer.
inline void gpuDestroyVertexBuffer(gpuVertexBuffer& buf);

/// Maps a vertex buffer into addressable memory.
inline void* gpuMapVertexBuffer(gpuVertexBuffer& buf, uint offset, uint size);

/// Un-maps a vertex buffer.
inline void gpuUnmapVertexBuffer(gpuVertexBuffer& buf);

#if VMATHD

/// Packs a vertex buffer.
GPU_EXTERN void gpuPackVertexBuffer(gpuVertexBuffer& vtx, uint vtxoffset, uint vertexsize, const gpuVertexElement* element, const math::Vector4* buf, uint start, uint count);

/// Packs a vertex buffer.
GPU_EXTERN void gpuPackVertexBuffer(void* vtx, uint vtxoffset, uint vertexsize, const gpuVertexElement* element, const math::Vector4* buf, uint start, uint count);

/// Unpacks a vertex buffer.
GPU_EXTERN void gpuUnpackVertexBuffer(gpuVertexBuffer& vtx, uint vtxoffset, uint vertexsize, const gpuVertexElement* element, math::Vector4* buf, uint start, uint count);

/// Unpacks a vertex buffer.
GPU_EXTERN void gpuUnpackVertexBuffer(void* vtx, uint vtxoffset, uint vertexsize, const gpuVertexElement* element, math::Vector4* buf, uint start, uint count);

#endif // VMATHD

// NGL_GPU_VERTEX_BUFFER
//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_INDEX_BUFFER Index buffers
	Index data storage.
	@{
*/

/** \typedef gpuIndexBuffer
	Portable representation of an index buffer.
*/

/// Creates an index buffer.
//@{
inline void gpuCreateIndexBuffer(gpuIndexBuffer& buf, uint nindices, gpuIndexType indextype, u32 flags=0, void* address=NULL);
inline gpuIndexBuffer gpuCreateIndexBuffer(uint nindices, gpuIndexType indextype, u32 flags=0, void* address=NULL)
{
	gpuIndexBuffer tmp;
	gpuCreateIndexBuffer(tmp, nindices, indextype, flags, address);
	return tmp;
}
//@}

/// Destroys an index buffer.
inline void gpuDestroyIndexBuffer(gpuIndexBuffer& buf);

/// Maps an index buffer into addressable memory.
inline void* gpuMapIndexBuffer(gpuIndexBuffer& buf, uint offset, uint size);

/// Un-maps an index buffer.
inline void gpuUnmapIndexBuffer(gpuIndexBuffer& buf);

/** Unpacks index buffer data into a user-specified array.
	This will convert the indices into an indexed triangle list.
	\param idx        Index buffer to unpack.
	\param primtype   Primitive type used to parse the indices.
	\param idxformat  Index format.
	\param buf        User buffer to write into.
	\param nindices   Number of indices to process.
	\returns  The number of triangles written out.
*/
GPU_EXTERN uint gpuUnpackIndexBuffer(gpuIndexBuffer& idx, gpuPrimType primtype, gpuIndexType idxformat, uint* buf, uint nindices);

// NGL_GPU_INDEX_BUFFER
//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_TEXTURE Textures
	Textures and formats.
	@{
*/

/** \typedef gpuTexture
	Portable representation of a texture.

	On Playstation3, XBox and Xenon, it's important to understand that the memory containing the actual texels is separate 
	from the gpuTexture object.  The gpuTexture object can be assigned, copied, destroyed, and recreated independently from 
	the texel data. 

	This allows flexibility for such things as pointing several gpuTexture objects at the same memory with different 
	formats, or pointing a gpuTexture object at the vertices of a gpuVertexBuffer.

	It can be helpful to think of a gpuTexture as a simple data pointer, with fields describing how the data should be
	interpreted.

	On PC, this flexibility is not possible, so gpuTexture objects must be treated as normal Direct3D textures would be.
*/

/** \enum gpuTextureFormat
	Texture formats.

	GPU_TEXTURE_DXT1 et al.

	\code  enum gpuTextureFormat; \endcode
*/

/// Calculates the number of levels needed for a texture.
inline uint gpuCalcMaxTextureLevels(uint width, uint height, uint depth)
{
	uint levels = 1;
	while (width > 1 || height > 1 || depth > 1)
	{
		width = tl_max(1, width/2);
		height = tl_max(1, height/2);
		depth = tl_max(1, depth/2);
		levels++;
	}
	return levels;
}

/** Returns true if the texture format is linear.
	Linear textures are stored in scanline order, left to right and top to bottom.  Note that the pitch of a linear
	texture may be greater than the amount of data in one row.
	\param format Texture format to test.
	\return A boolean value indicating whether the texture format is linear.
*/
inline bool gpuIsTextureFormatLinear(gpuTextureFormat format);

/** Returns true if the texture format is swizzled.
	Swizzled textures are stored in a pattern optimized for local data access when sampling textures, and are
	faster to render from.  The pattern is generated by interleaving alternating bits from the x and y coordinates
	to generate an offset.
	\param format Texture format to test.
	\return A boolean value indicating whether the texture format is swizzled.
*/
inline bool gpuIsTextureFormatSwizzled(gpuTextureFormat format);

/** Returns true if the texture format is a DXT format.
	\param format Texture format to test.
	\return A boolean value indicating whether the texture format is DXT.
*/
inline bool gpuIsTextureFormatDXT(gpuTextureFormat format);

/** Returns true if the texture format is compressed.
	Compressed texture formats are stored as blocks, where each block holds more than one texel.
	\param format Texture format to test.
	\return A boolean value indicating whether the texture format is compressed.
*/
inline bool gpuIsTextureFormatCompressed(gpuTextureFormat format);

/** Returns the number of bytes per pixel for a given texture format.
	\param format Texture format to test.
	\return The size of one pixel (or block, for compressed textures) in bytes.
	\remark For compressed textures, the returned value is the number of bytes per block.
	        Use gpuGetTextureFormatSizeInBlocks to convert from texture dimensions in pixels to texture dimensions in blocks.
*/
inline uint gpuGetTextureFormatBytesPerPixel(gpuTextureFormat format);

/** Returns the size of a block of pixels for a given texture format.
	Compressed texture formats, for example DXT, store multiple pixels in blocks.  When working with compressed textures, it is
	important to work in terms of blocks instead of individual pixels.
	Note that for uncompressed textures, this function returns a block size of 1x1.
	\param format Texture format.
	\param w      Pointer to a variable that will receive the block width, or NULL.
	\param h      Pointer to a variable that will receive the block height, or NULL.
*/
inline void gpuGetTextureFormatBlockSize(gpuTextureFormat format, int* w, int* h);

/** Converts the size of a texture from pixels to blocks. the size of a block of pixels for a given texture format.
	Compressed texture formats, for example DXT, store multiple pixels in blocks.  When working with compressed textures, it is
	important to work in terms of blocks, and not individual pixels.
	\param format Texture format.
	\param w      Pointer to a variable containing the width of the texture in pixels.  It will be replaced by the width of the
	              texture in blocks.
	\param h      Pointer to a variable containing the height of the texture in pixels.  It will be replaced by the height of the
	              texture in blocks.
*/
inline void gpuGetTextureSizeInBlocks(gpuTextureFormat format, int* w, int* h);

/** Returns the offset in bytes of a given face and mipmap level within the texture.
	\param tex   Texture object.
	\param face  Cubemap face, or else 0 for 2D textures.
	\param level Mipmap level, or else 0 for the base level.
	\return The offset in bytes to the pixel data, from the beginning of the texture data.
*/
inline uint gpuGetTextureLevelOffset(gpuTexture& tex, int face, int level);

/** Returns the size in bytes of one row of texture data for the given face and mipmap level.
	For compressed textures, the pitch returned is the pitch of one row of blocks, not pixels.
	\param tex   Texture object.
	\param face  Cubemap face, or else 0 for 2D textures.
	\param level Mipmap level, or else 0 for the base level.
	\return The total size of the pixel data for one row of the texture.
*/
inline uint gpuGetTextureLevelPitch(gpuTexture& tex, int face, int level);

/** Returns the size in bytes of the texture data for a texture.
	\param tex Texture object.
	\return The total size of the pixel data for the texture.
*/
inline uint gpuGetTextureSize(gpuTexture& tex);

/** Sets the header information for a texture without allocating memory for the texture data.
	\param tex          Texture object to fill in.
	\param width        Width of the texture.
	\param height       Height of the texture.
	\param levels       Number of miplevels (including the base).
	\param format       Format of the texture.
	\param flags        Texture creation flags.
	\param address      Address of texture data, or NULL to have it allocated automatically.
*/
inline void gpuSetTextureHeader(gpuTexture& texture, uint width, uint height, uint levels, gpuTextureFormat format, u32 flags=0);

/** Sets the header information for a cube texture without allocating memory for the texture data.
	\param tex          Texture object to fill in.
	\param size         Width and height of each cubemap face.
	\param levels       Number of miplevels (including the base).
	\param format       Format of the texture.
	\param flags        Texture creation flags.
	\param address      Address of texture data, or NULL to have it allocated automatically.
*/
inline void gpuSetCubeTextureHeader(gpuTexture& texture, uint size, uint levels, gpuTextureFormat format, u32 flags=0);

/** Sets the header information for a volume texture without allocating memory for the texture data.
	\param tex          Texture object to fill in.
	\param width        Width of the texture.
	\param height       Height of the texture.
	\param depth        Depth of the texture.
	\param levels       Number of miplevels (including the base).
	\param format       Format of the texture.
	\param flags        Texture creation flags.
	\param address      Address of texture data, or NULL to have it allocated automatically.
*/
inline void gpuSetVolumeTextureHeader(gpuTexture& texture, uint width, uint height, uint depth, uint levels, gpuTextureFormat format, u32 flags=0);

/// Sets the location of the texture data.
inline void gpuSetTextureAddress(gpuTexture& tex, void* address);

/// Allocates a buffer for the texture data and assigns it to the texture.
inline void gpuAllocTextureData(gpuTexture& tex);

/// Frees the buffer associated with this texture.
inline void gpuFreeTextureData(gpuTexture& tex);

/** Creates a texture.
	\param tex          Texture object to hold created texture.
	\param width        Width of the texture.
	\param height       Height of the texture.
	\param levels       Number of miplevels (including the base).
	\param format       Format of the texture.
	\param flags        Texture creation flags.
	\param address      Address of texture data, or NULL to have it allocated automatically.
*/
//@{
inline void gpuCreateTexture(gpuTexture& tex, uint width, uint height, uint levels, gpuTextureFormat format, u32 flags=0, void* address=NULL);
inline gpuTexture gpuCreateTexture(uint width, uint height, uint levels, gpuTextureFormat format, u32 flags=0, void* address=NULL)
{
	gpuTexture tmp;
	gpuCreateTexture(tmp, width, height, levels, format, flags, address);
	return tmp;
}
//@}

/** Creates a cubemap texture.
	\param tex          Texture object to hold created texture.
	\param size         Width and height of each cubemap face.
	\param levels       Number of miplevels (including the base).
	\param format       Format of the texture.
	\param flags        Texture creation flags.
	\param address      Address of texture data, or NULL to have it allocated automatically.
*/
//@{
inline void gpuCreateCubeTexture(gpuTexture& tex, uint size, uint levels, gpuTextureFormat format, u32 flags=0, void* address=NULL);
inline gpuTexture gpuCreateCubeTexture(uint size, uint levels, gpuTextureFormat format, u32 flags=0, void* address=NULL)
{
	gpuTexture tmp;
	gpuCreateCubeTexture(tmp, size, levels, format, flags, address);
	return tmp;
}
//@}

/** Creates a volume texture.
	\param tex          Texture object to hold created texture.
	\param width        Width of the texture.
	\param height       Height of the texture.
	\param depth        Depth of the texture.
	\param levels       Number of miplevels (including the base).
	\param format       Format of the texture.
	\param flags        Texture creation flags.
	\param address      Address of texture data, or NULL to have it allocated automatically.
*/
//@{
inline void gpuCreateVolumeTexture(gpuTexture& tex, uint width, uint height, uint depth, uint levels, gpuTextureFormat format, u32 flags=0, void* address=NULL);
inline gpuTexture gpuCreateVolumeTexture(uint width, uint height, uint depth, uint levels, gpuTextureFormat format, u32 flags=0, void* address=NULL)
{
	gpuTexture tmp;
	gpuCreateVolumeTexture(tmp, width, height, depth, levels, format, flags, address);
	return tmp;
}
//@}

/// Destroys a texture, freeing the associated texel data.
inline void gpuDestroyTexture(gpuTexture& tex);

/** Maps texture data into addressable memory.
	\param tex   Texture object.
	\param face  Cubemap face or volume texture slice.
	\param level Mipmap level (0 for the base).
*/
inline void* gpuMapTexture(gpuTexture& tex, int face=0, int level=0);

/** Unmaps texture data that was previously mapped into memory.
	\remark This function only has an effect on PC.
	\param tex   Texture object.
	\param face  Cubemap face or volume texture slice.
	\param level Mipmap level (0 for the base).
*/
inline void gpuUnmapTexture(gpuTexture& tex, int face=0, int level=0);

// NGL_GPU_TEXTURE
//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_SURFACE Surfaces
	Render targets and depth/stencil surfaces.
	@{
*/

/** \typedef gpuSurface
	Portable representation of a surface.
*/

/** \enum gpuMultiSampleType
	\code
	enum gpuMultiSampleType
	{
		GPU_MULTISAMPLE_NONE,
		GPU_MULTISAMPLE_2X,
		GPU_MULTISAMPLE_4X,
	};
	\endcode
*/

/** Creates a render or depth/stencil surface.
*/
//@{
inline void gpuCreateSurface(gpuSurface& surf, uint w, uint h, gpuSurfaceFormat format, gpuMultiSampleType multisample=GPU_MULTISAMPLE_NONE, u32 flags=0, void* address=NULL, void* hizaddress=NULL);
inline gpuSurface gpuCreateSurface(uint w, uint h, gpuSurfaceFormat format, gpuMultiSampleType multisample=GPU_MULTISAMPLE_NONE, u32 flags=0, void* address=NULL, void* hizaddress=NULL)
{
	gpuSurface tmp;
	gpuCreateSurface(tmp, w, h, format, multisample, flags, address, hizaddress);
	return tmp;
}
//@}

/** \enum gpuCubeMapFace
	\code
	enum gpuCubeMapFace
	{
		GPU_CUBE_POSITIVE_X,
		GPU_CUBE_NEGATIVE_X,
		GPU_CUBE_POSITIVE_Y,
		GPU_CUBE_NEGATIVE_Y,
		GPU_CUBE_POSITIVE_Z,
		GPU_CUBE_NEGATIVE_Z,
	};
	\endcode
*/

/// Resolves the current render target to a texture.
inline void gpuResolveRenderTarget(uint idx, gpuTexture& dst, uint miplevel);

/// Destroys a surface.
inline void gpuDestroySurface(gpuSurface& surf);

/// Sets the current render target.
inline void gpuSetRenderTarget(uint idx, gpuTexture& tex, uint miplevel);

/// Sets the current render target to a cubemap face.
inline void gpuSetCubeRenderTarget(uint idx, gpuTexture& tex, gpuCubeMapFace face, uint miplevel);

/// Sets the current depth target.
inline void gpuSetRenderDepthTarget(gpuSurface& render, gpuSurface& depth);

/// Sets the current depth target to a cubemap face.
inline void gpuSetCubeRenderDepthTarget(gpuTexture& tex, gpuCubeMapFace face, uint miplevel, gpuSurface& depth);

/// Sets the current render target.
inline void gpuSetRenderTarget(uint idx, gpuSurface& surf);

/// Sets the current depth & stencil target.
inline void gpuSetDepthStencilTarget(gpuSurface& surf);

/// Clears the current render target (disables rendering color).
inline void gpuClearRenderTarget(uint idx);

/// Clears the current depth & stencil target (disables rendering or testing depth & stencil).
inline void gpuClearDepthStencilTarget();

// NGL_GPU_SURFACE
//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_VERTEX_SHADER Vertex shaders
	Per-vertex shader operations.
	@{
*/

/** \typedef gpuVertexShader
	Portable representation of a vertex shader.
*/

/// Creates a vertex shader from microcode.
//@{
inline void gpuCreateVertexShader(gpuVertexShader& shader, const void* microcode, const void* decl=NULL);
inline gpuVertexShader gpuCreateVertexShader(const void* microcode, const void* decl=NULL)
{
	gpuVertexShader tmp;
	gpuCreateVertexShader(tmp, microcode, decl);
	return tmp;
}
//@}

/// Destroys a vertex shader.
inline void gpuDestroyVertexShader(gpuVertexShader& shader);

//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_PIXEL_SHADER Pixel shaders
	Per-pixel shader operations.
	@{
*/

/** \typedef gpuPixelShader
	Portable representation of a pixel shader.
*/

/// Creates a pixel shader from microcode.
//@{
inline void gpuCreatePixelShader(gpuPixelShader& shader, const void* microcode);
inline gpuPixelShader gpuCreatePixelShader(const void* microcode)
{
	gpuPixelShader tmp;
	gpuCreatePixelShader(tmp, microcode);
	return tmp;
}
//@}

/// Destroys a pixel shader.
inline void gpuDestroyPixelShader(gpuPixelShader& shader);

//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_SHADER_PARAMS Shader Parameters
	High-level shader parameters.
	@{
*/

/** \typedef gpuParameter
	Portable representation of a shader parameter.
*/

/// Retrieves a parameter by name.
inline gpuParameter gpuGetParameterByName(void* function, const char* name);

/// Retrieves a parameter by name.
/// \note  Not to be used in performance sensitive code, especially on D3D platforms.
inline gpuParameter gpuGetParameterByName(gpuVertexShader& shader, const char* name);

/// Retrieves a parameter by name.
/// \note  Not to be used in performance sensitive code, especially on D3D platforms.
inline gpuParameter gpuGetParameterByName(gpuPixelShader& shader, const char* name);

//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_SHADERS Shaders
	Vertex and pixel shader render states.
	@{
*/

/// Selects a vertex shader.
inline void gpuSetVertexShader(gpuVertexShader& shader);

/// Clears the current vertex shader out.
inline void gpuClearVertexShader();

/// Sets a vertex shader constant.
inline void gpuSetVertexConstant(gpuParameter idx, int val);

/// Sets a vertex shader constant.
inline void gpuSetVertexConstant(gpuParameter idx, float val);

/// Sets a vertex shader constant.
inline void gpuSetVertexConstant(gpuParameter idx, const gpuFloat4& val);

/// Sets a vertex shader constant.
inline void gpuSetVertexConstant(gpuParameter idx, const gpuFloat4* data, uint nelements);

/// Sets a vertex shader constant.
inline void gpuSetVertexConstant(gpuParameter idx, const float* data, uint nelements);

#if VMATHD

/// Sets a vertex shader constant.
inline void gpuSetVertexConstant(gpuParameter idx, math::Vector4Arg val);

/// Sets a vertex shader constant.
inline void gpuSetVertexConstant(gpuParameter idx, const math::Mat44& val);

/// Sets a vertex shader constant.
inline void gpuSetVertexConstant(gpuParameter idx, const math::Vector4* data, uint nelements);

/// Sets a vertex shader constant.
inline void gpuSetVertexConstant(gpuParameter idx, const math::Mat44* data, uint nelements);

#endif

/// Returns a pointer where shader constants can be written directly to the command buffer.
inline void* gpuBeginVertexConstant(gpuParameter idx, uint nelements);

/// Finishes setting up the shader constants.
inline void gpuEndVertexConstant(gpuParameter idx, uint nelements);

/// Acquires ownership of shader constants.
inline void gpuOwnVertexConstant(gpuParameter idx, uint nelements);

/// Releases ownership of shader constants.
inline void gpuDisownVertexConstant(gpuParameter idx, uint nelements);

/// Selects a pixel shader.
inline void gpuSetPixelShader(gpuPixelShader& shader);

/// Clears the current pixel shader out.
inline void gpuClearPixelShader();

/// Sets a pixel shader constant.
inline void gpuSetPixelConstant(gpuParameter idx, int val);

/// Sets a pixel shader constant.
inline void gpuSetPixelConstant(gpuParameter idx, float val);

/// Sets a pixel shader constant.
inline void gpuSetPixelConstant(gpuParameter idx, const gpuFloat4& val);

/// Sets a pixel shader constant.
inline void gpuSetPixelConstant(gpuParameter idx, const gpuFloat4* data, uint nelements);

/// Sets a pixel shader constant.
inline void gpuSetPixelConstant(gpuParameter idx, const float* data, uint nelements);

#if VMATHD

/// Sets a pixel shader constant.
inline void gpuSetPixelConstant(gpuParameter idx, math::Vector4Arg val);

/// Sets a pixel shader constant.
inline void gpuSetPixelConstant(gpuParameter idx, const math::Mat44& val);

/// Sets a pixel shader constant.
inline void gpuSetPixelConstant(gpuParameter idx, const math::Vector4* data, uint nelements);

/// Sets a pixel shader constant.
inline void gpuSetPixelConstant(gpuParameter idx, const math::Mat44* data, uint nelements);

#endif

/// Returns a pointer where shader constants can be written directly to the command buffer.
inline void* gpuBeginPixelConstant(gpuParameter idx, uint nelements);

/// Finishes setting up the shader constants.
inline void gpuEndPixelConstant(gpuParameter idx, uint nelements);

/// Acquires ownership of shader constants.
inline void gpuOwnPixelConstant(gpuParameter idx, uint nelements);

/// Releases ownership of shader constants.
inline void gpuDisownPixelConstant(gpuParameter idx, uint nelements);

/// Selects a vertex shader and a pixel shader together.
inline void gpuSetShaders(gpuVertexShader& vtx, gpuPixelShader& pixel)
{
	gpuSetVertexShader(vtx);
	gpuSetPixelShader(pixel);
}

// NGL_GPU_SHADERS
//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_SAMPLERS Samplers & Textures
	Texture and texture parameters.
	@{
*/

/** \enum gpuAddressMode
	\code
	enum gpuAddressMode
	{
		GPU_ADDRESS_WRAP,
		GPU_ADDRESS_CLAMP,
		GPU_ADDRESS_MIRROR,
		GPU_ADDRESS_BORDER,
	};
	\endcode
*/

/** \enum gpuFilterMode
	\code
	enum gpuFilterMode
	{
		GPU_FILTER_POINT,
		GPU_FILTER_LINEAR,
		GPU_FILTER_ANISOTROPIC,
	};
	\endcode
*/

/// Sets a texture sampler.
inline void gpuSetSamplerTexture(gpuSampler idx, gpuTexture& tex);

/// Clears a texture sampler.
inline void gpuClearSamplerTexture(gpuSampler idx);

/// Sets the filters for a sampler.
/// \note  Must be called after gpuSetTexture.
inline void gpuSetSamplerFilter(gpuSampler idx, gpuFilterMode min, gpuFilterMode mag, gpuFilterMode mip, uint maxanisotropy=1);

/// Sets the wrap modes for a sampler.
/// \note  Must be called after gpuSetTexture.
inline void gpuSetSamplerWrap(gpuSampler idx, gpuAddressMode uwrap, gpuAddressMode vwrap);

/// Sets a sampler.
inline void gpuSetSampler(gpuSampler idx, gpuTexture& tex, gpuAddressMode uwrap, gpuAddressMode vwrap, 
                          gpuFilterMode min, gpuFilterMode mag, gpuFilterMode mip, uint maxanisotropy=1)
{
	gpuSetSamplerTexture(idx, tex);
	gpuSetSamplerWrap(idx, uwrap, vwrap);
	gpuSetSamplerFilter(idx, min, mag, mip, maxanisotropy);
}

// Functions for 3D samplers.

/// Sets the wrap modes for a 3D sampler.
/// \note  Must be called after gpuSetTexture.
inline void gpuSetSamplerWrap3D(gpuSampler idx, gpuAddressMode uwrap, gpuAddressMode vwrap, gpuAddressMode wwrap);

/// Sets a 3D sampler.
inline void gpuSetSampler3D(gpuSampler idx, gpuTexture& tex, 
                            gpuAddressMode uwrap, gpuAddressMode vwrap, gpuAddressMode wwrap, 
                            gpuFilterMode min, gpuFilterMode mag, gpuFilterMode mip, uint maxanisotropy=1)
{
	gpuSetSamplerTexture(idx, tex);
	gpuSetSamplerWrap3D(idx, uwrap, vwrap, wwrap);
	gpuSetSamplerFilter(idx, min, mag, mip, maxanisotropy);
}

// NGL_GPU_SAMPLERS
//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_PRIMITIVE Primitives
	Primitive drawing functions.
	@{
*/

/** \enum gpuPrimType
	\code
	enum gpuPrimType
	{
		GPU_PRIM_POINTLIST,
		GPU_PRIM_LINELIST,
		GPU_PRIM_LINESTRIP,
		GPU_PRIM_TRIANGLELIST,
		GPU_PRIM_TRIANGLESTRIP,
		GPU_PRIM_TRIANGLEFAN,
	};
	\endcode
*/

/** \enum gpuIndexType
	Index sizes.

	\code
	enum gpuIndexType
	{
		GPU_INDEX_16,
		GPU_INDEX_32,
	};
	\endcode
*/

/** Calculates the number of primitives for a given index count.
	\param prim      Primitive type
	\param nindices  Number of indices
	\returns  The number of primitives represented by nindices
*/
inline uint gpuGetPrimCount(gpuPrimType prim, uint nindices)
{
	switch (prim)
	{
	case GPU_PRIM_POINTLIST:     return nindices;
	case GPU_PRIM_LINELIST:      return nindices/2;
	case GPU_PRIM_LINESTRIP:     return nindices - 1;
	case GPU_PRIM_TRIANGLELIST:  return nindices/3;
	case GPU_PRIM_TRIANGLESTRIP: return nindices - 2;
	case GPU_PRIM_TRIANGLEFAN:   return nindices - 2;
	default: return 0;
	}
}

/// Sets the current vertex buffer.
inline void gpuSetVertexBuffer(gpuVertexBuffer& vtx, gpuVertexFormat* vertexformat, 
                               uint vtxoffset = 0, uint streamidx = 0);

/// Clears the current vertex buffer.
inline void gpuClearVertexBuffer(uint streamidx = 0);

/// Sets the current index buffer.
inline void gpuSetIndexBuffer(gpuIndexBuffer& idx);

/// Clears the current index buffer.
inline void gpuClearIndexBuffer();

/// Renders primitives from index and vertex buffers.
inline void gpuDrawIndexedPrimitive(
	gpuPrimType prim, uint nindices, uint idxoffset, gpuIndexBuffer& idx, gpuIndexType indexformat, 
	uint nverts, uint vtxoffset, gpuVertexBuffer& vtx, gpuVertexFormat* vertexformat);

/// Same as #gpuDrawIndexedPrimitive, except draws inputs from memory without using vertex/index buffers.
inline void gpuDrawIndexedPrimitivePtr(
	gpuPrimType prim, uint nindices, void* idx, gpuIndexType indexformat, 
	uint nverts, void* vtx, gpuVertexFormat* vertexformat);

/// Renders primitives from a vertex buffer.
inline void gpuDrawPrimitive(gpuPrimType prim, uint nverts, uint vtxoffset, gpuVertexBuffer& vtx, gpuVertexFormat* vertexformat);

/// Renders primitives from a vertex buffer.
inline void gpuDrawPrimitivePtr(gpuPrimType prim, uint nverts, void* vtx, gpuVertexFormat* vertexformat);

/// Sets the current primitive reset index.
/// This can be used in place of degenerate triangles, to mark
/// the start/end of a triangle strip.
inline void gpuSetPrimitiveResetIndex(uint index);

/// Renders primitives from an index buffer.
inline void gpuDrawIndexedPrimitive(gpuPrimType prim, uint nindices, gpuIndexType indexformat, uint nverts);

/// Renders primitives.
inline void gpuDrawPrimitive(gpuPrimType prim, uint nverts);

//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_RENDER_STATES Render States
	Render states.
	@{
*/

/** \enum gpuCullMode

	\code
	enum gpuCullMode
	{
		GPU_CULL_CW,
		GPU_CULL_CCW,
		GPU_CULL_NONE
	};
	\endcode
*/

/// Sets the cull mode.
inline void gpuSetCullMode(gpuCullMode cull);

/** \enum gpuFillMode

	\code
	enum gpuFillMode
	{
		GPU_FILL_POINT,
		GPU_FILL_WIREFRAME,
		GPU_FILL_SOLID
	};
	\endcode
*/

/// Sets the fill mode.
inline void gpuSetFillMode(gpuFillMode fill);

/** \enum gpuShadeMode

	\code
	enum gpuShadeMode
	{
		GPU_SHADE_FLAT,
		GPU_SHADE_SMOOTH,
	};
	\endcode
*/

/// Sets the shade mode.
inline void gpuSetShadeMode(gpuShadeMode shade);

/// Disables the Z-test.
/// \see gpuSetZTest
inline void gpuDisableZTest();

/** \enum gpuCmpMode

	\code
	enum gpuCmpMode
	{
		GPU_CMP_NEVER,
		GPU_CMP_ALWAYS,
		GPU_CMP_LESS,
		GPU_CMP_LESSEQUAL,
		GPU_CMP_GREATER,
		GPU_CMP_GREATEREQUAL,
		GPU_CMP_EQUAL,
		GPU_CMP_NOTEQUAL,
	};
	\endcode
*/

/// Sets the Z-test.
/// \see gpuDisableZTest
inline void gpuSetZTest(gpuCmpMode cmp);

/// Enables Z-write
inline void gpuEnableZWrite(bool enable);

// Alpha blending related render states.

/// Disables alpha blending.
/// \see gpuSetAlpha
inline void gpuDisableAlpha();

/** \enum gpuBlendOp

	\code
	enum gpuBlendOp
	{
		GPU_BLENDOP_ADD,
		GPU_BLENDOP_SUBTRACT,
		GPU_BLENDOP_REVSUBTRACT,
		GPU_BLENDOP_MIN,
		GPU_BLENDOP_MAX,
	};
	\endcode
*/

/** \enum gpuBlendMode

	\code
	enum gpuBlendMode
	{
		GPU_BLEND_ZERO,
		GPU_BLEND_ONE,
		GPU_BLEND_SRCCOLOR,
		GPU_BLEND_INVSRCCOLOR,
		GPU_BLEND_SRCALPHA,
		GPU_BLEND_INVSRCALPHA,
		GPU_BLEND_DESTALPHA,
		GPU_BLEND_INVDESTALPHA,
		GPU_BLEND_DESTCOLOR,
		GPU_BLEND_INVDESTCOLOR,
		GPU_BLEND_SRCALPHASAT,
		// etc
	};
	\endcode
*/

/// Sets the alpha blend parameters.
/// \see gpuDisableAlpha
/// \todo State by state settings.
inline void gpuSetAlpha(gpuBlendMode src, gpuBlendMode dest, uint constant=0, gpuBlendOp op=GPU_BLENDOP_ADD);

// Separate alpha blend render states (affecting alpha only).

/** Disables separate alpha blending.
*/
inline void gpuDisableSeparateAlpha();

/// Sets the alpha blend parameters.
inline void gpuSetSeparateAlpha(
	gpuBlendMode colorsrc, gpuBlendMode colordest, uint colorconstant, 
	gpuBlendMode alphasrc, gpuBlendMode alphadest, uint alphaconstant, 
	gpuBlendOp colorop=GPU_BLENDOP_ADD, gpuBlendOp alphaop=GPU_BLENDOP_ADD );

// Alpha test render states

/// Disables alpha test.
/// \see gpuSetAlphaTest
inline void gpuDisableAlphaTest();

/// Sets the alpha test parameters.
/// \see gpuDisableAlphaTest
inline void gpuSetAlphaTest(gpuCmpMode cmp, uint ref);

/** \enum gpuStencilOp

	\code
	enum gpuStencilOp
	{
		GPU_STENCIL_KEEP,
		GPU_STENCIL_ZERO,
		GPU_STENCIL_REPLACE,
		GPU_STENCIL_INCREMENT,
		GPU_STENCIL_INCREMENT_WRAP,
		GPU_STENCIL_DECREMENT,
		GPU_STENCIL_DECREMENT_WRAP,
		GPU_STENCIL_INVERT,
	};
	\endcode
*/

/// Disables stencil operations.
/// \see gpuSetStencil
inline void gpuDisableStencil();

/// Sets the stencil parameters.
/// \see gpuDisableStencil
inline void gpuSetStencil(gpuCmpMode op, uint ref, gpuStencilOp pass, gpuStencilOp fail, gpuStencilOp zfail, 
                          uint mask=~0, uint writemask=~0);

/// Disables two-sided stencil support.
/// \see gpuSetTwoSidedStencil
inline void gpuDisableTwoSidedStencil();

/** Sets two-sided stencil parameters.
	\note  Don't forget to disable culling when using two sided stencil mode!
	\pc
		ref, mask, and writemask are all ignored
	\see gpuDisableTwoSidedStencil
*/
inline void gpuSetTwoSidedStencil(gpuStencilOp pass, gpuStencilOp fail, gpuStencilOp zfail, gpuCmpMode op, 
								  uint ref, uint mask=~0, uint writemask=~0);

//@}

//___________________________________________________________________________

/** \defgroup NGL_GPU_MISC Misc
	Miscellaneous states.
	@{
*/

/** \enum gpuWriteMask

	\code
	enum gpuWriteMask
	{
		GPU_WRITE_R,
		GPU_WRITE_G,
		GPU_WRITE_B,
		GPU_WRITE_A,
		GPU_WRITE_RGB,
		GPU_WRITE_RGBA,
	};
	\endcode
*/

/// Sets the color write mask.
inline void gpuSetColorWrite(gpuWriteMask mask);

/// Sets the color write mask.
inline void gpuSetColorWrite(uint target, gpuWriteMask mask);

/// Enables gamma correction.
inline void gpuEnableGammaCorrect(bool enable);

/// Disables scissoring.
inline void gpuDisableScissor();

/// Sets the scissor rectangle.
inline void gpuSetScissor(int x1, int y1, int x2, int y2);

/// Sets the viewport rectangle and z range
inline void gpuSetViewport(int x, int y, int w, int h, float minZ, float maxZ );

/// Enables dithering.
inline void gpuEnableDither(bool enable);

/** \enum gpuClearMask

	\code
	enum gpuClearMask
	{
		GPU_CLEAR_COLOR,
		GPU_CLEAR_DEPTH,
		GPU_CLEAR_STENCIL,
		GPU_CLEAR_ALL
	};
	\endcode
*/

// ARGB format
#define GPU_CLEAR_COLOR(r,g,b,a) ( \
	(unsigned(tl_clamp(r,0.0f,1.0f)*255)<<16) | \
	(unsigned(tl_clamp(g,0.0f,1.0f)*255)<<8) | \
	(unsigned(tl_clamp(b,0.0f,1.0f)*255)<<0) | \
	(unsigned(tl_clamp(a,0.0f,1.0f)*255)<<24) )

/** Clears the render target.
	Only the components selected by 'flags' will be cleared.
	\param flags    A combination of GPU_CLEAR_XXXX flags.
	\param color    Color to clear the render target to.
	\param depth    Depth value to clear the Z-buffer to.
	\param stencil  Value to clear the stencil buffer to.
	\see gpuClearMask
*/
inline void gpuClear(gpuClearMask flags, uint color, float depth, uint stencil);

/// Sets a depth-bias.
inline void gpuSetDepthBias(float slopebias, float depthbias);

/** Controls the balance between vertex and pixel shaders.
	\param Balance   Power going to the pixel unit. 1.0=full pixel, 0.0=full vertex, 0.5=default
*/
inline void gpuSetShaderBalance(float Balance);

// Initialization / presentation functions

/// Initializes the gpu layer.
void gpuInit(gpuDevice* device);

/// Displays the current frame.
inline void gpuPresent();

/// Invalidates the currently cached states.
inline void gpuStateInvalidate()
{
	for (uint n = 0; n < GPU_NUM_SAMPLERS; n++)
		gpuStateCache.Texture[n] = 0xdeadbeef;
	for (uint n = 0; n < GPU_NUM_STREAMS; n++)
		gpuStateCache.VertexBuffer[n] = 0xdeadbeef;
	gpuStateCache.IndexBuffer   = 0xdeadbeef;
	gpuStateCache.PixelShader   = 0xdeadbeef;
	gpuStateCache.VertexShader  = 0xdeadbeef;
	gpuStateCache.VertexFormat  = 0xdeadbeef;
	gpuStateCache.ShaderBalance = 12345678.0f;
}

/// Clears any objects currently set on the GPU.
inline void gpuClearDevice()
{
	gpuClearVertexBuffer();
	gpuClearIndexBuffer();
	gpuClearVertexShader();
	gpuClearPixelShader();
	for (uint n=0;n<GPU_NUM_SAMPLERS;n++)
		gpuClearSamplerTexture(n);
}

#if NGL_MULTITHREADED 

/// Stalls the current thread until it can acquire ownership of the D3D device.
NGL_API void nglGpuAcquireDevice();

/// Releases ownership of the D3D device to other threads.
NGL_API void nglGpuReleaseDevice();

/// Initializes the mutex for the device.
NGL_API void nglGpuInitDeviceLock();

#else

inline void nglGpuAcquireDevice() {}
inline void nglGpuReleaseDevice() {}
inline void nglGpuInitDeviceLock() {}

#endif

// NGL_GPU_MISC
//@}

//___________________________________________________________________________

// Per-platform implementation.
#if NGL_GPU_D3D
#include "texture_export/ngl/dx/ngl_dx_gpu.h"
#elif NGL_GPU_GCM
#include "ps3/ngl_ps3_gpu.h"
#elif NGL_GPU_REV
#include "rev/ngl_rev_gpu.h"
#endif

#endif

// NGL_GPU
//@}
