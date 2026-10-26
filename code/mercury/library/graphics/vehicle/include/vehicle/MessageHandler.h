/////////////////////////////////////////////////////////////
// MessageHandler
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_MESSAGE_HANDLER_H__
#define __VEHICLE_MESSAGE_HANDLER_H__

namespace Vehicle
{

#define MAX_VEHICLE_MESSAGES 128

/* 
===============================================================
					enum E_MessageType

NOTES: 
===============================================================
*/
enum E_MessageType
{
	E_MessageType_Unknown = 0,

	E_MessageType_Visibility,

	E_MessageType_Max,
};

/* 
===============================================================
					class C_Message

NOTES: 
===============================================================
*/
class C_Message
{
public:

	C_Message();
	~C_Message();

	//! Type-specific process
	virtual void Process() = 0;

private:

	E_MessageType				m_eType;							//!< Message type
};

/* 
===============================================================
					class C_PartVisibleMessage

NOTES:					Example message
===============================================================
*/
class C_PartVisibleMessage
{
public:

	C_PartVisibleMessage();
	~C_PartVisibleMessage();

	//! Type-specific process
	void Process();

private:

	U32							m_iVehicleIndex;					//!< Index of vehicle that the message is aimed at

	U8							m_ePartCode;						//!< Code for part to turn on/off
	U8							m_ePositionCode;					//!< Positional code for part

	bool						m_bVisible;							//!< Visibility
};

/* 
===============================================================
					class C_MessageHandler

NOTES: Collects and handles messages 
===============================================================
*/
class C_MessageHandler
{
public:

	C_MessageHandler();
	~C_MessageHandler();

	//! Shuts down the message handler
	bool Destroy();

	//! Adds a message to the handler
	bool RaiseMessage(C_Message* p_pMessage);

	//! Processes messages raised
	void ProcessMessages();

private:

	Btl::Vector<C_Message*>				m_pMessages;				//!< Message list
};

} //namespace Vehicle

#endif

