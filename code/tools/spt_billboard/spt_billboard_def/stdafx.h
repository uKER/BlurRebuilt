#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PC)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Utility/hashops.h>
#include <Core/Containers/string.h>

#include <core/utility/HashOps.h>
#include <core/containers/vector.h>
#include <core/containers/OpenHashTable.h>
#include <core/containers/SortedBinaryTree.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>

#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>
#include <system/serialise/TextSerialise.h>

#include <system/xml/XmlDom.h>
#include <system/xml/XmlDomUtility.h>

namespace SpeedtreeBillboard
{
	extern Debug::Logger& g_Log;
}

#endif // STDAFX_H