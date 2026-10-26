#ifndef EFFECT_SHADOW_MAP_DEBUG_H
#define EFFECT_SHADOW_MAP_DEBUG_H

#if _MSC_VER > 1000
#pragma once
#endif

enum E_ShadowMapView
{
	E_ShadowMapView_Standard,
	E_ShadowMapView_Trapezoid,
	E_ShadowMapView_ParallelSplit,
	E_ShadowMapView_StableCascaded,
	E_ShadowMapView_Count
};

DEBUG_ENUM(E_ShadowMapView)
	DE_OPTION(E_ShadowMapView_Standard,			"Standard")
	DE_OPTION(E_ShadowMapView_Trapezoid,		"Trapezoid")
	DE_OPTION(E_ShadowMapView_ParallelSplit,	"ParallelSplit")
	DE_OPTION(E_ShadowMapView_StableCascaded,	"StableCascaded")
END_DEBUG_ENUM

enum E_ShadowMapType
{
	E_ShadowMapType_Standard,
	E_ShadowMapType_Variance,
	E_ShadowMapType_Exponential,
	E_ShadowMapType_Count
};

DEBUG_ENUM(E_ShadowMapType)
	DE_OPTION(E_ShadowMapType_Standard,			"Standard")
	DE_OPTION(E_ShadowMapType_Variance,			"Variance")
	DE_OPTION(E_ShadowMapType_Exponential,		"Exponential")
END_DEBUG_ENUM

namespace Effect
{
}; // namespace Effect

#endif // EFFECT_SHADOW_MAP_DEBUG_H
