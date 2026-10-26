#include "stdafx.h"
#include "Display/Device/GCM/GCMDeviceStates.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

#ifdef DONT_INLINE_GCM_DEVICE_STATES_INL
#undef INLINE
#define INLINE
#include "Display/Device/GCM/GCMDeviceStates.inl"
#undef INLINE
#define INLINE inline
#endif

namespace Display
{
namespace GCM
{

C_TextureSampler::C_TextureSampler() :
	m_BorderColour(0)
{
}

}; // namespace GCM
}; // namespace Display
