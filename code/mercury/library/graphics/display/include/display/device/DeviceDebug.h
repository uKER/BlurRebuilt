#ifndef DISPLAY_DEVICE_DEVICE_DEBUG_H
#define DISPLAY_DEVICE_DEVICE_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"

namespace Display
{

//! C_Resolution

struct C_Resolution
{
	S32							m_Value;
	U32							m_Width;
	U32							m_Height;
	#ifdef PLATFORM_PC
	U32							m_RefreshRate;		// [Blur2PC] retail PC
	#endif
	Display::E_Multisample		m_Multisample;
	Util::C_String 				m_Name;
};

#ifdef PLATFORM_PC
//! C_MultisampleOption. [Blur2PC] retail PC: one entry per supported multisample, named "<samples>x".

struct C_MultisampleOption
{
	Display::E_Multisample		m_Multisample;
	Util::C_String 				m_Name;
};
#endif

//! C_ResolutionManager

class C_ResolutionManager
{
public:

	C_ResolutionManager();
	~C_ResolutionManager();

	void ClearResolutions();
	void ResetResolutions();
	void AddResolution(const U32 width, const U32 height, const E_Format format, const E_Multisample multisample, const U32 flags);
	void SetResolutions(Debug::EnumBase* pEnum);
	void SetMaxResolution(const U32 width, const U32 height, const E_Format format, const E_Multisample multisample, const U32 flags);
	C_Resolution* GetResolution(const S32 resolutionIndex);

	#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC's resolution manager (the global at 0x011437c4); see docs/analysis.md "Graphics options (PC)"
	void AddResolution(const U32 width, const U32 height, const E_Format format, const E_Multisample multisample, const U32 flags, const U32 refreshRate);
	void AddMultisample(const E_Multisample multisample);
	void SetResolutions(Debug::EnumBase* pResolutionEnum, Debug::EnumBase* pMultisampleEnum);
	const S32 GetResolutionCount() const		{ return m_Resolutions.Size(); }
	const S32 GetMultisampleCount() const		{ return m_Multisamples.Size(); }
	const S32 FindResolution(const U32 width, const U32 height) const;
	const S32 FindClosestResolution(const U32 width, const U32 height) const;
	const C_MultisampleOption* GetMultisample(const S32 multisampleIndex) const;
	#endif

private:

	U32								m_MaxSize;
	Ut::Vector<C_Resolution>		m_Resolutions;
	Ut::Vector<Debug::Option>		m_Options;
	#ifdef PLATFORM_PC
	Ut::Vector<C_MultisampleOption>	m_Multisamples;			// [Blur2PC]
	Ut::Vector<Debug::Option>		m_MultisampleOptions;	// [Blur2PC]
	#endif
};

extern C_ResolutionManager g_Resolutions;

}; // namespace Display

#endif // DISPLAY_DEVICE_DEVICE_DEBUG_H
