#ifndef MODELCACHEMESSAGES_A2AD03AE_0960_488E_8E85_E7DC2D9F5EFC_H
#define MODELCACHEMESSAGES_A2AD03AE_0960_488E_8E85_E7DC2D9F5EFC_H

#ifdef _MSC_VER
#pragma once
#endif


#include <misc_shared/Messaging/MessageValue.h>
#include <misc_shared/UID/FourCC.h>
#include <xt/XtGame.h>


namespace BasePhysicalObject
{

enum E_ModelCacheMsg
{
	E_ModelCacheMsg_HiBits = FOURCC_HI20('MDCH'), 

	E_ModelCacheMsg_Invalidated, ///< @see C_ModelCacheInvalidatedMsg.

	E_ModelCacheMsg_End, 
	E_ModelCacheMsg_Begin = E_ModelCacheMsg_HiBits + 1, 
	E_ModelCacheMsg_Count = E_ModelCacheMsg_End - E_ModelCacheMsg_Begin
};

/// GetValue() gives the id of the Xt record for which the C_ElementDef data has been invalidated.
typedef Messaging::C_MessageValue<XtId, E_ModelCacheMsg_Invalidated> C_ModelCacheInvalidatedMsg;

} // namespace BasePhysicalObject


#endif // MODELCACHEMESSAGES_A2AD03AE_0960_488E_8E85_E7DC2D9F5EFC_H
