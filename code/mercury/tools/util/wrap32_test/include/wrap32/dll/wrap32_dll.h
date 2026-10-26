#pragma once

#ifdef SERVER_EXPORTS
#define SERVER_API __declspec(dllexport)
#else
#define SERVER_API __declspec(dllimport)
#endif

#include "wrap32_xgraphics.h"

