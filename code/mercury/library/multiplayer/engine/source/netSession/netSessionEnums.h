#ifndef NETSESSIONENUMS_H
#define NETSESSIONENUMS_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netEnums/netEnums.h"
#include "netStream/netSerialise.h"
#include "netStream/netVector.h"
#include <network/secureKey.h>
#include <network/session.h>
#include <network/sessionParameter.h>

namespace NetPlay
{
	typedef NetPlay::C_Session::SlotConfig SessionSlotConfig;

	typedef C_Session::SessionUserList SessionUserList;

	typedef Btl::Vector<SessionParameter, Btl::VectorParameters_Static<32> > SessionParameterList;
}

#endif //NETSESSIONENUMS_H