#include "stdafx.h"
#include "Display/Occlusion/OcclusionQuery.h"
#include "Display/Shaders/Constants.h"

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Display/Shaders/Occlusion/PS3/vsOcclusion.shader"
#include "Display/Shaders/Occlusion/PS3/psOcclusion.shader"
#endif
#endif

EXTERN_SHADER(vsOcclusion);
EXTERN_SHADER(psOcclusion);

namespace Display
{

C_OcclusionQueryRender::C_OcclusionQueryRender() : I_Renderer("OcclusionQuery")
{
	C_VertexFormat vertexFormat;
	vertexFormat.AddElement(0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0);
	m_pVertexDeclaration = Device()->CreateVertexDeclaration(vertexFormat);

	assert(g_pOcclusionQueryRender == NULL);
	g_pOcclusionQueryRender = this;
}

C_OcclusionQueryRender::~C_OcclusionQueryRender()
{
	DestroyRenderer();

	assert(g_pOcclusionQueryRender == this);
	g_pOcclusionQueryRender = NULL;
}

bool C_OcclusionQueryRender::OnCreate()
{
	RESOURCE_MEMORY_ID("OcclusionQuery");

// [Blur2PC] The native DX11 backend also creates these PC built-in shaders.
#if defined(DEVICE_PCD3D11) || defined(DEVICE_PS3GCM)
	if (!CreateVertexShaderFromFunc("Shaders/vsOcclusion", 0, &m_VertexShader, REF_SHADER(vsOcclusion)))
		return false;
	if (!CreatePixelShaderFromFunc("Shaders/psOcclusion", 0, &m_PixelShader, REF_SHADER(psOcclusion)))
		return false;
#endif
	if (!Build())
		return false;

	return true;
}

void C_OcclusionQueryRender::OnDestroy()
{
	RESOURCE_MEMORY_ID("OcclusionQuery");

	Device()->Destroy(&m_VertexShader);
	Device()->Destroy(&m_PixelShader);
	Device()->Destroy(&m_VertexBuffer);
	Device()->Destroy(&m_IndexBuffer);
}

void C_OcclusionQueryRender::Bind(const bool show)
{
	Device()->SetCullMode(E_CullMode_None);
	Device()->SetDepthWriteEnable(E_Bool_False);
	Device()->SetBlendMode(E_BlendMode_Solid);
	Display::C_StencilState stencilState;
	stencilState.m_StencilEnable		= Display::E_Bool_False;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
	stencilState.m_StencilRef			= 0x00;
	stencilState.m_StencilMask			= 0x00;
	stencilState.m_StencilWriteMask		= 0x00;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail		= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass		= Display::E_StencilOp_Replace;
	Device()->SetStencilState(stencilState);

	Device()->SetVertexShader(&m_VertexShader);
	Device()->SetPixelShader(show ? &m_PixelShader : NULL);
	#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	Device()->SetPixelShader(&m_PixelShader);
	#else
	Device()->SetPixelShader(show ? &m_PixelShader : NULL);
	#endif
	#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	Device()->SetColourWriteEnable(show ? E_ColourWriteEnable_All : 0);
	#endif
	Device()->SetVertexDeclaration(m_pVertexDeclaration);
	Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(C_Vertex));
	Device()->SetIndexBuffer(&m_IndexBuffer);
}

void C_OcclusionQueryRender::Render(const Maths::HMat43& matrix, const Maths::HVec3& scale)
{
	// set the object scale
	MathsStorage::Float4 objectScale(scale.X(), scale.Y(), scale.Z(), 0.f);
	Device()->SetVertexShaderConstantF(CV_DISPLAY_OBJECT_SCALE, (F32*)&objectScale, 1);

	// set the world matrix
	Maths::HMat44 hWorldMatrix = Maths::HMat44(matrix);
	Display::C_Matrix worldMatrix;
	Maths::Store(hWorldMatrix, &worldMatrix);
	Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);

	Device()->DrawIndexedPrimitive(E_PrimitiveType_TriangleList, 0, 0, 8, 0, 12 * 3);
}

void C_OcclusionQueryRender::Unbind()
{
	#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	Device()->SetColourWriteEnable(E_ColourWriteEnable_Default);
	#endif
	Device()->SetDepthWriteEnable(E_Bool_True);
	Device()->SetCullMode(E_CullMode_Default);
}

bool C_OcclusionQueryRender::Build()
{
	const U32 vertexCount = 8;
	if (!Device()->CreateVertexBuffer(sizeof(C_Vertex) * vertexCount, 0, &m_VertexBuffer))
		return false;

	C_Vertex* pVertex = (C_Vertex*)m_VertexBuffer.Lock();

	pVertex->x = -1.0f;	pVertex->y = -1.0f;	pVertex->z = -1.0f;	pVertex++;
    pVertex->x = -1.0f;	pVertex->y = -1.0f;	pVertex->z =  1.0f;	pVertex++;
    pVertex->x = -1.0f;	pVertex->y =  1.0f;	pVertex->z = -1.0f;	pVertex++;
    pVertex->x = -1.0f;	pVertex->y =  1.0f;	pVertex->z =  1.0f;	pVertex++;
	pVertex->x =  1.0f;	pVertex->y = -1.0f;	pVertex->z = -1.0f;	pVertex++;
    pVertex->x =  1.0f;	pVertex->y = -1.0f;	pVertex->z =  1.0f;	pVertex++;
    pVertex->x =  1.0f;	pVertex->y =  1.0f;	pVertex->z = -1.0f;	pVertex++;
    pVertex->x =  1.0f;	pVertex->y =  1.0f;	pVertex->z =  1.0f;	pVertex++;

	m_VertexBuffer.Unlock();

	const U32 indexCount = 6 * 2 * 3;
	if (!Device()->CreateIndexBuffer(sizeof(U16) * indexCount, 0, E_Format_Index16, &m_IndexBuffer))
		return false;

	U16* pIndex = (U16*)m_IndexBuffer.Lock();
	
	// top
	*(pIndex++) = 2; *(pIndex++) = 7; *(pIndex++) = 3; 
	*(pIndex++) = 2; *(pIndex++) = 6; *(pIndex++) = 7; 
	// bottom
	*(pIndex++) = 0; *(pIndex++) = 1; *(pIndex++) = 5; 
	*(pIndex++) = 0; *(pIndex++) = 5; *(pIndex++) = 4; 					 
	// left
	*(pIndex++) = 1; *(pIndex++) = 2; *(pIndex++) = 3; 
	*(pIndex++) = 1; *(pIndex++) = 0; *(pIndex++) = 2; 						 
	// right
	*(pIndex++) = 4; *(pIndex++) = 5; *(pIndex++) = 6; 
	*(pIndex++) = 5; *(pIndex++) = 7; *(pIndex++) = 6; 						 
	// front
	*(pIndex++) = 2; *(pIndex++) = 0; *(pIndex++) = 6; 
	*(pIndex++) = 0; *(pIndex++) = 4; *(pIndex++) = 6; 							 
	// back
	*(pIndex++) = 1; *(pIndex++) = 3; *(pIndex++) = 7; 
	*(pIndex++) = 1; *(pIndex++) = 7; *(pIndex++) = 5;

	m_IndexBuffer.Unlock();

	return true;
}

C_OcclusionQueryRender* g_pOcclusionQueryRender = NULL;

void OcclusionQueryRender(C_OcclusionQueryRender* pOcclusionQueryRender)
{
	g_pOcclusionQueryRender = pOcclusionQueryRender;
}

}; // namespace Display
