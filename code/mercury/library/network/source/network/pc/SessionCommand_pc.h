#ifndef SESSION_COMMAND_H
#define SESSION_COMMAND_H

#ifdef _MSC_VER
#pragma once
#endif

// This command is only applicable when using games for windows live
// if using demonware for pc then we will use SessionCommand_demonware.h
#if defined (LIVE_ON_WINDOWS)

#include "Btl/Containers/List.h"
#include "AsyncTask_pc.h"

namespace NetPlay
{
	/// Used inside the C_Session object for holding a specific session command.
	class C_SessionCommand : public Btl::ListElementDouble<C_SessionCommand>
	{
	public:
		/// List of different commands we can give to the session.
		typedef enum
		{
			eCommandNone = 0,
			eCommandCreateSession,
			eCommandAddPlayer,
			eCommandRemovePlayer,
			eCommandModifySession,
			eCommandArbitrationRegistration,
			eCommandMigrateSession,
			eCommandModifySkill,
			eCommandStartSession,
			eCommandEndSession,
		}eCommandType;

		C_SessionCommand() : m_Command(eCommandNone), m_Active(false)
		{};
		virtual ~C_SessionCommand()
		{};

		void Clear() 
		{
			m_Command = eCommandNone; 
			m_Active = false; 
			m_AsyncTask.Close();
		}
		void Destroy(void) {Clear();} // this is called then the object is removed from the linked list, required for ListElementDouble to function.

		// data required when creating a session.
		struct CreateSessionCommandData
		{
			U8	m_Temp;
		};

		// data required when adding a player to the session
		struct AddPlayerCommandData
		{
			XUID	m_Xuids[NETPLAY_SESSION_MAX_PLAYERS_ONE_OPERATION];
			BOOL	m_PrivateSlots[NETPLAY_SESSION_MAX_PLAYERS_ONE_OPERATION];
			U8		m_NumToAdd;
		};

		// data required when removing a player from the session
		struct RemovePlayerCommandData
		{
			XUID	m_Xuids[NETPLAY_SESSION_MAX_PLAYERS_ONE_OPERATION];
			U8		m_NumToRemove;
		};
		
		/// data required when modifying a session
		struct ModifySessionCommandData
		{
			U32		m_PublicSlotCount;
			U32		m_PrivateSlotCount;
			U32		m_SessionFlags;
			void	*m_pNotification;
			U32		m_NotificationId;
		};

		// data required when registering session for arbitration
		struct ArbitrationRegistrationCommandData
		{
			U64		m_ArbitrationNonce;
			void	*m_pNotification;
			U32		m_NotificationId;
		};

		// data required when registering session for arbitration
		struct ModifySkillCommandData
		{
			void *m_pNotification;
			U32	m_NotificationId;
			U32 m_NumPlayers;
		};

		// data required when registering session for arbitration
		struct MigrateSessionCommandData
		{
			void *m_pNotification;
			U32	m_NotificationId;
			bool m_NewHost;
		};

		// data required when starting the session
		struct StartSessionCommandData
		{
			U8 m_Temp;
		};

		// data required when ending the session.
		struct EndSessionCommandData
		{
			U8 m_Temp;
		};
		
	public:

		// data
		C_AsyncTask			m_AsyncTask;
		eCommandType		m_Command;
		bool				m_Active;
		union
		{
			CreateSessionCommandData			m_Create;
			AddPlayerCommandData				m_Add;
			RemovePlayerCommandData				m_Remove;
			ModifySessionCommandData			m_Modify;
			ArbitrationRegistrationCommandData	m_ArbitrationRegistration;
			ModifySkillCommandData				m_ModifySkill;
			MigrateSessionCommandData			m_Migrate;
			StartSessionCommandData				m_Start;		
			EndSessionCommandData				m_End;
		}m_CommandData;
	};

	// to make things easier to read !!
	typedef Btl::List<C_SessionCommand>		C_SessionCommandList;



	/// Used inside the C_Session object for holding a specific session command.
	class C_SessionDeletionCommand : public Btl::ListElementDouble<C_SessionDeletionCommand>
	{
	public:
		/// List of different commands we can give to the session.
		typedef enum
		{
			eCommandDeleteSession,
		}eCommandType;

		C_SessionDeletionCommand() : m_Command(eCommandNone), m_Active(false)
		{};
		virtual ~C_SessionDeletionCommand()
		{};

		void Clear() 
		{
			m_Command = eCommandNone; 
			m_Active = false; 
			m_AsyncTask.Close();
		}
		void Destroy(void) {Clear();} // this is called then the object is removed from the linked list, required for ListElementDouble to function.

		// data required when deleting the session.
		struct DeleteSessionCommandData
		{
			HANDLE m_SessionHandle;	
		};

	public:

		// data
		C_AsyncTask			m_AsyncTask;
		eCommandType		m_Command;
		bool				m_Active;
		union
		{
			DeleteSessionCommandData	m_Delete;
		}m_CommandData;
	};

	// to make things easier to read !!
	typedef Btl::List<C_SessionDeletionCommand>		C_SessionDeletionCommandList;
}

#endif // LIVE_ON_WINDOWS

#endif