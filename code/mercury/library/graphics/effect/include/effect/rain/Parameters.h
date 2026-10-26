#ifndef EFFECT_RAIN_PARAMETERS_H
#define EFFECT_RAIN_PARAMETERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Effect
{
namespace Rain
{

struct DropParameters
{
	DropParameters() : iAtlasSize(1), fDensity(0.0f), fSpeed(10.0f), fLength(0.5f), fWidth(0.05f), fSizeVariation(0.0f), fColourScale(1.0f), fColourScaleVariation(0.0f), fWind(0.0f) {}
	~DropParameters() {};
		
	Util::C_String	texture;
	U32 iAtlasSize;
	F32 fDensity;
	F32 fSpeed;
	F32 fLength;
	F32 fWidth;
	F32 fSizeVariation;
	F32 fColourScale;
	F32 fColourScaleVariation;
	F32 fWind;
	F32	fAlphaFadeSpeedBegin;
	F32	fAlphaFadeSpeedEnd;
};

struct SplashParameters
{
	SplashParameters() : fEmitRate(0.0f), fEmitRateRcp(0.1f), fSizeStart(0.1f), fSizeEnd(0.2f), fSizeVariation(0.0f), fHeightOffset(0.0f), fLife(0.5f), fLifeVariation(0.5f), fColourScale(1.0f), fColourScaleVariation(0.0f), fSpawnScaleSpeedBegin(10.0f), fSpawnScaleSpeedEnd(20.0f) {}
	~SplashParameters() {};

	Util::C_String	texture;

	F32		fEmitRate;
	F32		fEmitRateRcp;
	F32		fSizeStart;
	F32		fSizeEnd;
	F32		fSizeVariation;
	F32		fHeightOffset;
	F32		fLife;
	F32		fLifeVariation;
	F32		fColourScale;
	F32		fColourScaleVariation;
	F32		fSpawnScaleSpeedBegin;
	F32		fSpawnScaleSpeedEnd;
};

struct SheetParameters
{
	F32		fValue;
};

struct RainParameters
{
	DropParameters		dropParameters;
	SplashParameters	splashParameters;
	SheetParameters		sheetParameters;
};

} // namespace Rain
} // namespace Effect

#endif // EFFECT_RAIN_PARAMETERS_H

