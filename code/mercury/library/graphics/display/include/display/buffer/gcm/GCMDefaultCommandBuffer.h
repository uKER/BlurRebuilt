#ifndef DISPLAY_GCM_DEFAULT_COMMAND_BUFFER_H
#define DISPLAY_GCM_DEFAULT_COMMAND_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

bool InitDefaultCommandBuffer(const U32 commandBufferSize = 1024*32);

}; // namespace Display

#endif // DISPLAY_GCM_DEFAULT_COMMAND_BUFFER_H
