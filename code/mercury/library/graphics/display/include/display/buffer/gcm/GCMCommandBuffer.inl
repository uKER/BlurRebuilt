#ifndef DISPLAY_BUFFER_GCM_GCM_COMMAND_BUFFER_INL
#define DISPLAY_BUFFER_GCM_GCM_COMMAND_BUFFER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace GCM
{

INLINE const bool C_CommandBuffer::IsCreated() const
{
	return m_pAddress != NULL;
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_BUFFER_GCM_GCM_COMMAND_BUFFER_INL
