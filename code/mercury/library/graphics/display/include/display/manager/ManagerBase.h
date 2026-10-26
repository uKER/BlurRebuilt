#ifndef DISPLAY_MANAGER_MANAGER_BASE_H
#define DISPLAY_MANAGER_MANAGER_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Manager/ManagerInterface.h"
#include <Core/Containers/Genesis_Containers/Array.h> // [Blur1PCDX11] Direct header dependency.

namespace Display
{

//! E_Error

enum E_Error
{
	E_Error_NotSupported_D3D9 = 0,
	E_Error_NotSupported_HAL,
	E_Error_NotSupported_PS20,
	E_Error_NotSupported_VS20,
	E_Error_NotSupported_CAPS,
	E_Error_Count,
};

//! C_Error

struct C_Error
{
	Util::C_String		m_Text;
};

//! C_ManagerBase

class C_ManagerBase : public I_Manager
{
public:

	C_ManagerBase();
	~C_ManagerBase();

	static void SetError(const E_Error error, const Util::C_String& text);
	static void PrintError(const E_Error error);
	static void CreateErrors();
	static void DestroyErrors();
	
protected:

	static Util::C_Array<C_Error, E_Error_Count>	m_ErrorList;
};

}; // namespace Display

#include "Display/Manager/ManagerBase.inl"

#endif // DISPLAY_MANAGER_MANAGER_BASE_H
