#ifndef RLDEBUGLINE_CONTROLLER_H
#define RLDEBUGLINE_CONTROLLER_H

#include "../Components/IRLComponent.h"
#include "../Data/RLDefinitions.h"

namespace Game
{
namespace Actors
{
class RLDebugLineController : public IRLComponent
{
//Functions
public:
	RLDebugLineController();
	~RLDebugLineController();

	void										Initialise();
	void										Update();

	void										Render();

	RLDebugLine									*AddDebugLine(	const enum E_DEBUG_LINE_TYPE eType,
																const Maths::HVec3 start, 
																const Maths::HVec3 end, 
																const Maths::RGBA colour);

	void										RemoveDebugLine(RLDebugLine *pLineToBeRemoved);
	void										RemoveDebugLinesByType(const enum E_DEBUG_LINE_TYPE eType);
	void										CleanUpDebugLines();


};

};	//namespace Actors
};	//namespace Game


#endif //RLDEBUGLINE_CONTROLLER_H

