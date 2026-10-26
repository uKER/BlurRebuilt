#ifndef MESSAGEDISPATCHER_DAA8576B_A3EF_4338_BE71_D627B08C799F_H
#define MESSAGEDISPATCHER_DAA8576B_A3EF_4338_BE71_D627B08C799F_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/Map.h>
#include <misc_shared/messaging/MessagingDeclare.h>


namespace Messaging
{

/**
@brief Utility for dispatching messages to functions.

A common pattern for implementing C_MessageHandler::MessageHandler is to
switch on the message type and dispatch the message to another function:
@code
class C_MyClass : public Messaging::C_MessageHandler
{
	virtual void OnFoo(const C_FooMsg* pMessage);
	static void OnBar();

	virtual bool C_MyClass::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		switch (pMessage->Type())
		{
		case E_MyMessageTypes_Foo:	OnFoo(Messaging::MessageCast<C_FooMsg>(pMessage);	return true;
		case E_MyMessageTypes_Bar:	OnBar();											return true;
		default:					return Messaging::C_MessageHandler::MessageHandler(pMessage);
		}
	}
};
@endcode

We can use C_MessageDispatchTable to automate this process:
@code
class C_MyClass : public Messaging::C_MessageHandler
{
	Messaging::C_MessageDispatchTable<C_MyClass> m_MessageDispatchTable;

	C_MyClass()
	{
		m_MessageDispatchTable.Add(&C_MyClass::OnFoo);
		m_MessageDispatchTable.Add(&C_MyClass::OnBar, E_MyMessageTypes_Bar);
	}

	virtual void OnFoo(const C_FooMsg* pMessage);
	static void OnBar();

	virtual bool C_MyClass::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		if (m_MessageDispatchTable.DispatchMessage(pMessage))
			return true;

		return Messaging::C_MessageHandler::MessageHandler(pMessage);
	}
};
@endcode

One powerful feature of this mechanism is that it can handle class hierarchies
and virtual methods:
@code
class C_MyDerivedClass : public C_MyClass
{
	C_MyDerivedClass()
	{
		m_MessageDispatchTable.Add(&C_MyDerivedClass::OnWibble, E_MyMessageTypes_Wibble);
	}

	virtual void OnFoo(const C_FooMsg* pMessage); // this gets called because the base class adds it's own OnFoo to the dispatch table
	void OnWibble();

	// we don't need to override MessageHandler as everything is done through the dispatch table
};
@endcode

For code-driven cases note that we only need one dispatch table per class 
rather than one per object, so we can do things like this:
@code
class C_MyClass : public Messaging::C_MessageHandler
{
	static Messaging::C_MessageDispatchTable<C_MyClass> s_MessageDispatchTable;
	static void RegisterMessages(Messaging::C_MessageDispatchTable<C_MyClass>& messageDispatchTable)
	{
		messageDispatchTable.Add(&C_MyClass::OnFoo);
		messageDispatchTable.Add(&C_MyClass::OnBar, E_MyMessageTypes_Bar);
	}

	virtual const Messaging::C_MessageDispatchTable<C_MyClass>& GetMessageDispatchTable() const {return s_MessageDispatchTable;}

	C_MyClass()
	{
		static bool initialised = false;
		if (!initialised)
		{
			initialised = true;
			RegisterMessages(s_MessageDispatchTable);
		}
	}

	virtual void OnFoo(const C_FooMsg* pMessage);
	static void OnBar();

	virtual bool C_MyClass::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		if (GetMessageDispatchTable().DispatchMessage(pMessage))
			return true;

		return Messaging::C_MessageHandler::MessageHandler(pMessage);
	}
};

class C_MyDerivedClass : public C_MyClass
{
	static Messaging::C_MessageDispatchTable<C_MyClass> s_MessageDispatchTable;
	static void RegisterMessages(Messaging::C_MessageDispatchTable<C_MyClass>& messageDispatchTable)
	{
		C_MyClass::RegisterMessages(messageDispatchTable);
		messageDispatchTable.Add(&C_MyDerivedClass::OnWibble, E_MyMessageTypes_Wibble);
	}

	virtual const Messaging::C_MessageDispatchTable<C_MyClass>& GetMessageDispatchTable() const {return s_MessageDispatchTable;}

	C_MyDerivedClass()
	{
		static bool initialised = false;
		if (!initialised)
		{
			initialised = true;
			RegisterMessages(s_MessageDispatchTable);
		}
	}

	virtual void OnFoo(const C_FooMsg* pMessage); // this gets called because the base class adds it's own OnFoo to the dispatch table
	void OnWibble();
};
@endcode

For data-driven cases we can use arbitrary objects as handlers:
@code
class C_MyArbitraryClass
{
	void OnBar(const C_BarMsg* pMessage);
};

class C_MyClass : public Messaging::C_MessageHandler
{
	Messaging::C_MessageDispatchTable<C_MyClass> m_MessageDispatchTable;

	C_MyArbitraryClass m_ArbitraryObject;

	C_MyClass()
	{
		m_MessageDispatchTable.Add(&C_MyClass::OnFoo);
		m_MessageDispatchTable.Add(&C_MyArbitraryClass::OnBar, &m_ArbitraryObject);
	}

	virtual void OnFoo(const C_FooMsg* pMessage);

	virtual bool C_MyClass::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		if (m_MessageDispatchTable.DispatchMessage(pMessage))
			return true;

		return Messaging::C_MessageHandler::MessageHandler(pMessage);
	}
};
@endcode
*/
template <class T_MessageHandlerBase>
class C_MessageDispatchTable
{
public:
	C_MessageDispatchTable();
	~C_MessageDispatchTable();

	bool															Add(void (*pFn)(), const TMsgType messageType);
	template <class T_MessageClass>	bool							Add(void (*pFn)(const T_MessageClass*));
	template <class T_MessageHandler> bool							Add(void (T_MessageHandler::*pMemberFn)(), const TMsgType messageType);
	template <class T_MessageHandler, class T_MessageClass> bool	Add(void (T_MessageHandler::*pMemberFn)(const T_MessageClass*));
	template <class T_MessageHandler> bool							Add(void (T_MessageHandler::*pMemberFn)(), const TMsgType messageType, T_MessageHandler* pMessageHandler);
	template <class T_MessageHandler, class T_MessageClass> bool	Add(void (T_MessageHandler::*pMemberFn)(const T_MessageClass*), T_MessageHandler* pMessageHandler);

	bool Remove(const TMsgType messageType);

	bool Dispatch(T_MessageHandlerBase* pMessageHandler, const C_MessageBase* pMessage) const; ///< Dispatches the message if it has a handling function.

private:
	class I_MessageDispatchFunctor; ///< Interface for dispatching messages to functions.

	typedef Btl::Map<TMsgType, I_MessageDispatchFunctor*> C_DispatchTable; ///< Maps message types to functions.
	
	C_DispatchTable	m_DispatchTable; ///< Maps message types to functions.

	const I_MessageDispatchFunctor* Get(const TMsgType messageType) const; ///< Gets the dispatcher interface for this message type.
	bool Add(const TMsgType messageType, I_MessageDispatchFunctor* pFunctor); ///< Adds a dispatcher interface for this message type.

};

} // namespace Messaging


#include <misc_shared/messaging/MessageDispatcher.inl>

#endif // MESSAGEDISPATCHER_DAA8576B_A3EF_4338_BE71_D627B08C799F_H
