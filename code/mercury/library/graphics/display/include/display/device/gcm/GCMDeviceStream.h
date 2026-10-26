#ifndef DISPLAY_GCMDEVICESTREAM_H
#define DISPLAY_GCMDEVICESTREAM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Buffer/GCM/GCMVertexBuffer.h"

namespace NewDisplay
{
namespace GCM
{

class C_GCMStream
{
public:

	C_GCMStream();
	~C_GCMStream();

public:

	const C_VertexBuffer*	m_pVertexBuffer;
	U32						m_VertexStride;	
	U32						m_VertexOffset;
};

}; // namespace GCM
}; // namespace NewDisplay

#endif // DISPLAY_GCMDEVICESTREAM_H
