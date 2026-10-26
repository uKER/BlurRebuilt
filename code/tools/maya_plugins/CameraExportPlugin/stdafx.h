
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <ostream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <winsock2.h>
#include <assert.h>
#include <system/Artery/Artery.h>
#include <core/maths.h>
#include <core/containers/string.h>
#include <core/threads.h>
#include <system/time/time.h>

void dprintf(char *format, ...);
