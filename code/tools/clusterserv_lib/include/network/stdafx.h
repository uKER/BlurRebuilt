// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include <core/core.h>

const F32 PI = 3.1415926535897932384626433832795f;

#include "cc++/Persist.h"
#include "cc++/Thread.h"
#include "cc++/SocketPort.h"
#include "cc++/Socket.h"
#include "cc++/Address.h"
typedef ost::BaseObject BaseObject;

#include <cstdio>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

#ifdef __GNUC__
#define min(A,B) ((A<B)?A:B)
#define max(A,B) ((A>B)?A:B)
#endif

#include <core/UtMaths.h>
#include <core/containers/vector.h>
#include <core/maths/utmaths/mmatrix.h>
#include <core/maths/utmaths/colour.h>

#include <tools_debug/DebugManager.h>

#include "Network/Socket.h"
#include "Network/Network.h"
#include "Network/IServer.h"
#include "Network/Server.h"
#include "Network/Connection.h"
#include "tools_debug/DebugManager.h"

#include <system\debuglog.h>

