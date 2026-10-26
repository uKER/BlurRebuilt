#include "stdafx.h"
#include "Display/Device/GCM/GCMDeviceMarkers.h"
#include <Display2/Debug/Debug.h>

namespace Display
{

C_DeviceMarker::C_DeviceMarker()
{
	m_Id = UINT_MAX;
	m_Name = "";
}

C_DeviceMarkerManager C_DeviceMarkerManager::s_Instance;

C_DeviceMarkerManager::C_DeviceMarkerManager()
{
	m_Id = 0;
}

C_DeviceMarkerManager::~C_DeviceMarkerManager()
{
	ClearMarkers();
}

void C_DeviceMarkerManager::ClearMarkers()
{
	for(S32 i=0; i<m_List.Size(); i++)
		SAFE_DELETE_PTR(m_List[i]);
	m_List.Clear();
}

S32 C_DeviceMarkerManager::GetMarkerCount() const
{
	return m_List.Size();
}

C_DeviceMarker* C_DeviceMarkerManager::GetMarker(const S32 i)
{
	return i >= 0 && i < m_List.Size() ? m_List[i] : NULL;
}

const C_DeviceMarker* C_DeviceMarkerManager::GetMarker(const S32 i) const
{
	return i >= 0 && i < m_List.Size() ? m_List[i] : NULL;
}

S32 C_DeviceMarkerManager::FindMarker(const Util::C_String& name) const
{
	for(S32 i=0; i<m_List.Size(); i++)
		if(name == m_List[i]->m_Name)
			return i;
	return -1;
}

S32 C_DeviceMarkerManager::AddMarker(const Util::C_String& name)
{
	S32 i = FindMarker(name);
	if(i == -1)
	{
		i = m_List.Size();
		C_DeviceMarker* pMarker = new C_DeviceMarker();
		pMarker->m_Id = m_Id++;
		pMarker->m_Name = name;
		m_List.PushBack(pMarker);
	}
	return i;
}

void C_DeviceMarkerManager::SetMarker(const S32 i)
{
	GCM_NAMESPACE::cellGcmSetWriteCommandLabel(gCellGcmCurrentContext, g_LastMarkerEaLabel, (U32)gCellGcmCurrentContext->current);

	const C_DeviceMarker* pMarker = GetMarker(i);

	if(pMarker)
	{
		GCM_NAMESPACE::cellGcmSetWriteCommandLabel(gCellGcmCurrentContext, g_LastMarkerIdLabel, pMarker->m_Id);
	}
	else
	{
		LOGERROR(g_Log, "Failed to find marker %i\n", i);
	}

	GCM_NAMESPACE::cellGcmSetBackStencilFunc(gCellGcmCurrentContext, CELL_GCM_ALWAYS, pMarker->m_Id, 0);
}

void C_DeviceMarkerManager::Dump(bool walkCommandBuffer)
{
#if CFG_GOLD
	//char str[1024];
#endif
	for(S32 i=0; i<m_List.Size(); i++)
	{
		const C_DeviceMarker* pMarker = m_List[i];

#if CFG_GOLD
		//sprintf_s(str, sizeof(str), "Marker : %2i : %s\n", i, pMarker->m_Name.Chars());
		//printf(str);
#else
		LOGINFO(g_Log, "Marker : %2i : %s\n", i, pMarker->m_Name.Chars());
#endif
	}

	U32 id = *cellGcmGetLabelAddress(g_LastMarkerIdLabel);
		
	U32 ea = *cellGcmGetLabelAddress(g_LastMarkerEaLabel);

	const C_DeviceMarker* pMarker = GetMarker(id);

	if(pMarker)
	{
#if CFG_GOLD
		//sprintf_s(str, sizeof(str), "Last executed marker %s 0x%.8x\n", pMarker->m_Name.Chars(), ea);
		//printf(str);
#else
		LOGINFO(g_Log, "Last executed marker %s 0x%.8x\n", pMarker->m_Name.Chars(), ea);
#endif
	}
	else
	{
#if CFG_GOLD
		//sprintf_s(str, sizeof(str), "Failed to find marker %i\n", id);
		//printf(str);
#else
		LOGERROR(g_Log, "Failed to find marker %i\n", id);
#endif
	}

#if !CFG_GOLD
	if(walkCommandBuffer)
	{
		CellGcmControl* pControl = cellGcmGetControlRegister();

		U32 headOffset = 0;
		S32 result = cellGcmAddressToOffset((const void*)ea, &headOffset);
		BIZ_ASSERT(result == CELL_OK);
		U32 tailOffset = pControl->get;

		Display2::WalkCommandBufferOffsets(headOffset, tailOffset, true);
	}
#endif
}

}; // namespace Display
