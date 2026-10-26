#ifndef __SYSTEM_LOGEVENT_H__
#define __SYSTEM_LOGEVENT_H__

#pragma once

#include "DynamicLocationInfo.h"

namespace Debug
{

struct LogEvent
{
	DebugLevel::Level	level;
	Util::C_String		msg;
	LocationInfo		location;
	U64					time;
	Util::C_String		logger;
	Util::C_String		host;
	int					pid;


	// save 
	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar	<< (int)level
			<< msg
			<< logger
			<< time
			<< host
			<< location.m_LineNumber
			<< location.m_FileName
			<< location.m_FunctionName
			<< pid;
	}

	// load
	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		int iLevel;
		ar	>> iLevel
			>> msg
			>> logger
			>> time
			>> host
			>> location.m_LineNumber
			>> location.m_FileName
			>> location.m_FunctionName
			>> pid;

		level = (DebugLevel::Level)iLevel;
	}


};

}

#endif
