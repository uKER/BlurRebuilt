#ifndef SHARED_ACTORS_MODEL_RESOURCE_USAGE_H
#define SHARED_ACTORS_MODEL_RESOURCE_USAGE_H

#include <world_render/RenderModelResourceUsage.h>


namespace DbUtil
{
	class I_DebugRender;
}

namespace Debug
{
	class Logger;
}

namespace Sys
{
	class I_File;
}

namespace WorldRender
{
	class C_ModelInstance;
	class C_RenderModelData;
}


class C_ModelResourceUsage
{
public:
	C_ModelResourceUsage();
	~C_ModelResourceUsage();

	void AddModelResource(const WorldRender::C_RenderModelData* pRenderModelData, const S32 modelIndex = -1);
	void AddModelInstance(const WorldRender::C_ModelInstance* pModelInstance);

	U32 CalcTotalGraphicsBytes() const;

	void DebugRender(DbUtil::I_DebugRender& debugRender) const;
	void DebugDump(Debug::Logger& debugLogger) const;
	void DebugDump(Sys::I_File& file) const;

private:
	WorldRender::RenderModelResourceUsage	m_WorldRenderUsage;

};


#endif // SHARED_ACTORS_MODEL_RESOURCE_USAGE_H
