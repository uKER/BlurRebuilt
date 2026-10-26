#ifndef __FX_SHADERS_IMPL_H__
#define __FX_SHADERS_IMPL_H__

#include <Display.h>

#include "Fx/Shaders.h"

namespace FX
{

/// pixel shader derived class
class PixelShader : public Shader, public Display2::C_PixelShader {};

/// vertex shader derived class
class VertexShader : public Shader, public Display2::C_VertexShader {};

}

#endif
