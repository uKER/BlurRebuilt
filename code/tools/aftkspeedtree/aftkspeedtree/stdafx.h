#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_SPEEDTREE_STDAFX_H
#define AFTK_SPEEDTREE_STDAFX_H

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include <string>
#include <set>


#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/abstractDOMParser.hpp>
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

using namespace XERCES_CPP_NAMESPACE;

#ifndef COMPILER_GNU
#define WIN32_LEAN_AND_MEAN	
#include <windows.h>
#endif


#include <core/core.h>
#include <core/maths.h>
#include <system/debuglog.h>

#include <aftk/Aftk.h>
#include <aftkut/AftkUt.h>
#include <core/containers/sortedbinarytree.h>
#include <core/containers/vector.h>
#include <system/misc/stringutil.h>

//Hackery to be removed!

void CleanupActorForLightingTool(Aftk::C_SceneNode* pSceneNode);

bool HasSPTData(Aftk::C_SceneNode* pSceneNode);

#endif
