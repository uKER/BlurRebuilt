#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef PLATFORM_XENON
#include <xtl.h>
#include <xgraphics.h>
#elif defined( PLATFORM_PS3 )
#include <limits.h>
#include <cell/atomic.h>
#include <string.h>
#include <wchar.h>
#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>
#include <cell/pad.h>
#endif

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <system\debug\I_DebugRender.h>


#include <System/DebugLog.h>
#include <Core/Containers/String.h>
#include <System/Debug/Debug.h>
#include <System/Misc/Random.h>
#include <core\utility\endian.h>
#include <system\serialise\C_FileSystem.h>
#include <system\serialise\I_BinaryStream.h>
#include <system\serialise\BinarySerialise.h>
#include <core\containers\sortedbinarytree.h>
#include <core\containers\vector.h>
#include <system\debug\log\logmanager.h>

//#include "gravity/sharedPhysics/SolverSetting.h"

#include <Display/Core/Includes.h>

#include <Display.h>

#include <Render.h>

#include <Render/Debug/Debug.h>

#include "Xt\xtgame.h"

#include "Samples/System/Debug.h"

#endif // STDAFX_H
