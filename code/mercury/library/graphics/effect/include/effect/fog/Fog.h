#ifndef EFFECT_FOG_FOG_H
#define EFFECT_FOG_FOG_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Effect
{
namespace Fog
{

class C_Fog : public Display::I_Renderer
{
public:

	C_Fog();
	~C_Fog();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroy();
	void OnDestroyVolatile();

	void Render();

private:

	Display::C_PixelShader	m_PixelShader;

	Display::C_PixelShaderParameter	m_LuminanceParam;
	Display::C_PixelShaderParameterArray m_DeviceToWorldParam;
	Display::C_PixelShaderParameter m_ViewPositionParam;
	Display::C_PixelShaderParameter m_FogSettingsParam;
	Display::C_PixelShaderParameter m_FogColourParam;
	Display::C_PixelShaderParameter m_DepthScaleParam;

};

}; // namespace Fog
}; // namespace Effect

#endif // EFFECT_FOG_FOG_H
