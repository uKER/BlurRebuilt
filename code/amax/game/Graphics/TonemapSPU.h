#ifndef AMAX_GRAPHICS_TONEMAPSPU_H
#define AMAX_GRAPHICS_TONEMAPSPU_H

namespace Scheduler
{
	struct jqModule;
};

namespace Tonemap
{
	struct TaskData
	{
		Display::C_LockedBox lockedBoxSrc;
		Display::C_LockedBox lockedBoxGamma;
		Display::C_LockedBox lockedBoxDst;
		Maths::Mat44* mColour;
		float fGamma;
		int UpdateGamma;
	};
};

extern Scheduler::jqModule TonemapModule;

#endif // AMAX_GRAPHICS_TONEMAPSPU_H
