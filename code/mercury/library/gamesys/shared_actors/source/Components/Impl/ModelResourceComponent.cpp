#include "SharedActorsPch.h"

#include <shared_actors/Components/Impl/ModelResourceComponent.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorMessages.h>

#include <resource_utils/GameModel.h>
#include <game_context/ResourceContext.h>
#include <game_resource/LevelResource.h>


namespace Actors { extern Debug::Logger &g_Log; }

namespace Components
{
// File Model Resource Component
// -----------------------------

// Resource notify callback
void C_FileModelResourceBase::ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData)
{
	switch (result)
	{
	case Resource::E_Notify_NoData:		
		LOGERROR(Actor::GetDebugLogAsset(pResource->GetResourceFilename()), DBSTR"Model Resource Not Found");
		break;
	case Resource::E_Notify_DataReady:	
		if(GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
			SetPrimed();
		m_pModelResource = m_ResourceHandle;
		BIZ_ASSERT(m_pModelResource->GetModelData() != NULL);
		break;
	case Resource::E_Notify_DataError:	
		LOGERROR(Actor::GetDebugLogAsset(pResource->GetResourceFilename()), DBSTR"Model Resource Data Error");
		if(GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
			SendMessageToActor(Actor::E_ActorMsg_Kill);
		break;
	}
}

// Prime the component for loading
bool C_FileModelResourceBase::PrimeImpl()
{
	bool primed = I_ModelResource::PrimeImpl();

	if (!GetCreationData()->IsValid())
	{
		LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), "Invalid Model File Info");
		return false;
	}

	// Tell the resource system to load the resource
	/*primed &= */m_ResourceHandle.Load(
		GetCreationData()->GetFileName(),
		this,
		NULL);

	primed = ((GameResource::C_ModelResource*)m_ResourceHandle)->IsDataReady();

	//primed &= (m_pModelResource != NULL);
	//	if(primed)
	//		m_pModelResource = m_ResourceHandle;

	return primed;
}

// Interface Implementation
const WorldRender::C_RenderModelData *C_FileModelResourceBase::GetRenderModelData(void) const
{
	if(m_pModelResource)
		return m_pModelResource->GetRenderModelData();
	else
		return NULL;
}

const GameResource::C_ModelData* C_FileModelResourceBase::GetModelData() const
{
	if(m_pModelResource)
		return m_pModelResource->GetModelData();
	else
		return NULL;
}

// Level Model Resource
// This is the model data for the level model hierachy (platforms in olde speak)

// Interface Implementation
const WorldRender::C_RenderModelData *C_LevelModelResource::GetRenderModelData(void) const
{
	const GameContext::I_ResourceContext *pResourceContext = GetActor()->GetContext<GameContext::I_ResourceContext>();

	if(pResourceContext)
	{
		GameResource::C_LevelResource* pLevelResource = pResourceContext->GetLevelResource();

		if(pLevelResource)
			return pLevelResource->GetRenderModelData();
	}

	return NULL;
}

const GameResource::C_ModelData* C_LevelModelResource::GetModelData() const
{
	const GameContext::I_ResourceContext *pResourceContext = GetActor()->GetContext<GameContext::I_ResourceContext>();

	if(pResourceContext)
	{
		GameResource::C_LevelResource* pLevelResource = pResourceContext->GetLevelResource();

		if(pLevelResource)
			return pLevelResource->GetModelData();
	}

	return NULL;	
}

}