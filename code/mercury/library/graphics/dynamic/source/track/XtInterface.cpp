#include "stdafx.h"

#include "Dynamic/Track/XtInterface.h"
#include "Dynamic/Track/Track.h"
#include "Dynamic/Generic/XtInterface.h"

namespace DynamicEffect
{
namespace Track
{
namespace XtInterface
{

typedef Ut::UtSet<Material*>	Materials;
C_Renderer*						g_pRenderer = NULL;
C_TrackRenderer*				g_pTrackRenderer = NULL;
Materials						m_Materials;

void InvalidateMaterial(Material* pData)
{
	if(pData->pMaterial)
	{
		g_pTrackRenderer->CleanMaterial(pData->pMaterial);
		delete pData->pMaterial;
		pData->pMaterial = NULL;
	}
}

void MaterialCallback(XtEventType event, XtRecord* record, Material* pData, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		pData->bInitialised = false;
		m_Materials.Insert(pData);
	}
	else if(event == kXtEtDestruct)
	{
		InvalidateMaterial(pData);
		m_Materials.Erase(pData);
	}
}

void Initialise(C_TrackRenderer* pTrackRenderer)
{
	g_pTrackRenderer = pTrackRenderer;
	Update();
}

void Update()
{
	for(Materials::Iterator it = m_Materials.Begin(); it != m_Materials.End(); ++it)
	{
		Material* pData = it->GetValue();
		if(!pData->bInitialised)
		{
			Util::C_String diffuse(pData->texture);
			Util::C_String normal(pData->normal);

			DynamicEffect::Track::MaterialSettings settings;
			settings.pDiffuse = DynamicEffect::XtInterface::Renderer()->LoadTexture(diffuse);
			settings.pNormal = DynamicEffect::XtInterface::Renderer()->LoadTexture(normal);
			settings.blendMode = (Display::E_BlendMode)pData->blendMode;

			if(!settings.pDiffuse)
			{
				settings.pDiffuse = (DynamicEffect::C_Texture*)&DynamicEffect::XtInterface::Renderer()->GetGreyTexture();
				settings.blendMode = Display::E_BlendMode_AlphaPreMultiply;
			}
			else if(settings.blendMode == 0)
				settings.blendMode = Display::E_BlendMode_AlphaPreMultiply;

			pData->pMaterial = new DynamicEffect::Track::C_Material(g_pTrackRenderer, settings);
			pData->bInitialised = true;
		}
	}
}

void Shutdown()
{
	for(Materials::Iterator it = m_Materials.Begin(); it != m_Materials.End(); ++it)
	{
		Material* pData = it->GetValue();
		InvalidateMaterial(pData);
		pData->bInitialised = false;
	}

	g_pTrackRenderer = 0;
}

} // XtInterface
} // Track
} // DynamicEffect





