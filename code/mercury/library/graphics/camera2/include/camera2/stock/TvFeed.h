
#ifndef CAMERA2_TVFEED_H
#define CAMERA2_TVFEED_H

#include <camera2/Feed.h>

#pragma warning (disable:4481) // nonstandard extension used: override specifier

namespace Cam2
{
class C_AnimCamera;
class C_CameraScene;

class C_TvFeed : public I_Feed
{
public:
	C_TvFeed();
	virtual ~C_TvFeed();
	virtual void Update(F32 period) OVERRIDE;
	virtual const C_Camera& Live() const OVERRIDE;
	void LoadTvScene(Util::C_String filename);
	void FreeTvScene();

	void DebugRender() const;
	S32 GetCameraCount();
	const C_Camera* GetCamera(S32 index);
	Util::C_String CameraName();

private:
	S32 m_liveZone, m_liveCam, m_prevCam;
	C_CameraScene* m_pScene;

	Ut::Vector<C_AnimCamera*> m_tvcam;
};

} // End of namespace Cam2

#endif
