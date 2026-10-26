#ifndef DISPLAY_BUFFER_GCM_COMMAND_BUFFER_BUILDER_H
#define DISPLAY_BUFFER_GCM_COMMAND_BUFFER_BUILDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Buffer/GCM/GCMVertexBuffer.h"
#include "Display/Buffer/GCM/GCMVertexDeclaration.h"

namespace Display
{
namespace GCM
{

static const U32 POSITION_INDEX			= 0;
static const U32 NORMAL_INDEX			= 2;
static const U32 COLOUR_INDEX			= 3;
static const U32 TEXCOORD_INDEX			= 8;
static const U32 TANGENT_INDEX			= 14;
static const U32 BLEND_INDICES_INDEX	= TEXCOORD_INDEX+3;
static const U32 BLEND_WEIGHTS_INDEX	= TEXCOORD_INDEX+4;

template <class Context>
void GCMCreateCommandBufferSetVertexDeclaration(Context& context, const U32 vertexStride, C_VertexBuffer* pVertexBuffer, C_VertexDeclaration* pVertexDeclaration)
{
	bool positionUnused			= true;
	bool normalUnused			= true;
	bool colourUnused			= true;
	bool texcoordUnused[2]		= { true, true };
	bool blendIndicesUnused		= true;
	bool blendWeightUnused		= true;
	bool tangentUnused			= true;

	const S32 vertexElementCount = pVertexDeclaration->m_ElementCount;
	for(S32 vertexElementIndex=0; vertexElementIndex<vertexElementCount; vertexElementIndex++)
	{
		const C_VertexElement& vertexElement = pVertexDeclaration->m_ElementArray[vertexElementIndex];
		
		switch(vertexElement.m_Usage)
		{
		case(E_VertexElementUsage_Position):
			{
				assert(vertexElement.m_Stream < E_Stream_Count);
				assert(vertexElement.m_UsageIndex == 0);
				if(pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
					case E_VertexElementType_F32_1:
					case E_VertexElementType_F32_2:
					case E_VertexElementType_F32_3:
					case E_VertexElementType_F32_4:
						{
							positionUnused = false;
							context.SetVertexDataArray(
								POSITION_INDEX,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					}
				}
			}
			break;
		case E_VertexElementUsage_Normal:
			{
				assert(vertexElement.m_Stream < E_Stream_Count);
				assert(vertexElement.m_UsageIndex == 0);
				if(pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
					case E_VertexElementType_F32_3:
						{
							normalUnused = false;
							context.SetVertexDataArray(
								NORMAL_INDEX,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U10_3N:
						{
							normalUnused = false;
							context.SetVertexDataArray(
								NORMAL_INDEX,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_CMP,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U8_4N:
						{
							normalUnused = false;
							context.SetVertexDataArray(
								NORMAL_INDEX,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_UB,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					}
				}
			}
			break;
		case E_VertexElementUsage_Colour:
			{
				assert(vertexElement.m_Stream < E_Stream_Count);
				assert(vertexElement.m_UsageIndex < 2);
				if(pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
					case E_VertexElementType_F32_1:
					case E_VertexElementType_F32_2:
					case E_VertexElementType_F32_3:
					case E_VertexElementType_F32_4:
						{
							colourUnused = false;
							context.SetVertexDataArray(
								COLOUR_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_Colour:
						{
							colourUnused = false;
							context.SetVertexDataArray(
								COLOUR_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStride,
								4,
								CELL_GCM_VERTEX_UB,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					}
				}
			}
			break;
		case E_VertexElementUsage_TexCoord:
			{
				assert(vertexElement.m_Stream < E_Stream_Count);
				assert(vertexElement.m_UsageIndex < 2);
				if(pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
					case E_VertexElementType_F32_1:
					case E_VertexElementType_F32_2:
					case E_VertexElementType_F32_3:
					case E_VertexElementType_F32_4:
						{
							texcoordUnused[vertexElement.m_UsageIndex] = false;
							context.SetVertexDataArray(
								TEXCOORD_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_F16_2:
					case E_VertexElementType_F16_4:
						{
							texcoordUnused[vertexElement.m_UsageIndex] = false;
							context.SetVertexDataArray(
								TEXCOORD_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_SF,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_S16_2N:
						{
							texcoordUnused[vertexElement.m_UsageIndex] = false;
							context.SetVertexDataArray(
								TEXCOORD_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_S1,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					}
				}
			}
			break;
		case E_VertexElementUsage_BlendIndices:
			{
				assert(vertexElement.m_Stream < E_Stream_Count);
				if(pVertexBuffer)
				{
					blendIndicesUnused = false;
					context.SetVertexDataArray(
						BLEND_INDICES_INDEX,
						0,
						vertexStride,
						4,
						CELL_GCM_VERTEX_S32K,
							Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
							pVertexBuffer->GetOffset() + vertexElement.m_Offset);
				}
			}
			break;
		case E_VertexElementUsage_BlendWeight:
			{
				assert(vertexElement.m_Stream < E_Stream_Count);
				if(pVertexBuffer)
				{
					blendWeightUnused = false;
					context.SetVertexDataArray(
						BLEND_WEIGHTS_INDEX,
						0,
						vertexStride,
						4,
						CELL_GCM_VERTEX_UB256,
						Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
						pVertexBuffer->GetOffset() + vertexElement.m_Offset);
				}
			}
			break;
		case E_VertexElementUsage_Tangent:
			{
				assert(vertexElement.m_Stream < E_Stream_Count);
				assert(vertexElement.m_UsageIndex == 0);
				if(pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
					case E_VertexElementType_F32_3:
						{
							tangentUnused = false;
							context.SetVertexDataArray(
								TANGENT_INDEX,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U10_3N:
						{
							tangentUnused = false;
							context.SetVertexDataArray(
								TANGENT_INDEX,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_CMP,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U8_4N:
						{
							tangentUnused = false;
							context.SetVertexDataArray(
								TANGENT_INDEX,
								0,
								vertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_UB,
								Display2::FlagsToLocation(pVertexBuffer->GetFlags()),
								pVertexBuffer->GetOffset() + vertexElement.m_Offset);
						}
						break;
					}
				}
			}
			break;
		}
	}

	if(positionUnused)
	{
		context.SetVertexDataArray(POSITION_INDEX,		0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
	}
	if(normalUnused)
	{
		context.SetVertexDataArray(NORMAL_INDEX,		0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
	}
	if(colourUnused)
	{
		context.SetVertexDataArray(COLOUR_INDEX,		0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
	}
	if(texcoordUnused[0])
	{
		context.SetVertexDataArray(TEXCOORD_INDEX+0,	0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
	}
	if(texcoordUnused[1])
	{
		context.SetVertexDataArray(TEXCOORD_INDEX+1,	0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
	}
	if(blendIndicesUnused)
	{
		context.SetVertexDataArray(BLEND_INDICES_INDEX,	0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
	}
	if(blendWeightUnused)
	{
		context.SetVertexDataArray(BLEND_WEIGHTS_INDEX,	0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
	}
	if(tangentUnused)
	{
		context.SetVertexDataArray(TANGENT_INDEX,	0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
	}

}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_BUFFER_GCM_COMMAND_BUFFER_BUILDER_H
