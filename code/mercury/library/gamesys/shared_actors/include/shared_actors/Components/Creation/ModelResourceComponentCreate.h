#ifndef MODEL_RESOURCE_COMPONENTS_CREATE_H
#define MODEL_RESOURCE_COMPONENTS_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource_utils/ResourceUtilsDeclare.h>
#include <shared_actors/Components/SharedComponents.h>

// Model Resource Components

namespace Components
{

// Model from a file resource
class C_FileModelResourceCreateBase : public I_GameComponentCreator
{
public:
	virtual bool			IsValid(void) const = 0;
	virtual Util::C_String	GetFileName(void) const = 0;
};

// Model from a file resource
class C_FileModelResourceCreate : public C_FileModelResourceCreateBase
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_FileModelResource;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_FileModelResource;}

	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);
	virtual bool				IsValid(void) const;
	virtual Util::C_String		GetFileName(void) const;
public:

	XtHandle<ResourceUtils::C_GameModel>		m_ModelHandle;			
};

// Model from a file resource
class C_FileNameModelResourceCreate : public C_FileModelResourceCreateBase
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_FileNameModelResource;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_FileNameModelResource;}

	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);
	virtual bool				IsValid(void) const {return m_Filename.Length() > 0;}
	virtual Util::C_String		GetFileName(void) const {return m_Filename;}

public:

	Util::C_String				m_Filename;			
};

// Model from a level (platform in ye olde speak)
class C_LevelModelResourceCreate : public I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_LevelModelResource;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_LevelModelResource;}

	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);
public:
};

} //namespace Components

#endif