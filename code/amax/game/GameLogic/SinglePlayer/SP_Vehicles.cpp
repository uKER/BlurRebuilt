#include "StdAfx.h"
#include "SinglePlayerImpl.h"

bool MP_IsLockedLegendCar(XtId vehicleId);

bool SP_IsVehicleUnlocked(SP_SaveGame* sg, XtId vehicleId)
{
	if (!sg)
		return false;

	SP_SaveVehicle* sv = SP_GetSaveVehicle(sg, vehicleId);

	if (!sv)
		return false;

	return sv->IsUnlocked();
}

bool SP_IsVehicleNew(SP_SaveGame* sg, XtId vehicleId)
{
	if (!sg)
		return false;

	SP_SaveVehicle* sv = SP_GetSaveVehicle(sg, vehicleId);

	if (!sv)
		return false;

	return sv->IsNew();
}

int SP_GetVehiclesAvailableCount(SP_SaveGame* sg)
{
	 if (!sg || !gSpRankData)
		 return false;

	 int count = 0;

	 for(XtUt::Set<SP_VehicleIndex>::Iterator it(&gSpIndexData->vehicles); it; ++it)
	 {
		 if (SP_IsVehicleUnlocked(sg, it->id) && !MP_IsLockedLegendCar(it->id))
			 count++;
	 }

	 return count;
}