#ifndef DISPLAY_DEVICE_GCM_GCM_DEBUG_H
#define DISPLAY_DEVICE_GCM_GCM_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCMDeviceTypes.h"

namespace Display
{
namespace GCM
{

void DebugOutput(CellCgbProgram* pProgram);
void DebugOutput(CellCgbVertexProgramConfiguration* pConfig);
void DebugOutput(CellCgbFragmentProgramConfiguration* pConfig);

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_DEBUG_H
