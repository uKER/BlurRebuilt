#ifndef VEHICLE_LAPS_INFO_H
#define VEHICLE_LAPS_INFO_H

#ifdef _MSC_VER
#pragma once
#endif

struct VehicleLapsInfo
{
	S8				mCurrentLap;
	float			mCurrentLapTime;

	float			mBestLapTime;
};

#endif //VEHICLE_LAPS_INFO_H