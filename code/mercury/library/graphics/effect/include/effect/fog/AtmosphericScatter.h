#ifndef EFFECT_FOG_ATMOSPHERICSCATTER_H
#define EFFECT_FOG_ATMOSPHERICSCATTER_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Effect
{
namespace Fog
{

class C_AtmosphericScatter : public Display::I_Renderer
{
public:

	C_AtmosphericScatter();
	~C_AtmosphericScatter();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroy();
	void OnDestroyVolatile();

	void Render();

public:

	void SetSunDirection(const Display::C_Float4& vSunDirection) { m_vSunDirection = vSunDirection; }

private:

	Display::C_PixelShader			m_PixelShader;

	Display::C_PixelShaderParameter	m_LuminanceParam;
	Display::C_PixelShaderParameterArray m_DeviceToWorldParam;
	Display::C_PixelShaderParameter m_ViewPositionParam;
	Display::C_PixelShaderParameter m_FogSettingsParam;
	Display::C_PixelShaderParameter m_FogColourParam;
	Display::C_PixelShaderParameter m_DepthScaleParam;
	Display::C_PixelShaderParameter	m_SunDirectionParam;
	Display::C_PixelShaderParameter	m_SunIntensityParam;
	Display::C_PixelShaderParameter	m_MieGParam;

	Display::C_Float4				m_vSunDirection;
};

}; // namespace Fog
}; // namespace Effect

#endif // EFFECT_FOG_ATMOSPHERICSCATTER_H
