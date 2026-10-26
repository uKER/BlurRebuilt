#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500 

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Containers.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Debug/Log/Subscribers/LogFileSubscriber.h>

#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>
#include <system/xml/XmlDom.h>
#include <system/xml/XmlDomUtility.h>
#include <system/debug/log/debuglog.h>

#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.h>


#include "Aftk/Aftk.h"
#include "aftkspeedtree/AftkSpeedTree.h"

#endif // STDAFX_H
