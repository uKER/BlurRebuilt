
#ifndef CAMERA2_DEBUGTOOLS_H
#define CAMERA2_DEBUGTOOLS_H

#include <camera2/network/NetworkLink.h>
#include <camera2/Camera.h>

namespace Cam2
{
class C_AnimCamera;
class C_Camera;
class C_CamObj;
class C_PositionLogger;
class C_NetworkLink;
class C_TvFeed;
class I_Feed;

class C_DebugTools
{
public:
	C_DebugTools();
	~C_DebugTools();
	bool Init();
	void Exit();
	void Update(F32 period);
	void SetObject(const C_CamObj& object);
	void DebugRender();
	C_TvFeed* TvFeed() { return m_TvFeed; }

	// SetLocalCamera should be called before update with the live camera (so m_DebugFeed.Live() usually)
	// so that the camera position can be sent to Maya	
	void SetLocalCamera(const C_Camera& cam);
	E_NetNotify Notify();
	C_AnimCamera* AnimCamera() { return m_pAnimCamera; }
	const C_Camera& RemoteCamera() const;
	const C_Camera& LocalCamera() const { return m_LocalCamera; }
	void LogFrame(const C_Camera& cam);

private:
	C_NetworkLink* m_pNetworkLink;
	C_AnimCamera* m_pAnimCamera;
	C_TvFeed* m_TvFeed;
	C_Camera m_LocalCamera, m_RemoteCamera; // Remote camera used for setting debug positions
	E_NetNotify m_Notify;
	C_PositionLogger* m_PosLogger;

	void GetCameraPosition();
	void SetCameraPosition();
	void DoMapWizard();
};

} // End of namespace

#endif
