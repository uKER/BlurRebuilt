#ifndef CYCLERESOURCEDEF_879AAD3C_67FF_485C_81B1_89AA98706963_H
#define CYCLERESOURCEDEF_879AAD3C_67FF_485C_81B1_89AA98706963_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource/TypeRegistry.h>
#include <resource_utils/ResourceUtilsDeclare.h>
#include <xt/XtGame.h>


namespace ResourceUtils
{

struct C_XtCycleResourceDef
{
public:
	XtSz8	m_FilenameMinusExt;
	bool	m_Loopable;
	
	C_XtCycleResourceDef();
	~C_XtCycleResourceDef();

	Util::C_String GetFileName() const;
	static Resource::C_ResourceTypeId GetResourceTypeId();

private:
	C_XtCycleResourceDef(const C_XtCycleResourceDef& copy); // can not copy
	C_XtCycleResourceDef& operator = (const C_XtCycleResourceDef& copy); // can not copy

};

} // namespace ResourceUtils


#endif // CYCLERESOURCEDEF_879AAD3C_67FF_485C_81B1_89AA98706963_H
