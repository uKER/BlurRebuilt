#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_ENUMS_H
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_ENUMS_H

namespace Display
{
namespace GCM
{

#ifdef BUILD_CONFIG_DEBUG
#define GCM_DEBUG
#endif

//! E_AllocHeap

enum E_AllocHeap
{
	E_AllocHeap_Video,
	E_AllocHeap_System,
	E_AllocHeap_Main,
};

//! E_AllocType

enum E_AllocType
{
	E_AllocType_Default,
	E_AllocType_Start,
	E_AllocType_End,
};

//! E_Downsample

enum E_Downsample
{
	E_Downsample_Default = 0,
	E_Downsample_Quincunx,
	E_Downsample_QuincunxAlt,
	E_Downsample_QuincunxShifted,
	E_Downsample_Count,
};

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_H
