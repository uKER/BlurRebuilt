#ifndef NETINVITE_H
#define NETINVITE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netEnums/netEnums.h"
#include "netStream/netSerialise.h"

#include <Network/SecureKey.h>
#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/GameInvite.h>
#include <SystemProfile/SystemProfileManager.h>

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>

BD_REFERENCE(bdGameInfo);
/////////////////////////////////////////////////////////////////////////////
// Class       : netInvite
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 18 March 2008
// Description : invite received
/////////////////////////////////////////////////////////////////////////////
BD_REFERENCE(netInvite);
class netInvite: public SysProfile::GameInvite, public bdReferencable
{
public:
	netInvite(const SysProfile::GameInvite& inviteData);
	
	int debugstring(char* buff, int len);
	
	bdGameInfoRef getGameInfo();
};

#endif //NETINVITE_H