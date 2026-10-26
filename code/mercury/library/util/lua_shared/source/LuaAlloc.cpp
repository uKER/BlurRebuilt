#include "LuaSharedPch.h"

#include <core/Core.h>


namespace Lua
{

void* LuaAlloc( const U32 size )
{
	return (void*)new U8[size];
}

void LuaFree( void* ptr )
{
	delete [](U8*)ptr;
}

} // namespace Lua
