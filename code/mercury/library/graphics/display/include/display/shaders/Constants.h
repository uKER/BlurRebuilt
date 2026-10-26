#ifndef DISPLAY_CONSTANTS_H
#define DISPLAY_CONSTANTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "display/shaders/vsConstants.h"
#include "display/shaders/psConstants.h"

#ifndef MAX_NUMBER_SKINNING_TRANSFORMS
// This needs to match to corresponding define in "Graphics/Shader/Constants.h"
#define MAX_NUMBER_SKINNING_TRANSFORMS 32
#endif

#endif // DISPLAY_CONSTANTS_H
