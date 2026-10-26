#include "stdafx.h"
#include "Display/Device/GCM/GCMDeviceStream.h"

namespace NewDisplay
{
namespace GCM
{

C_GCMStream::C_GCMStream() :
	m_pVertexBuffer(NULL),
	m_VertexStride(0),
	m_VertexOffset(0)
{
}

C_GCMStream::~C_GCMStream()
{
}

}; // namespace GCM
}; // namespace NewDisplay
