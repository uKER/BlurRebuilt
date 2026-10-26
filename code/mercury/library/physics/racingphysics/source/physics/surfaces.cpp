#include "stdafx.h"
#include "racingphysics/physics/surfaces.h"
#include "racingphysics/physics/physics.h"
#include "btl/containers/vector.h"
#include "btl/utility/autopointer.h"
#include "system/DebugLog.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

/////////////////////////////////////////////

namespace RacingPhysics {
namespace Surfaces 
{

#if !PLATFORM_SPU
Debug::Logger &g_log = Debug::GetLogger("mercury.racingphysics");
#endif

struct SurfaceData
{
	Btl::Vector<bool> effectDepth;
	Btl::Vector<S32> effectMappings;
	Btl::Vector<Btl::Vector<const RP_TrackSurface*> > surfaceProperties;
};

Btl::AutoPointer<SurfaceData> g_pSurfaceData;
static U32 g_iSurfacesXtId = 0;
static bool g_bMultipleSurfaceMappings = false;

struct ALIGN16 DmaData
{
	XtRecord* pRecord;
	U8* pXtBase;
	U32 XtSize;
	Btl::Vector<U32> XtFixups;
};

DmaData g_dd;

void GenerateXtFixups(DmaData& dd)
{
#if !PLATFORM_SPU
	// xt data
	dd.pXtBase = XtGetRecordBaseAddr(dd.pRecord);
	dd.XtSize = XtGetRecordSize(dd.pRecord);
	U32 fixupCount = XtGetRecordFixupCount(dd.pRecord);
	dd.XtFixups.Resize(fixupCount);

	if (fixupCount)
		XtGenerateRecordFixups(dd.pRecord, &dd.XtFixups[0], fixupCount);
#endif
}

void AllocateSurfaceData(XtRecord* record, RP_SurfaceMappingList* object)
{
	if (!object)
		return;

	Allocate(g_pSurfaceData);

	g_pSurfaceData->effectMappings.Resize(object->mappings.count);
	g_pSurfaceData->effectDepth.Resize(object->mappings.count);

	for (int i = 0; i < object->mappings.count; i++)
	{
		int offset = object->mappings[i].dynamicsSurface - object->surfaceTypes.elements;

		if (offset > -1 && offset < object->surfaceTypes.count)
		{
			g_pSurfaceData->effectMappings[i] = offset;
			g_pSurfaceData->effectDepth[i] = object->surfaceTypes[offset].hasDepth;
		}
		else
		{
#if !PLATFORM_SPU
			LOGERROR(g_log, "No surface type for effect material '%s'in Surface Mapping List (Racing Physics)", object->mappings[i].effectMaterial.string);
			g_pSurfaceData->effectMappings[i] = 0;
			g_pSurfaceData->effectDepth[i] = false;
#endif
		}
	}

	g_pSurfaceData->surfaceProperties.Reserve(object->surfaceTypes.count);
	g_pSurfaceData->surfaceProperties.Resize(object->surfaceTypes.count);

	for (int i = 0; i < object->surfaceTypes.count; i++)
	{
		g_pSurfaceData->surfaceProperties[i].Resize(object->wheelTypes.count);

		for (int j = 0; j < object->wheelTypes.count; j++)
			g_pSurfaceData->surfaceProperties[i][j] = 0;
	}

	for (int i = 0; i < object->properties.count; i++)
	{
		int surfaceIndex = object->properties[i].surface - object->surfaceTypes.elements;
		int wheelIndex = object->properties[i].wheel - object->wheelTypes.elements;

		if (surfaceIndex > -1 && surfaceIndex < object->surfaceTypes.count)
			if (wheelIndex > -1 && wheelIndex < object->wheelTypes.count)
				g_pSurfaceData->surfaceProperties[surfaceIndex][wheelIndex] = &object->properties[i];
	}

	g_dd.pRecord = record;
	GenerateXtFixups(g_dd);
}

void FreeSurfaceData()
{
	Free(g_pSurfaceData);
}

void BindSurfaceMappings(XtRecord* record, RP_SurfaceMappingList* object)
{
	if (!record || !object)
		return;

	g_bMultipleSurfaceMappings	= true;
	g_iSurfacesXtId				= record->id;

	AllocateSurfaceData(record, object);
}

void SurfaceHandler(XtEventType event, XtRecord* record, RP_SurfaceMappingList* object, XtCallbackContext* context)
{
	if (event == kXtEtConstruct)
	{
		if (g_bMultipleSurfaceMappings == 0 || (g_bMultipleSurfaceMappings != 0 && record && record->id == g_iSurfacesXtId))
			AllocateSurfaceData(record, object);
	}
	else if (event == kXtEtDestruct)
	{
		Free(g_pSurfaceData);
	}
}

#if !PLATFORM_SPU

void GetDmaData(void** pDmaData)
{
	// return a pointer to the dma data
	*pDmaData = &g_dd;
}

void SpuFetch(const void* pDmaData) {}

#else

void SpuFetch(const void* pDmaData)
{
	static DmaData s_dd;

	// get the dma data block
	Spu::Fetch(&s_dd, pDmaData, sizeof(DmaData));

	if (s_dd.pXtBase)
	{
		U8* pXtData = (U8*) Spu::FetchAllocAlign(s_dd.pXtBase, s_dd.XtSize);

		SpuFetch(s_dd.XtFixups);

		if (!s_dd.XtFixups.Empty())
			XtApplyRecordFixups(pXtData, s_dd.pXtBase, &s_dd.XtFixups[0], s_dd.XtFixups.Size(), s_dd.XtSize);

		SurfaceHandler(kXtEtConstruct, 0, (RP_SurfaceMappingList*)pXtData, 0);
	}
}

#endif

}}

//////////////////////////////////////////////////////////
#if !PLATFORM_SPU

XT_TYPE(RacingPhysics::Surfaces::RP_SurfaceDef, "RP_SurfaceDef")
	XT_FIELD(name, "name")
	XT_FIELD(hasDepth, "hasDepth")
XT_END_TYPE

XT_TYPE(RacingPhysics::Surfaces::RP_WheelDef, "RP_WheelDef")
	XT_FIELD(name, "name")
XT_END_TYPE

XT_TYPE(RacingPhysics::Surfaces::RP_SurfaceMapping, "RP_SurfaceMapping")
	XT_FIELD(effectMaterial, "effectMaterial")
	XT_FIELD(dynamicsSurface, "dynamicsSurface")
XT_END_TYPE

XT_TYPE(RacingPhysics::Surfaces::RP_SurfaceMappingList, "RP_SurfaceMappingList")
	XT_CALLBACK(RacingPhysics::Surfaces::SurfaceHandler)
	XT_FIELD(surfaceTypes, "surfaceTypes")
	XT_FIELD(wheelTypes, "wheelTypes")
	XT_FIELD(mappings, "mappings")
	XT_FIELD(properties, "properties")
XT_END_TYPE

XT_TYPE(RacingPhysics::Surfaces::RP_TrackSurface, "RP_TrackSurface")
	XT_FIELD(surface, "Surface")
	XT_FIELD(wheel, "Wheel")
	XT_FIELD(gripScaleStatic, "GripScaleStatic")
	XT_FIELD(gripScaleDynamic, "GripScaleDynamic")
	XT_FIELD(gripScaleDynamicSpeed_Mph, "GripScaleDynamicSpeed_Mph")
	XT_FIELD(rollingFriction, "RollingFriction")
	XT_FIELD(roughnessFactor, "RoughnessFactor")
	XT_FIELD(rearGripScale, "RearGripScale")
	XT_FIELD(MIScale, "MIScale")
XT_END_TYPE

XT_LIBRARY(RacingPhysics_Surfaces)
	XT_REGISTER_TYPE(RacingPhysics::Surfaces::RP_SurfaceDef)
	XT_REGISTER_TYPE(RacingPhysics::Surfaces::RP_WheelDef)
	XT_REGISTER_TYPE(RacingPhysics::Surfaces::RP_SurfaceMapping)
	XT_REGISTER_TYPE(RacingPhysics::Surfaces::RP_SurfaceMappingList)
	XT_REGISTER_TYPE(RacingPhysics::Surfaces::RP_TrackSurface)
XT_END_LIBRARY

#endif
//////////////////////////////////////////

//using namespace Maths;

///////////////////////////////////////////////

namespace RacingPhysics {
namespace Surfaces 
{

const RP_TrackSurface* GetTrackSurface(S32 effectMaterial, S32 wheelType)
{
	S32 surfaceType = 0;

	if (effectMaterial < g_pSurfaceData->effectMappings.Size())
		surfaceType = g_pSurfaceData->effectMappings[effectMaterial];

	return g_pSurfaceData->surfaceProperties[surfaceType][wheelType];
}

float GetGripScale(S32 effectMaterial, S32 wheelType, float speed)
{
	if (g_pSurfaceData != 0)
	{
		const RP_TrackSurface* p = GetTrackSurface(effectMaterial, wheelType);

		if (p)
		{
			float g1 = p->gripScaleStatic;
			float g2 = p->gripScaleDynamic;
			float s = p->gripScaleDynamicSpeed_Mph;
			return Maths::Lerp(g1, g2, Maths::Clamped(speed / (Abs(s) + 0.01f), 0.f, 1.f));
		}
	}

	return 1.f;
}

float GetRollingFriction(S32 effectMaterial, S32 wheelType)
{
	if (g_pSurfaceData != 0)
	{
		const RP_TrackSurface* p = GetTrackSurface(effectMaterial, wheelType);
		if (p) return p->rollingFriction;
	}

	return 0.f;
}

float GetRoughnessFactor(S32 effectMaterial, S32 wheelType)
{
	if (g_pSurfaceData != 0)
	{
		const RP_TrackSurface* p = GetTrackSurface(effectMaterial, wheelType);
		if (p) return p->roughnessFactor;
	}

	return 0.f;
}

float GetRearGripScale(S32 effectMaterial, S32 wheelType)
{
	if (g_pSurfaceData != 0)
	{
		const RP_TrackSurface* p = GetTrackSurface(effectMaterial, wheelType);
		if (p) return p->rearGripScale;
	}

	return 1.f;
}

float GetMIScale(S32 effectMaterial, S32 wheelType)
{
	if (g_pSurfaceData != 0)
	{
		const RP_TrackSurface* p = GetTrackSurface(effectMaterial, wheelType);
		if (p) return p->MIScale;
	}

	return 1.f;
}

bool HasDepth(S32 effectMaterial)
{
	if (g_pSurfaceData != 0)
		if (effectMaterial < g_pSurfaceData->effectDepth.Size())
			return g_pSurfaceData->effectDepth[effectMaterial];

	return false;
}

float GetGripScale(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType, float speed)
{
	float f1 = GetGripScale(effectMaterial, wheelType, speed);
	float f2 = GetGripScale(surfaceModifier, wheelType, speed);

	bool hasDepth = HasDepth(surfaceModifier);

	if (hasDepth)
		return Maths::Lerp(f1, f2, f);
	else
		return f2;
}

float GetRollingFriction(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType)
{
	float f1 = GetRollingFriction(effectMaterial, wheelType);
	float f2 = GetRollingFriction(surfaceModifier, wheelType);

	bool hasDepth = HasDepth(surfaceModifier);

	if (hasDepth)
		return Maths::Lerp(f1, f2, f);
	else
		return f2;
}

float GetRoughnessFactor(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType)
{
	float f1 = GetRoughnessFactor(effectMaterial, wheelType);
	float f2 = GetRoughnessFactor(surfaceModifier, wheelType);

	bool hasDepth = HasDepth(surfaceModifier);

	if (hasDepth)
		return Maths::Lerp(f1, f2, f);
	else
		return f2;
}

float GetRearGripScale(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType)
{
	float f1 = GetRearGripScale(effectMaterial, wheelType);
	float f2 = GetRearGripScale(surfaceModifier, wheelType);

	bool hasDepth = HasDepth(surfaceModifier);

	if (hasDepth)
		return Maths::Lerp(f1, f2, f);
	else
		return f2;
}

float GetMIScale(S32 effectMaterial, S32 surfaceModifier, float f, S32 wheelType)
{
	float f1 = GetMIScale(effectMaterial, wheelType);
	float f2 = GetMIScale(surfaceModifier, wheelType);

	bool hasDepth = HasDepth(surfaceModifier);

	if (hasDepth)
		return Maths::Lerp(f1, f2, f);
	else
		return f2;
}

}}
