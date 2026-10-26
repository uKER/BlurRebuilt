#include "stdafx.h"

#include "Dynamic/Node.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"

#include "Dynamic/Tracer/Enums.h"
#include "Dynamic/Tracer/Generic/ShapeBeamCapped.h"
#include "Dynamic/Tracer/Generic/XtInterface.h"



namespace DynamicEffect
{
namespace Tracer
{
namespace XtInterface
{

Debug::Logger &g_SorceryLog = Debug::GetLogger("mercury.Graphics.Dynamic.tracer.sorcery");

struct Effect
{
	Effect() : bInitialised(false), pEffect(0), id(0) {}

	bool				bInitialised;
	I_EffectBase *		pEffect;
	XtU32				id;

	XtSz8				name;
	E_Shape				shapeType;
	E_PixelShader		shader;
	XtSz8				texture;
	XtVec2				size;
	bool				fade;
};

typedef Ut::UtMap<Util::C_String, Effect*> Effects;

void		EffectCallback(XtEventType event, XtRecord* record, Effect* pData, XtCallbackContext*);
void		CreateEffect(Effect* pEffect);
void		UpdateEffects();
void		InvalidateEffect(Effect* pEffect);

Effects		m_Effects;
C_Manager*	m_pManager;

void Initialise(C_Manager* pManager)
{
	m_pManager = pManager;

	Update();
}

void Shutdown()
{
	m_pManager = 0;

	for(Effects::Iterator it = m_Effects.Begin(); it != m_Effects.End(); ++it)
		InvalidateEffect(it->GetValue());
}

void Update()
{
	if(DynamicEffect::XtInterface::Renderer())
	{
		UpdateEffects();
	}
}

void Destroy()
{
	while(m_Effects.Begin() != m_Effects.End())
	{
		Effect* pEffect = m_Effects.Begin()->GetValue();
		InvalidateEffect(pEffect);
		//m_Effects.Erase(pEffect->id);
		m_Effects.Erase(Util::C_String(pEffect->name));
	}
	m_Effects.Destroy();
}

void InvalidateEffect(Effect* pEffect)
{
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

void EffectCallback(XtEventType event, XtRecord* record, Effect* pData, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		pData->id = record->id;
		//m_Effects.Insert(Effects::DataType(record->id, pData));
		m_Effects.Insert(Effects::DataType(Util::C_String(pData->name), pData));
	}
	else if(event == kXtEtDestruct)
	{
		InvalidateEffect(pData);
		//m_Effects.Erase(pData->id);
		m_Effects.Erase(Util::C_String(pData->name));
	}
}

void UpdateEffects()
{
	for(Effects::Iterator it = m_Effects.Begin(); it != m_Effects.End(); ++it)
	{
		Effect* pEffect = it->GetValue();
		if(!pEffect->bInitialised)
		{
			CreateEffect(pEffect);
		}
	}

	for(Effects::Iterator it = m_Effects.Begin(); it != m_Effects.End(); ++it)
	{
		Effect* pEffect = it->GetValue();
		assert(pEffect->bInitialised);
	}
}

void CreateEffect(Effect* pEffect)
{
	RenderSettings renderSettings;
	renderSettings.ePixelShader = pEffect->shader;
	renderSettings.vSize.x = pEffect->size.vx;
	renderSettings.vSize.y = pEffect->size.vy;
	renderSettings.fFade = pEffect->fade ? 0.0f : 1.0f;

	renderSettings.pTexture = DynamicEffect::XtInterface::Renderer()->LoadTexture(Util::C_String(pEffect->texture.string));
	if (!renderSettings.pTexture)
		LOGWARN(g_SorceryLog, "DynamicEffect: Could not find texture '%s' for effect '%s' (%X)", (const char*)pEffect->texture.string, (const char*)pEffect->name, pEffect->id);

	if (pEffect->shapeType == E_Shape_BeamCapped)
	{
		pEffect->pEffect = new ShapeBeamCapped(DynamicEffect::XtInterface::Renderer(), m_pManager, pEffect->id, E_EffectTypeIndex_TracerBeamCapped);
	}
	else
	{
		LOGWARN(g_SorceryLog, "DynamicEffect: Invalid shape on effect '%s' (%X)", (const char*)pEffect->name, pEffect->id);
	}

	if (pEffect->pEffect)
		pEffect->pEffect->Create(renderSettings);

	pEffect->bInitialised = true;
}

I_EffectBase* FindEffect(const Util::C_String& name)
{
	Effects::Iterator it = m_Effects.Find(name);
	if(it != m_Effects.End())
		return it->GetValue()->pEffect;
	else
		return 0;
}

}; // namespace XtInterface
}; // namespace Tracer
}; // namespace DynamicEffect



XT_TYPE(DynamicEffect::Tracer::E_Shape, "Tracer_Shape")
	XT_OPTION(DynamicEffect::Tracer::E_Shape_BeamCapped, "BeamCapped")
XT_END_TYPE

XT_TYPE(DynamicEffect::Tracer::E_PixelShader, "Tracer_Shader")
	XT_OPTION(DynamicEffect::Tracer::E_PixelShader_Default, "Default")
	XT_OPTION(DynamicEffect::Tracer::E_PixelShader_Temperature, "Temperature")
XT_END_TYPE

XT_TYPE(DynamicEffect::Tracer::XtInterface::Effect, "Tracer_Effect")
	XT_CALLBACK(DynamicEffect::Tracer::XtInterface::EffectCallback)
	XT_FIELD(name, "name")
	XT_FIELD(shapeType, "shapeType")
	XT_FIELD(shader, "shader")
	XT_FIELD(texture, "texture")
	XT_FIELD(size, "size")
	XT_FIELD(fade, "fade")
XT_END_TYPE	

XT_LIBRARY(GraphicsDynamicTracer)
	XT_REGISTER_TYPE(DynamicEffect::Tracer::E_Shape)
	XT_REGISTER_TYPE(DynamicEffect::Tracer::E_PixelShader)
	XT_REGISTER_TYPE(DynamicEffect::Tracer::XtInterface::Effect)
XT_END_LIBRARY
