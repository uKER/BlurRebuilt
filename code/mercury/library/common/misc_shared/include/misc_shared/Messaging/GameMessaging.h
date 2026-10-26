#ifndef GAME_MESSAGES_H
#define GAME_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/Messaging/MessageValue.h>
#include <misc_shared/UID/FourCC.h>
#include <system/Debug/I_DebugRender.h>

class C_ContextualStream;


namespace GameMessaging
{

/**
@brief Basic message types.
Provides values for Messaging::TMsgType that should not clash with user-
defined values.
*/
enum E_GameMsg
{
	E_GameMsg_None = FOURCC_HI20('MSGM'),	///< NULL identifier, should never be returned from C_MessageBase::Type().

	E_GameMsg_Update,			///< @see C_UpdateMsg.
	E_GameMsg_PreUpdate,		///< @see C_UpdateMsg.
	E_GameMsg_PostUpdate,		///< @see C_UpdateMsg.
	E_GameMsg_FinalUpdate,		///< @see C_UpdateMsg.
	E_GameMsg_UpdateEnable,		///< Enable response to E_GameMsg_Update.
	E_GameMsg_UpdateDisable,	///< Disable response to E_GameMsg_Update.
	E_GameMsg_ClientUpdate,		///< Update for client state
	E_GameMsg_PausedUpdate,		///< Update sent when game is paused

	E_GameMsg_ChangeState,		///< @see C_ChangeStateMsg.

	E_GameMsg_DebugRenderOn,	///< Enable response to E_GameMsg_DebugRenderSpecific.
	E_GameMsg_DebugRenderOff,	///< Disable response to E_GameMsg_DebugRenderSpecific.
	E_GameMsg_DebugRenderSpecific,
	E_GameMsg_DebugBreak,		///< Debug break type exception

	// if you add more message types, please update GameMessaging::RegisterMessageDebugInfo() also

	E_GameMsg_End, 
	E_GameMsg_Begin = E_GameMsg_None + 1, 
	E_GameMsg_Count = E_GameMsg_End - E_GameMsg_Begin
};


/**
@brief Generic update message.
Tells the handler to update for the specified time step.
@see E_GameMsg_UpdateEnable E_GameMsg_UpdateDisable.
*/
typedef Messaging::C_MessageValue<F32, E_GameMsg_PreUpdate> C_PreUpdateMsg;
typedef Messaging::C_MessageValue<F32, E_GameMsg_Update> C_UpdateMsg;
typedef Messaging::C_MessageValue<F32, E_GameMsg_PostUpdate> C_PostUpdateMsg;
typedef Messaging::C_MessageValue<F32, E_GameMsg_FinalUpdate> C_FinalUpdateMsg;
typedef Messaging::C_MessageValue<F32, E_GameMsg_ClientUpdate> C_ClientUpdateMsg;
typedef Messaging::C_MessageValue<F32, E_GameMsg_PausedUpdate> C_PausedUpdateMsg;

/**
@brief Generic state change message.
The interpretation of the state is user-defined, so this message should be
passed between code that understands the state context.
*/
typedef Messaging::C_MessageValue<S32, E_GameMsg_ChangeState> C_ChangeStateMsg;


class C_DebugRenderSpecificMsg : public Messaging::C_MessageBase
{
public:
	C_DebugRenderSpecificMsg() : Messaging::C_MessageBase(E_GameMsg_DebugRenderSpecific, sizeof(C_DebugRenderSpecificMsg)) {}
	C_DebugRenderSpecificMsg(DbUtil::I_DebugRenderPtr pDebugRender) : Messaging::C_MessageBase(E_GameMsg_DebugRenderSpecific, sizeof(C_DebugRenderSpecificMsg)), m_pDebugRender(pDebugRender) {}

	static Messaging::TMsgType	StaticType() {return E_GameMsg_DebugRenderSpecific;}

	DbUtil::I_DebugRenderPtr	GetValue() const {return m_pDebugRender;}
	void						SetValue(DbUtil::I_DebugRenderPtr pDebugRender) {m_pDebugRender = pDebugRender;}

private:
	DbUtil::I_DebugRenderPtr	m_pDebugRender;

};

#ifndef CFG_GOLD

/// Registers debug info for message types in E_GameMsg
extern void RegisterMessageDebugInfo();

#endif // ndef CFG_SPEED

} // namespace GameMessaging


#endif
