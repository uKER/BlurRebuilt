#include "stdafx.h"
#include "Display/Memory/MemoryTracker.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Display
{

MT::ThreadLocalStorage C_MemoryCounter::m_TLSpParent;

// Memory system.

C_MemorySystem::C_MemorySystem() :
	m_MaxSize(-1),
	m_Count(0),
	m_Name("")
{
	memset(m_Size, 0, sizeof(m_Size));
}

C_MemorySystem::~C_MemorySystem()
{
}

// Memory tracker.

C_MemoryTracker::C_MemoryTracker()
{
	m_System.Resize(2);
	m_System[0].m_Name = "Total";
	m_System[1].m_Name = "Unknown";
}

C_MemoryTracker::~C_MemoryTracker()
{
	m_System.Clear();
}

const S32 C_MemoryTracker::MemoryGetSystemID(const Util::C_String& name) const
{
	for(S32 id = 0; id < m_System.Size(); id++)
		if(m_System[id].m_Name == name)
			return id;
	return -1;
}

const S32 C_MemoryTracker::MemoryRegSystem(const Util::C_String& name, const S32 size)
{
	S32 id = MemoryGetSystemID(name);
	if(id == -1)
	{
		id = m_System.Size();
		C_MemorySystem system;
		system.m_Name = name;
		system.m_MaxSize = size;
		m_System.PushBack(system);
	}
	else if(size != -1)
	{
		m_System[id].m_MaxSize = size;		
	}

	return id;
}

void C_MemoryTracker::MemoryIncSystem(const S32 id, const U32 size, const E_MemoryType type)
{
	if(id == -1)
		return;
	
	assert(type < E_MemoryType_Count);
	assert(id >= 0 && id < m_System.Size());
	m_System[id].m_Size[E_MemoryType_Total] += size;
	m_System[id].m_Size[type] += size;
	m_System[id].m_Count++;

	assert(m_System.Size() > 0);
	m_System[0].m_Size[E_MemoryType_Total] += size;
	m_System[0].m_Size[type] += size;
	m_System[0].m_Count++;
}

void C_MemoryTracker::MemoryDecSystem(const S32 id, const U32 size, const E_MemoryType type)
{
	if(id == -1)
		return;

	assert(type < E_MemoryType_Count);
	assert(id >= 0 && id < m_System.Size());
	m_System[id].m_Size[E_MemoryType_Total] -= size;
	m_System[id].m_Size[type] -= size;
	m_System[id].m_Count--;

	assert(m_System.Size() > 0);
	m_System[0].m_Size[E_MemoryType_Total] -= size;
	m_System[0].m_Size[type] -= size;
	m_System[0].m_Count--;
}

void C_MemoryTracker::MemoryPrintSystems()
{
	char temp[1024];

	LOGWARN(g_Log, "\n:: Graphics Memory Systems ::\n\n");
	
	for(S32 id = 0; id < m_System.Size(); id++)
	{
		const C_MemorySystem& system = m_System[id];

		const F32 size = (F32)system.m_Size[E_MemoryType_Total] / (1024.f * 1024.f);

		if(system.m_MaxSize != -1)
		{
			const F32 maxSize = (F32)system.m_MaxSize / (1024.f * 1024.f);
			sprintf_s(temp, sizeof(temp), "Size '%.5fmb / %.5fmb'", size, maxSize);
		}
		else
		{
			sprintf_s(temp, sizeof(temp), "Size '%.5fmb'", size);
		}

		sprintf_s(temp, sizeof(temp), "%s Count '%i' Name '%s'", temp, system.m_Count, system.m_Name.Chars());

		for(U32 type = 1; type < E_MemoryType_Count; type++)
		{
			if((F32)system.m_Size[type] >= (0.1f * 1024.f * 1024.f))
			{
				switch (type)
				{
				case E_MemoryType_Surface:		sprintf_s(temp, sizeof(temp), "%s Surface '%.1fmb'",		temp, (F32)system.m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_Texture2D:	sprintf_s(temp, sizeof(temp), "%s Texture2D '%.1fmb'",		temp, (F32)system.m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_Texture3D:	sprintf_s(temp, sizeof(temp), "%s Texture3D '%.1fmb'",		temp, (F32)system.m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_TextureCube:	sprintf_s(temp, sizeof(temp), "%s TextureCube '%.1fmb'",	temp, (F32)system.m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_VertexBuffer: sprintf_s(temp, sizeof(temp), "%s VertexBuffer '%.1fmb'",	temp, (F32)system.m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_IndexBuffer:	sprintf_s(temp, sizeof(temp), "%s IndexBuffer '%.1fmb'",	temp, (F32)system.m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_VertexShader:	sprintf_s(temp, sizeof(temp), "%s VertexShader '%.1fmb'",	temp, (F32)system.m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_PixelShader:	sprintf_s(temp, sizeof(temp), "%s PixelShader '%.1fmb'",	temp, (F32)system.m_Size[type] / (1024.f * 1024.f)); break;
				}
			}
		}

		LOGWARN(g_Log, temp);
	}

	LOGWARN(g_Log, "\n");
}

}; // namespace NewcDisplay
