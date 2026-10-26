#include "AftkRenderPch.h"
#include "common/VertexFormat.h"

using namespace AftkRender;
using namespace Display;


void AftkRender::CreateAftkRenderVertexDeclarations()
{
	Vertex_XYZ2::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ2::m_Elements);
	Vertex_XYZ2_C::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ2_C::m_Elements);
	Vertex_UV::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_UV::m_Elements);
	Vertex_W4_Idx4::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_W4_Idx4::m_Elements);
}

C_VertexElement Vertex_XYZ2::m_Elements[Vertex_XYZ2::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0),
	C_VertexElement( 0, 12, E_VertexElementType_F32_3, E_VertexElementUsage_Normal, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ2::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ2_C::m_Elements[Vertex_XYZ2_C::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0),
	C_VertexElement( 0, 12, E_VertexElementType_F32_3, E_VertexElementUsage_Normal, 0),
	C_VertexElement( 0, 24, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ2_C::m_pDeclaration = NULL;


C_VertexElement Vertex_UV::m_Elements[Vertex_UV::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_UV::m_pDeclaration = NULL;

C_VertexElement Vertex_W4_Idx4::m_Elements[Vertex_W4_Idx4::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_4, 	E_VertexElementUsage_BlendWeight, 0),
	C_VertexElement( 0, 16, E_VertexElementType_U8_4, E_VertexElementUsage_BlendIndices, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_W4_Idx4::m_pDeclaration = NULL;
