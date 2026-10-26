#ifndef SPATIAL_DIVIDER_COMMON_H
#define SPATIAL_DIVIDER_COMMON_H

#include <malloc.h>

enum SpatialDividerStructureEnum
{
	SPATIAL_DIVIDER_STRUCTURE_FLAT,
	SPATIAL_DIVIDER_STRUCTURE_BSP,
	SPATIAL_DIVIDER_STRUCTURE_KD,

	SPATIAL_DIVIDER_STRUCTURE_COUNT
};

inline int Log2Int(float v) {
	return ((*(int *) &v) >> 23) - 127;
}

#include <core/containers/ContainerSerialisation.h>

#endif
