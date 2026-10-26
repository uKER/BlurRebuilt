#ifndef RLLAPSPLITTIME_CONTROLLER_H
#define RLLAPSPLITTIME_CONTROLLER_H

#include "../Components/IRLComponent.h"
#include "../Data/RLDefinitions.h"

namespace Game
{
namespace Actors
{
class RLLapSplitTimeController : public IRLComponent
{
//Functions
public:
	RLLapSplitTimeController();
	~RLLapSplitTimeController();

	void										Update();

//Functions
private:
	void										Initialise();

	void										UpdateVehicle(const U8 vehicleIndex);
	void										UpdateVehicleLapTime(const U8 vehicleIndex);	//Update the lap and split times
	void										UpdateVehicleTimers(const U8 vehicleIndex);		//Update the lap end and split display timers

	void										VehicleCompletedLap(const U8 vehicleIndex);
	void										VehicleCompletedSplit(const U8 vehicleIndex);

	void										StoreLap(const U8 vehicleIndex, LapTimeData newLap);

	void										CheckFastestLap(const U8 vehicleIndex);
	void										CheckSlowestLap(const U8 vehicleIndex);

	void										FinaliseSplit(const U8 vehicleIndex);
	void										InitialiseCurrentSplit(const U8 vehicleIndex);

//Variable
private:

};
};	//namespace Actors
};	//namespace Game


#endif //RLLAPSPLITTIME_CONTROLLER_H
