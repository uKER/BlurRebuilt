#include "ResourceUtilsPch.h"
#include <resource_utils/GameModel.h>

#include <game_resource/ModelResource.h>


XT_TYPE(ResourceUtils::C_GameModel, "GameModel")
	XT_FIELD(m_Filename, "Filename")
	XT_FIELD(m_Flags, "Flags")
XT_END_TYPE


namespace ResourceUtils
{

C_GameModel::C_GameModel()
	: m_Filename()
	, m_Flags(0)
{
}

C_GameModel::~C_GameModel()
{
}

Util::C_String C_GameModel::GetFileName() const
{
	return Util::C_String(m_Filename.Text());
}

Resource::C_ResourceTypeId C_GameModel::GetResourceTypeId()
{
	return GameResource::C_ModelResource::StaticGetResourceTypeId();
}

} // namespace ResourceUtils
