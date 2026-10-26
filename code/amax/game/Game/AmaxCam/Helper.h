
#pragma warning (push, 3)
#include <actor_system/ActorDeclare.h>
#pragma warning (pop)

namespace Actor
{
	class C_ActorList;
}

namespace Cam2
{
	class C_CamObj;
}

namespace Game
{
	class C_GameWorld;
	class C_RaceLogicLayer;
	namespace Actors
	{
		class RaceLogicCore;
		class C_DriverActor;
		class C_VehicleActor;
		class C_VehicleLogicActor;
	}
}

namespace AmaxCam
{
using namespace Actor;
using namespace Cam2;
using namespace Game::Actors;
using namespace Maths;

// Helper functions to get the various objects that the camera system needs:

Game::C_GameWorld* GetGameWorld();
C_ActorList* GetActorList();
RaceLogicCore* GetRaceLogicCore();
Game::C_RaceLogicLayer* GetRaceLogicLayer();
C_ActorHandle GetVehicleHandle(S32 index);
const C_VehicleActor* GetVehicleActor(const C_ActorHandle& handle);
const C_VehicleLogicActor* GetVehicleLogicActor(int index);
const Game::Actors::C_DriverActor* GetDriverActor(const C_ActorHandle& handle);	// [Blur2PC] qualified; a Game::C_DriverActor forward declaration is visible too
C_CamObj VehicleActorToCameraObject(const C_VehicleActor* vehicleActor);
HVec3 GetVehicleVelocity(const C_VehicleActor* vehicleActor);

}
