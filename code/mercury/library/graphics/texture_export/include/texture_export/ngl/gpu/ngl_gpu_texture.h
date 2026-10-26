#ifndef _NGL_GPU_TEXTURE_H_
#define _NGL_GPU_TEXTURE_H_

namespace apk
{
	class apkFile;
}

// Texture flags.
enum
{
	// Flags to be passed to nglCreateTexture.
	NGLTEX_ANIMATED          = 0x00000001, //< Texture is an IFL file (that is, the Frames array is valid and there is no texture data).
	NGLTEX_CUBE              = 0x00000002, //< Texture is a cubemap.
	NGLTEX_VOLUME            = 0x00000004, //< Texture is a volume texture.

	NGLTEX_DATA_TEMP         = 0x00000010, //< Texture data will be allocated in a temporary area on each frame the texture is used.
	NGLTEX_DATA_OWNER        = 0x00000020, //< If set, the texture owns its data and will release it when nglDestroyTexture is called.

	NGLTEX_RENDER_TARGET     = 0x00000040, //< The texture will be created as a render target.
	NGLTEX_Z_TARGET          = 0x00000080, //< The texture will be created as a depth target.

	NGLTEX_AUTO_Z_TARGET     = 0x00000100, //< Automatically creates a compatible Z buffer when creating the texture.
	NGLTEX_FRAMEBUFFER       = 0x00000200, //< Uses a special coordinate system when rendering 2D UI elements into the texture.

#if NGL_PS3
	NGLTEX_TILED             = 0x00001000, //< Texture will be in tiled memory (PS3).
	NGLTEX_ZCULL             = 0x00002000, //< Texture will be in zcull memory (PS3).

	NGLTEX_MAIN_MEMORY       = 0x00004000, //< Texture will be allocated in main memory (PS3).
	NGLTEX_LOCAL_MEMORY      = 0x00008000, //< Texture will be allocated in local memory (PS3).

	NGLTEX_MULTISAMPLE_2X    = 0x00010000, //< Texture will be used as a render target with 2X multisampling, which causes the width to be doubled (PS3).
	NGLTEX_MULTISAMPLE_4X    = 0x00020000, //< Texture will be used as a render target with 4X multisampling, which causes the width and height to be doubled (PS3).
#else
	NGLTEX_TILED             = 0,
	NGLTEX_ZCULL             = 0,

	NGLTEX_MAIN_MEMORY       = 0,
	NGLTEX_LOCAL_MEMORY      = 0,

	NGLTEX_MULTISAMPLE_2X    = 0,
	NGLTEX_MULTISAMPLE_4X    = 0,
#endif

	// Internal NGL flags.
	NGLTEX_PREDICATED_TILING = 0x01000000, //< Texture needs predicated tiling (Xenon).  This flag can be removed in the future.

	NGLTEX_SWIZZLED          = 0x02000000, //< Texture data is swizzled (as opposed to linear).
	NGLTEX_YUY2              = 0x04000000, //< Texture data is in YUY2 format (obsolete?).
};

struct NGL_API nglTexture
{
	apk::apkFile  * File;  //< File containing this texture.
	int            LastFrameRef; //< Last frame number this resource was referenced by.
	Util::C_String FileName; //< Original filename.
	uint           Flags;  //< Flags describing the texture.

	gpuTexture     Texture; //< Texture object.
	gpuSurface     RenderTarget; //< Render target/depth buffer.

	nglTexture    * ZTexture; //< ZBuffer if this texture is a render target created with NGLTEX_AUTO_Z_TARGET.

	// Pointers to animated texture frames.
	uint           NFrames; //< Number of texture pointers in Frames array.
	nglTexture   ** Frames; //< Array of texture ptrs for the individual frames.

	int GetWidth() const
	{
		return Texture.GetWidth();
	}

	int GetHeight() const
	{
		return Texture.GetHeight();
	}

	int GetDepth() const
	{
		return Texture.GetDepth();
	}
};

INLINE const Util::C_String& GetKeyPtr(const nglTexture* Texture)
{
	return Texture->FileName;
}

/** Creates a new texture, allocating memory for the texture structure and data.
 \param Flags     A combination of the NGLTEX_XXXX flags.
 \param Format    Pixel Format.
 \param Width     Width in pixels.
 \param Height    Height in pixels.
 \param Depth     Depth for volume textures.  Ignored unless NGLTEX_VOLUME is passed.
 \param NLevels   Number of mipmaps levels (1 for no mimpaps).
 */
NGL_API nglTexture* nglCreateTexture(u32 Flags, u32 Format, int Width, int Height, int Depth, int Levels = 1);

/// Map flags.
enum
{
	// Filtering mode.
	NGLMAP_POINT_FILTER         = 0x00000000, ///< Point filter (nearest)
	NGLMAP_BILINEAR_FILTER      = 0x00000001, ///< Bilinear filter
	NGLMAP_TRILINEAR_FILTER     = 0x00000002, ///< Trilinear filter (blends between mipmap levels)
	NGLMAP_ANISOTROPIC_FILTER   = 0x00000003, ///< Anisotropic filter (takes view angle into account)

	NGLMAP_FILTER_BITS          = 0x00000003, ///< Mask for all filters

	// Clamping mode.  The default is to wrap.
	NGLMAP_CLAMP_U              = 0x00000010, ///< Clamp along the U axis
	NGLMAP_CLAMP_V              = 0x00000020, ///< Clamp along the V axis
	NGLMAP_CLAMP_W              = 0x00000040, ///< Clamp along the W axis
	NGLMAP_CLAMP                = NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_CLAMP_W, ///< Clamp along the U, V and W axes
	NGLMAP_WRAP_U               = 0, ///< Wrap along the U axis
	NGLMAP_WRAP_V               = 0, ///< Wrap along the V axis
	NGLMAP_WRAP_W               = 0, ///< Wrap along the W axis
	NGLMAP_WRAP                 = 0, ///< Wrap along the U, V and W axes
};

/** Assigns a texture to a stage and sets its mapping and filtering parameters.
 \param idx            Texture stage.
 \param Tex            Texture object.
 \param MapFlags       Combination of NGLMAP_xxx flags.
 \param MaxAnisotropy  Maximum anisotropy.
 */
NGL_API void nglGpuSetTexture(gpuSampler idx, nglTexture* Tex, const u32 MapFlags, const u32 MaxAnisotropy = 3);

#endif // _NGL_GPU_TEXTURE_H_
