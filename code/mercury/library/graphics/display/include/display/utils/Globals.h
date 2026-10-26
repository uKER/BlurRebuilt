#ifndef DISPLAY_UTILS_GLOBALS_H
#define DISPLAY_UTILS_GLOBALS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "core/containers/string.h"

namespace Sys
{
	class I_FileSystem;
}

namespace Display
{

Util::C_String FormatString(const Util::C_String& input);

extern Sys::I_FileSystem* gFileSystem;

}; // namespace Display

#endif // DISPLAY_UTILS_GLOBALS_H
