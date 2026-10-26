#include "Stdafx.h"

#include "ProfileInterface.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// global instance
C_ProfileInterface C_ProfileInterface::m_sInstance;

// ------------------
// C_ProfileInterface
// ------------------
C_ProfileInterface::C_ProfileInterface()
	: m_bAllowProfileChanges( true ),
	m_eAcceptGameInviteMode( E_AcceptGameInviteMode_Delay ),
	m_bAcceptedGameInvite( false ),
	m_sInvitePadNo( -1 ),
	m_bAllowAllPadInput( true ),
	m_sLockToPad( -1 )
{
#ifdef PLATFORM_XENON
	// reset devices id's
	for ( S32 i = 0; i < m_xDeviceIds.Size(); i++ )
		m_xDeviceIds[ i ] = XCONTENTDEVICE_ANY;
#endif
}

C_ProfileInterface::~C_ProfileInterface()
{
	//
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
