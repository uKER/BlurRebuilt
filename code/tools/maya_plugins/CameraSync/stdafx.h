
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <winsock2.h>
#include <assert.h>
#include <system/Artery/Artery.h>
#include <core/containers/string.h>
#include <core/threads.h>
#include <core/maths.h>
#include <system/time/time.h>

#include <ostream>
#define NT_PLUGIN
#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MDoubleArray.h>
#include <maya/MArgList.h>

#pragma warning (push,4)
