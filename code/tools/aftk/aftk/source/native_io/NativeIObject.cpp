#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeUtility.h"
#include "NativeFtk.h"

using namespace Aftk;

C_Status C_NativeIObject::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const I_Object* pObject)
{
	//As an optimisation we don't save the 'this' pointer for GenericAttributes as we do not
	//allow connections to these.
	//Serialise this node's integer mapped 'this' pointer.
	*pDiskWriter <<	pNativeFtk->GetIdFromPtr(pObject);

	return C_Status::E_Status_Success;
}

C_Status C_NativeIObject::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	I_Object* pObject)
{
	//Deserialise this node's id, in other words its integer mapped 'this' pointer.
	S32 id;
	*pDiskReader >> id;
	//We know this node's true this pointer, and so we can add this to our mapping table.
	//This allows us to rebuild the relationship pointers after loading the scene.
	pNativeFtk->AddToIdPtrMapping(id, pObject);

	return C_Status::E_Status_Success;
}

