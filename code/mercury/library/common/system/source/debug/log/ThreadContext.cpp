#include "SystemPch.h"

#if !defined( COMPILER_VC )
#include <pthread.h>
#endif
#include <core/threads/threadlocalstorage.h>
#include "system/debug/log/debuglog.h"
#include "system/debug/log/threadcontext.h"
#include "system/debug/log/threadcontexttemplate.h"

namespace Debug
{

ThreadContextTemplate<Util::C_String> g_ThreadContext;

/// Push context item onto the stack
void ThreadContext::Push(const Util::C_String &item)
{
	return g_ThreadContext.Push(item);
}

/// Pop context item from the stack
void ThreadContext::Pop()
{
	g_ThreadContext.Pop();
}

/// Get current thread context item
const Util::C_String &ThreadContext::GetContext()
{
	return g_ThreadContext.GetContext();
};

void ThreadContext::CleanUp()
{
	g_ThreadContext.CleanUp();
}
} // namespace
