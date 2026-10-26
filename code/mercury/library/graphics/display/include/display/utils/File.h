#ifndef DISPLAY_UTILS_FILE_H
#define DISPLAY_UTILS_FILE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

U64 GetFileTime(const Util::C_String& fileName);
bool GetFileData(const Util::C_String& fileName, U32& fileSize, U8** ppFileData);

}; // namespace Display

#endif // DISPLAY_UTILS_FILE_H
