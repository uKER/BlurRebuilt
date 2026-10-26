#ifndef RESOURCE_CONTEXT_H
#define RESOURCE_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/context/I_Anything.h>
#include <misc_shared/uid/FourCC.h>


namespace GameResource { class C_LevelResource; }

namespace GameContext
{

class I_ResourceContext : public Sys::C_Anything<FOURCC_U32('IADC')>
{
public:
	virtual GameResource::C_LevelResource* GetLevelResource() const = 0;

protected:
	explicit I_ResourceContext(const Sys::C_AnythingUid typeUid) : C_Super(typeUid) {}
};

class C_ResourceContext : public Sys::C_Anything<FOURCC_U32('CADC'), I_ResourceContext>
{
public:
	explicit C_ResourceContext(GameResource::C_LevelResource* pLevelResource) : m_pLevelResource(pLevelResource){}

	virtual GameResource::C_LevelResource* GetLevelResource() const {return m_pLevelResource; }

private:
	GameResource::C_LevelResource* m_pLevelResource;
};

} // namespace GameContext


#endif // RESOURCE_CONTEXT_H
