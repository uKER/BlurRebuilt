#ifndef IMPOSTERGEN_PCH_H
#define IMPOSTERGEN_PCH_H

#ifdef _MSC_VER
#pragma once
#endif

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <core/Core.h>
#include <core/Maths.h>
#include <Core/Containers/String.h>

#include <Btl/Containers/vector.h>
#include <Btl/Containers/openhashmap.h>
#include <Btl/Containers/openhashset.h>
#include <Btl/Containers/map.h>
#include <Btl/Containers/list.h>
#include <Btl/Utility/string.h>
#include <Btl/Utility/Hashops.h>

#include <system/DebugLog.h>
#include <system/misc/Progress.h>

#include <display/core/Includes.h>
#include <display.h>

#include <aftk/Aftk.h>
#include <aftkut/AftkUt.h>

#include <attk/Attk.h>

#include <AftkRender.h>

#include <file_tag_db/FileTagDB.h>

#endif