#ifndef _GAME_STATE
#define _GAME_STATE

#include "netEntity/entityState.h"
#include "acksystem.h"
#include "utils/gameStateDataAnalyser.h"
#include "netEnums/netEnums.h"
#include <system/debug/I_DebugRender.h>
#include "netstate/StateEnums.h"

// game state
class C_GameState : public C_AckSystem
{
public:
	class C_GameStateHeader
	{
	public:
		C_GameStateHeader() : m_uTheirStateSQN(0), m_uMyStateAck(0), m_eGameStateMode(0) {;}
		C_GameStateHeader(int uTheirSQN, int uMyStateAck, char eGameStateMode) 
			: m_uTheirStateSQN(uTheirSQN)
			, m_uMyStateAck(uMyStateAck)
			, m_eGameStateMode(eGameStateMode)
		{;}
		bool	Serialise(class netStream* stream) const;
		bool	Deserialise(class netStream* stream);
	public:
		int		m_uTheirStateSQN;
		int		m_uMyStateAck;
		char	m_eGameStateMode;
	};
public:
	class C_NetState
	{
	public:
		C_NetState();
		C_EntityState*	EntityState();
		bool			EntityState(C_EntityState* pxEntityState);
		void			CreateMigrationStates(int uNumClients);
		void			DestroyMigrationStates();
		void			SetClientMigrationStatus(int uClientId, int uStatus, bool bServer);
		void			SetMigrationStatus(int uClientId, int uStatus, bool bServer);
		int				GetMigrationStatus(int uClientId);
		void			DestroyNetState();
		bool			IsActive();
		bool			StateSerialised(const C_Header& xHeader);
		bool			EntityValidityCheck(const C_Header& xHeader, const short int uEntitySize);
		bool			Update();
		
		// NEW
		void			SetMessageHeaderIdentifiction(C_Header& xMessageHeader);
		bool			SetMessageHeaderDataPackFlags(C_Header& xMessageHeader, int uClientId, class C_GameState& xGameState);
		bool			SetMessageHeaderMigrationFlags(C_Header& xMessageHeader, int uClientId, class C_GameState& xGameState);
		bool			PackMessage(C_Header& xMessageHeader, int uOutboundAck, class netStream* stream);
		bool			HandleCreationMsg(C_Header& xMessageHeader, class C_GameState& xGameState);
		bool			HandleDestructionMsg(C_Header& xMessageHeader, class C_GameState& xGameState);
		bool			HandleMigrationMsg(C_Header& xMessageHeader, class C_GameState& xGameState, int uClientId);
		bool			HandleUnPackMsg(C_Header& xHeader, class C_GameState& xGameState, class netStream* stream);
		
		bool			SkipState(int uSize, class netStream* stream);
	private:
		C_EntityState*	m_pxEntityState;
		int				m_uStateNumber;
		char			m_uActive;
		bool			m_bCopyState;		// copy state to game-side entity
		int*			m_uMigrationStatus;
	};
public:
					C_GameState(const int uMyGameStateId, const int uMyHostId, const int uNumEntities, const int uNumClients);
					~C_GameState();
	bool			ServerId(int uServerId);
	int				ServerId()		const					{ return m_uServerId; }
	int				MyHostId()		const					{ return m_uMyHostId; }
	int				GameStateId()	const					{ return m_uGameStateId; }
	int				SequenceNumber()const					{ return m_uSequenceNumber; }
	void			SequenceNumber(int uSqn)				{ m_uSequenceNumber = uSqn; }
	bool			IsServer()		const					{ return m_uMyHostId == m_uServerId; }
	int				NumEntities()	const					{ return m_uNumEntities; }
	bool			Synchronised() const					{ return C_AckSystem::OldestOutboundState() == m_uSequenceNumber || C_AckSystem::ActiveClients() == 0; }
	bool			NetworkTickInThisFrame() const			{ return m_bNetworkTickInThisFrame; }
	int				EntityAuthority(const int uEntityId);
	bool			IsAuthoritative(int uEntityId) const;
	int				CurrentGameState();

					// state accessors
	bool			EntityState(int uEntityId, C_EntityState* pxEntityState);
	C_EntityState*	EntityState(int uEntityId) const;
	C_NetState*		NetState(int uEntityId) const;

					// net state creation
	int				GetFreeNetId(int uRequestedId);
	bool			CreateNetState(C_Header& xStateInfo, class netSerialisable* pxNetEntity, int uSize);
	bool			RequestDeleteClientsEntities(int uClientId);
	bool			RequestDeleteNetState(int uEntityId);
	bool			RequestDeleteNetStates();
	
					// serialisation
	bool			StateToPacket(const int uClientId, class netStream* stream);
	bool			PacketToState(const int uClientId, class netStream* stream);
	void			Update(bool bNetworkFrame);
	void			Render(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h);
	void			RenderModeGameState();

	bool			Migrating()	const						{ return m_eGameStateMode >= E_Mode_Migration_First && m_eGameStateMode < E_Mode_Migration_Last; }
	bool			MigrationActive(int uClientId) const;
	int				GetGameStateMode() const				{ return m_eGameStateMode; }


private:
	void			MigrateServer(int uNewServerId, int uOldServerId);
	void			UpdateMigrationStages();
	void			GameStateMode(E_GameState_Mode eMode);
	bool			EntityValidityCheck(const C_Header& xHeader1, const C_Header& xHeader2);
	void			UpdateAcknowledgements();

private:
	// misalignmment debugging
	bool			DebugHeader(class netStream* stream, int streamStart);
	void			DebugFooter(C_Header& xHeader, class netStream* stream, int streamStart);

private:
	C_DataAnalyser		m_xDataAnalyser;
	E_GameState_Mode	m_eGameStateMode;
	const int			m_uGameStateId;
	const int			m_uMyHostId;
	const int			m_uNumEntities;
	C_NetState*			m_pxNetState;
	int					m_uSequenceNumber;
	int					m_uServerId;
	bool				m_bDeletionInThisFrame;
	bool				m_bNetworkTickInThisFrame;

public:
	static int	g_sCurrentWorkingState;
};

#endif