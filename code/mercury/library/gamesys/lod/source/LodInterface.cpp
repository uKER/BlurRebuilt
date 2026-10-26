#include "LodPch.h"

#include <Lod/LodInterface.h>
#include <Lod/LodManager.h>

namespace Lod
{

// C_LodInterface

I_LodInterface::I_LodInterface()
{
	C_LodManager::Instance().AddLodInterface(this);
}

I_LodInterface::~I_LodInterface()
{
	C_LodManager::Instance().RemoveLodInterface(this);
}

} // Lod
