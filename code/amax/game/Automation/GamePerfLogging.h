
#ifndef GAMEPERFORMANCELOGGING_H
#define GAMEPERFORMANCELOGGING_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Framework/Application/PerformanceLogging.h"
#include "misc_shared/Messaging/MessageHandler.h"
#include "system/time/simpletimer.h"

namespace AmaxPerfLogging
{
	enum eDurationTimer
	{
		kAiUpdate,
		kPhysicsUpdate,
		kUiUpdate,
		kActorUpdate,
		kLevelSceneUpdate,
		kDynEffectRenderUpdate,
		kPerkInfo,
		kGeneric,					// Generic counter we can use to put around stuff to check - don't long term use for anything
		kMaxUpdateTimer
	};

	void Initialise();
	void BeginFillStreamer();

	void AccumulateDuration(eDurationTimer timer, float duration);

	class MessageController : public Messaging::C_MessageHandler
	{
	public:

		MessageController(Messaging::C_MessageSubscriberSystem* pSubscriberSystem);
		virtual ~MessageController();

		bool MessageHandler(const Messaging::C_MessageBase *pMessage);
	};

	extern MessageController gMessageController;
}

#endif