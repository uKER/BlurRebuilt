#ifndef __FX_SAMPLERSTATES_H__
#define __FX_SAMPLERSTATES_H__

// TODO: create target-specific versions of this file for Xenon, PS3
// TODO: at load-time, convert the sampler state form the platform-independant version to the platform specific version
#ifdef FX_INCLUDE_DEVICE_TOOLS_ENUMS
#include "Display/Device/DeviceToolsEnums.h"
#else
#include "Display/Device/DeviceTypesEnums.h"
#endif
#include <btl/Containers/vector.h>
#include <system/scheduler/jobqueue.h>

#include "Display/context/SamplerState.h"

#include "fx/RenderMutex.h"

namespace FX
{

/// keep a shared list of all the different combinations of sampler states that are used
class SamplerStateManager
{
public:
	void SetDefaultStates();

	/// return a handle to a target-specific sampler state object
	int CreateSamplerState(Display::E_TextureAddress u, Display::E_TextureAddress v, Display::E_TextureAddress w,
				Display::E_TextureFilter mip, Display::E_TextureFilter min, Display::E_TextureFilter mag,
				U32 MaxAnisotropy, bool SRGBTexture, S8 MipMapLodBias);

	/// return a pointer to the given sampler state object
	Display::C_SamplerState *GetSamplerState(int i) { CheckRenderMutex(); return &m_States[i]; }

	/// clear the list of cached states - dangerous
	void Clear();

	/// fetch all data required for rendering from the SPU
	static void SpuFetch(SamplerStateManager *&pThis)
	{
		Spu::FetchAlloc(pThis);
		Btl::SpuFetch(pThis->m_States);
	}

private:
	Btl::Vector<Display::C_SamplerState> m_States;
};

extern SamplerStateManager g_SamplerStateMgr;

// DebugSamplerStates

struct DebugSamplerStates
{
	struct DebugSamplerState
	{
		DebugSamplerState() : bEnable(false) {}

		bool bEnable;
		S8 m_MipLodBias;			
		Display::E_TextureFilter m_FilterMode;	
		S8 m_MaxAniso;
	};

	DebugSamplerStates() : bEnable(false), bOverrideAllWithAlbedo(false) {}

	bool bEnable;
	bool bOverrideAllWithAlbedo;
	DebugSamplerState m_Sampler[4];
};

extern DebugSamplerStates g_DebugSamplerStates;

} // namespace FX

#endif
