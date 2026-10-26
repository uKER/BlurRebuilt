#include "stdafx.h"
#include "Render/Occlusion/Occlusion.h"

namespace Render
{
namespace Occlusion
{

void Begin(const bool show)
{
	Display::OcclusionQueryRender()->Bind(show);
}

void End()
{
	Display::OcclusionQueryRender()->Unbind();
}

}; // namespace Occlusion
}; // namespace Render
