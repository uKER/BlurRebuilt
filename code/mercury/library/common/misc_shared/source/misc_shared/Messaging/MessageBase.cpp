#include "MiscSharedPch.h"
#include <misc_shared/Messaging/MessageBase.h>

#include <misc_shared/Messaging/MessageDebug.h>


namespace Messaging
{

#ifndef CFG_GOLD

const char* C_MessageBase::GetDebugName() const
{
	return (m_pDebugInfo != NULL) ? m_pDebugInfo->m_MsgName : GetUnregisteredTypeName();
}

#endif // ndef CFG_GOLD

} // namespace Messaging
