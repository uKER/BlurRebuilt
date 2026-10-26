#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <vector>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>

#include <core/core.h>
#include <core/containers.h>

#include <system/debuglog.h>
#include <system/debug/log/debuglog.h>
#include <system/debug/log/AutoConfigure.h>
#include <system/debug/log/Subscribers/ConsoleSubscriber.h>
#include <system/debug/log/Subscribers/SocketSubscriber.h>
#include <System/Misc/StringUtil.h>
#include <System/Serialise.h>

#include "src/DebugOutput.h"
