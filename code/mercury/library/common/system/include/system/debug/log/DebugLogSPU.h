#ifndef __SYSTEM_DEBUGLOGSPU_H__
#define __SYSTEM_DEBUGLOGSPU_H__

#include "system/scheduler/jobqueue.h"

#ifndef GLOBAL_DEBUG_MESSAGE_LEVEL
#define GLOBAL_DEBUG_MESSAGE_LEVEL 5
#endif

// here is some code for doing sLOGDEBUG - logging static strings residing in PPU memory from the SPU
#define SSL__TOKEN_JOIN(X,Y) X##Y 
#define SSL_TOKEN_JOIN(X,Y) SSL__TOKEN_JOIN(X,Y)
#define SSL_UNIQUE_TOKEN SSL_TOKEN_JOIN(__tmp_token,__LINE__)
#define DEBUG_LOCATION_STRING(X,Y) X "(" #Y "): Assert failed"

void spuStaticLog(int iLevel, char *string, int iVar); // this function comes from ProfilerSPU.cpp

// the debug log macros
#define SPU_LOG(LEVEL, X, ...) static char SSL_UNIQUE_TOKEN[] PPUDATA = X ; spuStaticLog(LEVEL, SSL_UNIQUE_TOKEN, int(__VA_ARGS__));

#if GLOBAL_DEBUG_MESSAGE_LEVEL > 4
#define LOGDEBUG( LOGGER, ...)	(*((int*)&Scheduler::_jqThisWorker->pLogger) = 5, spu_printf(__VA_ARGS__))
#define sLOGDEBUG( LOGGER, X, ... )  SPU_LOG(5,X,__VA_ARGS__)
#else
#define LOGDEBUG( LOGGER, ...)
#define sLOGDEBUG( LOGGER, ... )
#endif

#if GLOBAL_DEBUG_MESSAGE_LEVEL > 3
#define LOGINFO( LOGGER, ...)	(*((int*)&Scheduler::_jqThisWorker->pLogger) = 4, spu_printf(__VA_ARGS__))
#define sLOGINFO( LOGGER, X, ... )   SPU_LOG(4,X,__VA_ARGS__)
#else
#define LOGINFO( LOGGER, ...)
#define sLOGINFO( LOGGER, ... )
#endif

#if GLOBAL_DEBUG_MESSAGE_LEVEL > 2
#define LOGWARN( LOGGER, ...)	(*((int*)&Scheduler::_jqThisWorker->pLogger) = 3, spu_printf(__VA_ARGS__))
#define sLOGWARN( LOGGER, X, ... )  SPU_LOG(3,X,__VA_ARGS__)
#else
#define LOGWARN( LOGGER, ...)
#define sLOGWARN( LOGGER, ...)
#endif

#if GLOBAL_DEBUG_MESSAGE_LEVEL > 1
#define LOGERROR( LOGGER, ...)	(*((int*)&Scheduler::_jqThisWorker->pLogger) = 2, spu_printf(__VA_ARGS__))
#define sLOGERROR( LOGGER, X, ...)  SPU_LOG(2,X,__VA_ARGS__)
#else
#define LOGERROR( LOGGER, ...)
#define sLOGERROR( LOGGER, ...)
#endif

#if GLOBAL_DEBUG_MESSAGE_LEVEL > 0
#define LOGFATAL( LOGGER, ...)	(*((int*)&Scheduler::_jqThisWorker->pLogger) = 1, spu_printf(__VA_ARGS__))
#define sLOGFATAL( LOGGER, X, ... )  SPU_LOG(1,X,__VA_ARGS__)
#else
#define LOGFATAL( LOGGER, ...)
#define sLOGFATAL( LOGGER, ...)
#endif

namespace Debug
{
struct Logger;
};

#endif
