#ifndef NETGAMEWORLDEMPTYAPP_H
#define NETGAMEWORLDEMPTYAPP_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netGame/netGameWorldInterface.h"

namespace netGame
{
	class worldEmptyApp: public worldInterface
	{
	public:
		worldEmptyApp();

		virtual bool MessageHandler(const Messaging::C_MessageBase* pMessage);
		virtual bool draw() { return true; };
		virtual bool update(float dt) { return true; };
	};
};
#endif //NETGAMEWORLDEMPTYAPP_H