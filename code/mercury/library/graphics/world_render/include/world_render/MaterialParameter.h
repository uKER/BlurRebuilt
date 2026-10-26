#ifndef WORLD_RENDER_RENDER_MATERIAL_PARAMETER_H
#define WORLD_RENDER_RENDER_MATERIAL_PARAMETER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace WorldRender
{

// MaterialParameter

struct MaterialParameter
{
	MaterialParameter():
		index(-1),
		offset(-1),
		size(0)
	{}
	
	S16 index;
	S16 offset;
	U16	size;
};

} // namespace WorldRender


#endif // WORLD_RENDER_RENDER_MATERIAL_PARAMETER_H

