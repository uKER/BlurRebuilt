#include "stdafx.h"

#include "GameWorld.h"

#include <fx/fx.h>

#define FX_ALLOW_INCLUDE
#include "fx\effect.h"

namespace Game
{

#ifndef CFG_GOLD

Debug::Bool g_OverrideCubeMap("Game.OverrideCubemap", false);

void C_GameWorld::UpdateCubemapOverride()
{
	bool shouldOverrideCubemap = false;

	if(g_OverrideCubeMap)
		shouldOverrideCubemap = true;

	if(shouldOverrideCubemap)
	{
		if(!m_bCubeMapOverridden)
		{
			FX::ParameterHandle p = FX::GetSharedParameter("g_tEnvironment");
			FX::Parameter* pParam = FX::GetParameterPtr(p);
			if(pParam)
			{
				pParam->bOverride = true;		
				m_bCubeMapOverridden = true;
			}
		}
	}
	else
	{
		if(m_bCubeMapOverridden)
		{
			FX::ParameterHandle p = FX::GetSharedParameter("g_tEnvironment");
			FX::Parameter* pParam = FX::GetParameterPtr(p);
			if(pParam)
			{
				pParam->bOverride = false;		
				m_bCubeMapOverridden = false;
			}
		}
	}
}

#endif

} // namespace Game
