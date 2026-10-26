
#ifndef CAMERA2_DEBUGFEED_H
#define CAMERA2_DEBUGFEED_H

#include <camera2/Feed.h>

namespace Cam2
{

using namespace Maths;

// A DebugFeed shows an application defined feed, a managed debug camera, or a LookAt camera
class CamInput;
class C_DebugFeed : public I_Feed
{
public:
	C_DebugFeed(I_Feed* feed = 0);
	virtual ~C_DebugFeed();
	virtual void Update(F32 period);
	virtual const C_Camera& Live() const;

	void SetInput(const CamInput& input);
	void ShowFeed(I_Feed* feed);
	void SetDebugType(S32 type);
	bool UsesInput() const;
	S32 PadUsed() const;
	
	void ShowLook(bool look);
	void SetLookTarget(const HVec3& target, F32 fov) { m_LookTarget = target; m_LookFov = fov; }

	void CopyFrom(const C_Camera& camera);
	void SetAppMotion(const HVec3& motion) { m_AppMotion = motion; }

	C_Camera* DebugCamera() { return m_DebugCamera; }
	float GameSpeed() const;

private:
	const C_Camera& AppOrDebugLive() const;

	I_Feed* m_AppFeed; // App feed
	C_Camera* m_DebugCamera; // Debug camera
	C_Camera* m_LookCamera; // Look At debug override
	HVec3 m_LookTarget;
	F32 m_LookFov;
	HVec3 m_AppMotion; // Last frame of motion from AppFeed
	S32 m_PadUsed;
};

} // End of namespace Cam2

#endif
