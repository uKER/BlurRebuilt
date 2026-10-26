#ifndef __CAMERASYSTEM_H__
#define __CAMERASYSTEM_H__

#include <camera2.h>

namespace Cam2
{
	class C_DebugTools;
}

namespace MGCam
{

class C_CameraSystem
{
public:
	C_CameraSystem();
	~C_CameraSystem();
	void Init();
	void Exit();

	// This update is called all the time:
	void RealUpdate();
	// This update is called when the game is running:
	void WorldUpdate();
	Cam2::C_DebugFeed& DebugFeed();
	void GameFeedShow(const Cam2::I_Feed* feed);
	void GameFeedRemove(const Cam2::I_Feed* feed);

	// This is the 'game camera', i.e. what the player sees in a Master Build:
	const Cam2::I_Feed& GameFeed();

	void RenderSplitscreen();
	void RenderFullscreen();
	void CopyFrom(const Cam2::C_Camera& other);

private:
	Cam2::C_DebugTools* m_DebugTools;
	Cam2::C_Feed* m_GameFeed;
	Cam2::C_Camera* m_Camera;

	Cam2::C_DebugFeed& DebugFeedPriv();
	void DebugFeedShow(S32 debugType);
	void PhotoModeTest();

	friend class C_CameraSwitcher;
};

// C_CameraSystem Singleton:
C_CameraSystem& System();

}

#endif
