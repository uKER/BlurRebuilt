#ifndef DISPLAY_OCCLUSION_OCCLUSION_QUERYTYPES_H
#define DISPLAY_OCCLUSION_OCCLUSION_QUERYTYPES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

//! E_OcclusionQueryResult

enum E_OcclusionQueryResult
{
	E_OcclusionQueryResult_NotVisible	= 0,
	E_OcclusionQueryResult_Visible		= 1,
	E_OcclusionQueryResult_Failed		= 2,
};

}; // namespace Display

#endif // DISPLAY_OCCLUSION_OCCLUSION_QUERYTYPES_H
