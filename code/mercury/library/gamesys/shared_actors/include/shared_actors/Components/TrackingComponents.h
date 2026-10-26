#ifndef TRACKINGCOMPONENTS_6045E489_27A5_4DF9_A73E_B09387A65632_H
#define TRACKINGCOMPONENTS_6045E489_27A5_4DF9_A73E_B09387A65632_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>


namespace Components
{

/// Component::C_ComponentInterfaceId values for shared tracking components
enum E_TrackingIF
{
	E_TrackingIF_HiBits = FOURCC_HI20('STKI'), // Shared actor TracKing Interface

	E_TrackingIF_Actor,	///< @see I_ActorTracker.

	E_TrackingIF_End, 
	E_TrackingIF_Begin = E_TrackingIF_HiBits + 1, 
	E_TrackingIF_Count = E_TrackingIF_End - E_TrackingIF_Begin
};

/// Component::C_ComponentTypeId values for shared tracking components
enum E_Tracking
{
	E_Tracking_HiBits = FOURCC_HI20('STKC'), // Shared actor TracKing Component

	E_Tracking_SpecificActor,		///< @see C_SpecificActorTracker.

	E_Tracking_End, 
	E_Tracking_Begin = E_Tracking_HiBits + 1, 
	E_Tracking_Count = E_Tracking_End - E_Tracking_Begin
};

} // namespace Components


#endif // TRACKINGCOMPONENTS_6045E489_27A5_4DF9_A73E_B09387A65632_H
