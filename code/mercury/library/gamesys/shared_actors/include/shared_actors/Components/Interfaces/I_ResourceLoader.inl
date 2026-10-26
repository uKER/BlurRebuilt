#include <actor_system/ActorDebug.h>


namespace Components
{

/**
@param index index of the resource object.
@returns NULL if the resource is not loaded.
*/
template <class T_Resource>
T_Resource* I_ResourceLoader::GetResource(const U32 index) const 
{
	if (index < GetNumResources())
	{
		Resource::C_ResourceBase* pBase = GetResourceBase(index);

		if (pBase != NULL)
		{
			const Resource::C_ResourceTypeId expectedType = Resource::C_Resource<T_Resource>::StaticGetResourceTypeId();

			if (pBase->GetResourceTypeId() == expectedType)
			{
				Resource::C_Handle<T_Resource> handle;
				handle = pBase;
				return handle;
			}
		
			LOGERROR(Actor::GetDebugLogAsset(pBase->GetResourceFilename()), 
				"incorrect resource type for I_ResourceLoaderComponent::GetResource, expected '%s' got '%s'", 
				Resource::C_TypeRegistry::GetInstance()->GetResourceTypeName(expectedType).Chars(), 
				pBase->GetResourceTypeName().Chars());
		}
	}
	else
	{
		LOGERROR(Actor::GetDebugLogGeneral(), 
			"%d out of range %d for resource index for I_ResourceLoaderComponent::GetResource", 
			index, 
			GetNumResources());
	}

	return NULL;
}

} // namespace Components
