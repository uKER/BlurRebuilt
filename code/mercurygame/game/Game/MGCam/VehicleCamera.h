#ifndef __VEHICLE_CAMERA_H__
#define __VEHICLE_CAMERA_H__

#include <camera2/stock/ChaseCamera2.h>

namespace Gravity { class C_Scene; };

namespace MGCam
{

class C_PhysicsChecks;

class C_VehicleCamera : public Cam2::C_ChaseCamera2
{
public:
	C_VehicleCamera(Gravity::C_Scene* physicsScene, S32 groupId);
	~C_VehicleCamera();
	void OnUpdate() OVERRIDE;

private:
	C_PhysicsChecks* m_PhysicsChecks;
};

} // End of namespace

#endif
