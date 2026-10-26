#include "stdafx.h"
#include "Dynamic/Fog/Manager.h"
#include "Dynamic/Fog/EffectInterface.h"
#include "Render/State/RenderState.h"

EXTERN_SHADER(vsFogVolumeBack);
EXTERN_SHADER(vsFogVolumeFront);
EXTERN_SHADER(vsFogCuboidBack);
EXTERN_SHADER(vsFogCuboidFront);
EXTERN_SHADER(vsFogCylinderBack);
EXTERN_SHADER(vsFogCylinderFront);
EXTERN_SHADER(vsFogEllipsoidBack);
EXTERN_SHADER(vsFogEllipsoidFront);
EXTERN_SHADER(psFogVolumeBack);
EXTERN_SHADER(psFogVolumeFront);
EXTERN_SHADER(psFogCuboidBack);
EXTERN_SHADER(psFogCuboidFront);
EXTERN_SHADER(psFogCylinderBack);
EXTERN_SHADER(psFogCylinderFront);
EXTERN_SHADER(psFogEllipsoidBack);
EXTERN_SHADER(psFogEllipsoidFront);
EXTERN_SHADER(psFogVolumeBackHDR);
EXTERN_SHADER(psFogVolumeFrontHDR);
EXTERN_SHADER(psFogCuboidBackHDR);
EXTERN_SHADER(psFogCuboidFrontHDR);
EXTERN_SHADER(psFogCylinderBackHDR);
EXTERN_SHADER(psFogCylinderFrontHDR);
EXTERN_SHADER(psFogEllipsoidBackHDR);
EXTERN_SHADER(psFogEllipsoidFrontHDR);
EXTERN_SHADER(psFogVolumeBackHDRU32);
EXTERN_SHADER(psFogVolumeFrontHDRU32);
EXTERN_SHADER(psFogCuboidBackHDRU32);
EXTERN_SHADER(psFogCuboidFrontHDRU32);
EXTERN_SHADER(psFogCylinderBackHDRU32);
EXTERN_SHADER(psFogCylinderFrontHDRU32);
EXTERN_SHADER(psFogEllipsoidBackHDRU32);
EXTERN_SHADER(psFogEllipsoidFrontHDRU32);

namespace DynamicEffect
{
namespace Fog
{

C_Manager::C_Manager(C_Renderer* pRenderer, const U32 iTypeIndex) : I_DynamicEffectManager(pRenderer, iTypeIndex)
{
}

C_Manager::~C_Manager()
{
	m_Effects.Destroy();
}

bool C_Manager::Create()
{
	if (!Display::CreateVertexShaderFromFunc("vsFogVolumeFront", 0, &m_VertexShaders[E_Shader_DefaultFrontFaces], REF_SHADER(vsFogVolumeFront)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsFogVolumeBack", 0, &m_VertexShaders[E_Shader_DefaultBackFaces], REF_SHADER(vsFogVolumeBack)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsFogCuboidFront", 0, &m_VertexShaders[E_Shader_CuboidFrontFaces], REF_SHADER(vsFogCuboidFront)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsFogCuboidBack", 0, &m_VertexShaders[E_Shader_CuboidBackFaces], REF_SHADER(vsFogCuboidBack)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsFogCylinderFront", 0, &m_VertexShaders[E_Shader_CylinderFrontFaces], REF_SHADER(vsFogCylinderFront)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsFogCylinderBack", 0, &m_VertexShaders[E_Shader_CylinderBackFaces], REF_SHADER(vsFogCylinderBack)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsFogEllipsoidFront", 0, &m_VertexShaders[E_Shader_EllipsoidFrontFaces], REF_SHADER(vsFogEllipsoidFront)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsFogEllipsoidBack", 0, &m_VertexShaders[E_Shader_EllipsoidBackFaces], REF_SHADER(vsFogEllipsoidBack)))
		return false;

	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psFogVolumeFrontHDRU32", 0, &m_PixelShaders[E_Shader_DefaultFrontFaces], REF_SHADER(psFogVolumeFrontHDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogVolumeBackHDRU32", 0, &m_PixelShaders[E_Shader_DefaultBackFaces], REF_SHADER(psFogVolumeBackHDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogCuboidFrontHDRU32", 0, &m_PixelShaders[E_Shader_CuboidFrontFaces], REF_SHADER(psFogCuboidFrontHDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogCuboidBackHDRU32", 0, &m_PixelShaders[E_Shader_CuboidBackFaces], REF_SHADER(psFogCuboidBackHDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogCylinderFrontHDRU32", 0, &m_PixelShaders[E_Shader_CylinderFrontFaces], REF_SHADER(psFogCylinderFrontHDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogCylinderBackHDRU32", 0, &m_PixelShaders[E_Shader_CylinderBackFaces], REF_SHADER(psFogCylinderBackHDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogEllipsoidFrontHDRU32", 0, &m_PixelShaders[E_Shader_EllipsoidFrontFaces], REF_SHADER(psFogEllipsoidFrontHDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogEllipsoidBackHDRU32", 0, &m_PixelShaders[E_Shader_EllipsoidBackFaces], REF_SHADER(psFogEllipsoidBackHDRU32)))
				return false;
		}
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psFogVolumeFrontHDR", 0, &m_PixelShaders[E_Shader_DefaultFrontFaces], REF_SHADER(psFogVolumeFrontHDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogVolumeBackHDR", 0, &m_PixelShaders[E_Shader_DefaultBackFaces], REF_SHADER(psFogVolumeBackHDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogCuboidFrontHDR", 0, &m_PixelShaders[E_Shader_CuboidFrontFaces], REF_SHADER(psFogCuboidFrontHDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogCuboidBackHDR", 0, &m_PixelShaders[E_Shader_CuboidBackFaces], REF_SHADER(psFogCuboidBackHDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogCylinderFrontHDR", 0, &m_PixelShaders[E_Shader_CylinderFrontFaces], REF_SHADER(psFogCylinderFrontHDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogCylinderBackHDR", 0, &m_PixelShaders[E_Shader_CylinderBackFaces], REF_SHADER(psFogCylinderBackHDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogEllipsoidFrontHDR", 0, &m_PixelShaders[E_Shader_EllipsoidFrontFaces], REF_SHADER(psFogEllipsoidFrontHDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psFogEllipsoidBackHDR", 0, &m_PixelShaders[E_Shader_EllipsoidBackFaces], REF_SHADER(psFogEllipsoidBackHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psFogVolumeFront", 0, &m_PixelShaders[E_Shader_DefaultFrontFaces], REF_SHADER(psFogVolumeFront)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psFogVolumeBack", 0, &m_PixelShaders[E_Shader_DefaultBackFaces], REF_SHADER(psFogVolumeBack)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psFogCuboidFront", 0, &m_PixelShaders[E_Shader_CuboidFrontFaces], REF_SHADER(psFogCuboidFront)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psFogCuboidBack", 0, &m_PixelShaders[E_Shader_CuboidBackFaces], REF_SHADER(psFogCuboidBack)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psFogCylinderFront", 0, &m_PixelShaders[E_Shader_CylinderFrontFaces], REF_SHADER(psFogCylinderFront)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psFogCylinderBack", 0, &m_PixelShaders[E_Shader_CylinderBackFaces], REF_SHADER(psFogCylinderBack)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psFogEllipsoidFront", 0, &m_PixelShaders[E_Shader_EllipsoidFrontFaces], REF_SHADER(psFogEllipsoidFront)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psFogEllipsoidBack", 0, &m_PixelShaders[E_Shader_EllipsoidBackFaces], REF_SHADER(psFogEllipsoidBack)))
			return false;
	}

	for (U32 shaderNo = 0; shaderNo < E_Shader_Count; ++shaderNo)
	{
		m_psDeviceToWorld[shaderNo]	= m_PixelShaders[shaderNo].GetParameterArray(CP_RENDER_SETTINGS, "g_mToWorld", 4);
		m_psViewPosition[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_SETTINGS_4, "g_vViewPosition");
		m_psDepthScale[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_SETTINGS_5, "g_vDepthScale");
		m_psLuminance[shaderNo]		= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");
	}

	return true;
}

void C_Manager::Destroy()
{
	for (U32 shaderNo = 0; shaderNo < E_Shader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_PixelShaders[shaderNo]);
		Display::Device()->Destroy(&m_VertexShaders[shaderNo]);
	}
}

void C_Manager::AddEffect(I_VolumeEffect* pEffect)
{
	m_Effects.Insert(pEffect);
}

void C_Manager::RemoveEffect(I_VolumeEffect* pEffect)
{
	m_Effects.Erase(pEffect);
}

void C_Manager::CleanNode(const C_Node* pNode)
{
}

void C_Manager::CleanEffect(const I_DynamicEffect* pEffect)
{
	RemoveEffect((I_VolumeEffect*)pEffect);
}

E_Shader C_Manager::GetVertexShader(const E_Shape eShape, const E_Draw eDraw, const E_Detail eDetail)
{
	const U32 shaderNo = 2 * (eShape + 1) + eDetail;
	const U32 defaultNo = eDraw;
	return static_cast<E_Shader>((eDetail == E_Detail_PerVertex) ? shaderNo : defaultNo);
}

E_Shader C_Manager::GetPixelShader(const E_Shape eShape, const E_Draw eDraw, const E_Detail eDetail)
{
	const U32 shaderNo = 2 * (eShape + 1) + eDetail;
	const U32 defaultNo = eDraw;
	return static_cast<E_Shader>((eDetail == E_Detail_PerPixel) ? shaderNo : defaultNo);
}

void C_Manager::SetVertexShader(const E_Shader eShader)
{
	Display::Device()->SetVertexShader(&m_VertexShaders[eShader]);
}

void C_Manager::SetPixelShader(const E_Shader eShader)
{
	Display::Device()->SetPixelShader(&m_PixelShaders[eShader]);
}

void C_Manager::SetShaderConstants(const E_Shader eShader)
{
	Display::C_Matrix mTemp;
	Display::MatrixSetIdentity(&mTemp);
	mTemp._11 = +2.0f;	mTemp._22 = -2.0f;
	mTemp._41 = -1.0f;	mTemp._42 = +1.0f;

	const Maths::HMat44 mTexture = Maths::Load<Maths::HMat44>(&mTemp);
	const Maths::HMat44 mDeviceToWorld = Display::Device()->GetViewSettings3D().GetWorldToDevice().Inverse();
	
	Display::C_Matrix deviceToWorldParam;
	Maths::Store(mTexture * mDeviceToWorld, &deviceToWorldParam);
	Display::Device()->SetPixelShaderParameterArrayF(m_psDeviceToWorld[eShader], (F32*)&deviceToWorldParam);
	const Display::C_Float4 viewPositionParam = Display::Device()->GetViewSettings3D().GetViewPosition();
	Display::Device()->SetPixelShaderParameterF(m_psViewPosition[eShader], (F32*)&viewPositionParam);
	const Display::C_Float4 depthScaleParam = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::Device()->SetPixelShaderParameterF(m_psDepthScale[eShader], (F32*)&depthScaleParam);
	const Display::C_Float4 gainParam = Render::C_RenderState::Instance().ShaderHDR() ? Render::C_RenderState::Instance().HDR()->GetShaderParameters() : Render::C_RenderState::Instance().GetGainShaderParameters();
	Display::Device()->SetPixelShaderParameterF(m_psLuminance[eShader], (F32*)&gainParam);
}

Maths::HVec4 C_Manager::EvaluateFogAt(const Maths::HVec3& vPoint)
{
	Maths::HF32 vZero(Maths::Constants<Maths::HF32>::Zero());
	Maths::HVec4 vDensity(vZero, vZero, vZero, vZero);
	for (Effects::Iterator it = m_Effects.Begin(); it != m_Effects.End(); ++it)
	{
		I_VolumeEffect* pEffect = it->GetValue();
		vDensity += pEffect->EvaluateFogAt(vPoint);
	}
	return vDensity;
}

}; // namespace Fog
}; // namespace DynamicEffect
