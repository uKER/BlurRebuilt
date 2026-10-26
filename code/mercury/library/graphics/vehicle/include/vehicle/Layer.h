/////////////////////////////////////////////////////////////
// Layer
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_LAYER_H__
#define __VEHICLE_LAYER_H__

namespace Sys { class C_FileBlock; }
namespace Arch { class BinaryInputArchive; }

namespace Vehicle
{

class C_MessageHandler;
class C_Message;

/* 
===============================================================
					class C_Layer

NOTES: Base layer class
===============================================================
*/
class C_Layer
{
public:

	C_Layer();

	~C_Layer();

	//! Loads layer
	virtual bool Load(Arch::BinaryInputArchive* p_Input) { return false; }

	//! Creates layer
	virtual bool Create() { return false; }

	//! Destroys layer
	virtual bool Destroy() { return false; }

	//! Updates layer
	virtual void Update() {}

	//! Sets the message handler pointer, used to send and receive messages
	void SetMessageHandler(C_MessageHandler* p_pMessageHandler) { m_pMessageHandler = p_pMessageHandler; }

	//! Raises a message and casts it out there into the ether
	bool RaiseMessage(C_Message* p_pMessage);

private:

	C_MessageHandler*					m_pMessageHandler;				//!< Handles message processing
};

} //namespace Vehicle

#endif

