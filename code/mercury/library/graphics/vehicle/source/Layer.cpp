/////////////////////////////////////////////////////////////
// Layer
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Layer.h"
#include "Vehicle/MessageHandler.h"

#include "system/serialise/c_filesystem.h"
#include "system/serialise/c_fileblock.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_Layer

NOTES: Base layer class
===============================================================
*/

/*
===============
NAME: C_Layer

NOTE: Constructor
===============
*/
C_Layer::C_Layer()
{
	m_pMessageHandler	= NULL;
}

/*
===============
NAME: ~C_Layer

NOTE: Constructor
===============
*/
C_Layer::~C_Layer()
{
	m_pMessageHandler	= NULL;
}

/*
===============
NAME: RaiseMessage

NOTE: Raises a message and casts it out there into the ether
===============
*/
bool C_Layer::RaiseMessage(C_Message* p_pMessage)
{
	if (m_pMessageHandler)
	{
		m_pMessageHandler->RaiseMessage(p_pMessage);
		return true;
	}

	return false;
}

} //namespace Vehicle

