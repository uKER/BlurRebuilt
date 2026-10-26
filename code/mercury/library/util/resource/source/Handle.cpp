#include "stdafx.h"

#include <resource/Resource.h>

namespace Resource
{

bool C_HandleBase::Load( const Sys::C_FilePath& filename, const C_ResourceTypeId typeId, C_Listener* pListener, void* pListenerData )
{
	C_Manager::GetInstance()->MutexWait();
	C_ResourceBase* const pResource = C_Manager::GetInstance()->GetResource( filename, typeId );
	*this = pResource;
	pResource->SetMemoryTrackerContextID();
	const bool result = C_Manager::GetInstance()->Load( pResource );
	if(pListener)
	{
		pResource->AddListener( pListener, pListenerData );
	}
	C_Manager::GetInstance()->MutexRelease();
	return true;
}

} // namespace Resource
