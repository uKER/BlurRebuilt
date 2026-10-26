#ifndef GLOBALSIGNALS_H
#define GLOBALSIGNALS_H

#include <sigslot/sigslot.h>

namespace Animation
{
	class C_Skeleton;
	class C_SkeletonInstance;
	class C_Pose;
}

enum E_CameraMode;

class C_GlobalSignals
{
public:

	static C_GlobalSignals&		Instance()
	{
		static C_GlobalSignals inst;
		return inst;
	}
	
	sigslot::signal1<F32>	animationUpdated;
	sigslot::signal1<S32>	jointSelected;
	sigslot::signal4<const Animation::C_Skeleton*, Animation::C_SkeletonInstance*, Animation::C_Pose*, const char*>	skeletonLoaded;
	sigslot::signal1<F32>	timelineUpdated;
	sigslot::signal1<E_CameraMode>	cameraChanged;
	
	sigslot::signal3<S32, S32, U32> onViewportMouseMove;
	sigslot::signal3<S32, S32, U32> onViewportMouseUp;
	sigslot::signal3<S32, S32, U32> onViewportMouseDown;
	sigslot::signal1<U32> onViewportKeyDown;
	sigslot::signal1<U32> onViewportKeyUp;
	sigslot::signal2<U32, U32> onViewportResize;
	
};


#endif
