#ifndef PICKUP_POSITION_MANAGER
#define PICKUP_POSITION_MANAGER

#ifdef _MSC_VER
#pragma once
#endif

#include "System/misc/Random.h"
#include "GameLogic/RaceLogicCore/Data/RLEnums.h"
#include "xt/XtCoreTypes.h"
#include "btl/Containers/map.h"

struct RLPickupPositionRow;
namespace Game { class C_RaceLogicLayer; }	// [Blur2PC] was declared at global scope; the class is Game::C_RaceLogicLayer

namespace Game
{
	struct PickupPositionInfo
	{
	public:
		PickupPositionInfo() {};
		PickupPositionInfo(U8 pickups) : mPickupsAllowed(pickups) { Init(); };

		void						Init();
		E_PICKUP_TYPE				AssignPickup(Util::C_Random& randGen);

	private:
		U8							mPickupsAllowed;
		Btl::Vector<E_PICKUP_TYPE>	mPickupsAvailable;
	};

	class C_PickupPositionManager
	{
	public:
		void			Init(C_RaceLogicLayer* pLayer);
		void			Reset();

		U64				GetMapId(U32 pickupId, XtId recordId);
		E_PICKUP_TYPE	AddPickupActor(U32 pickupId, const RLPickupPositionRow* pickupRow);

	private:
		Btl::Map<U64, PickupPositionInfo>	mPickupInfoList;
		C_RaceLogicLayer*					mpRaceLogicLayer;
	};
}

#endif // PICKUP_POSITION_MANAGER