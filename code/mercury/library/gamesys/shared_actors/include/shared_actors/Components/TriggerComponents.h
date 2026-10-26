#ifndef TRIGGERCOMPONENTS_1666D1CD_6A9B_4C02_8D05_E98CD6B5A3CD_H
#define TRIGGERCOMPONENTS_1666D1CD_6A9B_4C02_8D05_E98CD6B5A3CD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>


namespace Components
{

/// Component::C_ComponentInterfaceId values for shared trigger components
enum E_TriggerIF
{
	E_TriggerIF_HiBits = FOURCC_HI20('STGI'), // Shared actor TriGger Interface

	E_TriggerIF_Volume,	///< @see I_VolumeTrigger.
	E_TriggerIF_SourceVolume,

	E_TriggerIF_End, 
	E_TriggerIF_Begin = E_TriggerIF_HiBits + 1, 
	E_TriggerIF_Count = E_TriggerIF_End - E_TriggerIF_Begin
};

/// Component::C_ComponentTypeId values for shared trigger components
enum E_Trigger
{
	E_Trigger_HiBits = FOURCC_HI20('STGC'), // Shared actor TriGger Component

	E_Trigger_Box,		///< @see C_BoxTrigger.
	E_Trigger_Sphere,	///< @see C_SphereTrigger.

	E_TriggerSource_Box,		
	E_TriggerSource_Sphere,		

	E_Trigger_End, 
	E_Trigger_Begin = E_Trigger_HiBits + 1, 
	E_Trigger_Count = E_Trigger_End - E_Trigger_Begin
};

} // namespace Components


#endif // TRIGGERCOMPONENTS_1666D1CD_6A9B_4C02_8D05_E98CD6B5A3CD_H
