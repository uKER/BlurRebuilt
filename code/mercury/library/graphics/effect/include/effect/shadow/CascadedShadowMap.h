#ifndef EFFECT_SHADOW_MAP_CASCADED_H
#define EFFECT_SHADOW_MAP_CASCADED_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Shadow/ShadowMapSetup.h"
#include "Effect/Shadow/ShadowMapView.h"

namespace Effect
{

F32 LogarithmicSplit(const U32 iSplit, const F32 fNear, const F32 fFar, const U32 nSplits);
F32 UniformSplit(const U32 iSplit, const F32 fNear, const F32 fFar, const U32 nSplits);
F32 PracticalSplit(const U32 iSplit, const F32 fNear, const F32 fFar, const U32 nSplits);

Maths::C_Sphere GetMinimalEnclosingSphere(const Maths::C_Rect& viewExtents, const F32 fPerspDist, const F32 fNearClip, const F32 fFarClip);

void ParallelSplitShadowMapSetup(C_ShadowMapView& view, const F32 fNearSplit, const F32 fFarSplit, const C_ShadowMapSetup& setup);
void StableCascadedShadowMapSetup(C_ShadowMapView& view, const F32 fNearSplit, const F32 fFarSplit, const C_ShadowMapSetup& setup);
void StableCascadedShadowMapSetup(C_ShadowMapView& view, const Maths::C_Rect& vFrustumExtents, const F32 fNearSplit, const F32 fFarSplit, const C_ShadowMapSetup& setup);

}; // namespace Effect

#endif
