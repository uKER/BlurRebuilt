#ifndef RLRACEPOSITION_CONTROLLER_H
#define RLRACEPOSITION_CONTROLLER_H

#include "../Components/IRLComponent.h"
#include "../Data/RLDefinitions.h"

namespace Game
{
namespace Actors
{
class RLRacePositionController : public IRLComponent
{
//Functions
public:
	RLRacePositionController();
	~RLRacePositionController();

	void										Update();

private:
	void										Initialise();
	void										CalculateRacePositions(const float frameTime);
	void										UpdateLeaderboardByPosition(const float frameTime);
	void										UpdateLeaderboardByDamage(const float frameTime);

	int											mFirstPosition;
};

};	//namespace Actors
};	//namespace Game


#endif //RLRACEPOSITION_CONTROLLER_H
