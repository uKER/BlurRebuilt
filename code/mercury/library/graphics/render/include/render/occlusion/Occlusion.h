#ifndef RENDER_OCCLUSION_OCCLUSION_H
#define RENDER_OCCLUSION_OCCLUSION_H

#if _MSC_VER > 1000
#pragma once
#endif



#include "Render/Occlusion/Pool.h"
#include "Render/Occlusion/Query.h"



namespace Render
{
namespace Occlusion
{



void Begin(const bool show = false);
void End();



}; // namespace Occlusion
}; // namespace Render



#endif // RENDER_OCCLUSION_OCCLUSION_H
