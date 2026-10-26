#ifndef EFFECT_PERSPECTIVE_SHADOW_MAP_H
#define EFFECT_PERSPECTIVE_SHADOW_MAP_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Shadow/ShadowMapSetup.h"
#include "Effect/Shadow/ShadowMapView.h"

namespace Effect
{

void TrapezoidalShadowMapSetup(C_ShadowMapView& view, const F32 fNearSplit, const F32 fFarSplit, const C_ShadowMapSetup& setup);

}; // namespace Effect

#endif
