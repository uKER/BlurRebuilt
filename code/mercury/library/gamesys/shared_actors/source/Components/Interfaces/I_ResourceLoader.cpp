#include "SharedActorsPch.h"

#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>


namespace Components
{	
	
I_ResourceLoader::I_ResourceLoader() :
	Actor::I_ActorComponent()
{
	// intentionally blank
}

I_ResourceLoader::~I_ResourceLoader()
{
	// intentionally blank
}

bool I_ResourceLoader::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
	case ActorMsg::E_ResourceMsg_Load:		RequestLoad(Messaging::MessageCast<ActorMsg::C_ResourceLoadMsg>(pMessage)->GetValue());			return true;
	case ActorMsg::E_ResourceMsg_Unload:	RequestUnload(Messaging::MessageCast<ActorMsg::C_ResourceUnloadMsg>(pMessage)->GetValue());		return true;
	case ActorMsg::E_ResourceMsg_LoadAll:	LoadAll();																						return true;
	case ActorMsg::E_ResourceMsg_UnloadAll:	UnloadAll();																					return true;
	}

	return false;
}

void I_ResourceLoader::LoadAll()
{
	const U32 numResources = GetNumResources();
	for (U32 i = 0; i < numResources; ++i)
		RequestLoad(i);
}

void I_ResourceLoader::UnloadAll()
{
	const U32 numResources = GetNumResources();
	for (U32 i = 0; i < numResources; ++i)
		RequestUnload(i);
}

} // namespace Components
