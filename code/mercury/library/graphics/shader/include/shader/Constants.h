#ifndef SHADER_CONSTANTS_H
#define SHADER_CONSTANTS_H

#if _MSC_VER > 1000 && !defined(MAYA) && !defined(XSI)
#pragma once
#endif

#if JAM_NEEDS_THIS_DEFINE
#include "include/shader/vsConstants.h"
#include "include/shader/psConstants.h"
#include "include/shader/psTextures.h"
#else
#include "shader/vsConstants.h"
#include "shader/psConstants.h"
#include "shader/psTextures.h"
#endif

#ifndef MAX_NUMBER_SKINNING_TRANSFORMS
// This needs to match to corresponding define in "Graphics/Display/Shaders/Constants.h"
#define MAX_NUMBER_SKINNING_TRANSFORMS 32
#endif

#endif // SHADER_CONSTANTS_H
