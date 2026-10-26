#include "StdAfx.h"
#include "Display2/Context/WiiContext.h"

namespace Display2
{
	
E_SurfaceFormat GetSurfaceFormatFromTexture(const E_TextureFormat format)
{
	return (E_SurfaceFormat)format;
}

U32 GetSurfacePitchFromWidth(U32 width, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample)
{
	U32 pitch = 0;

	switch(format)
	{
	case E_SurfaceFormat_L8:
		pitch = width * 1;
		break;
	case E_SurfaceFormat_D16:
		pitch = width * 2;
		break;
	case E_SurfaceFormat_A8R8G8B8:
	case E_SurfaceFormat_D24S8:
	case E_SurfaceFormat_R32F:
		pitch = width * 4;
		break;
	case E_SurfaceFormat_A16B16G16R16F:
		pitch = width * 8;
		break;
	case E_SurfaceFormat_A32B32G32R32F:
		pitch = width * 16;
		break;
	default:
		BIZ_ASSERT(0 && "Unsupported surface format");
		break;
	}

	switch(multisample)
	{
	case E_Multisample_None:
		break;
	case E_Multisample_2:
	case E_Multisample_4:
		pitch = pitch * 2;
		break;
	default:
		BIZ_ASSERT(0 && "Unsupported surface multisample");
		break;
	}

	return pitch;
}

U32 GetSurfaceSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample)
{
	switch(multisample)
	{
	case E_Multisample_None:
	case E_Multisample_2:
		break;
	case E_Multisample_4:
		height = height * 2;
		break;
	default:
		BIZ_ASSERT(0 && "Unsupported surface multisample");
		break;
	}

	U32 size = pitch * height;

	return size;
}

U32 GetSurfaceSize(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample)
{
	const U32 pitch = GetSurfacePitchFromWidth(width, flags, format, multisample);
	return GetSurfaceSizeFromHeightAndPitch(height, pitch, flags, format, multisample);
}

void GetSurfaceSizeAndPitch(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample, U32& size, U32& pitch)
{
	pitch = GetSurfacePitchFromWidth(width, flags, format, multisample);
	size = GetSurfaceSize(width, height, flags, format, multisample);
}

U32 GetTexturePitchFromWidth(U32 width, const U32 flags, const E_TextureFormat format)
{
	U32 pitch = 0;

	switch(format)
	{
	case E_TextureFormat_DXT1:
		pitch = ((width + 3) / 4) * 8;
		break;
	case E_TextureFormat_DXT3:
	case E_TextureFormat_DXT5:
		pitch = ((width + 3) / 4) * 16;	
		break;
	case E_TextureFormat_L8:
		pitch = width * 1;
		break;
	case E_TextureFormat_D16:
		pitch = width * 2;
		break;
	case E_TextureFormat_A8R8G8B8:
	case E_TextureFormat_D24S8:
	case E_TextureFormat_R32F:
		pitch = width * 4;
		break;
	case E_TextureFormat_A16B16G16R16F:
		return width * 8;
	case E_TextureFormat_A32B32G32R32F:
		return width * 16;
	default:
		BIZ_ASSERT(0 && "Unsupported texture format");
		break;
	}

	return pitch;
}

U32 GetTextureSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_TextureFormat format)
{
	switch(format)
	{
	case E_TextureFormat_DXT1:
	case E_TextureFormat_DXT3:
	case E_TextureFormat_DXT5:
		height = ((height + 3) / 4);
		break;
	default:
		break;
	}

	U32 size = pitch * height;

	return size;
}


U32 GetTexturePitch(U32 width, const U32 flags, const E_TextureFormat format)
{
	return GetTexturePitchFromWidth(width, flags, format);
}

U32 GetTextureRowPitch(U32 width, const U32 flags, const E_TextureFormat format)
{
	return GetTexturePitchFromWidth(width, flags, format);
}

U32 GetTextureSlicePitch(U32 width, U32 height, U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 pitch = GetTexturePitchFromWidth(width, flags, format);
	U32 size = GetTextureSizeFromHeightAndPitch(height, pitch, flags, format);
	return size * levels;
}

U32 GetTexture2DSize(const U32 width, const U32 height, const U32 flags, const E_TextureFormat format)
{
	const U32 pitch = GetTexturePitchFromWidth(width, flags, format);
	return GetTextureSizeFromHeightAndPitch(height, pitch, flags, format);
}

U32 GetTexture2DSize(U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 size = 0;
	for(U32 level=0; level<levels; level++)
	{
		size += GetTexture2DSize(width, height, flags, format);
		if(width > 1)
			width >>= 1;
		if(height > 1)
			height >>= 1;
	}
	return size;
}

void GetTexture2DSizeAndPitch(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch)
{
	pitch = GetTexturePitchFromWidth(width, flags, format);
	size = GetTexture2DSize(width, height, levels, flags, format);
}

U32 GetTexture3DSize(const U32 width, const U32 height, const U32 depth, const U32 flags, const E_TextureFormat format)
{
	return GetTexture2DSize(width, height, flags, format) * depth;
}

U32 GetTexture3DSize(U32 width, U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 size = 0;
	for(U32 level = 0; level < levels; level++)
	{
		size += GetTexture3DSize(width, height, depth, flags, format);
		if(width > 1)
			width >>= 1;
		if(height > 1)
			height >>= 1;
	}
	return size;
}

void GetTexture3DSizeAndPitch(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch)
{
	pitch = GetTexturePitchFromWidth(width, flags, format);
	size = GetTexture3DSize(width, height, depth, levels, flags, format);
}

U32 GetTextureCubeSize(const U32 width, const U32 flags, const E_TextureFormat format)
{
	U32 size = 0;
	for(U32 i=0; i<6; i++)
	{
		size += GetTexture2DSize(width, width, flags, format);
	}
	return size;
}

U32 GetTextureCubeSize(U32 width, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 size = 0;
	for(U32 i=0; i<6; i++)
	{
		size += GetTexture2DSize(width, width, levels, flags, format);
	}
	return size;
}

void GetTextureCubeSizeAndPitch(const U32 width, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch)
{
	pitch = GetTexturePitchFromWidth(width, flags, format);
	size = GetTextureCubeSize(width, levels, flags, format);
}

U32 GetTexture2DLevelOffset(const U32 level, U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 offset = 0;

	for(U32 i=0; i<level; i++)
	{
		if(i >= levels)
			return offset;

		const U32 size = GetTexture2DSize(width, height, flags, format);

		offset += size;

		if(width > 1)
			width /= 2;
		if(height > 1)
			height /= 2;
	}

	return offset;
}

U32 GetTexture3DLevelOffset(const U32 level, U32 width, U32 height, U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 offset = 0;

	for(U32 i=0; i<level; i++)
	{
		if(i >= levels)
			return offset;

		const U32 size = GetTexture3DSize(width, height, depth, flags, format);

		offset += size;

		if(width > 1)
			width /= 2;
		if(height > 1)
			height /= 2;
		if(depth > 1)
			depth /= 2;
	}

	return offset;
}

U32 GetTextureCubeLevelOffset(const E_CubeMapFace face, const U32 level, U32 width, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 offset = 0;

	for(S32 f=0; f<face; f++)
	{
		const U32 size = GetTexture2DSize(width, width, levels, flags, format);

		offset += size;
	}

	for(U32 l=0; l<level; l++)
	{
		if(l >= levels)
			return offset;

		const U32 size = GetTextureCubeSize(width, flags, format);

		offset += size;

		if(width > 1)
			width /= 2;
	}

	return offset;
}

const Util::C_String GetTextureFormatAsString(const E_TextureFormat format)
{
	switch(format)
	{
	case E_TextureFormat_A8R8G8B8:					return "A8R8G8B8";
	case E_TextureFormat_DXT1:						return "DXT1";
	case E_TextureFormat_DXT3:						return "DXT3";
	case E_TextureFormat_DXT5:						return "DXT5";
	case E_TextureFormat_A16B16G16R16F:				return "A16B16G16R16F";
	case E_TextureFormat_A32B32G32R32F:				return "A32B32G32R32F";
	case E_TextureFormat_D16:						return "D16";
	case E_TextureFormat_D24S8:						return "D24S8";
	case E_TextureFormat_R32F:						return "R32F";
	case E_TextureFormat_L8:						return "L8";
	case E_TextureFormat_G16R16:					return "G16R16";
	case E_TextureFormat_G16R16F:					return "G16R16F";

	default:										return "Unknown";
	}
}

// C_Viewport

C_Viewport::C_Viewport()
{
}

C_Viewport::C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height)
{
}

C_Viewport::C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height, const F32 minZ, const F32 maxZ)
{
}

// C_VertexElement

C_VertexElement::C_VertexElement()
{
}

C_VertexElement::C_VertexElement(const U16 stream, const U32 offset, const U32 type, const U8 usage, const U8 usageIndex)
{
}

void C_VertexElement::SetStream(const U16 stream)
{
}

void C_VertexElement::SetOffset(const U32 offset)
{
}

void C_VertexElement::SetType(const U32 type)
{
}

void C_VertexElement::SetUsage(const U8 usage)
{
}

void C_VertexElement::SetUsageIndex(const U8 usageIndex)
{
}

// C_VertexDeclaration

C_VertexDeclaration::C_VertexDeclaration()
{
}

C_VertexDeclaration::~C_VertexDeclaration()
{
}

S32 C_VertexDeclaration::GetElementCount() const
{
	return 0;
}

U32 C_VertexDeclaration::GetStride(const U32 stream) const
{
	return 0;
}

// C_VertexBuffer

C_VertexBuffer::C_VertexBuffer()
{
}

C_VertexBuffer::~C_VertexBuffer()
{
}

void C_VertexBuffer::SetFlags(const U32 flags)
{
}

void* C_VertexBuffer::Lock(const U32 offset, const U32 size, const U32 flags)
{
	return NULL;
}

void C_VertexBuffer::Unlock()
{
}

// C_IndexBuffer

C_IndexBuffer::C_IndexBuffer()
{
}

C_IndexBuffer::~C_IndexBuffer()
{
}

void C_IndexBuffer::SetFlags(const U32 flags)
{
}

void* C_IndexBuffer::Lock(const U32 offset, const U32 size, const U32 flags)
{
	return NULL;
}

void C_IndexBuffer::Unlock()
{
}


// C_ShaderParam

C_ShaderParam::C_ShaderParam()
{
}

// C_VertexShader

C_VertexShader::C_VertexShader()
{
}

C_VertexShader::~C_VertexShader()
{
}

void C_VertexShader::SetFlags(const U32 flags)
{
}

const C_ShaderParam C_VertexShader::GetParamFromName(const char* name) const
{
	return C_ShaderParam();
}

const C_ShaderParam C_VertexShader::GetParameter(const S32 index, const char* name) const
{
	return GetParamFromName(name);
}

// C_PixelShader

C_PixelShader::C_PixelShader()
{
}

C_PixelShader::~C_PixelShader()
{
}

void C_PixelShader::SetFlags(const U32 flags)
{
}

const C_ShaderParam C_PixelShader::GetParamFromName(const char* name) const
{
	return C_ShaderParam();
}

const C_ShaderParam C_PixelShader::GetParameter(const S32 index, const char* name) const
{
	return GetParamFromName(name);
}

const C_ShaderParam C_PixelShader::GetParameterArray(const S32 index, const char* name, const U32 count) const
{
	return GetParamFromName(name);
}

// C_LockedRect

C_LockedRect::C_LockedRect()
{
	pBits = NULL;
	pitch = 0;
}

// C_ConstLockedRect

C_ConstLockedRect::C_ConstLockedRect()
{
	pBits = NULL;
	pitch = 0;
}

// C_LockedBox

C_LockedBox::C_LockedBox()
{
	pBits = NULL;
	rowPitch = 0;
	slicePitch = 0;
}

// C_ConstLockedBox

C_ConstLockedBox::C_ConstLockedBox()
{
	pBits = NULL;
	rowPitch = 0;
	slicePitch = 0;
}

// C_SurfaceParams

C_SurfaceParams::C_SurfaceParams()
{
}

C_SurfaceParams::~C_SurfaceParams()
{
}

void C_SurfaceParams::OverSurface(const C_Surface* pSurface)
{
}

void C_SurfaceParams::AfterSurface(const C_Surface* pSurface)
{
}

void C_SurfaceParams::BeforeSurface(const C_Surface* pSurface)
{
}

// C_Surface

C_Surface::C_Surface()
{
}

C_Surface::~C_Surface()
{
}

C_LockedRect C_Surface::LockRect(const U32 flags)
{
	return C_LockedRect();
}

void C_Surface::UnlockRect()
{
}

// C_Texture

C_Texture::C_Texture()
{
}

C_Texture::~C_Texture()
{
}

void C_Texture::SetSize(const U32 size)
{
}

void C_Texture::SetFlags(const U32 flags)
{
}

void C_Texture::SetSRGB(const bool enable)
{
}

C_LockedRect C_Texture::LockRect2D(const U32 level, const U32 flags)
{
	return C_LockedRect();
}

C_ConstLockedRect C_Texture::LockRect2D(const U32 level, const U32 flags) const
{
	return C_ConstLockedRect();
}

void C_Texture::UnlockRect2D(const U32 level) const
{
}

C_LockedBox C_Texture::LockBox3D(const U32 level, const U32 flags)
{
	return C_LockedBox();
}

C_ConstLockedBox C_Texture::LockBox3D(const U32 level, const U32 flags) const
{
	return C_ConstLockedBox();
}

void C_Texture::UnlockBox3D(const U32 level) const
{
}

C_LockedRect C_Texture::LockRectCube(const U32 face, const U32 level, const U32 flags)
{
	return C_LockedRect();
}

C_ConstLockedRect C_Texture::LockRectCube(const U32 face, const U32 level, const U32 flags) const
{
	return C_ConstLockedRect();
}

void C_Texture::UnlockRectCube(const U32 face, const U32 level) const
{
}

const Util::C_String C_Texture::GetFileName() const
{
	return "";
}

C_LockedRect C_Texture::Lock(const U32 level)
{
	return C_LockedRect();
}

C_ConstLockedRect C_Texture::Lock(const U32 level) const
{
}

void C_Texture::Unlock(const U32 level) const
{
}

// C_Context

C_Context::C_Context() : m_pState(NULL)
{
}

// C_ContextNoCheck

C_ContextNoCheck::C_ContextNoCheck() : m_pState(NULL)
{
}

// C_ContextNoCheckNoTrack

C_ContextNoCheckNoTrack::C_ContextNoCheckNoTrack()
{
}

} // namespace Display2
