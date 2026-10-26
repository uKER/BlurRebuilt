#ifndef DISPLAY_BUFFER_VERTEX_FORMAT_H
#define DISPLAY_BUFFER_VERTEX_FORMAT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"

namespace Display
{

#if PLATFORM_PS3 || PLATFORM_SPU

#define VERTEXELEMENT_END() Display2::C_VertexElement(Display2::E_Stream_Count, 0, Display2::E_VertexElementType_Unknown, Display2::E_VertexElementUsage_Unknown, 0, 0, 0, 0)

#else

#define VERTEXELEMENT_END() Display2::C_VertexElement(0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0)

#endif

typedef Display2::C_VertexElement C_VertexElement;

#if !PLATFORM_SPU

typedef Display2::C_VertexFormat C_VertexFormat;

#endif

}; // namespace Display

#include "Display/Buffer/VertexFormat.inl"

#endif // DISPLAY_BUFFER_VERTEX_FORMAT_H
