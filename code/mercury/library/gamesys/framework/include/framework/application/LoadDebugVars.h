#ifndef LOADDEBUGVAR_H
#define LOADDEBUGVAR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Framework
{
Util::C_String FieldValue(const Util::C_String& line, Util::C_String key);

void SaveDebugVars();
void LoadDebugVars();
}

#endif // LOADDEBUGVAR_H