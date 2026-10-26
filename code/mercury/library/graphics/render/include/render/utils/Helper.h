#ifndef RENDER_UTILS_HELPER_H
#define RENDER_UTILS_HELPER_H

#ifdef _MSC_VER
#pragma once
#endif

extern Debug::Int g_debugRenderQuality;

namespace Render
{

void BeginPostEffectStates(const bool disableMotionBlur = false);
void EndPostEffectStates();
void GetZScaleParameters( MathsStorage::Float4& fvec, const Display::C_ViewSettings3D& viewSettings3D );

}; // Render

#endif // RENDER_UTILS_HELPER_H
