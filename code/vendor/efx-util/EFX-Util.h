// EFX-Util.h — shim replacing Creative's EFX-Util library from the OpenAL 1.1 SDK.
// The engine's audio code (mercury/library/audio) stores reverb presets in the
// legacy EAX format and converts them with ConvertReverbParameters(). Only the
// types and functions the engine uses are provided.
#ifndef BLUR2PC_EFX_UTIL_H
#define BLUR2PC_EFX_UTIL_H

#include "efx-presets.h"   // OpenAL Soft: EFXEAXREVERBPROPERTIES

typedef struct _EAXVECTOR
{
	float x, y, z;
} EAXVECTOR;

typedef struct _EAXREVERBPROPERTIES
{
	unsigned long ulEnvironment;
	float flEnvironmentSize;
	float flEnvironmentDiffusion;
	long lRoom;
	long lRoomHF;
	long lRoomLF;
	float flDecayTime;
	float flDecayHFRatio;
	float flDecayLFRatio;
	long lReflections;
	float flReflectionsDelay;
	EAXVECTOR vReflectionsPan;
	long lReverb;
	float flReverbDelay;
	EAXVECTOR vReverbPan;
	float flEchoTime;
	float flEchoDepth;
	float flModulationTime;
	float flModulationDepth;
	float flAirAbsorptionHF;
	float flHFReference;
	float flLFReference;
	float flRoomRolloffFactor;
	unsigned long ulFlags;
} EAXREVERBPROPERTIES, *LPEAXREVERBPROPERTIES;

#define EAXREVERBFLAGS_DECAYTIMESCALE        0x00000001
#define EAXREVERBFLAGS_REFLECTIONSSCALE      0x00000002
#define EAXREVERBFLAGS_REFLECTIONSDELAYSCALE 0x00000004
#define EAXREVERBFLAGS_REVERBSCALE           0x00000008
#define EAXREVERBFLAGS_REVERBDELAYSCALE      0x00000010
#define EAXREVERBFLAGS_DECAYHFLIMIT          0x00000020
#define EAXREVERBFLAGS_ECHOTIMESCALE         0x00000040
#define EAXREVERBFLAGS_MODULATIONTIMESCALE   0x00000080

// EAX 4.0 "Generic" preset (values from the EAX SDK preset table)
#define REVERB_PRESET_GENERIC \
	{0, 7.5f, 1.000f, -1000, -100, 0, 1.49f, 0.83f, 1.00f, -2602, 0.007f, {0.0f, 0.0f, 0.0f}, 200, 0.011f, {0.0f, 0.0f, 0.0f}, 0.250f, 0.000f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.00f, 0x3f}

#ifdef __cplusplus
extern "C" {
#endif

void ConvertReverbParameters(EAXREVERBPROPERTIES* pEAXProp, EFXEAXREVERBPROPERTIES* pEFXEAXReverb);
void AdjustEnvironmentSize(EAXREVERBPROPERTIES* pEAXProp, float flEnvironmentSize);

#ifdef __cplusplus
}
#endif

#endif
