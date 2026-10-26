#include "stdafx.h"

#include <Display.h>
#include <Display/DisplayToDisplay2.h>

#include "fx/SamplerStates.h"
#include "fx/RenderMutex.h"

namespace FX
{

SamplerStateManager ALIGN16 g_SamplerStateMgr;

/// return a pointer to a target-specific sampler state object
int SamplerStateManager::CreateSamplerState(Display::E_TextureAddress u, Display::E_TextureAddress v, Display::E_TextureAddress w,
			Display::E_TextureFilter mip, Display::E_TextureFilter min, Display::E_TextureFilter mag,
			U32 Aniso, bool SRGB, S8 MipBias)
{
	CheckSerialiseMutex();

	Display::C_SamplerState samplerState;
	samplerState.SetDefaultStates();
	samplerState.SetTextureAddress(ToTextureAddress2(u), ToTextureAddress2(v), ToTextureAddress2(w));
	samplerState.SetTextureFilter(ToTextureFilter2(mag), ToTextureFilter2(min), ToTextureFilter2(mip));
#if PLATFORM_PS3
	samplerState.SetTextureGamma(SRGB ? Display2::E_TextureGamma_R|Display2::E_TextureGamma_G|Display2::E_TextureGamma_B : 0);
#elif PLATFORM_PC
	samplerState.SetTextureGamma(SRGB ? Display2::E_Bool_True : Display2::E_Bool_False);
#endif
	samplerState.SetTextureLodBias(MipBias);
	samplerState.SetMaxAnisotropy(Aniso);

	int i;
	for (i=0; i<m_States.Size(); i++)
	{
		if (m_States[i] == samplerState)
			return i;
	}
	m_States.PushBack(samplerState);
	return i;
}

/// clear the list of cached states - dangerous
void SamplerStateManager::Clear()
{
	CheckSerialiseMutex();
	m_States.Clear();
}

}
