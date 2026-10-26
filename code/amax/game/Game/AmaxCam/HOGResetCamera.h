#ifndef __HOGRESETCAMERA_H__
#define __HOGRESETCAMERA_H__

#include <camera2/camera.h>
#include <camera2/stock/ShakeCamera.h>
#pragma warning (push, 3)
#include <actor_system/ActorDeclare.h>
#pragma warning (pop)
#pragma warning (disable:4481) // nonstandard extension used: override

namespace AmaxCam
{
	using namespace Maths;

	class C_HOGResetCamera : public Cam2::C_Camera
	{
	public:
		C_HOGResetCamera();
		~C_HOGResetCamera();
		void OnUpdate() OVERRIDE;
		void Start(const Actor::C_ActorHandle& handle, const Cam2::C_Camera& startFrom);
		void GetObject();
		void SetInitialVelocity( const Maths::HVec3& velocity ) {mVelocity=velocity;}
		void SetCameraTracking(bool track) { mCameraTracking = track; };

	private:
		int		mVehicleIndex;
		bool	mCameraTracking;
		Cam2::C_ShakeCamera mPositionShake;
		Maths::HVec3 mVelocity;
		Maths::C_Quaternion mStartingOrient;
		F32		mTimer;
		F32		mOrientBlend;
		F32		mShakeBlend;
	};

} // End of namespace

#endif
