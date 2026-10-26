#include "stdafx.h"
#include "Display/Resource/Resource.h"

namespace Display
{

C_Resource::C_Resource() :
	m_RefCount(0)
{
	#ifdef TRACK_RENDERED_RESOURCES
	m_Rendered = 0;
	#endif
}

C_Resource::~C_Resource()
{
#ifndef TOOL
	BIZ_ASSERT(m_RefCount == 0);
#endif
}

}; // namespace Display
