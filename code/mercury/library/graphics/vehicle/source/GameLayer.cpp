/////////////////////////////////////////////////////////////
// GameLayer
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/GameLayer.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_GameLayer

NOTES: Handles the game-side logic and updating
===============================================================
*/

/*
===============
NAME: C_GameLayer

NOTE: Constructor
===============
*/
C_GameLayer::C_GameLayer()
{

}

/*
===============
NAME: ~C_GameLayer

NOTE: Constructor
===============
*/
C_GameLayer::~C_GameLayer()
{

}

/*
===============
NAME: Create

NOTE: Creates layer
===============
*/
bool C_GameLayer::Create()
{
	return true;
}

/*
===============
NAME: Destroy

NOTE: Destroys layer
===============
*/
bool C_GameLayer::Destroy()
{
	return true;
}

/*
===============
NAME: Update

NOTE: Updates layer
===============
*/
void C_GameLayer::Update()
{
	
}

} //namespace Vehicle

