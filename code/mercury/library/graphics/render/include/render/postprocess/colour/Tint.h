#ifndef RENDER_POSTPROCESS_TINT_H
#define RENDER_POSTPROCESS_TINT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

class C_Tint : public I_PostProcess
{
public:

	C_Tint( C_Manager& manager ) : I_PostProcess( manager ), m_vColour(MathsStorage::Float3(1.0f, 1.0f, 1.0f)) {}
	~C_Tint() {}

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}
	void Update() {}

	void Display() const;
	void SetColour(const MathsStorage::Float3& vColour);
	
private:
	MathsStorage::Float3					m_vColour;
	Display::C_PixelShader				m_Shader;
	Display::C_PixelShaderParameter		m_Parameter;

};

INLINE void C_Tint::SetColour(const MathsStorage::Float3& vColour)
{
	m_vColour = vColour;
}

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_GREYSCALE_H
