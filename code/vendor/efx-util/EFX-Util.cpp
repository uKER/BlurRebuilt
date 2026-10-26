// EFX-Util.cpp — see EFX-Util.h. Conversion follows the EFX Programmer's
// Guide ("Converting EAX Reverb parameters to EFX"): millibel values become
// linear gains (10^(mB/2000)), everything else maps 1:1.
#include "EFX-Util.h"
#include <math.h>

static float mB_to_gain(float mB)
{
	return (float)pow(10.0, mB / 2000.0);
}

static float clampf(float v, float lo, float hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

extern "C" void ConvertReverbParameters(EAXREVERBPROPERTIES* p, EFXEAXREVERBPROPERTIES* e)
{
	if (!p || !e)
		return;
	e->flDensity = clampf((float)(pow(p->flEnvironmentSize, 3.0) / 16.0), 0.0f, 1.0f);
	e->flDiffusion = clampf(p->flEnvironmentDiffusion, 0.0f, 1.0f);
	e->flGain = clampf(mB_to_gain((float)p->lRoom), 0.0f, 1.0f);
	e->flGainHF = clampf(mB_to_gain((float)p->lRoomHF), 0.0f, 1.0f);
	e->flGainLF = clampf(mB_to_gain((float)p->lRoomLF), 0.0f, 1.0f);
	e->flDecayTime = clampf(p->flDecayTime, 0.1f, 20.0f);
	e->flDecayHFRatio = clampf(p->flDecayHFRatio, 0.1f, 2.0f);
	e->flDecayLFRatio = clampf(p->flDecayLFRatio, 0.1f, 2.0f);
	e->flReflectionsGain = clampf(mB_to_gain((float)p->lReflections), 0.0f, 3.16f);
	e->flReflectionsDelay = clampf(p->flReflectionsDelay, 0.0f, 0.3f);
	e->flReflectionsPan[0] = p->vReflectionsPan.x;
	e->flReflectionsPan[1] = p->vReflectionsPan.y;
	e->flReflectionsPan[2] = p->vReflectionsPan.z;
	e->flLateReverbGain = clampf(mB_to_gain((float)p->lReverb), 0.0f, 10.0f);
	e->flLateReverbDelay = clampf(p->flReverbDelay, 0.0f, 0.1f);
	e->flLateReverbPan[0] = p->vReverbPan.x;
	e->flLateReverbPan[1] = p->vReverbPan.y;
	e->flLateReverbPan[2] = p->vReverbPan.z;
	e->flEchoTime = clampf(p->flEchoTime, 0.075f, 0.25f);
	e->flEchoDepth = clampf(p->flEchoDepth, 0.0f, 1.0f);
	e->flModulationTime = clampf(p->flModulationTime, 0.04f, 4.0f);
	e->flModulationDepth = clampf(p->flModulationDepth, 0.0f, 1.0f);
	e->flAirAbsorptionGainHF = clampf(mB_to_gain(p->flAirAbsorptionHF), 0.892f, 1.0f);
	e->flHFReference = clampf(p->flHFReference, 1000.0f, 20000.0f);
	e->flLFReference = clampf(p->flLFReference, 20.0f, 1000.0f);
	e->flRoomRolloffFactor = clampf(p->flRoomRolloffFactor, 0.0f, 10.0f);
	e->iDecayHFLimit = (p->ulFlags & EAXREVERBFLAGS_DECAYHFLIMIT) ? 1 : 0;
}

extern "C" void AdjustEnvironmentSize(EAXREVERBPROPERTIES* p, float flEnvironmentSize)
{
	// Scales the time-based parameters with the environment size, as the
	// EAX SDK helper did. Only called if the engine adjusts presets at runtime.
	if (!p || flEnvironmentSize <= 0.0f || p->flEnvironmentSize <= 0.0f)
		return;
	float scale = flEnvironmentSize / p->flEnvironmentSize;
	if (p->ulFlags & EAXREVERBFLAGS_DECAYTIMESCALE)
		p->flDecayTime = clampf(p->flDecayTime * scale, 0.1f, 20.0f);
	if (p->ulFlags & EAXREVERBFLAGS_REFLECTIONSDELAYSCALE)
		p->flReflectionsDelay = clampf(p->flReflectionsDelay * scale, 0.0f, 0.3f);
	if (p->ulFlags & EAXREVERBFLAGS_REVERBDELAYSCALE)
		p->flReverbDelay = clampf(p->flReverbDelay * scale, 0.0f, 0.1f);
	if (p->ulFlags & EAXREVERBFLAGS_ECHOTIMESCALE)
		p->flEchoTime = clampf(p->flEchoTime * scale, 0.075f, 0.25f);
	if (p->ulFlags & EAXREVERBFLAGS_MODULATIONTIMESCALE)
		p->flModulationTime = clampf(p->flModulationTime * scale, 0.04f, 4.0f);
	p->flEnvironmentSize = flEnvironmentSize;
}
