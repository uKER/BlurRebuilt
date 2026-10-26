#ifndef MODE_RESOURCE_COMPONENT_H
#define MODE_RESOURCE_COMPONENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource/TypeRegistry.h>
#include <resource/Resource.h>
#include <game_resource/ModelResource.h>
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/Creation/ModelResourceComponentCreate.h>
#include <shared_actors/Components/Interfaces/I_ModelResource.h>

namespace Components
{


// Model Resource Component Base Class
class C_FileModelResourceBase : public I_ModelResource, public Resource::C_Listener
{
public:
	C_FileModelResourceBase():m_pModelResource(NULL){}
	~C_FileModelResourceBase()	{}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_FileModelResource;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_FileModelResource;} // from Component::I_GameComponent
	const C_FileModelResourceCreateBase	*GetCreationData(void) const {return static_cast<const C_FileModelResourceCreateBase *>(GetCreator().GetData());}

	// from Resource::C_Listener
	virtual void					ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData); 

	// from I_ModelResource
	virtual const WorldRender::C_RenderModelData *GetRenderModelData(void) const;
	virtual const GameResource::C_ModelData* GetModelData(void) const;
#ifndef CFG_GOLD
	virtual bool HasModelResourceUsageDetails() const {return true;}
#endif

	// from Component::I_GameComponent
	virtual bool					PrimeImpl(); 

private:
	GameResource::C_ModelHandle		m_ResourceHandle;
	GameResource::C_ModelResource*	m_pModelResource;

};

// File model resource that gets the file name from an Xt Game Model record
class C_FileModelResource : public C_FileModelResourceBase
{
public:
	C_FileModelResource() {}
	~C_FileModelResource()	{}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_FileModelResource;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_FileModelResource;} // from Component::I_GameComponent
};

// File model resource that gets the file name from a string
class C_FileNameModelResource : public C_FileModelResourceBase
{
public:
	C_FileNameModelResource() {}
	~C_FileNameModelResource()	{}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_FileNameModelResource;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_FileNameModelResource;} // from Component::I_GameComponent
};

// Level Model Resource Component
class C_LevelModelResource : public I_ModelResource
{
public:
	C_LevelModelResource(){}
	~C_LevelModelResource()	{}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_LevelModelResource;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_LevelModelResource;} // from Component::I_GameComponent
	const C_LevelModelResourceCreate	*GetCreationData(void) const {return static_cast<const C_LevelModelResourceCreate *>(GetCreator().GetData());}

	// from Resource::C_Listener
	//virtual void					ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData); 

	// from I_ModelResource
	virtual const WorldRender::C_RenderModelData *GetRenderModelData(void) const;
	virtual const GameResource::C_ModelData* GetModelData(void) const;
#ifndef CFG_GOLD
	virtual bool HasModelResourceUsageDetails() const {return false;}
#endif

	// from Component::I_GameComponent
	//virtual bool					PrimeImpl(); 

private:

};


}//namespace Components

#endif