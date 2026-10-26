#ifndef SYSTEM_DEBUG_VARIABLES_NET_H
#define SYSTEM_DEBUG_VARIABLES_NET_H

#ifdef _MSC_VER
#pragma once
#endif

#define ARTERY_FOURCC(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

extern const char* kDebugVarNetId;

const ArteryU32 kDvmList				= ARTERY_FOURCC('L', 'I', 'S', 'T');
const ArteryU32 kDvmVarList				= ARTERY_FOURCC('V', 'A', 'R', 'L');
const ArteryU32 kDvmSetVar				= ARTERY_FOURCC('S', 'E', 'T', 'V');
const ArteryU32 kDvmUpdatedVars			= ARTERY_FOURCC('U', 'P', 'D', 'V');
const ArteryU32 kDvmDebugLog			= ARTERY_FOURCC('D', 'L', 'O', 'G');
const ArteryU32 kDvmDebugLogRegister	= ARTERY_FOURCC('D', 'L', 'R', 'E');
const ArteryU32 kDvmSetChannelSeverity	= ARTERY_FOURCC('C', 'H', 'S', 'E');
const ArteryU32 kDvmIdle				= ARTERY_FOURCC('I', 'D', 'L', 'E');
const ArteryU32 kDvmConsoleCommand		= ARTERY_FOURCC('C', 'C', 'M', 'D');
const ArteryU32 kDvmConsoleOutput		= ARTERY_FOURCC('C', 'O', 'U', 'T');
const ArteryU32 kDvmConsolePrompt		= ARTERY_FOURCC('C', 'P', 'R', 'T');

const ArteryU32 kDvtBool		= ARTERY_FOURCC('B', 'O', 'O', 'L');
const ArteryU32 kDvtInt			= ARTERY_FOURCC('I', 'N', '3', '2');
const ArteryU32 kDvtFloat		= ARTERY_FOURCC('F', 'L', '3', '2');
const ArteryU32 kDvtEnum		= ARTERY_FOURCC('E', 'N', 'U', 'M');
const ArteryU32 kDvtString		= ARTERY_FOURCC('S', 'T', 'R', '8');
const ArteryU32 kDvtFunction	= ARTERY_FOURCC('F', 'U', 'N', 'C');
const ArteryU32 kDvtVector		= ARTERY_FOURCC('V', 'E', 'C', 'T'); // Backwards compat.
const ArteryU32 kDvtVector2		= ARTERY_FOURCC('V', 'E', 'C', '2');
const ArteryU32 kDvtVector3		= ARTERY_FOURCC('V', 'E', 'C', '3');
const ArteryU32 kDvtVector4		= ARTERY_FOURCC('V', 'E', 'C', '4');
const ArteryU32 kDvtColour		= ARTERY_FOURCC('C', 'O', 'L', 'R');
const ArteryU32 kDvtEnd			= ARTERY_FOURCC('E', 'N', 'D', '_');

const ArteryU32 kDfRange		= 0x1;

#endif // SYSTEM_DEBUG_VARIABLES_NET_H
