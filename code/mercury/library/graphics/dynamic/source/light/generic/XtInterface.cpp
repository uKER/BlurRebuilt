#include "stdafx.h"
#include "Dynamic/Light/Generic/XtInterface.h"

#include "Dynamic/Light/Enums.h"
#include "Dynamic/Light/Generic/Shape2dBeam.h"
#include "Dynamic/Light/Generic/Shape2dCross.h"
#include "Dynamic/Light/Generic/Shape2dQuad.h"
#include "Dynamic/Light/Generic/Shape3dBeam.h"
#include "Dynamic/Light/Generic/Shape3dBeamCapped.h"
#include "Dynamic/Light/Generic/Shape3dDecal.h"
#include "Dynamic/Light/Generic/Shape3dQuad.h"
#include "Dynamic/TypeIndex.h"

namespace DynamicEffect
{
namespace Light
{
namespace XtInterface
{

Debug::Logger &g_SorceryLog = Debug::GetLogger("mercury.Graphics.Dynamic.light.sorcery");

C_Manager*	g_pManager = 0;

typedef Ut::UtMap<U32, Effect*>				Effects;
typedef Ut::UtMap<Util::C_String, Light*>	Lights;

void				EffectCallback(XtEventType event, XtRecord* record, Effect* pData, XtCallbackContext*);
void				LightCallback(XtEventType event, XtRecord* record, Light* pData, XtCallbackContext*);

Effects				g_Effects;
Lights				g_Lights;

void				InvalidateEffect(Effect* pEffect);
void				InvalidateLight(Light* pLight);

void				UpdateEffects();
void				CreateEffect(Effect* pEffect);

void				UpdateLights();
void				CreateLight(Light* pLight);
LightDefinition*	FindLight(const Util::C_String& name);

void Initialise(C_Manager * pManager)
{
	g_pManager = pManager;
	Update();
}

void Shutdown()
{
	g_pManager = 0;
	for(Effects::Iterator it = g_Effects.Begin(); it != g_Effects.End(); ++it)
		InvalidateEffect(it->GetValue());
	for(Lights::Iterator it = g_Lights.Begin(); it != g_Lights.End(); ++it)
		InvalidateLight(it->GetValue());
}

void Update()
{
	if(DynamicEffect::XtInterface::Renderer())
	{
		UpdateEffects();
		UpdateLights();
	}
}

void Destroy()
{
	while(g_Effects.Begin() != g_Effects.End())
	{
		Effect* pEffect = g_Effects.Begin()->GetValue();
		InvalidateEffect(pEffect);
		g_Effects.Erase(pEffect->id);
	}
	while(g_Lights.Begin() != g_Lights.End())
	{
		Light* pLight = g_Lights.Begin()->GetValue();
		InvalidateLight(pLight);
		g_Lights.Erase(pLight->id);
	}
	g_Effects.Destroy();
	g_Lights.Destroy();
}

void InvalidateEffect(Effect* pEffect)
{
	if(pEffect->bInitialised)
	{
		// invalidate all lights with instances with this effect
		for (Lights::Iterator it = g_Lights.Begin(); it != g_Lights.End(); ++it)
		{
			Light* pLight = it->GetValue();
			LightDefinition* pDefinition = pLight->pDefinition;
			if (!pDefinition)
				continue;
			for (S32 instanceNo = 0; instanceNo < pDefinition->m_Instances.Size(); ++instanceNo)
			{
				const U32 effectId = pDefinition->m_Instances[instanceNo].m_iEffect;
				if (effectId == pEffect->id)
				{
					InvalidateLight(pLight);
				}			
			}
		}

		// delete this effect
		if (pEffect->pEffect)
		{
			DynamicEffect::XtInterface::Renderer()->CleanEffect(pEffect->pEffect);
			pEffect->pEffect->Destroy();
			delete pEffect->pEffect;
			pEffect->pEffect = 0;
		}
		pEffect->bInitialised = false;
	}
}

void EffectCallback(XtEventType event, XtRecord* record, Effect* pData, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		pData->id = record->id;
		g_Effects.Insert(Effects::DataType(record->id, pData));
	}
	else if(event == kXtEtDestruct)
	{
		InvalidateEffect(pData);
		g_Effects.Erase(pData->id);
	}
}

void InvalidateLight(Light* pLight)
{
	if(pLight->bInitialised)
	{
		if (pLight->pDefinition && g_pManager)
		{
			g_pManager->FreeLightDefinition(pLight->pDefinition);
			pLight->pDefinition = 0;
		}
		pLight->bInitialised = false;
	}
}

void LightCallback(XtEventType event, XtRecord* record, Light* pData, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		pData->id = record->id;
		g_Lights.Insert(Lights::DataType(Util::C_String(pData->name), pData));
	}
	else if(event == kXtEtDestruct)
	{
		InvalidateLight(pData);
		g_Lights.Erase(pData->name.string);
	}
}

void UpdateEffects()
{
	for(Effects::Iterator it = g_Effects.Begin(); it != g_Effects.End(); ++it)
	{
		Effect* pEffect = it->GetValue();
		if(!pEffect->bInitialised)
		{
			CreateEffect(pEffect);
		}
	}

	for(Effects::Iterator it = g_Effects.Begin(); it != g_Effects.End(); ++it)
	{
		Effect* pEffect = it->GetValue();
		assert(pEffect->bInitialised);
	}
}

void CreateEffect(Effect* pEffect)
{
	RenderSettings renderSettings;
	
	// texture
	renderSettings.pTexture = DynamicEffect::XtInterface::Renderer()->LoadTexture(Util::C_String(pEffect->texture.string));
	renderSettings.nAtlasWidth = pEffect->atlasSize.width;
	renderSettings.nAtlasHeight = pEffect->atlasSize.height;
	if (!renderSettings.pTexture)
		LOGWARN(g_SorceryLog, "DynamicEffect: Could not find texture '%s' for effect '%s' (%X)", (const char*)pEffect->texture.string, (const char*)pEffect->name, pEffect->id);

	// cull distance
	F32 fCullDistance = pEffect->cullDistance;
	if (fCullDistance < 1.0f)
		fCullDistance = 1000.0f;
	renderSettings.fCullDistance = fCullDistance;
	renderSettings.fRcpCullDistance = 1.0f / fCullDistance;

	// build lookup tables
	const S32 nSteps = 8;
	renderSettings.viewTable.table.Resize(nSteps);
	renderSettings.distanceTable.table.Resize(nSteps);
	renderSettings.positionTable.table.Resize(nSteps);
	if (!DynamicEffect::XtInterface::CreateCurve(pEffect->viewCurve, renderSettings.viewTable, nSteps))
		LOGWARN(g_SorceryLog, "DynamicEffect: Invalid view curve on effect '%s' (%X)", (const char*)pEffect->name, pEffect->id);
	if (!DynamicEffect::XtInterface::CreateCurve(pEffect->distanceCurve, renderSettings.distanceTable, nSteps))
		LOGWARN(g_SorceryLog, "DynamicEffect: Invalid distance curve on effect '%s' (%X)", (const char*)pEffect->name, pEffect->id);
	if (!DynamicEffect::XtInterface::CreateCurve(pEffect->positionCurve, renderSettings.positionTable, nSteps))
		LOGWARN(g_SorceryLog, "DynamicEffect: Invalid position curve on effect '%s' (%X)", (const char*)pEffect->name, pEffect->id);

	if (pEffect->shapeType == E_Shape_Beam2d)
	{
		renderSettings.eVertexShader = E_VertexShader_Default2d;
		renderSettings.ePixelShader = E_PixelShader_Default;
		pEffect->pEffect = new Shape2dBeam(DynamicEffect::XtInterface::Renderer(), g_pManager, pEffect->id, E_EffectTypeIndex_Light2dBeam);
	}
	else if (pEffect->shapeType == E_Shape_Cross2d)
	{
		renderSettings.eVertexShader = E_VertexShader_Default2d;
		renderSettings.ePixelShader = E_PixelShader_Default;
		pEffect->pEffect = new Shape2dCross(DynamicEffect::XtInterface::Renderer(), g_pManager, pEffect->id, E_EffectTypeIndex_Light2dCross);
	}
	else if (pEffect->shapeType == E_Shape_Quad2d)
	{
		renderSettings.eVertexShader = E_VertexShader_Default2d;
		renderSettings.ePixelShader = E_PixelShader_Default;
		pEffect->pEffect = new Shape2dQuad(DynamicEffect::XtInterface::Renderer(), g_pManager, pEffect->id, E_EffectTypeIndex_Light2dQuad);
	}
	else if (pEffect->shapeType == E_Shape_Beam3d)
	{
		renderSettings.eVertexShader = E_VertexShader_Default3d;
		renderSettings.ePixelShader = E_PixelShader_Default;
		pEffect->pEffect = new Shape3dBeam(DynamicEffect::XtInterface::Renderer(), g_pManager, pEffect->id, E_EffectTypeIndex_Light3dBeam);
	}
	else if (pEffect->shapeType == E_Shape_BeamCapped3d)
	{
		renderSettings.eVertexShader = E_VertexShader_Default3d;
		renderSettings.ePixelShader = E_PixelShader_Default;
		pEffect->pEffect = new Shape3dBeamCapped(DynamicEffect::XtInterface::Renderer(), g_pManager, pEffect->id, E_EffectTypeIndex_Light3dBeamCapped);
	}
	else if (pEffect->shapeType == E_Shape_Decal3d)
	{
		renderSettings.eVertexShader = E_VertexShader_Default3d;
		renderSettings.ePixelShader = E_PixelShader_Default;
		pEffect->pEffect = new Shape3dDecal(DynamicEffect::XtInterface::Renderer(), g_pManager, pEffect->id, E_EffectTypeIndex_Light3dDecal);
	}
	else if (pEffect->shapeType == E_Shape_Quad3d)
	{
		renderSettings.eVertexShader = E_VertexShader_Default3d;
		renderSettings.ePixelShader = E_PixelShader_Default;
		pEffect->pEffect = new Shape3dQuad(DynamicEffect::XtInterface::Renderer(), g_pManager, pEffect->id, E_EffectTypeIndex_Light3dQuad);
	}
	else
	{
		LOGWARN(g_SorceryLog, "DynamicEffect: Invalid shape on effect '%s' (%X)", (const char*)pEffect->name, pEffect->id);
	}

	if (pEffect->pEffect)
		pEffect->pEffect->Create(renderSettings);

	pEffect->bInitialised = true;
}

void UpdateLights()
{
	for(Lights::Iterator it = g_Lights.Begin(); it != g_Lights.End(); ++it)
	{
		Light* pLight = it->GetValue();
		if(!pLight->bInitialised)
		{
			CreateLight(pLight);
		}
	}

	for(Lights::Iterator it = g_Lights.Begin(); it != g_Lights.End(); ++it)
	{
		Light* pLight = it->GetValue();
		assert(pLight->bInitialised);
	}
}

void CreateLight(Light* pLight)
{
	pLight->pDefinition = new LightDefinition;

	pLight->pDefinition->m_vSize = MathsStorage::Float3A16(pLight->size.vx, pLight->size.vy, pLight->size.vz);
	pLight->pDefinition->m_eOcclusion = pLight->occlusion;
	pLight->pDefinition->m_Instances.Clear();

	for (S32 instanceNo = 0; instanceNo < pLight->instances.count; ++instanceNo)
	{
		const Instance& xtInstance = pLight->instances[instanceNo];

		InstanceDefinition instanceDefinition;
		instanceDefinition.m_vSize = MathsStorage::Float2(xtInstance.size.vx, xtInstance.size.vy);
		instanceDefinition.m_fIntensity = xtInstance.intensity;
		instanceDefinition.m_iTexture = xtInstance.textureId;
		instanceDefinition.m_iEffect = xtInstance.effect.Id();
		instanceDefinition.m_vTransform0 = MathsStorage::Float3(xtInstance.transform0.vx, xtInstance.transform0.vy, xtInstance.transform0.vz);
		instanceDefinition.m_vTransform1 = MathsStorage::Float3(xtInstance.transform1.vx, xtInstance.transform1.vy, xtInstance.transform1.vz);

		pLight->pDefinition->m_Instances.PushBack(instanceDefinition);
	}

	pLight->bInitialised = true;

	if (g_pManager)
		g_pManager->AddLightDefinition(pLight->pDefinition);
}

LightDefinition* FindLight(const Util::C_String& name)
{
	Lights::Iterator it = g_Lights.Find(name);
	if(it != g_Lights.End())
		return it->GetValue()->pDefinition;
	else
		return 0;
}

}; // namespace XtInterface
}; // namespace Light
}; // namespace DynamicEffect

XT_TYPE(DynamicEffect::Light::E_Shape, "Light_Shape")
	XT_OPTION(DynamicEffect::Light::E_Shape_Beam2d, "Beam2d")
	XT_OPTION(DynamicEffect::Light::E_Shape_Cross2d, "Cross2d")
	XT_OPTION(DynamicEffect::Light::E_Shape_Quad2d, "Quad2d")
	XT_OPTION(DynamicEffect::Light::E_Shape_Beam3d, "Beam3d")
	XT_OPTION(DynamicEffect::Light::E_Shape_BeamCapped3d, "BeamCapped3d")
	XT_OPTION(DynamicEffect::Light::E_Shape_Decal3d, "Decal3d")
	XT_OPTION(DynamicEffect::Light::E_Shape_Quad3d, "Quad3d")
XT_END_TYPE

XT_TYPE(DynamicEffect::Light::E_Occlusion, "Light_Occlusion")
	XT_OPTION(DynamicEffect::Light::E_Occlusion_None, "None")
	XT_OPTION(DynamicEffect::Light::E_Occlusion_Query, "Query")
	XT_OPTION(DynamicEffect::Light::E_Occlusion_Custom, "Custom")
XT_END_TYPE

XT_TYPE(DynamicEffect::Light::XtInterface::Effect, "Light_Effect")
	XT_CALLBACK(DynamicEffect::Light::XtInterface::EffectCallback)
	XT_FIELD(name, "name")
	XT_FIELD(shapeType, "shapeType")
	XT_FIELD(texture, "texture")
	XT_FIELD(atlasSize, "atlasSize")
	XT_FIELD(cullDistance, "cullDistance")
	XT_FIELD(viewCurve, "viewCurve")
	XT_FIELD(distanceCurve, "distanceCurve")
	XT_FIELD(positionCurve, "positionCurve")
XT_END_TYPE	

XT_TYPE(DynamicEffect::Light::XtInterface::Instance, "Light_Instance")
	XT_FIELD(effect, "effect")
	XT_FIELD(transform0, "transform0")
	XT_FIELD(transform1, "transform1")
	XT_FIELD(size, "size")
	XT_FIELD(intensity, "intensity")
	XT_FIELD(textureId, "textureId")
XT_END_TYPE

XT_TYPE(DynamicEffect::Light::XtInterface::Light, "Light_Object")
	XT_CALLBACK(DynamicEffect::Light::XtInterface::LightCallback)
	XT_FIELD(name, "name")
	XT_FIELD(size, "size")
	XT_FIELD(occlusion, "occlusion")
	XT_FIELD(instances, "instances")
XT_END_TYPE

XT_LIBRARY(GraphicsDynamicLight)
	XT_REGISTER_TYPE(DynamicEffect::Light::E_Shape)
	XT_REGISTER_TYPE(DynamicEffect::Light::E_Occlusion)
	XT_REGISTER_TYPE(DynamicEffect::Light::XtInterface::Effect)
	XT_REGISTER_TYPE(DynamicEffect::Light::XtInterface::Instance)
	XT_REGISTER_TYPE(DynamicEffect::Light::XtInterface::Light)
XT_END_LIBRARY
