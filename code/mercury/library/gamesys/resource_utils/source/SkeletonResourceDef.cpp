#include "ResourceUtilsPch.h"
#include <resource_utils/SkeletonResourceDef.h>

#include <animation/resource/SkeletonResource.h>


XT_TYPE(ResourceUtils::C_XtSkeletonResourceDef, "SkeletonResourceDef")
	XT_FIELD(m_FilenameMinusExt, "FilenameMinusExt")
XT_END_TYPE


namespace ResourceUtils
{

C_XtSkeletonResourceDef::C_XtSkeletonResourceDef()
	: m_FilenameMinusExt()
{
}

C_XtSkeletonResourceDef::~C_XtSkeletonResourceDef()
{
}

Util::C_String C_XtSkeletonResourceDef::GetFileName() const
{
	Util::C_String filename(m_FilenameMinusExt.Text());
	filename += ".skeleton";
	return filename;
}

Resource::C_ResourceTypeId C_XtSkeletonResourceDef::GetResourceTypeId()
{
	return Animation::C_SkeletonResource::StaticGetResourceTypeId();
}

} // namespace ResourceUtils
