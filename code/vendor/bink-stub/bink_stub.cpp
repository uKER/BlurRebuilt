// bink_stub.cpp — see bink.h. No video is ever opened.
#include "bink.h"
#include <string.h>

extern "C" {

HBINK RADLINK BinkOpen(const char*, U32) { return NULL; }
void  RADLINK BinkClose(HBINK) {}
S32   RADLINK BinkDoFrame(HBINK) { return 0; }
void  RADLINK BinkNextFrame(HBINK) {}
S32   RADLINK BinkWait(HBINK) { return 0; }
S32   RADLINK BinkShouldSkip(HBINK) { return 0; }
S32   RADLINK BinkPause(HBINK, S32) { return 0; }
void  RADLINK BinkSetVolume(HBINK, U32, S32) {}
void  RADLINK BinkSetSoundTrack(U32, U32*) {}
S32   RADLINK BinkSoundUseWaveOut(void) { return 0; }
void  RADLINK BinkSetIOSize(U32) {}
void  RADLINK BinkGetSummary(HBINK, BINKSUMMARY* sum) { if (sum) memset(sum, 0, sizeof(*sum)); }
S32   RADLINK BinkGetRects(HBINK, U32) { return 0; }
void  RADLINK BinkGetFrameBuffersInfo(HBINK, BINKFRAMEBUFFERS* fb) { if (fb) memset(fb, 0, sizeof(*fb)); }
void  RADLINK BinkRegisterFrameBuffers(HBINK, BINKFRAMEBUFFERS*) {}
S32   RADLINK BinkSetMixBinVolumes(HBINK, U32, U32*, S32*, U32) { return 0; }

}
