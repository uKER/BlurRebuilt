#ifndef DISPLAY_SPU_FRAGMENT_PATCH_PARAMS_H
#define DISPLAY_SPU_FRAGMENT_PATCH_PARAMS_H

#include "Display/SPU/Edge/Geom/EdgeGeom_Structs.h"

namespace Display
{

struct ALIGN16 SpuFPEdgeParams
{
	uint32_t						m_Offsets[FP_EDGE_PATCH_MAX];
	F32								m_Data[FP_EDGE_PATCH_MAX * 4];
	uint32_t						m_CommandBufferHoleEa;
	uint32_t						m_CommandBufferHoleSize;
	uint32_t						m_OutputBufferInfoEa;
	uint32_t						m_FragmentProgramEa;
	uint32_t						m_FragmentProgramOffset;
	uint32_t						m_FragmentProgramSize;
	uint32_t						m_NumberToPatch;
	uint32_t						m_PrimitiveMode;
	uint32_t						m_IndexCount;
	uint32_t						m_IndexLocation;
	uint32_t						m_IndexOffset;
};

}; // namespace Display

#endif // DISPLAY_SPU_FRAGMENT_PATCH_PARAMS_H
