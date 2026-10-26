#ifndef RENDER_REDUCEDRENDER_ENUMS_H
#define RENDER_REDUCEDRENDER_ENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Reduced
{

enum E_Downsample
{
	E_Downsample_None = -1,
	E_Downsample_QuarterMSAA,
	E_Downsample_Quarter,
	E_Downsample_Sixteenth,
	E_Downsample_Count
};

} // namespace Reduced
} // namespace Render

#endif // RENDER_REDUCEDRENDER_ENUMS_H
