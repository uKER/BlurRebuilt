#ifndef GAME_RESOURCE_MODEL_RESOURCE_H
#define GAME_RESOURCE_MODEL_RESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

#include <resource/Resource.h>
#include <game_resource/ModelResource.h>
#include <game_resource/ModelTypeManager.h>

namespace MetaDebug
{
	class C_MetaNode;
}

namespace WorldRender
{
	class C_RenderModel;
	class C_RenderModelData;
};

namespace Gravity
{
	class C_Scene;
	class C_ImportScene;
};

namespace GameResource
{

class C_ModelData;
class C_ModelResource;

typedef Resource::C_Handle<C_ModelResource> C_ModelHandle;

//! C_ModelResource 
class C_ModelResource : public Resource::C_Resource<C_ModelResource>
{
public:
	C_ModelResource();

	~C_ModelResource();

	// Inherited from C_Resource
	// |------------------------
	bool LoadResourceData();
	bool FreeResourceData();
	bool PostLoadResourceData();
	bool PreFreeResourceData();
	S32 GetEvictDelay();
	static Util::C_String StaticGetResourceTypeName();
	// |------------------------

	void SetEvictDelay(const S32 evictDelay);
	GameResource::C_ModelData*	GetModelData();
	const GameResource::C_ModelData* GetModelData() const;
	WorldRender::C_RenderModelData* GetRenderModelData();
	const WorldRender::C_RenderModelData* GetRenderModelData() const;

	// returns NULL in gold build
	MetaDebug::C_MetaNode*		GetRootMetaNode() const;

private:
	bool LoadModelData( Sys::I_File* pFile, const Util::C_String& paramType, S32 lodLevel = INT_MAX );
	void SetDebugNames();

private:
	S32							m_EvictDelay;
	GameResource::C_ModelData*	m_pModelData;
	WorldRender::C_RenderModel*	m_pRenderModel;
	Gravity::C_ImportScene*		m_pPhysicsImport;
};

/* 
===============================================================
class C_ModelTypeCar

NOTES: Car model type for dynamic material setting
===============================================================
*/
class C_ModelType : public GameResource::I_ModelType
{
public:
	C_ModelType(){}
	void OnLoad(GameResource::C_ModelResource* p_pModelResource);
public:
};

void RegisterDefaultModelType();
void RefreshModelTextures();

} // namespace GameResource

#endif // GAME_RESOURCE_MODEL_H
