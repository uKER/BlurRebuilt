#ifndef RESOURCELOADERIMPL_23367513_6910_4F17_9E0A_22A9DD886850_H
#define RESOURCELOADERIMPL_23367513_6910_4F17_9E0A_22A9DD886850_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>


namespace Components
{

/**
@brief Helper for implementations of I_ResourceLoader.
Provides framework for managing resource requests using handle objects.
*/
class C_ResourceLoaderImpl : public I_ResourceLoader, public Resource::C_Listener
{
public:
	C_ResourceLoaderImpl();
	virtual ~C_ResourceLoaderImpl();

	virtual void							ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData); // from Resource::C_Listener

protected:
	/// States for resources
	enum E_State
	{
		E_State_None,		///< Data not requested
		E_State_Loading,	///< Load requested
		E_State_Loaded,		///< Loaded
		E_State_Unloading,	///< Unload requested
		E_State_Error,		///< Errors encountered
		E_State_Count
	};

	/// Resource item data.
	struct C_Item
	{
		Resource::C_HandleBase				m_Handle;	///< resource handle
		E_State								m_State;	///< resource state

		C_Item();
		~C_Item();
	};

	virtual const C_ResourceDesc*			GetDesc(const U32 index) const = 0;
	virtual C_Item*							GetItem(const U32 index) = 0;
	virtual U32								GetIndexOf(const C_Item* pItem) const = 0;

	virtual void							RequestLoad(const U32 index); // from I_ResourceLoader
	virtual void							RequestUnload(const U32 index); // from I_ResourceLoader

	virtual bool							PrimeImpl(); // from Component::I_GameComponent

private:
	virtual Resource::C_ResourceBase*		GetResourceBase(const U32 index) const; // from I_ResourceLoader

	void									SetState(C_Item* pItem, const E_State state);

	bool									PollForPrimed() const; ///< Checks if all resources are primed

	virtual const C_Item*					GetItem(const U32 index) const {return const_cast<C_ResourceLoaderImpl*>(this)->GetItem(index);}

};

/**
@brief Implements I_ResourceLoader for a single resource item.
*/
class C_ResourceItem : public C_ResourceLoaderImpl
{
public:
	C_ResourceItem();
	virtual ~C_ResourceItem();

	static Component::C_ComponentTypeId		ClassType() {return E_Resource_Item;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return E_Resource_Item;} // from Component::I_GameComponent

	virtual U32								GetNumResources() const; // from I_ResourceLoader

	const C_ResourceItemCreate*				GetCreationData() const {return static_cast<const C_ResourceItemCreate*>(GetCreator().GetData());}

protected:
	virtual const C_ResourceDesc*			GetDesc(const U32 index) const;	// from C_ResourceLoaderImpl
	virtual C_Item*							GetItem(const U32 index); // from C_ResourceLoaderImpl
	virtual U32								GetIndexOf(const C_Item* pItem) const; // from C_ResourceLoaderImpl

	virtual U32								GetResourceNotificationFlags() const; // from I_ResourceLoader

private:
	C_Item									m_Item;	///< resource item

};

/**
@brief Implements I_ResourceLoader for a list of resource items.
*/
class C_ResourceList : public C_ResourceLoaderImpl
{
public:
	C_ResourceList();
	virtual ~C_ResourceList();

	static Component::C_ComponentTypeId		ClassType() {return E_Resource_List;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return E_Resource_List;} // from Component::I_GameComponent

	virtual U32								GetNumResources() const; // from I_ResourceLoader

	const C_ResourceListCreate*				GetCreationData() const {return static_cast<const C_ResourceListCreate*>(GetCreator().GetData());}

protected:
	virtual bool							PrimeImpl(); // from Component::I_GameComponent

	virtual const C_ResourceDesc*			GetDesc(const U32 index) const;	// from C_ResourceLoaderImpl
	virtual C_Item*							GetItem(const U32 index); // from C_ResourceLoaderImpl
	virtual U32								GetIndexOf(const C_Item* pItem) const; // from C_ResourceLoaderImpl

	virtual U32								GetResourceNotificationFlags() const; // from I_ResourceLoader

private:
	typedef Btl::Vector<C_Item>				C_Items;
	C_Items									m_Items;

};

} // namespace Components


#endif // RESOURCELOADERIMPL_23367513_6910_4F17_9E0A_22A9DD886850_H
