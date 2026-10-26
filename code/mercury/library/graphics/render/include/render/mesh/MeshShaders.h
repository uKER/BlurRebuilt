#ifndef RENDER_MESHSHADERS_H
#define RENDER_MESHSHADERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Mesh
{

enum E_VertexShader
{
	E_VertexShader_Default = 0,
	E_VertexShader_Albedo,
	E_VertexShader_Detail,
	E_VertexShader_StandardShadowMap,
	E_VertexShader_VarianceShadowMap,
	E_VertexShader_Count,
};

enum E_PixelShader
{
	E_PixelShader_Default = 0,
	E_PixelShader_Albedo,
	E_PixelShader_Detail,
	E_PixelShader_StandardShadowMap,
	E_PixelShader_VarianceShadowMap,
	E_PixelShader_Count,
};

class C_Shaders
{
public:

	C_Shaders();
	~C_Shaders();

	bool CreateShaders();
	void DestroyShaders();

public:

	Display::C_VertexShader	m_VertexShader[E_VertexShader_Count];
	Display::C_PixelShader	m_PixelShader[E_PixelShader_Count];
};

}; // namespace Mesh
}; // namespace Render

#endif // RENDER_MESHSHADERS_H
