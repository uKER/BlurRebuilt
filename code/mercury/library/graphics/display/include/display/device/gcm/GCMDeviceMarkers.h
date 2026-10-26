#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_MARKERS_H
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_MARKERS_H

#pragma once

#include "Display/Device/DeviceDefines.h"
#include "Display/Device/GCM/GCMDeviceLabels.h"
#include <Btl/Containers/Vector.h>

namespace Display
{

struct C_DeviceMarker
{
	C_DeviceMarker();

	U32								m_Id;
	Util::C_String					m_Name;
};

class C_DeviceMarkerManager
{
public:

	C_DeviceMarkerManager();
	~C_DeviceMarkerManager();

	void ClearMarkers();
	S32 GetMarkerCount() const;
	C_DeviceMarker* GetMarker(const S32 i);
	const C_DeviceMarker* GetMarker(const S32 i) const;
	S32 FindMarker(const Util::C_String& name) const;
	S32 AddMarker(const Util::C_String& name);
	void SetMarker(const S32 i);

	void Dump(bool walkCommandBuffer = false);

	static C_DeviceMarkerManager& GetInstance() { return s_Instance; }

private:

	U32								m_Id;
	Btl::Vector<C_DeviceMarker*>	m_List;
	static C_DeviceMarkerManager	s_Instance;
};

}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_MARKERS_H
