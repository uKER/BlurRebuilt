#ifndef GAMESYS_CROWD_SYSTEM_CROWDSYSTEMPCH_H
#define GAMESYS_CROWD_SYSTEM_CROWDSYSTEMPCH_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined (PLATFORM_PC)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <Core/Core.h>
#include <Core/Maths.h>

#include <core/maths/hvec/new/IncludeMinimal.h>
#include <core/maths/hvec/new/IncludeMinimal.inl>

#ifndef PLATFORM_SPU
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#endif // !PLATFORM_SPU

#endif // GAMESYS_CROWD_SYSTEM_CROWDSYSTEMPCH_H
