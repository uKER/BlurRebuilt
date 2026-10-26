#ifndef __FX_RENDERSTATES_H__
#define __FX_RENDERSTATES_H__

#include "Display/Device/DeviceTypesEnums.h"
#include <btl/containers/vector.h>
#include <system/scheduler/jobqueue.h>

namespace FX
{

struct BlendState
{
	Display::E_Blend srcBlend;
	Display::E_Blend destBlend;
	Display::E_BlendOp blendOp;
	Display::E_Blend srcBlendAlpha;
	Display::E_Blend destBlendAlpha;
	Display::E_BlendOp blendOpAlpha;
	Display::E_Bool alphaToMaskEnable;
	Display::E_Bool alphaTestEnable;
	Display::E_CullMode cullMode;
	U32 alphaRef;
};

class BlendStateManager
{
public:
	static BlendStateManager& Instance();

	S32 InsertRenderState(const BlendState& blendState);
	BlendState *GetRenderState(S32 i) { return &m_blendStates[i]; }

	BlendStateManager() {}

	/// fetch all data required for rendering from the SPU
	static void SpuFetch(BlendStateManager *&pThis)
	{
		Spu::FetchAlloc(pThis);
		Btl::SpuFetch(pThis->m_blendStates);
	}

private:
	Btl::Vector<BlendState> m_blendStates;
};

extern BlendStateManager g_RenderStateMgr;

} // namespace FX

#endif
