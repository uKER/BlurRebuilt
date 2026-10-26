#include "stdafx.h"
#include "Dynamic/Tracer/Manager.h"
#include "Dynamic/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Dynamic/Tracer/EffectInterface.h"

EXTERN_SHADER(vsTracer);
EXTERN_SHADER(psTracerDefault);
EXTERN_SHADER(psTracerDefaultHDR);
EXTERN_SHADER(psTracerDefaultHDRU32);
EXTERN_SHADER(psTracerTemperature);
EXTERN_SHADER(psTracerTemperatureHDR);
EXTERN_SHADER(psTracerTemperatureHDRU32);

namespace DynamicEffect
{
namespace Tracer
{

C_Manager::C_Manager(C_Renderer* pRenderer, const U32 iTypeIndex) : I_DynamicEffectManager(pRenderer, iTypeIndex)
{
}

C_Manager::~C_Manager()
{
}

bool C_Manager::Create()
{
	if (!Display::CreateVertexShaderFromFunc("vsTracer", 0, &m_VertexShader, REF_SHADER(vsTracer)))
		return false;

	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		// choose which set of shaders we need to use - we only have depth fog for shader tonemap at the moment
		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psTracerDefaultHDR", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psTracerDefaultHDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psTracerTemperatureHDR", 0, &m_PixelShader[E_PixelShader_Temperature], REF_SHADER(psTracerTemperatureHDR)))
				return false;
		}
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psTracerDefaultHDRU32", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psTracerDefaultHDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psTracerTemperatureHDRU32", 0, &m_PixelShader[E_PixelShader_Temperature], REF_SHADER(psTracerTemperatureHDRU32)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psTracerDefault", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psTracerDefault)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psTracerTemperature", 0, &m_PixelShader[E_PixelShader_Temperature], REF_SHADER(psTracerTemperature)))
			return false;
	}

	for (U32 shaderNo = 0; shaderNo < E_PixelShader_Count; ++shaderNo)
	{
		m_LuminanceParam[shaderNo] = m_PixelShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");
	}

	return true;
}

bool C_Manager::CreateVolatile()
{
	return true;
}

bool C_Manager::Initialise()
{
	return true;
}

void C_Manager::Destroy()
{
	Display::Device()->Destroy(&m_VertexShader);
	for (U32 shaderNo = 0; shaderNo < E_PixelShader_Count; ++shaderNo)
		Display::Device()->Destroy(&m_PixelShader[shaderNo]);
}

void C_Manager::DestroyVolatile()
{
}

void C_Manager::Shutdown()
{
}

void C_Manager::Reset()
{
}

void C_Manager::SetShaderConstants() const
{
}

void C_Manager::SetVertexShader()
{
	Display::Device()->SetVertexShader(&m_VertexShader);
}

void C_Manager::SetPixelShader(const E_PixelShader ePixelShader)
{
	MathsStorage::Float4 hdrParam = Render::C_RenderState::Instance().ShaderHDR() ? Render::C_RenderState::Instance().HDR()->GetShaderParameters() : Render::C_RenderState::Instance().GetGainShaderParameters();
	Display::Device()->SetPixelShaderParameterF(m_LuminanceParam[ePixelShader], (F32*)&hdrParam);

	Display::Device()->SetPixelShaderParameters(&m_PixelShader[ePixelShader]);
	Display::Device()->SetPixelShader(&m_PixelShader[ePixelShader]);
}

void C_Manager::CleanEffect(const I_DynamicEffect* pEffect)
{
}

void C_Manager::CleanNode(const C_Node* pNode)
{
}

// 3D debug graphics
void C_Manager::DisplayDebugGraphics() const
{
}

// 2D overlays etc
MathsStorage::Int2 C_Manager::DisplayDebugInfo(MathsStorage::Int2 position) const
{
	return position;
}


void C_Manager::AddEffect(I_EffectBase* pEffect)
{
	pEffect->m_EffectPosition = m_Effects.Size();
	m_Effects.PushBack(pEffect);

//	m_ActivedEffects.Resize(m_Effects.Size());

	assert(m_EffectsMap.Find(pEffect->m_iIndex) == m_EffectsMap.End());
	m_EffectsMap.Insert(EffectsMap::DataType(pEffect->m_iIndex, pEffect));
}

void C_Manager::RemoveEffect(I_EffectBase* pEffect)
{
	S32 effectPosition = pEffect->m_EffectPosition;
	I_EffectBase* pEnd = m_Effects[m_Effects.Size() - 1];
	pEnd->m_EffectPosition = pEffect->m_EffectPosition;
	m_Effects[effectPosition] = pEnd;
	m_Effects.PopBack();

//	if(pEffect->m_ActivePosition != -1)
//		RemoveActiveEffect(pEffect);

	EffectsMap::Iterator it = m_EffectsMap.Find(pEffect->m_iIndex);
	assert(it != m_EffectsMap.End());
	if(it != m_EffectsMap.End())
		m_EffectsMap.Erase(it);
}

//void C_Manager::AddActiveEffect(I_EffectBase* pEffect)
//{
//	for(S32 i = 0; i < m_ActiveEffects.Size(); ++i)
//		assert(m_ActiveEffects[i] != pEffect);
//
//	pEffect->m_ActivePosition = m_ActiveEffects.Size();
//	m_ActiveEffects.PushBack(pEffect);
//}
//
//void C_Manager::RemoveActiveEffect(I_EffectBase* pEffect)
//{
//	S32 effectPosition = pEffect->m_ActivePosition;
//	I_ParticleEffect* pEnd = m_ActiveEffects[m_ActiveEffects.Size() - 1];
//	pEnd->m_ActivePosition = pEffect->m_ActivePosition;
//	pEffect->m_ActivePosition = -1;
//	m_ActiveEffects[effectPosition] = pEnd;
//	m_ActiveEffects.PopBack();
//}

}; // namespace Tracer
}; // namespace DynamicEffect
