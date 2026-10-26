#ifndef CROWD_MESSAGING_H
#define CROWD_MESSAGING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>
#include <misc_shared/Messaging/MessageValue.h>

namespace AnimatedCrowd
{

enum E_CrowdMsg
{
	E_CrowdMsg_None = FOURCC_HI20('CSAM'),

	E_CrowdMsg_LoadResources,

	E_CrowdMsg_End, 
	E_CrowdMsg_Begin = E_CrowdMsg_None + 1, 
	E_CrowdMsg_Count = E_CrowdMsg_End - E_CrowdMsg_Begin
};

#ifndef CFG_GOLD
extern void RegisterMessageDebugInfo();
#endif

}; // namespace AnimatedCrowd

#endif // CROWD_MESSAGING_H
