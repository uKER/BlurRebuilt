#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Game/Gadgets/ButtonMessaging.h"
#include "ui/Game/UILua.h"

namespace UI
{

// Type for a pointer to a Button Creator function
typedef C_GadgetBase* (*CreateButtonFunctionPtr)();

// Template for to create a button of registered type
template< class T >C_GadgetBase* CreateNewButton() 
{   
    return new T;     
}

// Container for creating buttons
class C_ButtonCreatorItem
{
public:
	C_ButtonCreatorItem( ) : m_Creator(NULL) { }

	CreateButtonFunctionPtr	m_Creator;
	Util::C_String32		m_Name;
};

class C_ButtonController
{
public:
	C_ButtonController();
	~C_ButtonController();

	const U32		NumberOfGadgets() const; // returns the number of gadgets across all contexts

	// Button creation
	void			InitCreator( const U32 maxGadgets, const bool createDefaults = true );
	void			ExitCreator( );
	void			CreateLuaEnums( lua_State* pState );

	C_GadgetBase*	AddButton(const E_UIButtonType type, const S32 context = -1 );
	C_GadgetBase*	Find( const U32 ID );
	C_GadgetBase*	GetCurrentObject( const S32 context = -1 );
	C_GadgetBase*	FindByName( const char *pName );

	// Messages
	Messaging::C_MessageSubscriberSystem&	GetSubscriberSystem(void)	{return *m_pMessageSubscriberSystem;}

	void									BroadcastMessage(const Messaging::C_MessageBase* pMessage, U32 mask = 0xffffffff);
	void									ProcessDeferredMessages();
	
	void									BroadcastMessageNow( const U32 mask, const Messaging::C_MessageBase* pMessage );
	void									SendMessageNow( const C_ButtonHandle& pHandle, const Messaging::C_MessageBase* pMessage );

	template<class T_Button > void Register()
    {
		UIButtonTypeID		id	= T_Button::TypeID();

		if(m_ButtonCreator[ id ].m_Creator == NULL)
		{
			m_ButtonCreator[ id ].m_Creator	= (CreateButtonFunctionPtr)CreateNewButton<T_Button>;
			m_ButtonCreator[ id ].m_Name		= T_Button::TypeName();
		}
		else
		{
			LOGFATAL( g_Log,  "%s is already registered\n",T_Button::TypeName().Chars());
		}
    }

	C_GadgetBase *CreateButton( const UIButtonTypeID id )
	{
		if(id >= m_ButtonCreator.Capacity())
		{
			LOGFATAL( g_Log,  "%d number %d is an invalid type\n",id);
			return NULL;
		}
		if(m_ButtonCreator[ id ].m_Creator == NULL)
		{
			LOGFATAL( g_Log,  "%d has not been registered\n",id);
			return NULL;
		}

		return m_ButtonCreator[ id ].m_Creator();
	}
private:


private:
	MT::Lock									m_CriticalSecion;
	
	E_UIButtonType								m_CurrentType;
	C_GadgetBase *								m_CurrentObject;
	int											m_CurrentID;

	Btl::Vector< C_ButtonCreatorItem >			m_ButtonCreator;
//	C_List										m_ButtonList[ E_ButtonList_Num ];

	Messaging::C_MessageSubscriberSystem *		m_pMessageSubscriberSystem;	///< handles message subscriptions for actors and components

	C_DeferredUiMessageQueue *					m_pCurrentDeferredMessageQueue;
	C_DeferredUiMessageQueue					m_DeferredMessageQueue1;
	C_DeferredUiMessageQueue					m_DeferredMessageQueue2;
};

// Extern for singleton (old school style)
extern C_ButtonController gButtons;

}; // namespace UI

#endif
