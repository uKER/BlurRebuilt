#pragma once

Util::C_String GetRawFilename(const char* pchPathAndFname);
bool S16ToF32(S16* s16Src, S32 sSrcLength, F32** ppf32Dest, S32* psDestLength);
bool F32ToS16(F32* pf32Src, S32 sSrcLength, S16** pps16Dst, S32* psDstLength);
Util::C_String GetWindowsErrorMessage(int errorCode);
Util::C_String FormatTime(F32 fTimeInSecs);


