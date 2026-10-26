#ifndef DEFERRED_CONSTANTS_H
#define DEFERRED_CONSTANTS_H

#ifdef _MSC_VER
#pragma once
#endif

#if THIS_DEFINE_DOES_NOT_NEED_TO_BE_DEFINED
#include "include/deferred/shaders/vsConstants.h"
#include "include/deferred/shaders/psConstants.h"
#else
#include "deferred/shaders/vsConstants.h"
#include "deferred/shaders/psConstants.h"
#endif


#endif // DEFERRED_CONSTANTS_H
