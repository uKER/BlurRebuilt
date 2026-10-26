#ifndef SPU_STDAFX_H
#define SPU_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#pragma warning(disable:4146)

#if defined (PLATFORM_PC)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>
#include <Core/Utility/Utility.h>
#include <Core/Containers.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

#include <Display.h>

#endif // SPU_STDAFX_H
