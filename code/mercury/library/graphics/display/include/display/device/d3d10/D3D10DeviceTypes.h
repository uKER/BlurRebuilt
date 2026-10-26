#ifndef DISPLAY_D3D10DEVICETYPES_H
#define DISPLAY_D3D10DEVICETYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"

namespace NewDisplay
{
namespace D3D10
{

INLINE BOOL D3D10Bool(const E_Bool boolean)
{
	return boolean == E_Bool_True ? TRUE : FALSE;
}

INLINE E_Bool D3D10Bool(const BOOL boolean)
{
	return boolean == TRUE ? E_Bool_True : E_Bool_False;
}

}; // namespace D3D10
}; // namespace NewDisplay

#endif // DISPLAY_D3D10DEVICETYPES_H
