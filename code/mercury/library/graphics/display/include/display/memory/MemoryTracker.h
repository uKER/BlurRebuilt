#ifndef DISPLAY_MEMORY_MEMORY_TRACKER_H
#define DISPLAY_MEMORY_MEMORY_TRACKER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/threads/ThreadLocalStorage.h>
#include "Display/Device/DeviceDefines.h"

namespace Display
{

//! Memory type

enum E_MemoryType
{
	E_MemoryType_Total = 0,
	E_MemoryType_Surface,
	E_MemoryType_Texture2D,
	E_MemoryType_Texture3D,
	E_MemoryType_TextureCube,
	E_MemoryType_VertexBuffer,
	E_MemoryType_IndexBuffer,
	E_MemoryType_VertexShader,
	E_MemoryType_PixelShader,
	E_MemoryType_Count,
};

//! Memory counter

class C_MemoryCounter
{
public:

	C_MemoryCounter(const S32 id);
	~C_MemoryCounter();

public:

	S32					m_ID;
	C_MemoryCounter*	m_pParent;

	static MT::ThreadLocalStorage m_TLSpParent;
};

//! Memory system

class C_MemorySystem
{
public:

	C_MemorySystem();
	~C_MemorySystem();

public:

	S32					m_Size[E_MemoryType_Count];
	S32					m_MaxSize;
	S32					m_Count;
	Util::C_String		m_Name;
};

//! Memory tracker

class C_MemoryTracker
{
public:

	C_MemoryTracker();
	~C_MemoryTracker();

	const S32 MemoryGetSystemID() const;
	const S32 MemoryGetSystemID(const Util::C_String& name) const;
	const S32 MemoryGetSystemCount() const;
	C_MemorySystem* MemoryGetSystem(const S32 id);
	
	const S32 MemoryRegSystem(const Util::C_String& name, const S32 size = -1);	
	void MemoryIncSystem(const S32 id, const U32 size, const E_MemoryType type);
	void MemoryDecSystem(const S32 id, const U32 size, const E_MemoryType type);

	void MemoryPrintSystems();

private:

	Ut::Vector<C_MemorySystem> m_System;
};

}; // namespace Display

#include "Display/Memory/MemoryTracker.inl"

#ifdef USE_DISPLAY_MEMORY_TRACKER
	#define RESOURCE_MEMORY_ID(_name) \
		static S32 memorySystemID = Display::Device()->MemoryRegSystem(_name); \
		Display::C_MemoryCounter memoryCounter(memorySystemID)
#else
	#define RESOURCE_MEMORY_ID(_name)
#endif

#endif // DISPLAY_MEMORY_MEMORY_TRACKER_H
