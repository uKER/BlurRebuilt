#ifndef GAMESYS_ACTORS_SHAREDACTORSPCH_H
#define GAMESYS_ACTORS_SHAREDACTORSPCH_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#else

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#endif

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

#include <btl/containers/list.h>
#include <btl/containers/vector.h>

#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <misc_shared/Messaging/MessageValue.h>
#include <misc_shared/Messaging/MessageValueStorage.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorComponent.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorList.h>
#include <actor_system/ActorMessageHandle.h>
#include <actor_system/ActorMessageValue.h>

#include <xt/XtGame.h>

#endif // GAMESYS_ACTOR_SYSTEM_SHAREDACTORSPCH_H
