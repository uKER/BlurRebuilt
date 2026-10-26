#ifndef FRAMEWORK_TIMING_COLOURS_H
#define FRAMEWORK_TIMING_COLOURS_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(USE_CPU_TIMINGS) || defined(USE_GPU_TIMINGS)

// Primary colours...

const Maths::RGBA g_TimingColour_Update								(0,		0,		255,	255); // blue
const Maths::RGBA g_TimingColour_Render								(0,		255,	0,		255); // green
const Maths::RGBA g_TimingColour_RenderSplitscreenWithAA			(0,		255,	255,	255); // cyan
const Maths::RGBA g_TimingColour_RenderSplitscreenWithoutAA			(255,	255,	0,		255); // yellow
const Maths::RGBA g_TimingColour_RenderFullscreen					(255,	0,		255,	255); // magenta
const Maths::RGBA g_TimingColour_Present							(255,	0,		0,		255); // red

#endif

#endif // FRAMEWORK_TIMING_COLOURS_H
