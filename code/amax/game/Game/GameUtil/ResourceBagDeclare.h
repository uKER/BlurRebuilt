#ifndef GAME_RESOURCEBAGDECLARE_H
#define GAME_RESOURCEBAGDECLARE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource_utils/ResourceUtilsDeclare.h>
#include "core/utility/SmartPtr.h"

class C_ResourceBag;
class C_ResourceBagListener; ///< Listens for resource bags loading.

typedef Sys::RefPtr<C_ResourceBag> C_ResourceBagHandle; ///< Holds a reference to a resource bag.

#endif // GAME_RESOURCEBAGDECLARE_H
