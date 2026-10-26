#ifndef __CAMERASYSTEM_H__
#define __CAMERASYSTEM_H__

#include <actor_system/ActorDeclare.h>

namespace SceneGraph
{
	template <typename T> class C_Handle;
	class C_SpotLight;
	typedef C_Handle<C_SpotLight> C_SpotLightHandle;
}

namespace Cam2 
{
	class C_DebugFeed;
	class C_DebugTools;
	class C_Feed;
	class I_Feed;
}

namespace AmaxCam
{

enum E_GameCam
{
	E_GameCam_None = 0,
	E_GameCam_VehicleFeed,
};

class C_CubeCapture;
class C_HOGResetCamera;
class C_FatalCamera;
class C_OrbitCamera;
class C_VehicleFeed;
class C_PhotoCamera2;
class C_SetPieceCamera;

// Multiple viewport stuff:
class C_Viewport
{
public:
	C_Viewport();
	~C_Viewport();
	void Exit();

	Cam2::C_Feed *m_GameFeed, *m_FutureFeed;
	C_VehicleFeed *m_VehicleFeed, *m_VehicleFuture;
	Actor::C_ActorHandle m_FocusHandle;
	E_GameCam m_CameraType;
	C_HOGResetCamera* m_HOGResetCamera;
};

class C_CameraSystem
{
public:
	C_CameraSystem();
	~C_CameraSystem();
	void Exit();
	
	// This update is called all the time:
	void RealUpdate();
	// This update is called when the game is running:
	void CameraChangeUpdate(F32 period);
	void WorldUpdate(F32 period);
	void WorldUpdate2(F32 period);
	Cam2::C_DebugFeed& DebugFeed(S32 viewport = 0);
	const Cam2::C_DebugFeed& DebugFeed(S32 viewport = 0) const;
	void GameFeedShow(S32 viewport, E_GameCam cameraType, int whichFeed = 0);
	void GameFeedShow(S32 viewport, const Cam2::I_Feed* feed, int whichFeed = 0);
	void GameFeedRemove(const Cam2::I_Feed* feed);

	// This is the 'game camera', i.e. what the player sees in a Master Build:
	const Cam2::I_Feed& GameFeed(S32 vp = 0) const;
	const Cam2::I_Feed& FutureFeed(S32 vp = 0) const;
	
	void RenderSplitscreen();
	void RenderFullscreen();

	void SetFocusHandle(S32 viewport, const Actor::C_ActorHandle handle);
	Actor::C_ActorHandle FocusHandle(S32 vp = 0) const;
	void ResetVehicleFeed(S32 vp = 0);

	C_VehicleFeed* VehicleFeed(S32 vp = 0);
	void StartFatalCamera();
	void StopFatalCamera();

	void StartHOGResetCamera( bool objectTracking, int viewport );
	void StopHOGResetCamera( int viewport );
	bool HOGResetCameraIsActive(int viewport = 0) const;

	void StartSetPieceCamera(Maths::HVec3 p_Position, Maths::HVec3 p_LookAt, float p_Fov, bool p_TrackObject);
	void StopSetPieceCamera();

	void StartPhotoCamera();
	void StopPhotoCamera();
	bool IsPhotoCamera() const;
	U32 IsPhotoCamera_Mask() const;

	void DebugFeedShow(S32 debugViewport, S32 debugType, S32 gameViewport = 0);

	C_PhotoCamera2* PhotoCamera2() { return m_PhotoCamera2; }
	bool GameViewIsInsideCar(U32 viewport) const;
	bool PadUsedForDebug(S32 pad) const;
	void SetViewConfig(S32 viewConfig) { m_ViewConfig = viewConfig; }
	S32 ViewConfig() const { return m_ViewConfig; }
	F32 FovScale() const;
	Maths::HVec2 ScreenOffset( int viewportIdx ) const;

	U32 GetViewportCount() const;
private:
	void CheckVehicleFeeds();
	void SetViewportCount(S32 count);
	void FreeViewports();
	bool IsOkay();
	void RenderCameraInfo(S32 viewport);
	void ProcessInput();
	void GetCurrentFocusObject();
	void UpdateDebugTools(F32 period);
	void DoLookAt();
	void CopyDepthOfFieldInfo();
	void DebugRender();
	void UpdateSpotlight();
	void UpdateWreckedEffect();
	friend class C_CameraSwitcher;

	Cam2::C_DebugTools* m_DebugTools;

	E_GameCam m_GameCam; // enum for which camera is being shown
	
	C_FatalCamera* m_FatalCamera;
	C_OrbitCamera* m_OrbitCamera;
	C_SetPieceCamera* m_pSetPieceCamera;
	C_PhotoCamera2* m_PhotoCamera2;
	bool			m_Lod;

	// Multiple viewport stuff:
	Btl::Vector<C_Viewport> m_Viewport;

	SceneGraph::C_SpotLightHandle* m_SpotLight;
	S32 m_ViewConfig;

#ifndef CFG_GOLD
	C_CubeCapture* m_CubeCapture;
#endif
};

// C_CameraSystem Singleton:
C_CameraSystem& CameraSystem();

}

#endif
