// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include <windows.h>
#include <windowsx.h>

#include <cstdio>
#include <cassert>
#include <string>

#include <core\core.h>

const F32 PI = 3.1415926535897932384626433832795f;
#define NM_MATHS_PROJECT_NAMESPACE TMathsNM

#include <D3DX9.h>
#include <d3d9types.h>
#include <dxerr.h>

#include <core/UtMaths.h>
#include <core/containers/vector.h>
#include <core/containers/sortedbinarytree.h>
#include <core/maths/utmaths/mmatrix.h>
#include <core/maths/utmaths/colour.h>
#include <system\debuglog.h>

#include "vis_core\types.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMAttr.hpp>

#include "vis_core/stringmanipulation.h"

#define Assert(X) { if (!(X)) assert(X); }
#define Mem_Alloc(X) malloc(X)
#define Mem_Free(X) free(X)
#define Mem_Realloc(X,Y) realloc(X,Y)

#include <Old_Nightmare_Maths/Include/maths.h>
#include <Old_Nightmare_Maths/Include/Plane.hpp>

#include <Aftk/Aftk.h>

#include <winsock2.h>
#include "winsock2.h"
#include "Ws2tcpip.h"

#define SIN_ADDR sin_addr.S_un.S_addr

#include "cc++/Persist.h"

typedef ost::BaseObject BaseObject;

inline ost::Engine& operator << (ost::Engine& ar, const float3 &v) { ar << v.x << v.y << v.z; return ar; }
inline ost::Engine& operator >> (ost::Engine& ar, float3 &v) { ar >> v.x >> v.y >> v.z; return ar; }
inline ost::Engine& operator << (ost::Engine& ar, const box3f &v) { ar << v.vmin << v.vmax; return ar; }
inline ost::Engine& operator >> (ost::Engine& ar, box3f &v) { ar >> v.vmin >> v.vmax; return ar; }

typedef unsigned char BYTE;

extern bool gbDebugMode;

extern Debug::Logger &g_Log;

#include "CServ/ClusterServHeaders.h"

