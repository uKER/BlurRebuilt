namespace Messaging
{

/**
We need a virtual method to handle the dispatch, not only to perform any 
necessary type conversions but also because the physical size and layout of
function pointers differs between the various types of handling function that
we want to support, and the syntax for invoking these functions is at best a
little baroque.
*/
template <class T_MessageHandlerBase>
class C_MessageDispatchTable<T_MessageHandlerBase>::I_MessageDispatchFunctor
{
public:
	virtual ~I_MessageDispatchFunctor() {}
	virtual void DispatchMessage(T_MessageHandlerBase* pMessageHandler, const C_MessageBase* pMessage) const = 0; ///< Called to dispatch a message.
};

template <class T_MessageHandlerBase>
C_MessageDispatchTable<T_MessageHandlerBase>::C_MessageDispatchTable()
{
	// intentionally blank
}

template <class T_MessageHandlerBase>
C_MessageDispatchTable<T_MessageHandlerBase>::~C_MessageDispatchTable()
{
	// delete all the dispatcher functors
	for (typename C_DispatchTable::Iterator it = m_DispatchTable.Begin(); it != m_DispatchTable.End(); ++it)
		delete it->GetValue();
}

/**
@param messageType type of message to get the dispatcher for.
@returns NULL if no dispatcher was added for this message type.
*/
template <class T_MessageHandlerBase>
const typename C_MessageDispatchTable<T_MessageHandlerBase>::I_MessageDispatchFunctor* C_MessageDispatchTable<T_MessageHandlerBase>::Get(const TMsgType messageType) const
{
	typename C_DispatchTable::ConstIterator it = m_DispatchTable.Find(messageType);
	return (it == m_DispatchTable.End()) ? NULL : it->GetValue();
}

/**
@param messageType type of message to add a dispatcher for.
@param pFunctor dispatcher to invoke for this message type.
@returns false if the message type already has a dispatcher.
*/
template <class T_MessageHandlerBase>
bool C_MessageDispatchTable<T_MessageHandlerBase>::Add(const TMsgType messageType, I_MessageDispatchFunctor* pFunctor)
{
	if (Get(messageType) != NULL)
	{
		delete pFunctor;
		return false;
	}

	m_DispatchTable[messageType] = pFunctor;
	return true;
}

/**
@brief Adds a dispatcher for a funtion with no parameters.
Example usage:
@code
void OnFoo();

class C_MyClass
{
public:
	static void OnBar();
};

messageDispatchTable.Add(&OnFoo, E_MyMessageTypes_Foo);
messageDispatchTable.Add(&C_MyClass::OnBar, E_MyMessageTypes_Bar);
@endcode
@param pFn function to invoke.
@param messageType type of message to add a dispatcher for.
@returns false if the message type already has a dispatcher.
*/
template <class T_MessageHandlerBase>
bool C_MessageDispatchTable<T_MessageHandlerBase>::Add(void (*pFn)(), const TMsgType messageType)
{
	class C_MessageDispatchFunctor : public I_MessageDispatchFunctor
	{
	public:
		typedef void (*F_Fn)();
		F_Fn m_pFn;
		C_MessageDispatchFunctor(F_Fn pFn) : m_pFn(pFn) {}
		virtual void DispatchMessage(T_MessageHandlerBase*, const C_MessageBase*) const {(m_pFn)();}
	};

	return Add(messageType, new C_MessageDispatchFunctor(pFn));
}

/**
@brief Adds a dispatcher for a function with a message parameter.
Example usage:
@code
void OnFoo(const C_FooMsg* pMessage);

class C_MyClass
{
public:
	static void OnBar(const C_BarMsg* pMessage);
};

messageDispatchTable.Add(&OnFoo);
messageDispatchTable.Add(&C_MyClass::OnBar);
@endcode
@param pFn function to invoke.
@param messageType type of message to add a dispatcher for.
@returns false if the message type already has a dispatcher.
*/
template <class T_MessageHandlerBase>
template <class T_MessageClass>
bool C_MessageDispatchTable<T_MessageHandlerBase>::Add(void (*pFn)(const T_MessageClass*))
{
	class C_MessageDispatchFunctor : public I_MessageDispatchFunctor
	{
	public:
		typedef void (*F_Fn)(const T_MessageClass* pMessage);
		F_Fn m_pFn;
		C_MessageDispatchFunctor(F_Fn pFn) : m_pFn(pFn) {}
		virtual void DispatchMessage(T_MessageHandlerBase*, const C_MessageBase* pMessage) const {(m_pFn)(static_cast<const T_MessageClass*>(pMessage));}
	};

	return Add(T_MessageClass::StaticType(), new C_MessageDispatchFunctor(pFn));
}

/**
@brief Adds a dispatcher for a member funtion with no parameters.
Example usage:
@code
class C_MyClass
{
public:
	void OnFoo();
	virtual void OnBar();
};

messageDispatchTable.Add(&C_MyClass::OnFoo, E_MyMessageTypes_Foo);
messageDispatchTable.Add(&C_MyClass::OnBar, E_MyMessageTypes_Bar);
@endcode
@param pFn function to invoke.
@param messageType type of message to add a dispatcher for.
@returns false if the message type already has a dispatcher.
*/
template <class T_MessageHandlerBase>
template <class T_MessageHandler>
bool C_MessageDispatchTable<T_MessageHandlerBase>::Add(void (T_MessageHandler::*pMemberFn)(), const TMsgType messageType)
{
	class C_MessageDispatchFunctor : public I_MessageDispatchFunctor
	{
	public:
		typedef void (T_MessageHandler::*F_Fn)();
		F_Fn m_pFn;
		C_MessageDispatchFunctor(F_Fn pFn) : m_pFn(pFn) {}
		virtual void DispatchMessage(T_MessageHandlerBase* pMessageHandler, const C_MessageBase*) const {(static_cast<T_MessageHandler*>(pMessageHandler)->*m_pFn)();}
	};

	return Add(messageType, new C_MessageDispatchFunctor(pMemberFn));
}

/**
@brief Adds a dispatcher for a member funtion with a message parameter.
Example usage:
@code
class C_MyClass
{
public:
	void OnFoo(const C_FooMsg* pMessage);
	virtual void OnBar(const C_BarMsg* pMessage);
};

messageDispatchTable.Add(&C_MyClass::OnFoo);
messageDispatchTable.Add(&C_MyClass::OnBar);
@endcode
@param pFn function to invoke.
@param messageType type of message to add a dispatcher for.
@returns false if the message type already has a dispatcher.
*/
template <class T_MessageHandlerBase>
template <class T_MessageHandler, class T_MessageClass>
bool C_MessageDispatchTable<T_MessageHandlerBase>::Add(void (T_MessageHandler::*pMemberFn)(const T_MessageClass*))
{
	class C_MessageDispatchFunctor : public I_MessageDispatchFunctor
	{
	public:
		typedef void (T_MessageHandler::*F_Fn)(const T_MessageClass* pMessage);
		F_Fn m_pFn;
		C_MessageDispatchFunctor(F_Fn pFn) : m_pFn(pFn) {}
		virtual void DispatchMessage(T_MessageHandlerBase* pMessageHandler, const C_MessageBase* pMessage) const {(static_cast<T_MessageHandler*>(pMessageHandler)->*m_pFn)(static_cast<const T_MessageClass*>(pMessage));}
	};

	return Add(T_MessageClass::StaticType(), new C_MessageDispatchFunctor(pMemberFn));
}

/**
@brief Adds a dispatcher for an object with member funtion taking no parameters.
Example usage:
@code
class C_MyClass
{
public:
	void OnFoo();
	virtual void OnBar();
};

C_MyClass myObj;

messageDispatchTable.Add(&C_MyClass::OnFoo, E_MyMessageTypes_Foo, &myObj);
messageDispatchTable.Add(&C_MyClass::OnBar, E_MyMessageTypes_Bar, &myObj);
@endcode
@param pFn function to invoke.
@param messageType type of message to add a dispatcher for.
@returns false if the message type already has a dispatcher.
*/
template <class T_MessageHandlerBase>
template <class T_MessageHandler> 
bool C_MessageDispatchTable<T_MessageHandlerBase>::Add(void (T_MessageHandler::*pMemberFn)(), const TMsgType messageType, T_MessageHandler* pMessageHandler)
{
	class C_MessageDispatchFunctor : public I_MessageDispatchFunctor
	{
	public:
		typedef void (T_MessageHandler::*F_Fn)();
		F_Fn m_pFn;
		T_MessageHandler* m_pMessageHandler;
		C_MessageDispatchFunctor(F_Fn pFn, T_MessageHandler* pMessageHandler) : m_pFn(pFn), m_pMessageHandler(pMessageHandler) {}
		virtual void DispatchMessage(T_MessageHandlerBase*, const C_MessageBase*) const {(m_pMessageHandler->*m_pFn)();}
	};

	return Add(messageType, new C_MessageDispatchFunctor(pMemberFn));
}

/**
@brief Adds a dispatcher for an object with a member funtion taking a message parameter.
Example usage:
@code
class C_MyClass
{
public:
	void OnFoo(const C_FooMsg* pMessage);
	virtual void OnBar(const C_BarMsg* pMessage);
};

C_MyClass myObj;

messageDispatchTable.Add(&C_MyClass::OnFoo, &myObj);
messageDispatchTable.Add(&C_MyClass::OnBar, &myObj);
@endcode
@param pFn function to invoke.
@param messageType type of message to add a dispatcher for.
@returns false if the message type already has a dispatcher.
*/
template <class T_MessageHandlerBase>
template <class T_MessageHandler, class T_MessageClass> 
bool C_MessageDispatchTable<T_MessageHandlerBase>::Add(void (T_MessageHandler::*pMemberFn)(const T_MessageClass*), T_MessageHandler* pMessageHandler)
{
	class C_MessageDispatchFunctor : public I_MessageDispatchFunctor
	{
	public:
		typedef void (T_MessageHandler::*F_Fn)(const T_MessageClass* pMessage);
		F_Fn m_pFn;
		T_MessageHandler* m_pMessageHandler;
		C_MessageDispatchFunctor(F_Fn pFn, T_MessageHandler* pMessageHandler) : m_pFn(pFn), m_pMessageHandler(pMessageHandler) {}
		virtual void DispatchMessage(T_MessageHandlerBase*, const C_MessageBase* pMessage) const {(m_pMessageHandler->*m_pFn)(static_cast<const T_MessageClass*>(pMessage));}
	};

	return Add(T_MessageClass::StaticType(), new C_MessageDispatchFunctor(pMemberFn));
}

/**
@brief Removes the handler for this message type.
@param messageType type of message to remove handler for.
@returns false if there was no handler for the message type.
*/
template <class T_MessageHandlerBase>
bool C_MessageDispatchTable<T_MessageHandlerBase>::Remove(const TMsgType messageType)
{
	typename C_DispatchTable::Iterator it = m_DispatchTable.Find(messageType);
	
	if (it != m_DispatchTable.End())
	{
		delete it->GetValue();
		m_DispatchTable.Erase(it);
		return true;
	}

	return false;
}

/**
If a handler function has been added for the message type then it will be 
invoked.
@param pMessageHandler object handling the message.
@param pMessage message to handle.
@returns false if no function was added for the message type.
*/
template <class T_MessageHandlerBase>
bool C_MessageDispatchTable<T_MessageHandlerBase>::Dispatch(T_MessageHandlerBase* pMessageHandler, const C_MessageBase* pMessage) const
{
	if (pMessage != NULL)
	{
		const I_MessageDispatchFunctor* pFunctor = Get(pMessage->Type());
		if (pFunctor != NULL)
		{
			pFunctor->DispatchMessage(pMessageHandler, pMessage);
			return true;
		}
	}

	return false;
}

} // namespace Messaging
