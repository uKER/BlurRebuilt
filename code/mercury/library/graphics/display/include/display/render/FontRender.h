#ifndef DISPLAY_RENDER_FONT_RENDER_H
#define DISPLAY_RENDER_FONT_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/Device.h"

namespace Display
{

//! E_FontRenderAlign

enum E_FontRenderAlign
{
	E_FontRenderAlign_Lt,
	E_FontRenderAlign_Ct,
	E_FontRenderAlign_Rt,
};

//! C_FontVertex

struct C_FontVertex
{
	F32 x, y;
	U32 c;
	F32 u, v;
};

//! C_FontRender

class C_FontRender : public I_Renderer
{
public:

	C_FontRender();
	~C_FontRender();

	bool IsCreated() const;

	bool OnCreate();
	void OnDestroy();

	void Draw(const char* pChars, F32 x, F32 y, F32 w, F32 h, const Maths::RGBA col, const E_FontRenderAlign align = E_FontRenderAlign_Lt, const bool leading = false);
	void Draw2D(const char* pChars, const Maths::HVec2& pos, const Maths::HVec2& dim, const Maths::RGBA col, const E_FontRenderAlign align = E_FontRenderAlign_Lt);
	void Draw3D(const char* pChars, const Maths::HVec3& pos, const F32 size, const Maths::RGBA col, const E_FontRenderAlign align = E_FontRenderAlign_Lt);
	
	F32 GetSpacing(const char* pChars, const F32 w) const;

	bool Transform3Dto2D(const Maths::HVec3& worldPos, const F32 worldSize, Maths::HVec2& windowPos, Maths::HVec2& windowDim);

public:

	#ifdef SHADERS_CG_VS
	CGparameter				m_cgClipVector;
	#endif

	C_VertexDeclaration*	m_pVertexDeclaration;
	C_VertexShader			m_VertexShader;
	C_PixelShader			m_PixelShader;
	C_Texture2D				m_Texture;

private:

	bool					m_Created;
};

extern C_FontRender* g_pFontRender;
void FontRender(C_FontRender* pFontRender);
C_FontRender* FontRender();
bool FontRenderExists();

}; // namespace Display

#include "Display/Render/FontRender.inl"

#endif // DISPLAY_RENDER_FONT_RENDER_H
