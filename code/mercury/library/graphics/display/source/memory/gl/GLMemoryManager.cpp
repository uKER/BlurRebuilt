#include "stdafx.h"
#include "Display/Memory/GL/GLMemoryManager.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) 
#endif

namespace NewDisplay
{
namespace GL
{

C_GLMemoryBuffer::C_GLMemoryBuffer() : m_Size(0), m_Align(0), m_Flags(0)
{
}

C_GLMemoryBuffer::~C_GLMemoryBuffer()
{
}

bool C_GLMemoryBuffer::CreateAddress()
{
	return false;
}

void C_GLMemoryBuffer::DestroyAddress()
{
}

}; // namespace GL
}; // namespace NewDisplay
