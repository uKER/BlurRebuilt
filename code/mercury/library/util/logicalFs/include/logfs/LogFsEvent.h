#ifndef LOGICALFS_LOG_FS_EVENT_H
#define LOGICALFS_LOG_FS_EVENT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace LogicalFs
{

class C_LogFsEvent
{
public:
	C_LogFsEvent(const char* name);
	~C_LogFsEvent();
};

}

#define LOGFS_EVENT(name) LogicalFs::C_LogFsEvent logFsEvent(name);

#endif
