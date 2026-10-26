#ifndef RLDATABASE_CONTROLLER_H
#define RLDATABASE_CONTROLLER_H

#include "../Components/IRLComponent.h"
#include "../Data/RLDefinitions.h"

namespace Game
{
namespace Actors
{
class RLDatabaseController : public IRLComponent
{
//Functions
public:
	RLDatabaseController();
	~RLDatabaseController();

	void										Update();


private:
	void										Initialise();
};

};	//namespace Actors
};	//namespace Game


#endif //RLDATABASE_CONTROLLER_H
