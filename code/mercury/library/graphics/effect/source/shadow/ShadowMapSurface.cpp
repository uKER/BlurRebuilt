#include "stdafx.h"
#include "Effect/Shadow/ShadowMapSurface.h"
#include "Display/Device/DeviceTypes.h"

namespace Effect
{

#ifdef PLATFORM_PS3

bool PS3CreateRenderTextureFromSurface(const Display::C_Surface* pSurface, Display::C_RenderTexture2D* pRenderTexture)
{
	CellGcmTexture* pGcmTexture = &pRenderTexture->GetTexture().m_gcmTexture;
	pGcmTexture->format 			= Display::GCMSurfaceFormat(pSurface->GetFormat()) | CELL_GCM_TEXTURE_LN;
	pGcmTexture->mipmap				= 1;
	pGcmTexture->dimension			= CELL_GCM_TEXTURE_DIMENSION_2;
	pGcmTexture->cubemap			= 0;
	pGcmTexture->remap				=	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
										CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
										CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
										CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
										CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
										CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
										CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
										CELL_GCM_TEXTURE_REMAP_FROM_A;
	pGcmTexture->width			= pSurface->GetWidth();
	pGcmTexture->height			= pSurface->GetHeight();
	pGcmTexture->depth			= 1;
	pGcmTexture->location		= CELL_GCM_LOCATION_LOCAL;
	pGcmTexture->pitch			= pSurface->GetPitch();
	pGcmTexture->offset			= pSurface->GetOffset();
}

bool PS3CreateDepthTextureFromSurface(const Display::C_Surface* pSurface, Display::C_RenderTexture2D* pRenderTexture)
{
	CellGcmTexture* pGcmTexture = &pRenderTexture->GetTexture().m_gcmTexture;
	pGcmTexture->format				= CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_LN;
	pGcmTexture->mipmap				= 1;
	pGcmTexture->dimension			= CELL_GCM_TEXTURE_DIMENSION_2;
	pGcmTexture->cubemap			= 0;
	pGcmTexture->remap				=	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
										CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
										CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
										CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
										CELL_GCM_TEXTURE_REMAP_FROM_G << 6 |
										CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
										CELL_GCM_TEXTURE_REMAP_FROM_A << 2 |
										CELL_GCM_TEXTURE_REMAP_FROM_B;
	pGcmTexture->width				= pSurface->GetWidth();
	pGcmTexture->height				= pSurface->GetHeight();
	pGcmTexture->depth				= 1;
	pGcmTexture->location			= CELL_GCM_LOCATION_LOCAL;
	pGcmTexture->pitch				= pSurface->GetPitch();
	pGcmTexture->offset				= pSurface->GetOffset();
}

#endif

}; // namespace Effect
