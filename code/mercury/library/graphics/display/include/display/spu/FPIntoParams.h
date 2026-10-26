#ifndef DISPLAY_SPU_FP_INTO_PARAMS_H
#define DISPLAY_SPU_FP_INTO_PARAMS_H

#include "Display/SPU/Edge/Geom/EdgeGeom_Structs.h"

namespace Display
{

struct ALIGN16 SpuFPIntoParams
{
	uint32_t						m_Offsets[FP_INTO_PATCH_MAX];
	F32								m_Data[FP_INTO_PATCH_MAX * 4];
	uint32_t						m_NumberToPatch;
	uint32_t						m_FPInAddress;
	uint32_t						m_FPInOffset;
	uint32_t						m_FPInLocation;
	uint32_t						m_FPOutAddress;
	uint32_t						m_FPOutOffset;
	uint32_t						m_FPOutLocation;
	uint32_t						m_FPSize;
	uint32_t						m_J2SAddress;
};

}; // namespace Display

#endif // DISPLAY_SPU_FP_INTO_PARAMS_H
