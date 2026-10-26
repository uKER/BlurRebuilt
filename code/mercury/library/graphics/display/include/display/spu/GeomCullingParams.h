#ifndef DISPLAY_SPU_GEOM_CULLING_PARAMS_H
#define DISPLAY_SPU_GEOM_CULLING_PARAMS_H

#include "Display/SPU/Edge/Geom/EdgeGeom_Structs.h"

namespace Display
{

struct ALIGN16 SpuGeomCullingParams
{
	EdgeGeomViewportInfo			m_ViewportInfo;
	EdgeGeomLocalToWorldMatrix		m_LocalToWorld;
	EdgeGeomSpuConfigInfo			m_SpuConfigInfo;
	uint32_t						m_HoleEa;
	uint32_t						m_OutputBufferInfoEa;
	uint32_t						m_IndexAddress;
	uint32_t						m_VertexAddress;
	uint32_t						m_VertexStride;
};

}; // namespace Display

#endif // DISPLAY_SPU_GEOM_CULLING_PARAMS_H
