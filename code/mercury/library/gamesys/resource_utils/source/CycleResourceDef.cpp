#include "ResourceUtilsPch.h"
#include <resource_utils/CycleResourceDef.h>

#include <animation/resource/CycleResource.h>


XT_TYPE(ResourceUtils::C_XtCycleResourceDef, "CycleResourceDef")
	XT_FIELD(m_FilenameMinusExt, "FilenameMinusExt")
	XT_FIELD(m_Loopable, "Loopable")
XT_END_TYPE


namespace ResourceUtils
{

C_XtCycleResourceDef::C_XtCycleResourceDef()
	: m_FilenameMinusExt()
	, m_Loopable(false)
{
}

C_XtCycleResourceDef::~C_XtCycleResourceDef()
{
}

Util::C_String C_XtCycleResourceDef::GetFileName(void) const
{
	Util::C_String filename(m_FilenameMinusExt.Text());
	filename += ".cycle";
	return filename;
}

Resource::C_ResourceTypeId C_XtCycleResourceDef::GetResourceTypeId()
{
	return Animation::C_CycleResource::StaticGetResourceTypeId();
}

} // namespace ResourceUtils
