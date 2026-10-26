#ifndef AFTK_SCENE_MESH_WELD_COMPONENT_AXIS_H
#define AFTK_SCENE_MESH_WELD_COMPONENT_AXIS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	struct C_MeshWeldComponentAxis
	{
		bool operator < (const C_MeshWeldComponentAxis &rhs) const
		{
			if (axis < rhs.axis)
			{
				return true;
			}
			return false;
		}

		F32 axis;
		U32 index;
	};
}

#endif
