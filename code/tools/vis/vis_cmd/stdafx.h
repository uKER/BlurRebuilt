// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef __STDAFX_H__
#define __STDAFX_H__

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <crtdbg.h>
#include <iostream>
#include <tchar.h>

#include <string>
#include <algorithm>

#include <windows.h>
#include <windowsx.h>

#include "CmdLine.h"

#include <cstdio>
#include <cassert>

#include <core/core.h>

const F32 PI = 3.1415926535897932384626433832795f;
#define NM_MATHS_PROJECT_NAMESPACE TMathsNM

#include <D3DX9.h>
#include <d3d9types.h>
#include <dxerr.h>

#include <core/UtMaths.h>
#include <core/containers/vector.h>
#include <core/maths/utmaths/mmatrix.h>
#include <core/maths/utmaths/colour.h>

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

#define Assert(X) { if (!(X)) assert(X); }
#define Mem_Alloc(X) malloc(X)
#define Mem_Free(X) free(X)
#define Mem_Realloc(X,Y) realloc(X,Y)

#include <Old_Nightmare_maths/Include/maths.h>

#include <Aftk/Aftk.h>

#include "CServ/ClusterServHeaders.h"

// serialization using "common C++" libs
inline ost::Engine& operator << (ost::Engine& ar, const float3 &v) { ar << v.x << v.y << v.z; return ar; }
inline ost::Engine& operator >> (ost::Engine& ar, float3 &v) { ar >> v.x >> v.y >> v.z; return ar; }


#endif