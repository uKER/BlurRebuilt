#ifndef MESSAGINGDECLARE_H
#define MESSAGINGDECLARE_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Messaging
{

	class C_MessageBase;
	class C_MessageHandler;
	class C_MessageSerialiser;
	class C_MessageStats;
	class C_MessageSubscriberSystem;
	class C_MessageTypeListFixed;
	class C_MessageTypeListVariable;

	class I_MessageAllocator;
	class I_MessageTypeList;
	class I_MessageTypeVisitor;

	struct C_MessageDebugInfo;
	struct C_MessageSubscriber;
	struct C_MessageTypeRange;

	/// @brief Type identifier for manual RTTI.
	/// A unique integer identifier is assigned to each distinct message type 
	/// (user is responsible for ensuring uniqueness). This allows the message 
	/// system to obtain run-time type information from a message object.
	typedef S32 TMsgType;

} // namespace Messaging


#endif // MESSAGINGDECLARE_H
