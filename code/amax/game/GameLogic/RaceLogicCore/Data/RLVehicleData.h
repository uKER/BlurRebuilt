#ifndef RLVEHICLEDATA_H
#define RLVEHICLEDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include "RLDefinitions.h"
#include "../../../Game/Messaging/VehicleActorMessages.h"
#include "../../../Game/GameActors/Vehicles/VehiclePerformanceModifiers.h"

#define INVALID_VEHICLE_IDX 255

namespace Vehicle 
{ 
	class C_ModelInput; 
}

namespace Game
{
namespace Actors
{
class C_VehicleActor;


class RLVehicleData
{
//Functions
public:
	RLVehicleData()	{ Initialise();	}
	~RLVehicleData(){};

	void										Initialise()
	{
		mVehicleIndex = -1;
		mCreated = false;
		mLoaded = false;

		m_pVehicleActor = 0;
		mActorHandle = Actor::C_ActorHandle();
	}

	void										PrepareForFrame()
	{
		//Make sure this vehicle's valid for this update
		mValid = true;
	}


//Variables
public:

	S8											mVehicleIndex;					//Index of this vehicle in the RLC.mVehicles array
	bool										mValid;							//Is this data valid for this frame
		
	///////////////////////////////////////////////////////////////////////////
	//Actor stuff
	///////////////////////////////////////////////////////////////////////////
	Actor::C_ActorHandle						mActorHandle;					//Handle to the actor
	C_VehicleActor								*m_pVehicleActor;				//The pointer to the actor itself...we probably only need one of these
	bool										mCreated,						//Has the actor been spawned/created yet
												mLoaded;						//Has the vehicle loaded all the resources it needs
	///////////////////////////////////////////////////////////////////////////
	//Actor stuff
	///////////////////////////////////////////////////////////////////////////

private:
	//RLVehicleData(const RLVehicleData& other);	//What's this doing here?
};

};//namespace Actors
};//namespace Game

#endif //RLVEHICLEDATA_H
