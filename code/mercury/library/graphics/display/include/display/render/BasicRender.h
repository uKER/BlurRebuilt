#ifndef DISPLAY_RENDER_BASIC_RENDER_H
#define DISPLAY_RENDER_BASIC_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/Device.h"

namespace Display
{

enum E_BasicRenderVertexShader
{
	E_BasicRenderVertexShader_Coloured2D = 0,
	E_BasicRenderVertexShader_Coloured3D,
	E_BasicRenderVertexShader_Textured2D,
	E_BasicRenderVertexShader_Textured3D,
	E_BasicRenderVertexShader_Count
};

enum E_BasicRenderPixelShader
{
	E_BasicRenderPixelShader_Coloured = 0,
	E_BasicRenderPixelShader_Textured,
	E_BasicRenderPixelShader_Count
};

struct C_BasicRenderColouredVertex2D
{
	F32 x, y;
	U32 c;
};

struct C_BasicRenderColouredVertex3D
{
	F32 x, y, z;
	F32 r, g, b, a;
};

struct C_BasicRenderTexturedVertex2D
{
	F32 x, y;
	U32 c;
	F32 u, v;
};

struct C_BasicRenderTexturedVertex3D
{
	F32 x, y, z;
	F32 r, g, b, a;
	F32 u, v;
};

class C_BasicRender : public I_Renderer
{
public:

	C_BasicRender();
	~C_BasicRender();

	bool IsCreated() const;

	bool OnCreate();
	void OnDestroy();

	void DrawColoured2DLine(const F32 x1, const F32 y1, const F32 x2, const F32 y2, const Maths::RGBA col1, const Maths::RGBA col2);
	void DrawColoured2DLine(const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA col1, const Maths::RGBA col2);
	void DrawColoured2DLine(const F32 x1, const F32 y1, const F32 x2, const F32 y2, const Maths::RGBA col);
	void DrawColoured2DLine(const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA col);
	
	void DrawColoured2DTri(const F32 x1, const F32 y1, const F32 x2, const F32 y2, const F32 x3, const F32 y3, const Maths::RGBA col);

	void DrawColoured2DRect(const F32 x, const F32 y, const F32 w, const F32 h, const Maths::RGBA col, const E_BlendMode blendMode = E_BlendMode_Alpha);
	void DrawColoured2DRect(const Maths::HVec2& pos, const Maths::HVec2& dim, const Maths::RGBA col);
	
	void DrawTextured2DTri(const F32 x1, const F32 y1, const F32 x2, const F32 y2, const F32 x3, const F32 y3, const C_Float2 uvs[3], const Maths::RGBA col, const C_Texture* pTexture, C_PixelShader* pPixelShader);

	void DrawTextured2DRect(const F32 x, const F32 y, const F32 w, const F32 h, const Maths::RGBA col, const C_Texture* pTexture, C_PixelShader* pPixelShader = NULL);
	void DrawTextured2DRect(const Maths::HVec2& pos, const Maths::HVec2& dim, const Maths::RGBA col, const C_Texture* pTexture, C_PixelShader* pPixelShader = NULL);
	void DrawTextured2DRect(const F32 x, const F32 y, const F32 w, const F32 h, const C_Float2* uv, const Maths::RGBA col, const C_Texture* pTexture, C_PixelShader* pPixelShader = NULL);
	
	void DrawColoured3DLine(const F32 x1, const F32 y1, const F32 z1, const F32 x2, const F32 y2, const F32 z2, const Maths::RGBA col); 
	void DrawColoured3DLine(const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA col);
	
	void DrawColoured3DBox(const F32 x1, const F32 y1, const F32 z1, const F32 x2, const F32 y2, const F32 z2, const Maths::RGBA col); 
	void DrawColoured3DBox(const Maths::HVec3& pos, const Maths::HVec3& dim, const Maths::RGBA col); 
	void DrawColoured3DBox(const Maths::HVec3& pos, const Maths::HVec3& dim, const Maths::C_RGBA_F32& col); 
	
	void DrawTextured3DTriangle(const Maths::HVec3& pos1, const Maths::HVec3& pos2, const Maths::HVec3& pos3, const C_Float2 uv[3], const Maths::C_RGBA_F32& col, const C_Texture* pTexture );
	
	void DrawColoured2DCircle(const F32 x0, const F32 y0, const F32 r, const Maths::RGBA col);

	void DrawColoured3DCircle(const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA col);
	void DrawColoured3DSphere(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA col);

	void BindColoured2D(const E_BlendMode blendMode = E_BlendMode_Alpha);
	void BindColoured3D(const MathsStorage::Matrix44* pWorldMatrix = NULL);

	void BindTextured2D(const C_Texture* pTexture, C_PixelShader* pPixelShader = NULL);
	void BindTextured3D(const C_Texture* pTexture, E_TextureAddress textureAddress = E_TextureAddress_Clamp, MathsStorage::Matrix44* pWorldMatrix = NULL);

public:

#ifdef SHADERS_CG_VS
	CGparameter				m_cgClipVector[E_BasicRenderVertexShader_Count];
	CGparameter				m_cgWorldMatrix[E_BasicRenderVertexShader_Count];
	CGparameter				m_cgViewProjMatrix[E_BasicRenderVertexShader_Count];
#endif

	C_VertexDeclaration*	m_pVertexDeclaration[E_BasicRenderVertexShader_Count];
	C_VertexShader			m_VertexShader[E_BasicRenderVertexShader_Count];
	C_PixelShader			m_PixelShader[E_BasicRenderPixelShader_Count];

private:

	bool					m_Created;
};

extern C_BasicRender* g_pBasicRender;
void BasicRender(C_BasicRender* pBasicRender);
C_BasicRender* BasicRender();
bool BasicRenderExists();

}; // namespace Display

#include "Display/Render/BasicRender.inl"

#endif // DISPLAY_RENDER_BASIC_RENDER_H
