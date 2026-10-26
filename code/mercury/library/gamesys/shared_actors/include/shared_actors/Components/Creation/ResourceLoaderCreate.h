#ifndef RESOURCELOADERCREATE_A09A9A6D_EB8F_4F9C_A3DE_A738E6232A9F_H
#define RESOURCELOADERCREATE_A09A9A6D_EB8F_4F9C_A3DE_A738E6232A9F_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/GameComponent.h>
#include <btl/containers/vector.h>
#include <resource/TypeRegistry.h>
#include <shared_actors/Components/ResourceComponents.h>


namespace Components
{

enum E_ResourceFlag
{
	E_ResourceFlag_Prime		= 1 << 0, 
	E_ResourceFlag_Critical		= 1 << 1, 
};

struct C_ResourceDesc
{
	Util::C_String				m_Name;		///< name of the resource
	Resource::C_ResourceTypeId	m_Type;		///< type of the resource
	U32							m_Flags;	///< bitfield of E_ResourceFlag

	C_ResourceDesc();

	inline bool GetFlag(const E_ResourceFlag flag) const {return (m_Flags & flag) != 0;}
	inline void SetFlag(const E_ResourceFlag flag, const bool value) 
	{
		if (value)
			m_Flags |= flag;
		else
			m_Flags &= ~flag;
	}

	inline bool GetPrime() const {return GetFlag(E_ResourceFlag_Prime);}
	inline void SetPrime(const bool value) {SetFlag(E_ResourceFlag_Prime, value);}

	inline bool GetCritical() const {return GetFlag(E_ResourceFlag_Critical);}
	inline void SetCritical(const bool value) {SetFlag(E_ResourceFlag_Critical, value);}
};

class C_ResourceItemCreate : public Component::I_ComponentCreator
{
public:
	C_ResourceDesc	m_Desc;							///< resource to load
	U32				m_ResourceNotificationFlags;	///< E_ResourceNotificationFlag bitfield

	C_ResourceItemCreate();
	virtual ~C_ResourceItemCreate();

	static Component::C_ComponentTypeId		ClassType() {return E_Resource_Item;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return E_Resource_Item;} // from Component::I_ComponentCreator

};

class C_ResourceListCreate : public Component::I_ComponentCreator
{
public:
	Btl::Vector<C_ResourceDesc>	m_List;							///< list of resources to load
	U32							m_ResourceNotificationFlags;	///< E_ResourceNotificationFlag bitfield

	C_ResourceListCreate();
	virtual ~C_ResourceListCreate();

	U32 AddItem(const Util::C_String& resourceName, const Resource::C_ResourceTypeId typeId, const U32 resourceFlags = 0);
	void SetItem(const U32 index, const Util::C_String& resourceName, const Resource::C_ResourceTypeId typeId, const U32 resourceFlags = 0);

	static Component::C_ComponentTypeId		ClassType() {return E_Resource_List;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return E_Resource_List;} // from Component::I_ComponentCreator

};

} // namespace Components


#endif // RESOURCELOADERCREATE_A09A9A6D_EB8F_4F9C_A3DE_A738E6232A9F_H
