#ifndef EFFECT_OCEAN_OCEAN_H
#define EFFECT_OCEAN_OCEAN_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Render/Geometry/InfiniteGrid.h"
#include "Effect/Ocean/FFTGPU.h"

namespace Effect
{
namespace Ocean
{

class C_Ocean : public Display::I_Renderer
{

public:

	C_Ocean();
	~C_Ocean();

	bool OnCreate();
	void OnDestroy();

	void Update(const F32 fTimeElapsed, const F32 fTimeStep);
	void PreRender();
	void Render();

private:

	Render::Geometry::C_InfiniteGrid	m_GeometryPolicy;
	Waves::C_FFTGPU						m_WavesPolicy;

	Display::C_VertexShader				m_VertexShader;
	Display::C_PixelShader				m_PixelShader;

	Display::C_PixelShaderParameter		m_LuminanceParam;

	F32									m_fHeight;
	F32									m_fTimeElapsed;

};

}; // namespace Ocean
}; // namespace Effect

#endif
