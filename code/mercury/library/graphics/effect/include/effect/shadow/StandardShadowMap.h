#ifndef EFFECT_STANDARD_SHADOW_MAP_H
#define EFFECT_STANDARD_SHADOW_MAP_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Shadow/ShadowMapSetup.h"
#include "Effect/Shadow/ShadowMapView.h"

namespace Effect
{

void DirectionalLightShadowMapSetup(C_ShadowMapView& view, const C_ShadowMapSetup& setup);
void PointLightShadowMapSetup(C_ShadowMapView& view, const C_ShadowMapSetup& setup);

}; // namespace Effect

#endif
