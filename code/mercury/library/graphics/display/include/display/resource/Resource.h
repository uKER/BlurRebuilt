#ifndef DISPLAY_RESOURCE_RESOURCE_H
#define DISPLAY_RESOURCE_RESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(USER_LLOYDW) || defined(USER_PAULM)
#ifndef CFG_SPEED
#define TRACK_RENDERED_RESOURCES
#endif
#endif

namespace Display
{

//! Resource.

class C_Resource
{
public:

	C_Resource();
	virtual ~C_Resource();

	const S32 RefCount() const;
	const S32 Acquire();
	const S32 Release();

public:

	#ifdef TRACK_RENDERED_RESOURCES
	U32 m_Rendered;
	#endif

protected:

	S32 m_RefCount;
};

}; // namespace Display

#include "Display/Resource/Resource.inl"

#endif // DISPLAY_RESOURCE_RESOURCE_H
