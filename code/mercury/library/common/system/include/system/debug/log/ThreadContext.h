#ifndef __SYSTEM_LOG_THREAD_CONTEXT__H__
#define __SYSTEM_LOG_THREAD_CONTEXT__H__

namespace Debug
{

class ThreadContext
{
public:
	/// Push context item onto the stack
	static void Push(const Util::C_String &item);
	
	/// Pop context item from the stack
	static void Pop();

	/// Get current thread context item
	static const Util::C_String &GetContext();

	static void CleanUp();
};

}

#endif
