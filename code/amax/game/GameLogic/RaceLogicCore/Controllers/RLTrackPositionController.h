#ifndef RLTRACKPOSITIION_CONTROLLER_H
#define RLTRACKPOSITIION_CONTROLLER_H

#include "../Data/RLDefinitions.h"
#include "../Components/IRLComponent.h"

namespace Game
{
namespace Actors
{
class RLTrackPositionController : public IRLComponent
{
//Functions
public:
	RLTrackPositionController();
	~RLTrackPositionController();

	void										Initialise();
	void										PreUpdate();
	void										Update();

	void										AnalyseSpline();
	void										AnalyseSplits();


	void										AnalyseActorStudioSplits();

	void										PreUpdateVehicle(const U8 vehicleIndex);
	void										UpdateVehicle(const U8 vehicleIndex);


	void										CalculateInitialNextAndPreviousSplitIndices(const U8 vehicleIndex);
	const bool									ValidSplit(const U8 vehicleIndex, const U8 splitIndex) const;
	void										SplitCrossed(const U8 vehicleIndex, const U8 splitIndex);
	void										CheckVehicleSplitPosition(const U8 vehicleIndex);

	const bool									Line2DIntersect(Maths::HVec3 currentPosition, Maths::HVec3 lastPosition, Maths::HVec3 lineStart, Maths::HVec3 lineEnd);

	void										CalculateProximities();
	void										CalculateVehicleProximities(const U8 vehicleIndex);

	void										CalculateVehicleTrackPercentage(const U8 vehicleIndex);
	const float									CalculateTrackPercentage(const float accurateWaypointPosition) const;

	void										CalculateVehicleRacePercentage(const U8 vehicleIndex);
	void										CalculateVehicleDistanceFromStartLine(const U8 vehicleIndex);
	void										CalculateVehicleDistanceToNextAndPreviousSplitLine(const U8 vehicleIndex);

	void										CalculateVehicleWrongWayDistance(const U8 vehicleIndex);
	void										CalculateVehicleDistancesToCarsAheadAndBehind(const U8 vehicleIndex);

	void										CalculateSplitOrder();

	const float									CalculateDistanceToVehicle(const U8 vehicleIndex, const U8 otherVehicleIndex);
	const float									CalculateTimeToVehicle(const U8 vehicleIndex, const U8 otherVehicleIndex);

	const SplitTimeData							GetSplitTimeDataAtRaceSplitNumber(const U8 vehicleIndex, const int raceSplitNumber) const;

	void										AddDebugLines();
	void										AddDebugLinesForSplitLines();
	void										AddDebugLinesForStartPositions();
	//void										AddDebugLinesForPickupPositions();

	void										ResetVehicleOnPoint(int vehicleIndex, Maths::HMat33 orientation, Maths::HVec3 velocity, Maths::HVec3 point);
	const bool									ValidateDatabase() const;
};
};	//namespace Actors
};	//namespace Game


#endif //RLTRACKPOSITIION_CONTROLLER_H
