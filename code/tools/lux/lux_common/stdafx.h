// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#ifndef __STDAFX_H__
#define __STDAFX_H__

#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <core/core.h>

#include "cc++/persist.h"

const F32 PI = 3.1415926535897932384626433832795f;

#include <string>
#include <cmath>
#include <float.h>

#ifdef __GNUC__
#define min(A,B) ((A<B)?A:B)
#define max(A,B) ((A>B)?A:B)
#endif

#include <core/maths/Constants.h>
#include <core/maths/Storage.h>
#include <old_nightmare_maths/maths.h>
#include <core/maths/MathsUtil.h>
#include <core/UtMaths.h>

#include <lux_common/Definitions.h>

#include <map>

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
#include <lux_common/XercesUtility.h>

#include <Aftk/Aftk.h>

#include <old_nightmare_maths/serializevecmat.h>


#endif
