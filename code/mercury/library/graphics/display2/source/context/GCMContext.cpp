#include "StdAfx.h"
#include "Display2/Context/GCMContext.h"

namespace Display2
{

U32 AddressToOffset(void* pAddress)
{
	U32 offset = UINT_MAX;
	if(cellGcmAddressToOffset(pAddress, &offset) == CELL_OK)
		return offset;
	else
		return UINT_MAX;
}

E_SurfaceFormat GetSurfaceFormatFromTexture(const E_TextureFormat format)
{
	return GetSurfaceFromTextureFormat(format);
}

E_SurfaceFormat GetSurfaceFromTextureFormat(const E_TextureFormat format)
{
	switch(format)
	{
	case E_TextureFormat_A8R8G8B8:			return E_SurfaceFormat_A8R8G8B8;
	case E_TextureFormat_D24S8:				return E_SurfaceFormat_D24S8;
	case E_TextureFormat_R32F:				return E_SurfaceFormat_R32F;
	case E_TextureFormat_A16B16G16R16F:		return E_SurfaceFormat_A16B16G16R16F;
	case E_TextureFormat_L8:				return E_SurfaceFormat_L8;
	case E_TextureFormat_Unknown:			return E_SurfaceFormat_Unknown;
	default:
		BIZ_ASSERT_FAIL("Unsupported texture format %i", format);
		return E_SurfaceFormat_Unknown;
	}
}

void CpuSetLabel(const U32 index, const U32 value)
{
	*(cellGcmGetLabelAddress(index)) = value;
}

void GpuSetLabelOnCommandEnd(const U32 index, const U32 value)
{
	BIZ_ASSERT(index >= 64 && index <= 255);
	cellGcmSetWriteCommandLabel(gCellGcmCurrentContext, index, value);
}

void GpuSetLabelOnTextureEnd(const U32 index, const U32 value)
{
	BIZ_ASSERT(index >= 64 && index <= 255);
	cellGcmSetWriteTextureLabel(gCellGcmCurrentContext, index, value);
}

void GpuSetLabelOnRasterEnd(const U32 index, const U32 value)
{
	BIZ_ASSERT(index >= 64 && index <= 255);
	cellGcmSetWriteBackEndLabel(gCellGcmCurrentContext, index, value);
}

void CpuWaitOnLabel(const U32 index, const U32 value)
{
	volatile U32 labelValue = *(cellGcmGetLabelAddress(index));
	while (labelValue != value)
	{
		sys_timer_usleep(100);
		labelValue = *(cellGcmGetLabelAddress(index));
	}
}

void GpuWaitOnLabel(const U32 index, const U32 value)
{
	BIZ_ASSERT(index >= 64 && index <= 255);
	cellGcmSetWaitLabel(gCellGcmCurrentContext, index, value);
}

C_BlendMode::C_BlendMode() :
		m_AlphaTestEnable(E_Bool_False),
		m_BlendEnable(E_Bool_False)
{
}

C_VideoMode::C_VideoMode() :
		m_Width(0),
		m_Height(0),
		m_WideScreen(0),
		m_HiDef(0)
{
}

C_TileInfo::C_TileInfo()
{
	SetToDefaults();
}

void C_TileInfo::SetToDefaults()
{
	enable							= false;
	indirect						= false;
	flags							= 0;

	index							= INVALID_TILE_INDEX;
	base							= INVALID_TILE_MEMORY;
	bank							= 0;
	
	location						= 0;
	offset							= 0;
	size							= 0;
	pitch							= 0;
	comp							= 0;
	
	name							= "";
}

C_SharedTiledArea::C_SharedTiledArea()
{
	totalSize = 0;
	size = 0;
	pAddress = NULL;
}

C_ZcullInfo::C_ZcullInfo()
{
	SetToDefaults();
}

void C_ZcullInfo::SetToDefaults()
{
	enable							= false;
	flags							= 0;

	index							= INVALID_ZCULL_INDEX;
	cullStart						= INVALID_ZCULL_MEMORY;

	size							= 0;
	offset							= 0;
	width							= 0;
	height							= 0;
	aaFormat						= 0;

	zFormat							= CELL_GCM_ZCULL_Z24S8;
	zcullDir						= CELL_GCM_ZCULL_GREATER;
	zcullFormat						= CELL_GCM_ZCULL_MSB;
	sFunc							= CELL_GCM_SCULL_SFUNC_EQUAL;
	sRef							= 0xFF;
	sMask							= 0xFF;
}

C_RenderBufferSetup::C_RenderBufferSetup()
{
	m_ColourMemorySize				= 0;
	m_pColourMemory					= NULL;
	m_ColourMemoryUsed				= 0;

	m_DepthMemorySize				= 0;
	m_pDepthMemory					= NULL;
	m_DepthMemoryUsed				= 0;
}

C_RenderBuffer::C_RenderBuffer()
{
	SetToDefaults();
}

void C_RenderBuffer::SetToDefaults()
{
	m_Name							= "";
	m_Width							= 0;
	m_Height						= 0;
	m_Levels						= 1;
	m_Flags							= 0;
	m_Multisample					= E_Multisample_None;
	
	for(U32 i=0; i<4; i++)
	{
		m_ColourFlags[i]			= 0;
		m_ColourSize[i]				= 0;
		m_ColourOffset[i]			= UINT_MAX;
		m_pColourAddress[i]			= NULL;
		m_ColourSurfaceFormat[i]	= E_SurfaceFormat_Unknown;
		m_ColourTextureFormat[i]	= E_TextureFormat_Unknown;

		m_ColourTileInfo[i].SetToDefaults();

		m_ColourTileInfo[i].bank	= 0;
	}

	m_DepthStencilFlags				= 0;
	m_DepthStencilSize				= 0;
	m_DepthStencilOffset			= UINT_MAX;
	m_pDepthStencilAddress			= NULL;
	m_DepthStencilSurfaceFormat		= E_SurfaceFormat_Unknown;
	m_DepthStencilTextureFormat		= E_TextureFormat_Unknown;

	m_DepthStencilTileInfo.SetToDefaults();
	m_DepthStencilZcullInfo.SetToDefaults();

	m_DepthStencilTileInfo.bank		= 1;

	m_OldWidth						= 0;
	m_OldHeight						= 0;
	m_OldMultisample				= E_Multisample_None;
}

const Util::C_String32& C_RenderBuffer::GetName() const
{
	return m_Name;
}

void C_RenderBuffer::SetName(const char* name)
{
	m_Name = name;
}

void C_RenderBuffer::SetWidth(const U32 width)
{
	m_Width = m_OldWidth = width;
}

void C_RenderBuffer::SetHeight(const U32 height)
{
	m_Height = m_OldHeight = height;
}

void C_RenderBuffer::SetLevels(const U32 levels)
{
	m_Levels = levels;
}

void C_RenderBuffer::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_RenderBuffer::SetMultisample(const E_Multisample multisample)
{
	m_Multisample = m_OldMultisample = multisample;
}

void C_RenderBuffer::SetColourFlags(const U32 i, const U32 flags)
{
	BIZ_ASSERT(i < 4);
	m_ColourFlags[i] = flags;
}

void C_RenderBuffer::SetColourOffset(const U32 i, const U32 offset)
{
	BIZ_ASSERT(i < 4);
	m_ColourOffset[i] = offset;
}

void C_RenderBuffer::SetColourAddress(const U32 i, U8* pAddress)
{
	BIZ_ASSERT(i < 4);
	m_pColourAddress[i] = pAddress;
}

void C_RenderBuffer::SetColourFormat(const U32 i, const E_TextureFormat textureFormat)
{
	BIZ_ASSERT(i < 4);
	m_ColourTextureFormat[i] = textureFormat;
	m_ColourSurfaceFormat[i] = GetSurfaceFromTextureFormat(textureFormat);
}

void C_RenderBuffer::SetColourSurfaceFormat(const U32 i, const E_SurfaceFormat surfaceFormat)
{
	BIZ_ASSERT(i < 4);
	m_ColourSurfaceFormat[i] = surfaceFormat;
}

void C_RenderBuffer::SetColourTextureFormat(const U32 i, const E_TextureFormat textureFormat)
{
	BIZ_ASSERT(i < 4);
	m_ColourTextureFormat[i] = textureFormat;
}

void C_RenderBuffer::SetColourTileEnable(const U32 i, const bool enable, const bool indirect)
{
	BIZ_ASSERT(i < 4);
	m_ColourTileInfo[i].enable = enable;
	m_ColourTileInfo[i].indirect = indirect;
}

void C_RenderBuffer::SetDepthStencilFlags(const U32 flags)
{
	m_DepthStencilFlags = flags;
}

void C_RenderBuffer::SetDepthStencilOffset(const U32 offset)
{
	m_DepthStencilOffset = offset;
}

void C_RenderBuffer::SetDepthStencilAddress(U8* pAddress)
{
	m_pDepthStencilAddress = pAddress;
}

void C_RenderBuffer::SetDepthStencilFormat(const E_TextureFormat textureFormat)
{
	switch(textureFormat)
	{
	case E_TextureFormat_D24S8:
		m_DepthStencilTextureFormat = E_TextureFormat_A8R8G8B8;
		break;
	default:
		m_DepthStencilTextureFormat = textureFormat;
		break;
	}
	m_DepthStencilSurfaceFormat = GetSurfaceFromTextureFormat(textureFormat);
}

void C_RenderBuffer::SetDepthStencilSurfaceFormat(const E_SurfaceFormat surfaceFormat)
{
	m_DepthStencilSurfaceFormat = surfaceFormat;
}

void C_RenderBuffer::SetDepthStencilTextureFormat(const E_TextureFormat textureFormat)
{
	m_DepthStencilTextureFormat = textureFormat;
}

void C_RenderBuffer::SetDepthStencilTileEnable(const bool enable, const bool indirect)
{
	m_DepthStencilTileInfo.enable = enable;
	m_DepthStencilTileInfo.indirect = indirect;
}

void C_RenderBuffer::SetDepthStencilZcullEnable(const bool enable)
{
	m_DepthStencilZcullInfo.enable = enable;
}

void C_RenderBuffer::ChangeResolution(const U32 width, const U32 height, const E_Multisample multisample)
{
	if(width == m_Width && height == m_Height && multisample == m_Multisample)
		return;

	BIZ_ASSERT(m_Width > 0);
	BIZ_ASSERT(m_Height > 0);

	m_OldWidth = m_Width;
	m_OldHeight = m_Height;
	m_OldMultisample = m_Multisample;

	BIZ_ASSERT(width > 0);
	BIZ_ASSERT(height > 0);

	m_Width = width;
	m_Height = height;
	m_Multisample = multisample;

	UpdateResolution();
}

void C_RenderBuffer::RestoreResolution()
{
	if(m_Width == m_OldWidth && m_Height == m_OldHeight && m_Multisample == m_OldMultisample)
		return;

	BIZ_ASSERT(m_OldWidth > 0);
	BIZ_ASSERT(m_OldHeight > 0);

	m_Width = m_OldWidth;
	m_Height = m_OldHeight;
	m_Multisample = m_OldMultisample;

	UpdateResolution();
}

void C_RenderBuffer::UpdateResolution()
{
	BIZ_ASSERT(m_Width > 0);
	BIZ_ASSERT(m_Height > 0);

	for(U32 i=0; i<4; i++)
	{
		if(m_ColourSurfaceFormat[i] != E_SurfaceFormat_Unknown)
		{
			C_Surface& surface = m_ColourBuffer[i].GetSurface();

			surface.m_Width = m_Width;
			surface.m_Height = m_Height;
			surface.m_Multisample = m_Multisample;
			//surface.m_Pitch = GetSurfacePitchFromWidth(surface.GetWidth(), surface.GetFlags(), surface.GetFormat(), (E_Multisample)surface.GetMultisample());

			C_Texture& texture = m_ColourBuffer[i].GetTexture();

			texture.m_gcmTexture.width = GetTextureWidthFromMultisample(m_Width, m_Multisample);
			texture.m_gcmTexture.height = GetTextureHeightFromMultisample(m_Height, m_Multisample);
			//texture.m_gcmTexture.pitch = GetTexturePitch(texture.GetWidth(), texture.GetFlags(), texture.GetFormat());
		}
	}

	if(m_DepthStencilSurfaceFormat != E_SurfaceFormat_Unknown)
	{
		C_Surface& surface = m_DepthStencilBuffer.GetSurface();

		surface.m_Width = m_Width;
		surface.m_Height = m_Height;
		surface.m_Multisample = m_Multisample;
		//surface.m_Pitch = GetSurfacePitchFromWidth(surface.GetWidth(), surface.GetFlags(), surface.GetFormat(), (E_Multisample)surface.GetMultisample());

		C_Texture& texture = m_DepthStencilBuffer.GetTexture();

		texture.m_gcmTexture.width = GetTextureWidthFromMultisample(m_Width, m_Multisample);
		texture.m_gcmTexture.height = GetTextureHeightFromMultisample(m_Height, m_Multisample);
		//texture.m_gcmTexture.pitch = GetTexturePitch(texture.m_gcmTexture.width, texture.GetFlags(), texture.GetFormat());
	}
}

bool C_RenderBuffer::IsCreated() const
{
	if(	m_ColourSurfaceFormat[0] != E_SurfaceFormat_Unknown ||
		m_ColourSurfaceFormat[1] != E_SurfaceFormat_Unknown ||
		m_ColourSurfaceFormat[2] != E_SurfaceFormat_Unknown ||
		m_ColourSurfaceFormat[3] != E_SurfaceFormat_Unknown ||
		m_DepthStencilSurfaceFormat != E_SurfaceFormat_Unknown)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void C_RenderBuffer::OverlapColourBuffer(const U32 i, const U32 size, U8* pAddress)
{
	BIZ_ASSERT(i < 4);
	m_ColourSize[i] = size;
	m_pColourAddress[i] = pAddress;
}

void C_RenderBuffer::OverlapDepthStencilBuffer(const U32 size, U8* pAddress)
{
	m_DepthStencilSize = size;
	m_pDepthStencilAddress = pAddress;
}

C_Surface& C_RenderBuffer::GetSurface()
{
	BIZ_ASSERT(GetColourSurface(0));
	return *GetColourSurface(0);
}

const C_Surface& C_RenderBuffer::GetSurface() const
{
	BIZ_ASSERT(GetColourSurface(0));
	return *GetColourSurface(0);
}

C_Texture& C_RenderBuffer::GetTexture()
{
	BIZ_ASSERT(GetColourTexture(0));
	return *GetColourTexture(0);
}

const C_Texture& C_RenderBuffer::GetTexture() const
{
	BIZ_ASSERT(GetColourTexture(0));
	return *GetColourTexture(0);
}

} // namespace Display2
