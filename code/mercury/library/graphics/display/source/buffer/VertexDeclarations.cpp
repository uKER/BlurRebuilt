#include "stdafx.h"
#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Device/Device.h"

namespace Display
{
	
void CreateVertexDeclarations()
{
	Vertex_XY::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XY::m_Elements);
	Vertex_XY_UV::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XY_UV::m_Elements);
	Vertex_XY_C::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XY_C::m_Elements);
	Vertex_XY_C_UV::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XY_C_UV::m_Elements);
	Vertex_XY_C_UVW::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XY_C_UVW::m_Elements);
	Vertex_XY_HDR_UV::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XY_HDR_UV::m_Elements);
	Vertex_XYZ::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ::m_Elements);
	Vertex_XYZ_C::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_C::m_Elements);
	Vertex_XYZ_UV::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_UV::m_Elements);
	Vertex_XYZ_UVW::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_UVW::m_Elements);
	Vertex_XYZ_SUV::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_SUV::m_Elements);
	Vertex_XYZ_UV2::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_UV2::m_Elements);
	Vertex_XYZ_C_UV::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_C_UV::m_Elements);
	Vertex_XYZ_HDR_UV::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_HDR_UV::m_Elements);
	Vertex_XYZ_C_UVW::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_C_UVW::m_Elements);
	Vertex_XYZ_HDR_UVW::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_HDR_UVW::m_Elements);
	Vertex_XYZ_HDR_UV2::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_HDR_UV2::m_Elements);
	Vertex_XYZ_HDR_SUV::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_HDR_SUV::m_Elements);
	Vertex_XYZ_C_UV_N::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_C_UV_N::m_Elements);
	Vertex_XYZ_HDR_UV_N::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_HDR_UV_N::m_Elements);
	Vertex_XYZ_C_UV2::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_C_UV2::m_Elements);
	Vertex_XYZ2_C_UV2::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ2_C_UV2::m_Elements);
	Vertex_XYZ_HDR_N2::m_pDeclaration = Device()->CreateVertexDeclaration(Vertex_XYZ_HDR_N2::m_Elements);
}

C_VertexElement Vertex_XY::m_Elements[Vertex_XY::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_2, E_VertexElementUsage_Position, 0), 
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XY::m_pDeclaration = NULL;

C_VertexElement Vertex_XY_UV::m_Elements[Vertex_XY_UV::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_2, E_VertexElementUsage_Position, 0),
	C_VertexElement( 0, 8, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XY_UV::m_pDeclaration = NULL;

C_VertexElement Vertex_XY_C::m_Elements[Vertex_XY_C::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_2, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 8, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XY_C::m_pDeclaration = NULL;

C_VertexElement Vertex_XY_C_UV::m_Elements[Vertex_XY_C_UV::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_2, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 8, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	C_VertexElement( 0, 12, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XY_C_UV::m_pDeclaration = NULL;

C_VertexElement Vertex_XY_C_UVW::m_Elements[Vertex_XY_C_UVW::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_2, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 8, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	C_VertexElement( 0, 12, E_VertexElementType_F32_3, E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XY_C_UVW::m_pDeclaration = NULL;

C_VertexElement Vertex_XY_HDR_UV::m_Elements[Vertex_XY_HDR_UV::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_2, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 8, E_VertexElementType_F32_4, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 24, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 1),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XY_HDR_UV::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ::m_Elements[Vertex_XYZ::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_C::m_Elements[Vertex_XYZ_C::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_C::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_UV::m_Elements[Vertex_XYZ_UV::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_UV::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_UVW::m_Elements[Vertex_XYZ_UVW::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_F32_3, E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_UVW::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_SUV::m_Elements[Vertex_XYZ_SUV::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_S16_2, E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_SUV::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_UV2::m_Elements[Vertex_XYZ_UV2::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 20, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 1),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_UV2::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_C_UV::m_Elements[Vertex_XYZ_C_UV::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	C_VertexElement( 0, 16, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_C_UV::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_C_UVW::m_Elements[Vertex_XYZ_C_UVW::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	C_VertexElement( 0, 16, E_VertexElementType_F32_3, E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_C_UVW::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_HDR_UV::m_Elements[Vertex_XYZ_HDR_UV::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_F32_4, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 28, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 1),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_HDR_UV::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_HDR_UVW::m_Elements[Vertex_XYZ_HDR_UVW::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_F32_4, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 28, E_VertexElementType_F32_3, E_VertexElementUsage_TexCoord, 1),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_HDR_UVW::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_HDR_UV2::m_Elements[Vertex_XYZ_HDR_UV2::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_F32_4, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 28, E_VertexElementType_F32_4, E_VertexElementUsage_TexCoord, 1),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_HDR_UV2::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_HDR_SUV::m_Elements[Vertex_XYZ_HDR_SUV::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_F32_4, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 28, E_VertexElementType_S16_2, E_VertexElementUsage_TexCoord, 1),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_HDR_SUV::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_C_UV_N::m_Elements[Vertex_XYZ_C_UV_N::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	C_VertexElement( 0, 16, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 24, E_VertexElementType_F32_3, E_VertexElementUsage_Normal, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_C_UV_N::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_HDR_UV_N::m_Elements[Vertex_XYZ_HDR_UV_N::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_F32_4, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 28, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 1),
	C_VertexElement( 0, 36, E_VertexElementType_F32_3, E_VertexElementUsage_Normal, 0),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_HDR_UV_N::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_C_UV2::m_Elements[Vertex_XYZ_C_UV2::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	C_VertexElement( 0, 16, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 24, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 1),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ_C_UV2::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ2_C_UV2::m_Elements[Vertex_XYZ2_C_UV2::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0),
	C_VertexElement( 0, 12, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 1), 
	C_VertexElement( 0, 24, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0),
	C_VertexElement( 0, 28, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 36, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 1),
	VERTEXELEMENT_END()
};
C_VertexDeclaration* Vertex_XYZ2_C_UV2::m_pDeclaration = NULL;

C_VertexElement Vertex_XYZ_HDR_N2::m_Elements[Vertex_XYZ_HDR_N2::m_ElementCount] =
{
	C_VertexElement( 0, 0, E_VertexElementType_F32_3, E_VertexElementUsage_Position, 0), 
	C_VertexElement( 0, 12, E_VertexElementType_F32_4, E_VertexElementUsage_TexCoord, 0),
	C_VertexElement( 0, 28, E_VertexElementType_F32_3, E_VertexElementUsage_Normal, 0),
	C_VertexElement( 0, 40, E_VertexElementType_F32_3, E_VertexElementUsage_Tangent, 0),
	VERTEXELEMENT_END()
};

C_VertexDeclaration* Vertex_XYZ_HDR_N2::m_pDeclaration = NULL;

}; // namespace Display
