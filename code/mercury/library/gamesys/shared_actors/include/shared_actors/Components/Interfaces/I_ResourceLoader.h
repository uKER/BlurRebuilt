#ifndef I_RESOURCELOADER_CD8FF559_D980_4C1B_A99B_448A6616B23E_H
#define I_RESOURCELOADER_CD8FF559_D980_4C1B_A99B_448A6616B23E_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <resource/Resource.h>
#include <shared_actors/Components/ResourceComponents.h>

namespace Resource { class C_ResourceBase; }


namespace Components
{

/// Controls which notification messages are sent
enum E_ResourceNotificationFlag
{
	E_ResourceNotificationFlag_LoadItem		= 1 << 0,	///< E_ResourceMsg_Loaded
	E_ResourceNotificationFlag_UnloadItem	= 1 << 1,	///< E_ResourceMsg_Unloaded
	E_ResourceNotificationFlag_ErrorItem	= 1 << 2,	///< E_ResourceMsg_Error
	E_ResourceNotificationFlag_LoadAll		= 1 << 3,	///< E_ResourceMsg_AllLoaded 
	E_ResourceNotificationFlag_UnloadAll	= 1 << 4,	///< E_ResourceMsg_AllUnloaded
	E_ResourceNotificationFlag_CriticalAll	= 1 << 5,	///< E_ResourceMsg_AllCritical
};

/// Default values for E_ResourceNotificationFlag bitfields
const U32 kDefaultResourceNotificationFlags = 0
	| E_ResourceNotificationFlag_LoadItem
	| E_ResourceNotificationFlag_UnloadItem
	| E_ResourceNotificationFlag_LoadAll
	| E_ResourceNotificationFlag_UnloadAll
	| E_ResourceNotificationFlag_CriticalAll
	;

/**
@brief Interface for resource loading components.

Implementation of this interface may wait for some resources to load before 
priming. To load or unload resources after this, use C_ResourceLoadMsg and
C_ResourceUnloadMsg.

Once the component has been created, subsequent resource events will be 
reported to the owning actor via C_ResourceLoadedMsg, C_ResourceUnloadedMsg
and C_ResourceErrorMsg. When all resources are loaded or unloaded the
E_ResourceMsg_AllLoaded and	E_ResourceMsg_AllUnloaded messages are sent.
*/
class I_ResourceLoader : public Actor::I_ActorComponent
{
public:
	I_ResourceLoader();
	virtual ~I_ResourceLoader();

	static Component::C_ComponentInterfaceId	InterfaceType() {return E_ResourceIF_Loader;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return E_ResourceIF_Loader;} // from Component::I_GameComponent
	
	virtual U32									GetNumResources() const = 0; ///< Gets the number of resources.
	template <class T_Resource> T_Resource*		GetResource(const U32 index) const; /// Gets a resource by index.

	virtual bool								OnMessage(const Messaging::C_MessageBase* pMessage);

protected:
	virtual void								RequestLoad(const U32 index) = 0; ///< Requests the resource to load. 
	virtual void								RequestUnload(const U32 index) = 0; ///< Requests the resource to unload.

	void										LoadAll(); ///< Requests all resources to load.
	void										UnloadAll(); ///< Requests all resources to unload.

	virtual U32									GetResourceNotificationFlags() const = 0; ///< Gets the E_ResourceNotificationFlag bitfield

private:
	virtual Resource::C_ResourceBase*			GetResourceBase(const U32 index) const = 0; ///< Gets the resource object.

};

} // namespace Components


#include <shared_actors/Components/Interfaces/I_ResourceLoader.inl>

#endif // I_RESOURCELOADER_CD8FF559_D980_4C1B_A99B_448A6616B23E_H
