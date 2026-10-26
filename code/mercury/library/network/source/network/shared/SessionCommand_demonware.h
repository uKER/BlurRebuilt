#ifndef SESSION_COMMAND_H
#define SESSION_COMMAND_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/SystemHeaders.h>
#include <Btl/Containers/List.h>
#include <Network/DemonwareTask.h>
#include <bdCore/bdSocket/bdSecurityID.h>

namespace NetPlay
{
	// Used inside the C_Session object for holding a specific session command.
	class C_SessionCommand : public Btl::ListElementDouble<C_SessionCommand>
	{
	public:
		// List of different commands we can give to the session.
		typedef enum
		{
			eCommandNone = 0,
			eCommandCreateSession,
			eCommandAddPlayer,
			eCommandRemovePlayer,
			eCommandModifySession,
			eCommandMigrateSession,
		}eCommandType;

		C_SessionCommand() : m_Command(eCommandNone), m_Active(false)
		{};
		virtual ~C_SessionCommand()
		{};

		void Clear() {m_Command = eCommandNone; m_Active = false; NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);}
		void Destroy(void) {Clear();} // this is called then the object is removed from the linked list, required for ListElementDouble to function.

		// data required when creating a session.
		struct CreateSessionCommandData
		{
			U8	m_Temp;
		};
	
		// data required when modifying a session
		struct ModifySessionCommandData
		{
			U32		m_PublicSlotCount;
			U32		m_PrivateSlotCount;
			U32		m_SessionFlags;
			void	*m_pNotification;
			U32		m_NotificationId;
		};

		// data required when registering session for arbitration
		struct MigrateSessionCommandData
		{
			void *m_pNotification;
			U32	m_NotificationId;
			bool m_NewHost;
		};

		// data required when adding a player to the session
		struct AddPlayerCommandData
		{
			void			*m_pNotification;
			U32				m_NotificationId;
			bdOnlineUserID	m_Users[NETPLAY_SESSION_MAX_PLAYERS_ONE_OPERATION];
			U8				m_NumToAdd;
		};

		// data required when removing a player from the session
		struct RemovePlayerCommandData
		{
			void			*m_pNotification;
			U32				m_NotificationId;
			bdOnlineUserID	m_Users[NETPLAY_SESSION_MAX_PLAYERS_ONE_OPERATION];
			U8				m_NumToRemove;
		};

	public:

		// data
		eCommandType		m_Command;
		bool				m_Active;
		bdRemoteTaskRef		m_AsyncTask;

		union
		{
			CreateSessionCommandData			m_Create;
			ModifySessionCommandData			m_Modify;
			MigrateSessionCommandData			m_Migrate;
			AddPlayerCommandData				m_Add;
			RemovePlayerCommandData				m_Remove;
		}m_CommandData;
	};

	// to make things easier to read !!
	typedef Btl::List<C_SessionCommand>		C_SessionCommandList;


	// Used inside the C_Session object for holding a specific session command.
	class C_SessionDeletionCommand : public Btl::ListElementDouble<C_SessionDeletionCommand>
	{
	public:
		// List of different commands we can give to the session.
		typedef enum
		{
			eCommandNone = 0,
			eCommandDeleteSession,
		}eCommandType;

		C_SessionDeletionCommand() : m_Command(eCommandNone), m_Active(false)
		{};
		virtual ~C_SessionDeletionCommand()
		{};

		void Clear() {m_Command = eCommandNone; m_Active = false; NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);}
		void Destroy(void) {Clear();} // this is called then the object is removed from the linked list, required for ListElementDouble to function.

		// data required when deleting the session.
		struct DeleteSessionCommandData
		{
			bdUByte8 keyId[BD_SECURITY_ID_LENGTH]; // can't just use a bdSecurityKeyID as it has a constructor..
		};

	public:

		// data
		eCommandType		m_Command;
		bool				m_Active;
		bdRemoteTaskRef		m_AsyncTask;

		union
		{
			DeleteSessionCommandData	m_Delete;
		}m_CommandData;
	};

	// to make things easier to read !!
	typedef Btl::List<C_SessionDeletionCommand>		C_SessionDeletionCommandList;
}


#endif
