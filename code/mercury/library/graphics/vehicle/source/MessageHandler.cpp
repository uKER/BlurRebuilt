/////////////////////////////////////////////////////////////
// MessageHandler
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/MessageHandler.h"
#include "Vehicle/VehicleModelManager.h"
#include "Vehicle/VehicleModel.h"
#include "Vehicle/RenderLayer.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_MessageHandler

NOTES: Collects and handles messages 
===============================================================
*/
/*
===============
NAME: C_MessageHandler

NOTE: Constructor
===============
*/
C_MessageHandler::C_MessageHandler()
{

}

/*
===============
NAME: ~C_MessageHandler

NOTE: Constructor
===============
*/
C_MessageHandler::~C_MessageHandler()
{

}

/*
===============
NAME: Destroy

NOTE: Shuts down the message handler
===============
*/
bool C_MessageHandler::Destroy()
{
	for (int iMessage = 0; iMessage < m_pMessages.Size(); iMessage++)
	{
		C_Message* pMsg = m_pMessages[iMessage];

		if (pMsg)
		{
			SAFE_DELETE_PTR(pMsg);
		}
	}

	m_pMessages.Clear();

	return true;
}

/*
===============
NAME: RaiseMessage

NOTE: Adds a message to the handler
===============
*/
bool C_MessageHandler::RaiseMessage(C_Message* p_pMessage)
{
	if (m_pMessages.Size() > MAX_VEHICLE_MESSAGES)
	{
		SAFE_DELETE_PTR(p_pMessage);
		return false;
	}

	m_pMessages.PushBack(p_pMessage);

	return true;
}

/*
===============
NAME: ProcessMessages

NOTE: Processes message raised between internal layers and externally
===============
*/
void C_MessageHandler::ProcessMessages()
{
	for (int iMessage = 0; iMessage < m_pMessages.Size(); iMessage++)
	{
		C_Message* pMsg = m_pMessages[iMessage];

		if (pMsg)
		{
			pMsg->Process();
			SAFE_DELETE_PTR(pMsg);
		}
	}

	m_pMessages.Clear();
}

/* 
===============================================================
					class C_Message

NOTES: 
===============================================================
*/

/*
===============
NAME: C_Message

NOTE: Constructor
===============
*/
C_Message::C_Message()
{
	m_eType			= E_MessageType_Unknown;
}

/*
===============
NAME: ~C_Message

NOTE: Constructor
===============
*/
C_Message::~C_Message()
{

}

/* 
===============================================================
					class C_PartVisibleMessage

NOTES:					Example message
===============================================================
*/
/*
===============
NAME: C_PartVisibleMessage

NOTE: Constructor
===============
*/
C_PartVisibleMessage::C_PartVisibleMessage()
{
	m_iVehicleIndex		= UINT_MAX;

	m_ePartCode			= 0;
	m_ePositionCode		= 0;

	m_bVisible			= true;
}

/*
===============
NAME: ~C_PartVisibleMessage

NOTE: Destructor
===============
*/
C_PartVisibleMessage::~C_PartVisibleMessage()
{

}

/*
===============
NAME: Process

NOTE: Type-specific process
===============
*/
void C_PartVisibleMessage::Process()
{
	if (!C_VehicleModelManager::Instance().GetVehicleModel(m_iVehicleIndex))
		return;

	if (!C_VehicleModelManager::Instance().GetVehicleModel(m_iVehicleIndex)->GetRenderLayer())
		return;

	C_VehicleModelManager::Instance().GetVehicleModel(m_iVehicleIndex)->GetRenderLayer()->SetPartVisible(m_ePartCode, m_ePositionCode, m_bVisible);	
}

} //namespace Vehicle

