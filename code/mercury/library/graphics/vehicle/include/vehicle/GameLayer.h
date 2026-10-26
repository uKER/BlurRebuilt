/////////////////////////////////////////////////////////////
// GameLayer
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_GAME_LAYER_H__
#define __VEHICLE_GAME_LAYER_H__

#include "Vehicle/Layer.h"

namespace Vehicle
{

/* 
===============================================================
					class C_GameLayer

NOTES: Handles the game-side logic and updating
===============================================================
*/
class C_GameLayer : public C_Layer
{
public:

	C_GameLayer();

	~C_GameLayer();

	//! Creates layer
	bool Create();

	//! Destroys layer
	bool Destroy();

	//! Updates layer
	void Update();

private:

};

} //namespace Vehicle

#endif

