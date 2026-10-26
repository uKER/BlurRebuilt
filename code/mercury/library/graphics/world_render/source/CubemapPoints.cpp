#include "stdafx.h"

#include <system/serialise/C_FileBlock.h>
#include <world_render/WorldRender.h>
#include <world_render/LightList.h>
#include <system/debug/I_DebugRender.h>
#include <world_render/CubemapPoints.h>
#include <renderingdata/renderingdata.h>
#include <System/Misc/StringUtil.h>

namespace WorldRender
{

Debug::Bool g_RenderCubemapZones( "Graphics.WorldRender.LocalisedCubemaps.RenderCubemapZones", false );
Debug::Float g_RenderCubemapZonesDrawDistance( "Graphics.WorldRender.LocalisedCubemaps.Settings.DrawDistance", 1000.0f, Debug::Range(0.0f, 10000.0f) );
Debug::Bool g_RenderCubemapNames( "Graphics.WorldRender.LocalisedCubemaps.ShowNames", false);
Debug::Bool g_RenderCubemapLocations( "Graphics.WorldRender.LocalisedCubemaps.RenderCubemapLocations", false );

/// constructor
C_LocalisedCubemapData::C_LocalisedCubemapData()
: m_bFoundTextures(false)
, m_pDefaultTexture(NULL)
{
}

/// destructor
C_LocalisedCubemapData::~C_LocalisedCubemapData()
{
	// decrease refcount on all textures
	for (int i=0; i<m_Locations.Size(); i++)
		RenderingData::Target::SharedTextureManager::Instance().DecreaseRefCount(m_Locations[i].pTexture);

	if (m_pDefaultTexture)
		RenderingData::Target::SharedTextureManager::Instance().DecreaseRefCount(m_pDefaultTexture);
}

/// return a shared texture pointer from the shared texture manager, given a filename
RenderingData::Target::SharedTextureData *C_LocalisedCubemapData::GetSharedTexture(Util::C_String sTextureName)
{
	// get texture pointer from 
	return RenderingData::Target::SharedTextureManager::Instance().FindTexture(
		(Sys::RemoveFileExtension(Sys::ExtractFilename(sTextureName))).Chars(),
		Btl::Hash(sTextureName.Chars()));
}

/// show all localised cubemap zones 
void C_LocalisedCubemapData::DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HVec3 PCREF vCamera )
{
	#ifndef CFG_GOLD
	Maths::HMat43 m;
	m.SetIdentity();

	// render the cubemap zones
	if(g_RenderCubemapZones)
	{
		const F32 drawDistanceSquared = (F32)g_RenderCubemapZonesDrawDistance * (F32)g_RenderCubemapZonesDrawDistance;

		Maths::C_AxisBox bbox(Maths::HVec3(-1.f,-1.f,-1.f), Maths::HVec3(1.f,1.f,1.f));

		for (int i=0; i<m_Zones.Size(); i++)
		{
			const CubemapZone *pZone = &m_Zones[i];
			Maths::HMat44 mtx(pZone->mtx);
			Maths::HMat43 mtxt(mtx.Inverse());

			Maths::HVec3 vZonePos = mtxt.Translation();

			if(vZonePos.DistanceSquared(vCamera) < drawDistanceSquared )
			{
				pDebugRender->Draw3dBox(bbox, mtxt, Maths::RGBA(0, 0, 255));
			}
		}
	}

	if(g_RenderCubemapLocations || g_RenderCubemapNames)
	{
		const F32 drawDistanceSquared = (F32)g_RenderCubemapZonesDrawDistance * (F32)g_RenderCubemapZonesDrawDistance;

		const S32 locationCount = m_Locations.Size();
		for (int i=0; i<locationCount; i++)
		{
			const CubemapLocation* pLocation = &m_Locations[i];

			if(pLocation->vPos.DistanceSquared(vCamera) < drawDistanceSquared )
			{
				if(g_RenderCubemapLocations)
				{
					pDebugRender->Draw3dSphereOutline(pLocation->vPos, 1.0f, Maths::RGBA(255, 0, 255));
				}

				if(g_RenderCubemapNames)
				{
					Util::C_String str;
					str.Printf( "[%s]", pLocation->sName.Chars());

					pDebugRender->Draw3dText( pLocation->vPos, 1.0f, str );
				}

			}
		}
	}

	#endif
}

/// returns a platform texture pointer to the cubemap texture for a given world position in ppOutput
/// returns true if a localised cubemap was found, or false if the default was given
/// if the user intends to use the texture they need to increase it's refcount
bool C_LocalisedCubemapData::GetCubemap(const Maths::HVec3 PCREF v, RenderingData::Target::SharedTextureData **ppTex) const
{
	if (!m_bFoundTextures)
		return false;

	// check whether this point is in a cubemap zone
	int iResult;
	if (m_Zones.Intersect(v, &iResult, 1) == 1)
	{
		*ppTex = m_Locations[m_Zones[iResult].iLocation].pTexture;
		return true;
	}

	*ppTex = m_pDefaultTexture;
	return false;
}

/// find all cubemap textures in the shared texture manager
void C_LocalisedCubemapData::ResolveTextureReferences()
{
	// get texture for each zone
	for (int i=0; i<m_Locations.Size(); i++)
	{
		m_Locations[i].pTexture = GetSharedTexture(m_sLocalisedCubemapPath + "/" + m_Locations[i].sName + ".dds");
		RenderingData::Target::SharedTextureManager::Instance().IncreaseRefCount(m_Locations[i].pTexture);
	}

	// get default texture
	m_pDefaultTexture = GetSharedTexture(m_sDefaultCubemapFilename);
	if (m_pDefaultTexture)
		RenderingData::Target::SharedTextureManager::Instance().IncreaseRefCount(m_pDefaultTexture);

	m_bFoundTextures = true;
}

S32 C_LocalisedCubemapData::GetLocationCount() const
{
	return m_Locations.Size();
}

const CubemapLocation* C_LocalisedCubemapData::GetLocation(S32 index) const
{
	return &m_Locations[index];
}

/// Get GPU texture pointer from Shared Texture, and increase the refcount
const void *C_LocalisedCubemapData::GetGPUTexture(RenderingData::Target::SharedTextureData *pTexture)
{
	RenderingData::Target::SharedTextureManager::Instance().IncreaseRefCount(pTexture);
	return pTexture->GetTexture()->m_data.GetGPUTexture();
}

/// release GPU texture
void C_LocalisedCubemapData::ReleaseGPUTexture(RenderingData::Target::SharedTextureData *pTexture)
{
	RenderingData::Target::SharedTextureManager::Instance().DecreaseRefCount(pTexture);
}

} // namespace WorldRender
