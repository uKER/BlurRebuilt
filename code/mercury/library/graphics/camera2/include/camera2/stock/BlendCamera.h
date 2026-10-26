
#ifndef CAMERA2_BLENDCAMERA_H
#define CAMERA2_BLENDCAMERA_H

#include <camera2/Camera.h>

namespace Cam2
{

// A BlendCamera blends between the position and orientation of several different cameras

class C_BlendCamera : public C_Camera
{
public:
	C_BlendCamera();
	void OnUpdate() OVERRIDE;

	void SetCameraSlot(S32 n, const C_Camera* camera) { SetFeedSlot(n, camera); } // Depreciated :-)
	void SetFeedSlot(S32 n, const I_Feed* feed);
	void BlendTo(S32 n, F32 blendTime = 1.f, F32 blendDelay = 0.0f);
	bool BlendActive();
	S32 GetIndex();

protected:
	Ut::Vector<const I_Feed *> m_FeedList;
	Ut::Vector<F32> m_weight; // Weighting of each camera
	F32 m_blendDelay;			// time before blend starts
	F32 m_counter, m_blendTime; // Time that the blend takes
	S32 m_blendFrom, m_blendTo;   // Which camera in the list we are currently blending from/to
	Util::C_String m_BlendName;
	
	void BlendWithWeights();
};

} // End of namespace Cam2

#endif
