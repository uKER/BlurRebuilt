#ifndef MULTIPLAYER_UTILITIES_H
#define MULTIPLAYER_UTILITIES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network\Misc\NetDefines.h>

typedef Btl::Vector<U8, Btl::VectorParameters_Static<RacerNet::NET_MAX_VEHICLES> > NetVehicleIndexStack;

namespace Game
{

bool											VehicleIsLocal(U8 p_VehicleIndex);
bool											HostIsLocal();
bool											VehiclesAreTeammates(S8 vehicleOne, S8 vehicleTwo);

S32												GetDriverRacePoints(U8 position, U8 numVehicles);

S8												GetLocalIndexFromJoinId(U8 joinId);
U8												GetJoinIdFromLocalIndex(S8 vehicleIndex);
U32												GetLocalVehicleList(NetVehicleIndexStack& localList);
U32												GetNetworkVehicleList(NetVehicleIndexStack& networkList);
U8												GetLocalPlayerList(NetVehicleIndexStack& localList);
U32												GetNumGlobalRacing();
U32												GetGlobalRacingList(NetVehicleIndexStack& globalRacingList);
U32												GetLocalRacingList(NetVehicleIndexStack& localList);
U32												GetGlobalInRaceList(NetVehicleIndexStack& globalList);
U32												GetNumConnected();
U32												GetNumDisconnected();
U32												GetDisconnectedList(NetVehicleIndexStack& disconnectedList);

bool											AnyVehiclesFinished();

void											IncrementUpdateTimestep();
void											ReduceUpdateTimestep(bool sendASAP = false);

} // namespace Game

#endif // MULTIPLAYER_UTILITIES_H
