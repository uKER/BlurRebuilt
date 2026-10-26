#ifndef AFTK_RENDER_MESHELEMENTS_INCLUDE
#define AFTK_RENDER_MESHELEMENTS_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
enum E_ElementType
{
	E_ElementType_Unknown		= 0x0,
	E_ElementType_Position		= 0x1,		//3*F32
	E_ElementType_Normal		= 0x2,		//3*F32
	E_ElementType_Tangent		= 0x4,		//3*F32
	E_ElementType_Binormal		= 0x8,		//3*F32
	E_ElementType_Weights		= 0x10,		//4*F32
	E_ElementType_BlendIndices	= 0x20,		//4*U8
	E_ElementType_Colour		= 0x40,		//4*U8
	E_ElementType_TexCoord0		= 0x80,		//2*F32
	E_ElementType_TexCoord1		= 0x100,	//2*F32
	E_ElementType_TexCoord2		= 0x200,	//2*F32
	E_ElementType_TexCoord3		= 0x400,	//2*F32
	E_ElementType_TexCoord4		= 0x800,	//2*F32
	E_ElementType_TexCoord5		= 0x1000,	//2*F32
	E_ElementType_TexCoord6		= 0x2000,	//2*F32
	E_ElementType_TexCoord7		= 0x4000	//2*F32
};

inline U32 GetElementSize(E_ElementType type)
{
	switch(type)
	{
	case E_ElementType_Position:
	case E_ElementType_Normal:
	case E_ElementType_Tangent:
	case E_ElementType_Binormal:
		return 3*sizeof(F32);

	case E_ElementType_Weights:
		return 4*sizeof(F32);

	case E_ElementType_BlendIndices:
	case E_ElementType_Colour:
		return 4*sizeof(U8);

	case E_ElementType_TexCoord0:
	case E_ElementType_TexCoord1:
	case E_ElementType_TexCoord2:
	case E_ElementType_TexCoord3:
	case E_ElementType_TexCoord4:
	case E_ElementType_TexCoord5:
	case E_ElementType_TexCoord6:
	case E_ElementType_TexCoord7:
		return 2*sizeof(F32);

	default:
		return 0;
	}
};

static const Display::E_PrimitiveType PrimitiveTypeLookup[] = 
{
	Display::E_PrimitiveType_Unknown,
	Display::E_PrimitiveType_PointList,
	Display::E_PrimitiveType_LineList,
	Display::E_PrimitiveType_LineStrip,
	Display::E_PrimitiveType_TriangleList,
	Display::E_PrimitiveType_TriangleStrip,
	Display::E_PrimitiveType_TriangleFan,
	Display::E_PrimitiveType_QuadList
};

enum E_PrimitiveType
{
	E_PrimitiveType_Unknown,
	E_PrimitiveType_PointList,
	E_PrimitiveType_LineList,
	E_PrimitiveType_LineStrip,
	E_PrimitiveType_TriList,
	E_PrimitiveType_TriStrip,
	E_PrimitiveType_TriFan,
	E_PrimitiveType_QuadList,
};

inline U32 GetPrimitiveVertexCount(E_PrimitiveType type)
{
	switch(type)
	{
	case E_PrimitiveType_PointList:
		return 1;
	case E_PrimitiveType_LineList:
	case E_PrimitiveType_LineStrip:
		return 2;
	case E_PrimitiveType_TriList:
	case E_PrimitiveType_TriStrip:
	case E_PrimitiveType_TriFan:
		return 3;
	case E_PrimitiveType_QuadList:
		return 4;
	default:
		return 0;
	};
}

inline U32 GetNumPrimitives(E_PrimitiveType type, U32 indexCount)
{
	switch (type)
	{
	case E_PrimitiveType_PointList:
		return indexCount;
	case E_PrimitiveType_LineList:
		assert(!(indexCount & 0x1) && "Invalid count for linelist");
		return indexCount/2;
	case E_PrimitiveType_LineStrip:
		assert(indexCount>1 && "Invalid count for linestrip");
		return indexCount-1;
	case E_PrimitiveType_TriList:
		assert(indexCount%3==0 && "Invalid count for trilist");
		return indexCount/3;
	case E_PrimitiveType_TriStrip:
		assert(indexCount>2 && "Invalid count for tristrip");
		return indexCount-2;
	case E_PrimitiveType_TriFan:
		assert(indexCount>2 && "Invalid count for tristrip");
		return indexCount-2;
	case E_PrimitiveType_QuadList:
		assert(indexCount%4==0 && "Invalid count for quadlist");
		return indexCount/4;
	default:
	case E_PrimitiveType_Unknown:
		return 0;
	};

};

inline void GetPrimitiveIndices(E_PrimitiveType type, U32 primitiveIndex, U32* pOut)
{
	switch (type)
	{
	case E_PrimitiveType_PointList:
		pOut[0] = primitiveIndex;
		return;
	case E_PrimitiveType_LineList:
		pOut[0] = primitiveIndex*2;
		pOut[1] = primitiveIndex*2+1;
		return;
	case E_PrimitiveType_LineStrip:
		pOut[0] = primitiveIndex;
		pOut[1] = primitiveIndex+1;
		return;
	case E_PrimitiveType_TriList:
		pOut[0] = primitiveIndex*3;
		pOut[1] = primitiveIndex*3+1;
		pOut[2] = primitiveIndex*3+2;
		return;
	case E_PrimitiveType_TriStrip:
		pOut[0] = primitiveIndex;
		pOut[1] = primitiveIndex+1;
		pOut[2] = primitiveIndex+2;
		return;
	case E_PrimitiveType_TriFan:
		pOut[0] = 0;
		pOut[1] = primitiveIndex+1;
		pOut[2] = primitiveIndex+2;
		return;
	case E_PrimitiveType_QuadList:
		pOut[0] = primitiveIndex*4;
		pOut[1] = primitiveIndex*4+1;
		pOut[2] = primitiveIndex*4+2;
		pOut[3] = primitiveIndex*4+3;
	};
};

}

#endif