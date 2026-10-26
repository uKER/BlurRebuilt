#include "SharedActorsPch.h"

#include <shared_actors/Utility/XtHandleSerialisation.h>


I_XtHandleContext::I_XtHandleContext(const Sys::C_AnythingUid typeUid) :
	C_Super()
{
	// intentionally blank
}

I_XtHandleContext::~I_XtHandleContext()
{
	// intentionally blank
}





C_XtHandleDatabaseContext::C_XtHandleDatabaseContext(XtDb& database) :
	C_Super(), 
	m_Database(database)
{
	// intentionally blank
}

C_XtHandleDatabaseContext::~C_XtHandleDatabaseContext()
{
	// intentionally blank
}

XtRecord* C_XtHandleDatabaseContext::GetXtRecord(const XtId id) const
{
	return XtGetRecord(&m_Database, id);
}
