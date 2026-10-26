#ifndef PROFILEINTERFACE_H
#define PROFILEINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/SystemProfileManager.h>

#define PROFILE_MAX_NAME_LENGTH		SYSTEM_PROFILE_NAME_LENGTH

#define	MAX_USERS					4

namespace UI
{

class I_GamerPictureManager;

// invite modes
enum E_AcceptGameInviteMode
{
	E_AcceptGameInviteMode_Accept = 0,
	E_AcceptGameInviteMode_Delay,
	E_AcceptGameInviteMode_Discard,
};

// ------------------
// C_ProfileInterface
// ------------------
class C_ProfileInterface
{
public:
	C_ProfileInterface();
	~C_ProfileInterface();

public:
	static C_ProfileInterface&		Instance()	{ return m_sInstance; }

public:
	void							Initialise();

	// pad profiles
	void							Flush();
	U32								NumActive();
	U32								Max() const													{ return MAX_USERS; }

	bool							AllowPadInput( const S32 sPadNo );

	// profiles
	void							AllowProfileChanges( const bool bAllowProfileChanges )		{ m_bAllowProfileChanges = bAllowProfileChanges; }
	bool							AllowProfileChanges()										{ return m_bAllowProfileChanges; }

	void							ActOnProfileChanges( const bool bActOnProfileChanges );
	bool							ActOnProfileChanges()										{ return m_bActOnProfileChanges; }

	void							ForceProfileUpdate()										{ m_bForceUpdateProfiles = true; Update(); }
	
	SysProfile::FullProfile&		GetPadProfile( const S32 sPadNo );
	S32								GetPadIndexFromID( const SysProfile::UserIdentifier sUserID ) const;

	bool							PadProfileActive( const S32 sPadNo );
	bool							PadProfileOnline( const S32 sPadNo );
	bool							PadProfileOffline( const S32 sPadNo );
	bool							PadProfileBlagged( const S32 sPadNo );

	// invites
	void							AcceptGameInviteMode( const E_AcceptGameInviteMode eMode )	{ m_eAcceptGameInviteMode = eMode; }
	E_AcceptGameInviteMode			AcceptGameInviteMode()										{ return m_eAcceptGameInviteMode; }

	// misc
	void							DisconnectFromXLSP()										{ m_bHasLostConnectionXLSP = true; }
	void							DisconnectFromLobby()										{ m_bHasLostConnectionLobby = true; }

	void							AllowAllPadInput( const bool bAllowAllPadInput )			{ m_bAllowAllPadInput = bAllowAllPadInput; }
	bool							AllowAllPadInput()											{ return m_bAllowAllPadInput; }

	void							SetLockToPad( const S32 sPadNo = -1 )						{ m_sLockToPad = sPadNo; }
	S32								GetLockToPad()												{ return m_sLockToPad; }
	void							Update();

	// Gamer picture helper functions
	void							GetLocalGamerPicture( const S32 sPadNo );
	void							ClearLocalGamerPicture( const S32 sPadNo );
	I_GamerPictureManager*			GamerPictureManager()										{ return m_pGamerPictureManager; }

	// saving/loading

private:
	static C_ProfileInterface		m_sInstance;

private:
	bool							m_bForceUpdateProfiles;

	// profiles
	bool							m_bAllowProfileChanges;
	bool							m_bActOnProfileChanges;

	// temps for storing actions when disabled (loading screens)
	bool							m_bHasLostConnection;
	bool							m_bHasLostConnectionXLSP;
	bool							m_bHasLostConnectionLobby;
	bool							m_bNetworkCableRemoved;
	Btl::Vector< S32 >				m_SignedInPads;
	Btl::Vector< S32 >				m_SignedOutPads;

	// pads
	bool							m_bAllowAllPadInput;
	S32								m_sLockToPad;

	// invites
	E_AcceptGameInviteMode			m_eAcceptGameInviteMode;

	// pad profiles
	Btl::Vector< SysProfile::FullProfile, Btl::VectorParameters_Array< MAX_USERS > >	m_PadProfiles;

	// saving/loading

	I_GamerPictureManager*			m_pGamerPictureManager;
};

}; //namespace UI

#endif /* PROFILEINTERFACE_H */
