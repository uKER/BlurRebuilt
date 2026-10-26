#ifndef RP_SURFACES_H
#define RP_SURFACES_H

#include <xt/xtgame.h>

namespace RacingPhysics {
namespace Surfaces {

	struct RP_SurfaceDef
	{
		XtSz8 name;
		bool hasDepth;
	};

	struct RP_WheelDef
	{
		XtSz8 name;
	};

	struct RP_SurfaceMapping
	{
		XtSz8 effectMaterial;
		RP_SurfaceDef* dynamicsSurface;
	};

	struct RP_TrackSurface
	{
		RP_SurfaceDef* surface;
		RP_WheelDef* wheel;
		float gripScaleStatic;
		float gripScaleDynamic;
		float gripScaleDynamicSpeed_Mph;
		float rollingFriction;
		float roughnessFactor;
		float rearGripScale;
		float MIScale;
	};

	struct RP_SurfaceMappingList
	{
		XtArray<RP_SurfaceDef> surfaceTypes;
		XtArray<RP_WheelDef> wheelTypes;
		XtArray<RP_SurfaceMapping> mappings;
		XtArray<RP_TrackSurface> properties;
	};	

	// surfaces
	float GetGripScale(S32 effectMaterial, S32 wheelType, float speed);
	float GetRollingFriction(S32 effectMaterial, S32 wheelType);
	float GetRoughnessFactor(S32 effectMaterial, S32 wheelType);
	float GetRearGripScale(S32 effectMaterial, S32 wheelType);
	float GetMIScale(S32 effectMaterial, S32 wheelType);

	float GetGripScale(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType, float speed);
	float GetRollingFriction(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType);
	float GetRoughnessFactor(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType);
	float GetRearGripScale(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType);
	float GetMIScale(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType);

	void GetDmaData(void** pDmaData);
	void SpuFetch(const void* pDmaData);
	void BindSurfaceMappings(XtRecord* record, RP_SurfaceMappingList* object);
}
}

#endif


