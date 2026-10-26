#ifndef SKELETONRESOURCEDEF_B2A656CC_A53D_4CB3_BDDE_1B1C0FC119F3_H
#define SKELETONRESOURCEDEF_B2A656CC_A53D_4CB3_BDDE_1B1C0FC119F3_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource/TypeRegistry.h>
#include <resource_utils/ResourceUtilsDeclare.h>
#include <xt/XtGame.h>


namespace ResourceUtils
{

struct C_XtSkeletonResourceDef //TODO this must inherit from game resource
{
public:
	XtSz8	m_FilenameMinusExt;
	
	C_XtSkeletonResourceDef();
	~C_XtSkeletonResourceDef();

	Util::C_String GetFileName() const;
	static Resource::C_ResourceTypeId GetResourceTypeId();

private:
	C_XtSkeletonResourceDef(const C_XtSkeletonResourceDef& copy); // can not copy
	C_XtSkeletonResourceDef& operator = (const C_XtSkeletonResourceDef& copy); // can not copy

};

} // namespace ResourceUtils


#endif // SKELETONRESOURCEDEF_B2A656CC_A53D_4CB3_BDDE_1B1C0FC119F3_H
