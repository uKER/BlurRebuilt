#include "stdafx.h"
#include "Display/Render/BasicRender.h"
#include "Display/Shaders/Constants.h"

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Display/Shaders/Render/PS3/vsBasicRenderColoured2D.shader"
#include "Display/Shaders/Render/PS3/vsBasicRenderColoured3D.shader"
#include "Display/Shaders/Render/PS3/vsBasicRenderTextured2D.shader"
#include "Display/Shaders/Render/PS3/vsBasicRenderTextured3D.shader"
#include "Display/Shaders/Render/PS3/psBasicRenderColoured.shader"
#include "Display/Shaders/Render/PS3/psBasicRenderTextured.shader"
#endif
#endif

EXTERN_SHADER(vsBasicRenderColoured2D);
EXTERN_SHADER(vsBasicRenderColoured3D);
EXTERN_SHADER(vsBasicRenderTextured2D);
EXTERN_SHADER(vsBasicRenderTextured3D);
EXTERN_SHADER(psBasicRenderColoured);
EXTERN_SHADER(psBasicRenderTextured);

namespace Display
{

C_BasicRender::C_BasicRender() : I_Renderer("BasicRender"), m_Created(false)
{
	for(S32 i = 0; i < E_BasicRenderVertexShader_Count; ++i)
		m_pVertexDeclaration[i] = NULL;

	assert(g_pBasicRender == NULL);
	g_pBasicRender = this;
}

C_BasicRender::~C_BasicRender()
{
	DestroyRenderer();

	assert(g_pBasicRender == this);
	g_pBasicRender = NULL;
}

bool C_BasicRender::OnCreate()
{
	assert(!m_Created);

	RESOURCE_MEMORY_ID("BasicRender");

	if(!m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured2D])
	{
		C_VertexFormat colouredVertexFormat2D;
		colouredVertexFormat2D.AddElement(0, E_VertexElementType_F32_2, E_VertexElementUsage_Position, 0);
		colouredVertexFormat2D.AddElement(0, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0);
		m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured2D] = Device()->CreateVertexDeclaration(colouredVertexFormat2D);
	}

	if(!m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured3D])
	{
		C_VertexFormat colouredVertexFormat3D;
		colouredVertexFormat3D.AddElement(0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0);
		colouredVertexFormat3D.AddElement(0, E_VertexElementType_F32_4, E_VertexElementUsage_Colour, 0);
		m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured3D] = Device()->CreateVertexDeclaration(colouredVertexFormat3D);
	}

	if(!m_pVertexDeclaration[E_BasicRenderVertexShader_Textured2D])
	{
		C_VertexFormat texturedVertexFormat2D;
		texturedVertexFormat2D.AddElement(0, E_VertexElementType_F32_2, E_VertexElementUsage_Position, 0);
		texturedVertexFormat2D.AddElement(0, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0);
		texturedVertexFormat2D.AddElement(0, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0);
		m_pVertexDeclaration[E_BasicRenderVertexShader_Textured2D] = Device()->CreateVertexDeclaration(texturedVertexFormat2D);
	}

	if(!m_pVertexDeclaration[E_BasicRenderVertexShader_Textured3D])
	{
		C_VertexFormat texturedVertexFormat3D;
		texturedVertexFormat3D.AddElement(0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0);
		texturedVertexFormat3D.AddElement(0, E_VertexElementType_F32_4, E_VertexElementUsage_Colour, 0);
		texturedVertexFormat3D.AddElement(0, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0);
		m_pVertexDeclaration[E_BasicRenderVertexShader_Textured3D] = Device()->CreateVertexDeclaration(texturedVertexFormat3D);
	}

#if defined(DEVICE_PCD3D11) || defined(DEVICE_PS3GCM) // [Blur1PCDX11] embedded built-ins
	if(!CreateVertexShaderFromFunc("Shaders/vsBasicRenderColoured2D", 0, &m_VertexShader[E_BasicRenderVertexShader_Coloured2D], REF_SHADER(vsBasicRenderColoured2D)))
		return false;
	if(!CreateVertexShaderFromFunc("Shaders/vsBasicRenderColoured3D", 0, &m_VertexShader[E_BasicRenderVertexShader_Coloured3D], REF_SHADER(vsBasicRenderColoured3D)))
		return false;
	if(!CreateVertexShaderFromFunc("Shaders/vsBasicRenderTextured2D", 0, &m_VertexShader[E_BasicRenderVertexShader_Textured2D], REF_SHADER(vsBasicRenderTextured2D)))
		return false;
	if(!CreateVertexShaderFromFunc("Shaders/vsBasicRenderTextured3D", 0, &m_VertexShader[E_BasicRenderVertexShader_Textured3D], REF_SHADER(vsBasicRenderTextured3D)))
		return false;
	if(!CreatePixelShaderFromFunc("Shaders/psBasicRenderColoured", 0, &m_PixelShader[E_BasicRenderPixelShader_Coloured], REF_SHADER(psBasicRenderColoured)))
		return false;
	if(!CreatePixelShaderFromFunc("Shaders/psBasicRenderTextured", 0, &m_PixelShader[E_BasicRenderPixelShader_Textured], REF_SHADER(psBasicRenderTextured)))
		return false;
#else
	if(!CreateVertexShaderFromFile("Shaders/vsBasicRenderColoured2D", 0, &m_VertexShader[E_BasicRenderVertexShader_Coloured2D]))
		return false;
	if(!CreateVertexShaderFromFile("Shaders/vsBasicRenderColoured3D", 0, &m_VertexShader[E_BasicRenderVertexShader_Coloured3D]))
		return false;
	if(!CreateVertexShaderFromFile("Shaders/vsBasicRenderTextured2D", 0, &m_VertexShader[E_BasicRenderVertexShader_Textured2D]))
		return false;
	if(!CreateVertexShaderFromFile("Shaders/vsBasicRenderTextured3D", 0, &m_VertexShader[E_BasicRenderVertexShader_Textured3D]))
		return false;
	if(!CreatePixelShaderFromFile("Shaders/psBasicRenderColoured", 0, &m_PixelShader[E_BasicRenderPixelShader_Coloured]))
		return false;
	if(!CreatePixelShaderFromFile("Shaders/psBasicRenderTextured", 0, &m_PixelShader[E_BasicRenderPixelShader_Textured]))
		return false;
#endif

#ifdef SHADERS_CG_VS
	for(U32 i = 0; i < E_BasicRenderVertexShader_Count; i++)
	{
		m_cgClipVector[i] = Device()->GetShaderParameter(m_VertexShader[i].m_cgProgram, "g_ClipVector");
		m_cgWorldMatrix[i] = Device()->GetShaderParameter(m_VertexShader[i].m_cgProgram, "g_WorldMatrix");
		m_cgViewProjMatrix[i] = Device()->GetShaderParameter(m_VertexShader[i].m_cgProgram, "g_ViewProjMatrix");
	}
#endif

	m_Created = true;

	return true;
}

void C_BasicRender::OnDestroy()
{
	Device()->Destroy(&m_VertexShader[E_BasicRenderVertexShader_Coloured2D]);
	Device()->Destroy(&m_VertexShader[E_BasicRenderVertexShader_Coloured3D]);
	Device()->Destroy(&m_VertexShader[E_BasicRenderVertexShader_Textured2D]);
	Device()->Destroy(&m_VertexShader[E_BasicRenderVertexShader_Textured3D]);
	Device()->Destroy(&m_PixelShader[E_BasicRenderPixelShader_Coloured]);
	Device()->Destroy(&m_PixelShader[E_BasicRenderPixelShader_Textured]);

	m_Created = false;
}

void C_BasicRender::BindColoured2D(const E_BlendMode blendMode)
{
	Device()->SetBlendMode(blendMode);
	Device()->SetVertexShader(&m_VertexShader[E_BasicRenderVertexShader_Coloured2D]);
	Device()->SetPixelShader(&m_PixelShader[E_BasicRenderPixelShader_Coloured]);

#ifdef SHADERS_CG_VS
	Device()->SetVertexShaderParameter(m_cgClipVector[E_BasicRenderVertexShader_Coloured2D], (F32*)&Device()->m_ClipVector);
#endif
}

void C_BasicRender::BindColoured3D(const MathsStorage::Matrix44* pWorldMatrix)
{
	Device()->SetBlendMode(E_BlendMode_Alpha);
	Device()->SetVertexShader(&m_VertexShader[E_BasicRenderVertexShader_Coloured3D]);
	Device()->SetPixelShader(&m_PixelShader[E_BasicRenderPixelShader_Coloured]);	

	if(pWorldMatrix)
	{
		Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (const F32*)pWorldMatrix, 4);
	
	#ifdef SHADERS_CG_VS
		Device()->SetVertexShaderParameter(m_cgWorldMatrix[E_BasicRenderVertexShader_Coloured3D], (F32*)pWorldMatrix);
	#endif
	}
	else
	{
		Display::C_Matrix worldMatrix;
		MatrixSetIdentity(&worldMatrix);
		Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (const F32*)&worldMatrix, 4);
	
	#ifdef SHADERS_CG_VS
		Device()->SetVertexShaderParameter(m_cgWorldMatrix[E_BasicRenderVertexShader_Coloured3D], (F32*)&worldMatrix);
	#endif
	}

#ifdef SHADERS_CG_VS
	Device()->SetVertexShaderParameter(m_cgViewProjMatrix[E_BasicRenderVertexShader_Coloured3D], (F32*)&Device()->GetViewProjMatrix());
#endif
}

void C_BasicRender::BindTextured2D(const C_Texture* pTexture, C_PixelShader* pPixelShader)
{
	Device()->SetBlendMode(E_BlendMode_Alpha);
	Device()->SetVertexShader(&m_VertexShader[E_BasicRenderVertexShader_Textured2D]);
	Device()->SetPixelShader(pPixelShader ? pPixelShader : &m_PixelShader[E_BasicRenderPixelShader_Textured]);

#ifdef SHADERS_CG_VS
	Device()->SetVertexShaderParameter(m_cgClipVector[E_BasicRenderVertexShader_Textured2D], (F32*)&Device()->m_ClipVector);
#endif

	if(pTexture && pTexture->IsCreated())
	{
		Device()->SetTexture(0, pTexture);
		Device()->SetTextureAddressUVW(0, E_TextureAddress_Clamp, E_TextureAddress_Clamp, E_TextureAddress_Clamp);

		#if PLATFORM_PS3
		E_Format format = GetFormatFromTextureFormat(pTexture->GetFormat());
		if(format == E_Format_R32F)
			Device()->SetTextureFilter(0, E_TextureFilter_Point);
		else
		#endif
			Device()->SetTextureFilterMagMinMip(0, E_TextureFilter_Linear, E_TextureFilter_Linear, E_TextureFilter_Linear);
	}
}

void C_BasicRender::BindTextured3D(const C_Texture* pTexture, E_TextureAddress textureAddress, MathsStorage::Matrix44* pWorldMatrix)
{
	Device()->SetBlendMode(E_BlendMode_Alpha);
	Device()->SetVertexShader(&m_VertexShader[E_BasicRenderVertexShader_Textured3D]);
	Device()->SetPixelShader(&m_PixelShader[E_BasicRenderPixelShader_Textured]);

	if(pWorldMatrix)
	{
		Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)pWorldMatrix, 4);
	
	#ifdef SHADERS_CG_VS
		Device()->SetVertexShaderParameter(m_cgWorldMatrix[E_BasicRenderVertexShader_Coloured3D], (F32*)pWorldMatrix);
	#endif
	}
	else
	{
		Display::C_Matrix worldMatrix;
		MatrixSetIdentity(&worldMatrix);
		Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);
	
	#ifdef SHADERS_CG_VS
		Device()->SetVertexShaderParameter(m_cgWorldMatrix[E_BasicRenderVertexShader_Coloured3D], (F32*)&worldMatrix);
	#endif
	}

#ifdef SHADERS_CG_VS
	Device()->SetVertexShaderParameter(m_cgViewProjMatrix[E_BasicRenderVertexShader_Coloured3D], (F32*)&Device()->GetViewProjMatrix());
#endif

	if(pTexture && pTexture->IsCreated())
	{
		Device()->SetTexture(0, pTexture);
		Device()->SetTextureAddressUVW(0, textureAddress, textureAddress, textureAddress);

		#if PLATFORM_PS3
		E_Format format = GetFormatFromTextureFormat(pTexture->GetFormat());
		if(format == E_Format_R32F)
			Device()->SetTextureFilterMagMinMip(0, E_TextureFilter_Point, E_TextureFilter_Point, E_TextureFilter_None);
		else
		#endif
			Device()->SetTextureFilterMagMinMip(0, E_TextureFilter_Linear, E_TextureFilter_Linear, E_TextureFilter_Linear);
	}
}

void C_BasicRender::DrawColoured2DLine(const F32 x1, const F32 y1, const F32 x2, const F32 y2, const Maths::RGBA col1, const Maths::RGBA col2)
{
	if(!m_Created)
		return;

	PERF_EVENT("DrawColoured2DLine");

	BindColoured2D();

	const U32 vertexCount = 2;
	C_BasicRenderColouredVertex2D* pVertex = (C_BasicRenderColouredVertex2D*)Device()->BeginVertices(E_PrimitiveType_LineList, vertexCount, sizeof(C_BasicRenderColouredVertex2D), m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured2D], true); // This can fail

	if(!pVertex)
		return;

	const U32 colour1 = GPU_RGBA(col1.R(), col1.G(), col1.B(), col1.A());
	const U32 colour2 = GPU_RGBA(col2.R(), col2.G(), col2.B(), col2.A());

	pVertex->x = x1;
	pVertex->y = y1;
	pVertex->c = colour1;
	pVertex++;

	pVertex->x = x2;
	pVertex->y = y2;
	pVertex->c = colour2;
	pVertex++;

	Device()->EndVertices(pVertex);
}

void C_BasicRender::DrawColoured2DLine(const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA col1, const Maths::RGBA col2)
{
	DrawColoured2DLine(start.X(), start.Y(), end.X(), end.Y(), col1, col2);
}

void C_BasicRender::DrawColoured2DLine(const F32 x1, const F32 y1, const F32 x2, const F32 y2, const Maths::RGBA col)
{
	DrawColoured2DLine(x1, y1, x2, y2, col, col);
}

void C_BasicRender::DrawColoured2DLine(const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA col)
{
	DrawColoured2DLine(start.X(), start.Y(), end.X(), end.Y(), col, col);
}

void C_BasicRender::DrawColoured2DTri(const F32 x1, const F32 y1, const F32 x2, const F32 y2, const F32 x3, const F32 y3, const Maths::RGBA col)
{
	if(!m_Created)
		return;

	PERF_EVENT("DrawColoured2DTri");

	BindColoured2D();

	const U32 vertexCount = 3;
	C_BasicRenderColouredVertex2D* pVertex = (C_BasicRenderColouredVertex2D*)Device()->BeginVertices(E_PrimitiveType_TriangleList, vertexCount, sizeof(C_BasicRenderColouredVertex2D), m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured2D], true); // This can fail

	if(!pVertex)
		return;

	const U32 colour = GPU_RGBA(col.R(), col.G(), col.B(), col.A());

	pVertex->x = x1;
	pVertex->y = y1;
	pVertex->c = colour;
	pVertex++;

	pVertex->x = x2;
	pVertex->y = y2;
	pVertex->c = colour;
	pVertex++;

	pVertex->x = x3;
	pVertex->y = y3;
	pVertex->c = colour;
	pVertex++;

	Device()->EndVertices(pVertex);
}

void C_BasicRender::DrawColoured2DCircle(const F32 x, const F32 y, const F32 radius, const Maths::RGBA col)
{
	if(!m_Created)
		return;

	PERF_EVENT("DrawColoured2DCircle");

	BindColoured2D();

	const S32 pointCount = 32;
	const F32 deltaTheta = (Maths::Pi * 2.f) / (F32)pointCount;
	
	const S32 vertexCount = 1 + pointCount;

	C_BasicRenderColouredVertex2D* pVertex = (C_BasicRenderColouredVertex2D*)Device()->BeginVertices(E_PrimitiveType_LineStrip, vertexCount, sizeof(C_BasicRenderColouredVertex2D), m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured2D]);

	if(!pVertex)
		return;

	Maths::HVec2 centre(x, y);
	Maths::HVec2 u(1.f, 0.f);
	Maths::HVec2 v(0.f, 1.f);

	const U32 colour = GPU_RGBA(col.R(), col.G(), col.B(), col.A());

	for(S32 i=0; i<=pointCount; i++)
	{
		const F32 theta = (F32)i * deltaTheta;
		Maths::SinCosAlpha angles(theta);
		const Maths::HVec2 pos = centre + (u * angles.SinAlpha() + v * angles.CosAlpha()) * radius;
        
		pVertex->x = pos.X();
		pVertex->y = pos.Y();
		pVertex->c = colour;
		pVertex++;
	}

	Device()->EndVertices();
}

void C_BasicRender::DrawColoured2DRect(const F32 x, const F32 y, const F32 w, const F32 h, const Maths::RGBA col, const E_BlendMode blendMode)
{
	if(!m_Created)
		return;

	PERF_EVENT("DrawColoured2DRect");

	BindColoured2D(blendMode);

	const U32 vertexCount = 4;
	C_BasicRenderColouredVertex2D* pVertex = (C_BasicRenderColouredVertex2D*)Device()->BeginVertices(E_PrimitiveType_TriangleStrip, vertexCount, sizeof(C_BasicRenderColouredVertex2D), m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured2D], true); // This can fail

	if(!pVertex)
		return;

	const U32 colour = GPU_RGBA(col.R(), col.G(), col.B(), col.A());

	pVertex->x = x;
	pVertex->y = y;
	pVertex->c = colour;
	pVertex++;

	pVertex->x = x + w;
	pVertex->y = y;
	pVertex->c = colour;
	pVertex++;

	pVertex->x = x;
	pVertex->y = y + h;
	pVertex->c = colour;
	pVertex++;

	pVertex->x = x + w;
	pVertex->y = y + h;
	pVertex->c = colour;
	pVertex++;

	Device()->EndVertices(pVertex);
}

void C_BasicRender::DrawColoured2DRect(const Maths::HVec2& pos, const Maths::HVec2& dim, const Maths::RGBA col)
{
	DrawColoured2DRect(pos.X(), pos.Y(), dim.X(), dim.Y(), col);
}

void C_BasicRender::DrawTextured2DTri(const F32 x1, const F32 y1, 
									  const F32 x2, const F32 y2, 
									  const F32 x3, const F32 y3, 
									  const C_Float2 uvs[3],
									  const Maths::RGBA col, 
									  const C_Texture* pTexture, 
									  C_PixelShader* pPixelShader)
{
	if(!m_Created)
		return;

	PERF_EVENT("DrawTextured2DTri");

	BindTextured2D(pTexture, pPixelShader);

	const U32 vertexCount = 3;
	C_BasicRenderTexturedVertex2D* pVertex = (C_BasicRenderTexturedVertex2D*)Device()->BeginVertices(E_PrimitiveType_TriangleList, vertexCount, sizeof(C_BasicRenderTexturedVertex2D), m_pVertexDeclaration[E_BasicRenderVertexShader_Textured2D], true); // This can fail
	
	if(!pVertex)
		return;

	const U32 colour = GPU_RGBA(col.R(), col.G(), col.B(), col.A());

	pVertex->x = x1;
	pVertex->y = y1;
	pVertex->u = uvs[0].x;
	pVertex->v = uvs[0].y;
	pVertex->c = colour;
	pVertex++;

	pVertex->x = x2;
	pVertex->y = y2;
	pVertex->u = uvs[1].x;
	pVertex->v = uvs[1].y;
	pVertex->c = colour;
	pVertex++;

	pVertex->x = x3;
	pVertex->y = y3;
	pVertex->u = uvs[2].x;
	pVertex->v = uvs[2].y;
	pVertex->c = colour;
	pVertex++;

	Device()->EndVertices(pVertex);
}

void C_BasicRender::DrawTextured2DRect(const F32 x, const F32 y, const F32 w, const F32 h, const Maths::RGBA col, const C_Texture* pTexture, C_PixelShader* pPixelShader)
{
	if(!m_Created)
		return;

	PERF_EVENT("DrawTextured2DRect");

	BindTextured2D(pTexture, pPixelShader);

	const U32 vertexCount = 4;
	C_BasicRenderTexturedVertex2D* pVertex = (C_BasicRenderTexturedVertex2D*)Device()->BeginVertices(E_PrimitiveType_TriangleStrip, vertexCount, sizeof(C_BasicRenderTexturedVertex2D), m_pVertexDeclaration[E_BasicRenderVertexShader_Textured2D], true); // This can fail

	if(!pVertex)
		return;

	const U32 colour = GPU_RGBA(col.R(), col.G(), col.B(), col.A());

	pVertex->x = x;
	pVertex->y = y;
	pVertex->c = colour;
	pVertex->u = 0.f;
	pVertex->v = 0.f;
	pVertex++;

	pVertex->x = x + w;
	pVertex->y = y;
	pVertex->c = colour;
	pVertex->u = 1.f;
	pVertex->v = 0.f;
	pVertex++;

	pVertex->x = x;
	pVertex->y = y + h;
	pVertex->c = colour;
	pVertex->u = 0.f;
	pVertex->v = 1.f;
	pVertex++;

	pVertex->x = x + w;
	pVertex->y = y + h;
	pVertex->c = colour;
	pVertex->u = 1.f;
	pVertex->v = 1.f;
	pVertex++;

	Device()->EndVertices(pVertex);
}

void C_BasicRender::DrawTextured2DRect(const Maths::HVec2& pos, const Maths::HVec2& dim, const Maths::RGBA col, const C_Texture* pTexture, C_PixelShader* pPixelShader)
{
	DrawTextured2DRect(pos.X(), pos.Y(), dim.X(), dim.Y(), col, pTexture, pPixelShader);
}

void C_BasicRender::DrawTextured2DRect(const F32 x, const F32 y, const F32 w, const F32 h, const C_Float2* uv, const Maths::RGBA col, const C_Texture* pTexture, C_PixelShader* pPixelShader)
{
	if(!m_Created)
		return;

	PERF_EVENT("DrawTextured2DRect");

	assert(uv);

	BindTextured2D(pTexture, pPixelShader);

	const U32 vertexCount = 4;
	C_BasicRenderTexturedVertex2D* pVertex = (C_BasicRenderTexturedVertex2D*)Device()->BeginVertices(E_PrimitiveType_TriangleStrip, vertexCount, sizeof(C_BasicRenderTexturedVertex2D), m_pVertexDeclaration[E_BasicRenderVertexShader_Textured2D], true); // This can fail

	if(!pVertex)
		return;

	const U32 colour = GPU_RGBA(col.R(), col.G(), col.B(), col.A());

	pVertex->x = x;
	pVertex->y = y;
	pVertex->c = colour;
	pVertex->u = uv[0].x;
	pVertex->v = uv[0].y;
	pVertex++;

	pVertex->x = x + w;
	pVertex->y = y;
	pVertex->c = colour;
	pVertex->u = uv[1].x;
	pVertex->v = uv[1].y;
	pVertex++;

	pVertex->x = x;
	pVertex->y = y + h;
	pVertex->c = colour;
	pVertex->u = uv[2].x;
	pVertex->v = uv[2].y;
	pVertex++;

	pVertex->x = x + w;
	pVertex->y = y + h;
	pVertex->c = colour;
	pVertex->u = uv[3].x;
	pVertex->v = uv[3].y;
	pVertex++;

	Device()->EndVertices(pVertex);
}

void C_BasicRender::DrawColoured3DLine(const F32 x1, const F32 y1, const F32 z1, const F32 x2, const F32 y2, const F32 z2, const Maths::RGBA col)
{
	if(!m_Created)
		return;

	//PERF_EVENT("DrawColoured3DLine");

	const F32 r = F32(col.R()) / 255.f;
	const F32 g = F32(col.G()) / 255.f;
	const F32 b = F32(col.B()) / 255.f;
	const F32 a = F32(col.A()) / 255.f;
	
	BindColoured3D();
	
	const U32 vertexCount = 2;
	volatile C_BasicRenderColouredVertex3D* pVertex = (C_BasicRenderColouredVertex3D*)Device()->BeginVertices(E_PrimitiveType_LineList, vertexCount, sizeof(C_BasicRenderColouredVertex3D), m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured3D], true); // This can fail

	if(!pVertex)
		return;

	pVertex->x = x1;
	pVertex->y = y1;
	pVertex->z = z1;
	pVertex->r = r;
	pVertex->g = g;
	pVertex->b = b;
	pVertex->a = a;
	pVertex++;

	pVertex->x = x2;
	pVertex->y = y2;
	pVertex->z = z2;
	pVertex->r = r;
	pVertex->g = g;
	pVertex->b = b;
	pVertex->a = a;
	pVertex++;

	Device()->EndVertices((void*)pVertex);
}

void C_BasicRender::DrawColoured3DLine(const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA col)
{
	DrawColoured3DLine(start.X(), start.Y(), start.Z(), end.X(), end.Y(), end.Z(), col);
}

void C_BasicRender::DrawColoured3DBox(const F32 x1, const F32 y1, const F32 z1, const F32 x2, const F32 y2, const F32 z2, const Maths::RGBA col)
{
	if(!m_Created)
		return;

	const Maths::HVec3 dim = Maths::HVec3(x2 - x1, y2 - y1, z2 - z1) * Maths::Constants<Maths::HF32>::Half();
	const Maths::HVec3 pos(x1 + dim.X(), y1 + dim.Y(), z1 + dim.Z());
	const Maths::C_RGBA_F32 col1(F32(col.R()) / 255.f, F32(col.G()) / 255.f, F32(col.B()) / 255.f, F32(col.A()) / 255.f);
	DrawColoured3DBox(pos, dim, col1);
}

void C_BasicRender::DrawColoured3DBox(const Maths::HVec3& pos, const Maths::HVec3& dim, const Maths::RGBA col)
{
	if(!m_Created)
		return;

	const Maths::C_RGBA_F32 col1(F32(col.R()) / 255.f, F32(col.G()) / 255.f, F32(col.B()) / 255.f, F32(col.A()) / 255.f);
	DrawColoured3DBox(pos, dim, col1);
}

void C_BasicRender::DrawColoured3DBox(const Maths::HVec3& pos, const Maths::HVec3& dim, const Maths::C_RGBA_F32& col)
{
	if(!m_Created)
		return;

	PERF_EVENT("DrawColoured3DBox");

	BindColoured3D();
	
	C_BasicRenderColouredVertex3D vertex[8] = 
	{
		{pos.X() - dim.X(), pos.Y() - dim.Y(), pos.Z() - dim.Z(), col.R(), col.G(), col.B(), col.A()},
		{pos.X() - dim.X(), pos.Y() - dim.Y(), pos.Z() + dim.Z(), col.R(), col.G(), col.B(), col.A()},
		{pos.X() - dim.X(), pos.Y() + dim.Y(), pos.Z() - dim.Z(), col.R(), col.G(), col.B(), col.A()},
		{pos.X() - dim.X(), pos.Y() + dim.Y(), pos.Z() + dim.Z(), col.R(), col.G(), col.B(), col.A()},
		{pos.X() + dim.X(), pos.Y() - dim.Y(), pos.Z() - dim.Z(), col.R(), col.G(), col.B(), col.A()},
		{pos.X() + dim.X(), pos.Y() - dim.Y(), pos.Z() + dim.Z(), col.R(), col.G(), col.B(), col.A()},
		{pos.X() + dim.X(), pos.Y() + dim.Y(), pos.Z() - dim.Z(), col.R(), col.G(), col.B(), col.A()},
		{pos.X() + dim.X(), pos.Y() + dim.Y(), pos.Z() + dim.Z(), col.R(), col.G(), col.B(), col.A()},
	};
	
	const U32 vertexCount = 36;
	C_BasicRenderColouredVertex3D* pVertex = (C_BasicRenderColouredVertex3D*)Device()->BeginVertices(E_PrimitiveType_TriangleList, vertexCount, sizeof(C_BasicRenderColouredVertex3D), m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured3D], true); // This can fail

	if(!pVertex)
		return;

	// top
	*(pVertex++) = vertex[2]; *(pVertex++) = vertex[3]; *(pVertex++) = vertex[7];
	*(pVertex++) = vertex[2]; *(pVertex++) = vertex[7]; *(pVertex++) = vertex[6];
	// bottom
	*(pVertex++) = vertex[0]; *(pVertex++) = vertex[5]; *(pVertex++) = vertex[1];
	*(pVertex++) = vertex[0]; *(pVertex++) = vertex[4]; *(pVertex++) = vertex[5];
	// left
	*(pVertex++) = vertex[1]; *(pVertex++) = vertex[3]; *(pVertex++) = vertex[2];
	*(pVertex++) = vertex[1]; *(pVertex++) = vertex[2]; *(pVertex++) = vertex[0];
	// right
	*(pVertex++) = vertex[4]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[5];
	*(pVertex++) = vertex[5]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[7];
	// front
	*(pVertex++) = vertex[2]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[0];
	*(pVertex++) = vertex[0]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[4];
	// back
	*(pVertex++) = vertex[1]; *(pVertex++) = vertex[7]; *(pVertex++) = vertex[3];
	*(pVertex++) = vertex[1]; *(pVertex++) = vertex[5]; *(pVertex++) = vertex[7];

	Device()->EndVertices(pVertex);
}

void C_BasicRender::DrawTextured3DTriangle(
	const Maths::HVec3& pos1,
	const Maths::HVec3& pos2,
	const Maths::HVec3& pos3,
	const C_Float2 uv[3],
	const Maths::C_RGBA_F32& col,
	const C_Texture* pTexture )
{
	if(!m_Created)
		return;
		
		
	if ( !pTexture || !pTexture->IsCreated() )
	{
		return;
	}

	BindTextured3D(pTexture, E_TextureAddress_Wrap);
	
	C_BasicRenderTexturedVertex3D vertex[3] = 
	{
		{ pos1.X(), pos1.Y(), pos1.Z(), col.R(), col.G(), col.B(), col.A(), uv[0].x, uv[0].y },
		{ pos2.X(), pos2.Y(), pos2.Z(), col.R(), col.G(), col.B(), col.A(), uv[1].x, uv[1].y },
		{ pos3.X(), pos3.Y(), pos3.Z(), col.R(), col.G(), col.B(), col.A(), uv[2].x, uv[2].y }
	};
	
	const U32 vertexCount = 3;
	C_BasicRenderTexturedVertex3D* pVertex = (C_BasicRenderTexturedVertex3D*)Device()->BeginVertices(E_PrimitiveType_TriangleList, vertexCount, sizeof(C_BasicRenderTexturedVertex3D), m_pVertexDeclaration[E_BasicRenderVertexShader_Textured3D], true); // This can fail

	if(!pVertex)
		return;
		
	*pVertex++ = vertex[0];
	*pVertex++ = vertex[1];
	*pVertex++ = vertex[2];

	Device()->EndVertices(pVertex);
}

void C_BasicRender::DrawColoured3DCircle(const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA col)
{
	if(!m_Created)
		return;

	BindColoured3D();

	Maths::HVec3 u, v;
	normal.PerpendicularAxes(u, v);

	const S32 pointCount = 32;
	const F32 deltaTheta = (Maths::Pi * 2.f) / (F32)pointCount;
	
	const S32 vertexCount = 1 + pointCount;
	C_BasicRenderColouredVertex3D* pVertex = (C_BasicRenderColouredVertex3D*)Device()->BeginVertices(E_PrimitiveType_LineStrip, vertexCount, sizeof(C_BasicRenderColouredVertex3D), m_pVertexDeclaration[E_BasicRenderVertexShader_Coloured3D], true); // This can fail
	
	if(!pVertex)
		return;

	for(S32 i=0; i<=pointCount; i++)
	{
		const F32 theta = (F32)i * deltaTheta;
		Maths::SinCosAlpha angles(theta);
		const Maths::HVec3 pos = centre + (u * angles.SinAlpha() + v * angles.CosAlpha()) * radius;
        
		pVertex->x = pos.X();
		pVertex->y = pos.Y();
		pVertex->z = pos.Z();
		pVertex->r = (F32)col.R() / 255.f;
		pVertex->g = (F32)col.G() / 255.f;
		pVertex->b = (F32)col.B() / 255.f;
		pVertex->a = (F32)col.A() / 255.f;
		pVertex++;
	}

	Device()->EndVertices(pVertex);
}

void C_BasicRender::DrawColoured3DSphere(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA col)
{
	DrawColoured3DCircle(centre, Maths::HVecX(1.f), radius, col);
    DrawColoured3DCircle(centre, Maths::HVecY(1.f), radius, col);
    DrawColoured3DCircle(centre, Maths::HVecZ(1.f), radius, col);
}

C_BasicRender* g_pBasicRender = NULL;

void BasicRender(C_BasicRender* pBasicRender)
{
	g_pBasicRender = pBasicRender;
}

}; // namespace Display
