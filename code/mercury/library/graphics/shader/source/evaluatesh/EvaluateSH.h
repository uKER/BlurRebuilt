#ifndef __SHADERS_EVALUATESH_H__
#define __SHADERS_EVALUATESH_H__

namespace Display
{
class C_DeviceContext;
}

namespace Shaders
{

/// returns an RGN colour for the given basis SH lighting data and normal
Maths::C_RGBA_F32 EvaluateBasisSH(Display::C_DeviceContext *pContext, float *pData, const Maths::HVec3 vNormal);

}

#endif
