#ifndef EFFECT_RAIN_SHEET_H
#define EFFECT_RAIN_SHEET_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Parameters.h"
#include <core\utility\Singleton.h>

namespace Effect
{

namespace Rain
{

class C_SheetRenderer : public Display::I_Renderer, public Ut::Singleton<C_SheetRenderer, Ut::CreateExternal>
{
public:

	C_SheetRenderer();
	~C_SheetRenderer();

	bool OnCreate();
	void OnDestroy();

	bool Initialise();
	
	void Update(const F32 fTime, const bool bPaused);

	void SetParameters(const SheetParameters& parameters);
	const SheetParameters& GetParameters() const { return m_Parameters; }

	void Display() const;
	
private:
	void Build();
	void SetDefaults();

	U32									m_iVertexCount;
	U32									m_iIndexCount;
	Display::C_VertexBuffer			m_VertexBuffer;
	Display::C_IndexBuffer			m_IndexBuffer;

	Display::C_Texture2D				m_Texture;
	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShader;

	SheetParameters						m_Parameters;

	F32									m_fHeight[4];
	F32									m_fWind[4];
};

} // namespace Rain
} // namespace Render

#endif // EFFECT_RAIN_DROP_H
