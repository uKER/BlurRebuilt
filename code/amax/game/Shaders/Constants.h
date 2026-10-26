#ifndef AMAX_SHADERS_COMMON_CONSTANTS_H
#define AMAX_SHADERS_COMMON_CONSTANTS_H

#if _MSC_VER > 1000 && !defined(MAYA) && !defined(XSI)
#pragma once
#endif

#include "vsConstants.h"
#include "psConstants.h"
#include "psTextures.h"

#ifndef MAX_NUMBER_SKINNING_TRANSFORMS
// This needs to match to corresponding define in "Graphics/Display/Shaders/Constants.h"
#define MAX_NUMBER_SKINNING_TRANSFORMS 32
#endif

#endif // AMAX_SHADERS_COMMON_CONSTANTS_H
