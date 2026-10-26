// bink.h — stub replacing RAD Game Tools' Bink 1.9r SDK header (decision: no
// video playback in this port). Declares exactly the types, flags and
// functions that mercury/library/graphics/movie uses, so the original
// W32Movie backend and RAD's binktextures sample compile unchanged. The
// functions live in bink_stub.cpp: BinkOpen() always returns NULL, so
// C_W32Movie reports "reached end" immediately and every screen that plays a
// movie falls through.
#ifndef BLUR2PC_BINK_STUB_H
#define BLUR2PC_BINK_STUB_H

#include <core/Core.h>	// U32 / S32 / F32

#define __RADNT__ 1
#define RADDEFFUNC
#define RADLINK __stdcall
#define PTR4
typedef unsigned int UINTa;
typedef int SINTa;

#define BINKMAXDIRTYRECTS 8
#define BINKMAXFRAMEBUFFERS 2

// BinkOpen flags
#define BINKALPHA          0x00100000
#define BINKSNDTRACK       0x00004000
#define BINKNOFRAMEBUFFERS 0x00000400
#define BINKPRELOADALL     0x00002000
#define BINKFROMMEMORY     0x04000000
#define BINKIOSIZE         0x01000000

// BinkGetRects / BinkCopyToBuffer surface types
#define BINKSURFACEFAST    0x00000000
#define BINKSURFACE32      3

typedef struct BINKRECT
{
	S32 Left, Top, Width, Height;
} BINKRECT;

typedef struct BINK
{
	U32 Width;
	U32 Height;
	U32 Frames;
	U32 FrameNum;
	U32 LastFrameNum;
	U32 FrameRate;
	U32 FrameRateDiv;
	U32 ReadError;
	U32 OpenFlags;
	U32 BinkType;
	S32 NumRects;
	BINKRECT FrameRects[BINKMAXDIRTYRECTS];
} BINK, *HBINK;

typedef struct BINKFRAMEPLANE
{
	void* Buffer;
	U32 Allocate;
	U32 BufferPitch;
} BINKFRAMEPLANE;

typedef struct BINKFRAMEPLANESET
{
	BINKFRAMEPLANE YPlane;
	BINKFRAMEPLANE cRPlane;
	BINKFRAMEPLANE cBPlane;
	BINKFRAMEPLANE APlane;
} BINKFRAMEPLANESET;

typedef struct BINKFRAMEBUFFERS
{
	S32 TotalFrames;
	U32 YABufferWidth;
	U32 YABufferHeight;
	U32 cRcBBufferWidth;
	U32 cRcBBufferHeight;
	S32 FrameNum;
	BINKFRAMEPLANESET Frames[BINKMAXFRAMEBUFFERS];
} BINKFRAMEBUFFERS;

typedef struct BINKSUMMARY
{
	U32 Width, Height, TotalTime, FileFrameRate, FileFrameRateDiv, FrameRate, FrameRateDiv;
	U32 TotalOpenTime, TotalFrames, TotalPlayedFrames, SkippedFrames, SkippedBlits, SoundSkips;
	U32 TotalBlitTime, TotalReadTime, TotalVideoDecompTime, TotalAudioDecompTime, TotalIdleReadTime, TotalBackReadTime;
	U32 TotalReadSpeed, SlowestFrameTime, Slowest2FrameTime, SlowestFrameNum, Slowest2FrameNum;
	U32 AverageDataRate, AverageFrameSize, HighestMemAmount, TotalIOMemory, HighestIOUsed, Highest1SecRate, Highest1SecFrame;
} BINKSUMMARY;

#ifdef __cplusplus
extern "C" {
#endif

HBINK RADLINK BinkOpen(const char* name, U32 flags);
void  RADLINK BinkClose(HBINK bink);
S32   RADLINK BinkDoFrame(HBINK bink);
void  RADLINK BinkNextFrame(HBINK bink);
S32   RADLINK BinkWait(HBINK bink);
S32   RADLINK BinkShouldSkip(HBINK bink);
S32   RADLINK BinkPause(HBINK bink, S32 pause);
void  RADLINK BinkSetVolume(HBINK bink, U32 trackid, S32 volume);
void  RADLINK BinkSetSoundTrack(U32 total_tracks, U32* tracks);
S32   RADLINK BinkSoundUseWaveOut(void);
void  RADLINK BinkSetIOSize(U32 iosize);
void  RADLINK BinkGetSummary(HBINK bink, BINKSUMMARY* sum);
S32   RADLINK BinkGetRects(HBINK bink, U32 flags);
void  RADLINK BinkGetFrameBuffersInfo(HBINK bink, BINKFRAMEBUFFERS* fbset);
void  RADLINK BinkRegisterFrameBuffers(HBINK bink, BINKFRAMEBUFFERS* fbset);
S32   RADLINK BinkSetMixBinVolumes(HBINK bink, U32 trackid, U32* vol_mix_bins, S32* volumes, U32 total);

#ifdef __cplusplus
}
#endif

#endif
