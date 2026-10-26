#ifndef DISPLAY_DEVICE_GCM_GCM_FENCE_H
#define DISPLAY_DEVICE_GCM_GCM_FENCE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace GCM
{

enum E_Fence
{
	E_Fence_Command = 0,
	E_Fence_Texture,
	E_Fence_Raster,
	E_Fence_Count
};

class C_FenceInfo
{
public:
	U32				m_LabelId;
	volatile U32*	m_LabelAddress;
	U32				m_Value;
	U32				m_Padding; // bring to 16 bytes for fast access
};

INLINE void GCMFenceInitialise(C_FenceInfo& fenceInfo, const U32 labelId)
{
	fenceInfo.m_LabelId			= labelId;
	fenceInfo.m_LabelAddress	= (volatile U32*) cellGcmGetLabelAddress(labelId);
	fenceInfo.m_Value			= 0;
	fenceInfo.m_Padding			= 0;
	*fenceInfo.m_LabelAddress	= 0;
}

INLINE U32 GCMFenceNextValue(C_FenceInfo& fenceInfo)
{
	return ++fenceInfo.m_Value;
}

INLINE U32 GCMFenceLabel(C_FenceInfo& fenceInfo)
{
	return fenceInfo.m_LabelId;
}

INLINE volatile U32* GCMFenceAddress(C_FenceInfo& fenceInfo)
{
	return fenceInfo.m_LabelAddress;
}

INLINE U32 GCMFenceValue(C_FenceInfo& fenceInfo)
{
	return *fenceInfo.m_LabelAddress;
}

INLINE bool GCMFenceIsPassed(C_FenceInfo& fenceInfo, const U32 value)
{
	const U32 fenceValue = GCMFenceValue(fenceInfo);
	return 1 - ((fenceValue - value) >> 31); // equivalent to: (int)(fenceValue - value) > 0
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_FENCE_H
