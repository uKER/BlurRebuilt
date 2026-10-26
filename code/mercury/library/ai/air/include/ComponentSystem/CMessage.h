#ifndef __CMESSAGE_H
#define __CMESSAGE_H

#ifdef _MSC_VER
#pragma once
#endif


#include <ComponentSystem/MessageEnums.h>

namespace airacingtech
{
	class CMessage
	{
	public:
		CMessage()																	{}
		CMessage(EMessageType type) : mType(type)									{}
		CMessage(EMessageType type, void *data) : mType(type), mpData(data)			{}

	public:
		EMessageType	mType;
		void*			mpData;

		
	};
}

#endif //__CMESSAGE_H
