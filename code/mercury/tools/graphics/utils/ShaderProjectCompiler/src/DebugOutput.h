#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#pragma once

#ifdef CFG_DEBUG
#define OUTPUT_DEBUG 0
#endif

#define OUTPUT_ARGS 0

void DBPrint(const char* pString, ...);

void DBError(const char* pString, ...);

extern Debug::Logger& g_Log;

#endif // DEBUG_OUTPUT_H
