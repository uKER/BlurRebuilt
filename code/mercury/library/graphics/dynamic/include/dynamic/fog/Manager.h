#ifndef RENDER_DYNAMICEFFECT_FOG_MANAGER_H
#define RENDER_DYNAMICEFFECT_FOG_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Fog/Enums.h"
#include "Dynamic/ManagerInterface.h"

namespace DynamicEffect
{
namespace Fog
{

class I_VolumeEffect;

class C_Manager : public I_DynamicEffectManager
{
public:

	C_Manager(C_Renderer* pRenderer, const U32 iTypeIndex);
	~C_Manager();

	bool Create();
	void Destroy();

	void AddEffect(I_VolumeEffect* pEffect);
	void RemoveEffect(I_VolumeEffect* pEffect);

	E_Shader GetVertexShader(const E_Shape eShape, const E_Draw eDraw, const E_Detail eDetail);
	E_Shader GetPixelShader(const E_Shape eShape, const E_Draw eDraw, const E_Detail eDetail);
	
	void SetVertexShader(const E_Shader eShader);
	void SetPixelShader(const E_Shader eShader);
	void SetShaderConstants(const E_Shader eShader);

public:

	// CPU Fog
	Maths::HVec4 EvaluateFogAt(const Maths::HVec3& vPoint);

protected:
	
	// interface methods
	virtual void CleanNode(const C_Node* pNode);
	virtual void CleanEffect(const I_DynamicEffect* pEffect);

protected:

	typedef Ut::UtSet<I_VolumeEffect*> Effects;
	Effects m_Effects;

	Display::C_VertexShader					m_VertexShaders[E_Shader_Count];
	Display::C_PixelShader					m_PixelShaders[E_Shader_Count];
	
	Display::C_PixelShaderParameterArray	m_psDeviceToWorld[E_Shader_Count];
	Display::C_PixelShaderParameter			m_psViewPosition[E_Shader_Count];
	Display::C_PixelShaderParameter			m_psDepthScale[E_Shader_Count];
	Display::C_PixelShaderParameter			m_psLuminance[E_Shader_Count];

};

}; // namespace Fog
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_FOG_MANAGER_H
