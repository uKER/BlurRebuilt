#ifndef DISPLAY_RENDER_RESOURCE_RENDER_H
#define DISPLAY_RENDER_RESOURCE_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

enum E_ResourceRender
{
	E_ResourceRender_Head = 0,
	E_ResourceRender_Texture,
	E_ResourceRender_Surface,
	E_ResourceRender_VertexDeclaration,
	E_ResourceRender_VertexBuffer,
	E_ResourceRender_IndexBuffer,
	E_ResourceRender_VertexShader,
	E_ResourceRender_PixelShader,
	E_ResourceRender_Tail,
};

class C_ResourceRender
{
public:

	C_ResourceRender();
	~C_ResourceRender();

	void Draw();

private:

	void DrawTexture();
	void DrawSurface();
	void DrawVertexDeclaration();
	void DrawVertexBuffer();
	void DrawIndexBuffer();
	void DrawVertexShader();
	void DrawPixelShader();
};

extern C_ResourceRender* g_pResourceRender;
void ResourceRender(C_ResourceRender* pResourceRender);
C_ResourceRender* ResourceRender();
const bool ResourceRenderExists();

}; // namespace Display

#include "Display/Render/ResourceRender.inl"

#endif // DISPLAY_RENDER_RESOURCE_RENDER_H
