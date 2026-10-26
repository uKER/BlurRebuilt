#ifndef PROFILEINTERFACE_H
#define PROFILEINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/SystemProfileManager.h>

#define PROFILE_MAX_NAME_LENGTH		SYSTEM_PROFILE_NAME_LENGTH

#ifdef PLATFORM_XENON
#define	MAX_USERS					XUSER_MAX_COUNT		// 4
#else
#define	MAX_USERS					4
#endif

// invite modes
enum E_AcceptGameInviteMode
{
	E_AcceptGameInviteMode_Accept = 0,
	E_AcceptGameInviteMode_Delay,
	E_AcceptGameInviteMode_Discard,
};

namespace UI
{

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
	// pad profiles
	void							AllowProfileChanges( const bool bAllowProfileChanges )		{ m_bAllowProfileChanges = bAllowProfileChanges; }
	bool							AllowProfileChanges()										{ return m_bAllowProfileChanges; }
	
	SysProfile::FullProfile&		GetPadProfile( const S32 sPadNo )							{ return m_PadProfiles[ sPadNo ]; }

	bool							PadProfileActive( const S32 sPadNo )						{ return m_PadProfiles[ sPadNo ].m_State != SysProfile::eProfileStateNotActive; }
	bool							PadProfileOnline( const S32 sPadNo )						{ return m_PadProfiles[ sPadNo ].m_State == SysProfile::eProfileStateOnline; }
	bool							PadProfileOffline( const S32 sPadNo )						{ return m_PadProfiles[ sPadNo ].m_State == SysProfile::eProfileStateOffline; }
	bool							PadProfileBlagged( const S32 sPadNo )						{ return m_PadProfiles[ sPadNo ].m_State == SysProfile::eProfileStateBlagged; }

	// invites
	void							AcceptGameInviteMode( const E_AcceptGameInviteMode eMode )	{ m_eAcceptGameInviteMode = eMode; }

	void							AcceptedGameInvite( const S32 sPadNo )						{ m_bAcceptedGameInvite = true; m_sInvitePadNo = sPadNo; }
	void							ClearGameInvite()											{ m_bAcceptedGameInvite = false; m_sInvitePadNo = -1; }

	bool							HasAcceptedGameInvite()										{ return m_bAcceptedGameInvite; }
	S32								GetInvitePadNo()											{ return m_sInvitePadNo; }

	// misc
	void							AllowAllPadInput( const bool bAllowAllPadInput )			{ m_bAllowAllPadInput = bAllowAllPadInput; }
	bool							AllowAllPadInput()											{ return m_bAllowAllPadInput; }

	void							SetLockToPad( const S32 sPadNo = -1 )						{ m_sLockToPad = sPadNo; }
	S32								GetLockToPad()												{ return m_sLockToPad; }

	// saving/loading
#ifdef PLATFORM_XENON
	void							DeviceId( const S32 sPadNo, const XCONTENTDEVICEID xDeviceId )		{ m_xDeviceIds[ sPadNo ] = xDeviceId; }
	XCONTENTDEVICEID				DeviceId( const S32 sPadNo )										{ return m_xDeviceIds[ sPadNo ]; }
#endif

	// game profiles
	virtual void					GameProfileReset( const S32 sPadNo )										{ /* TODO */ }
	virtual bool					GameProfileLoad( const S32 sPadNo, U8* pBuffer, const U32 uDataSize )		{ /* TODO */ return true; }
	virtual bool					GameProfileSave( const S32 sPadNo, U8* pBuffer, const U32 uDataSize )		{ /* TODO */ return true; }

private:
	static C_ProfileInterface		m_sInstance;

private:
	bool							m_bAllowProfileChanges;
	bool							m_bAllowAllPadInput;
	S32								m_sLockToPad;

	// invites
	E_AcceptGameInviteMode			m_eAcceptGameInviteMode;
	bool							m_bAcceptedGameInvite;
	S32								m_sInvitePadNo;

	// pad profiles
	Btl::Vector< SysProfile::FullProfile, Btl::VectorParameters_Array< MAX_USERS > >	m_PadProfiles;

	// saving/loading
#ifdef PLATFORM_XENON
	Btl::Vector< XCONTENTDEVICEID, Btl::VectorParameters_Array< MAX_USERS > >			m_xDeviceIds;
#endif
};

}; //namespace UI

#endif /* PROFILEINTERFACE_H */
