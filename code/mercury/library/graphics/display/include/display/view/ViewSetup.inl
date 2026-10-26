#ifndef DISPLAY_VIEW_VIEW_SETUP_INL
#define DISPLAY_VIEW_VIEW_SETUP_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE const Maths::C_Rect& C_ViewSetup::GetDeviceCoords()
{
	return m_DeviceCoords;
}

}; // namespace Display

#endif // DISPLAY_VIEW_VIEW_SETUP_INL
