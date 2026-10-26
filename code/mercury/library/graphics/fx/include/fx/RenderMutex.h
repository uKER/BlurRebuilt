#ifndef __FX_RENDERMUTEX_H__
#define __FX_RENDERMUTEX_H__

#include <core/threads/Mutex.h>

namespace FX
{

struct RenderMutex
{
	MT::Lock m_Mutex;
	enum
	{
		E_None,
		E_RenderMutex,
		E_SerialiseMutex,
	} m_LockType;
};

extern RenderMutex g_RenderMutex;

#if !PLATFORM_SPU

/// get mutex for rendering
inline void LockRenderMutex() { g_RenderMutex.m_Mutex.Enter(); BIZ_ASSERT(g_RenderMutex.m_LockType == RenderMutex::E_None); g_RenderMutex.m_LockType = RenderMutex::E_RenderMutex; }

/// get mutex for serialisation
inline void LockSerialiseMutex() { g_RenderMutex.m_Mutex.Enter(); BIZ_ASSERT(g_RenderMutex.m_LockType == RenderMutex::E_None); g_RenderMutex.m_LockType = RenderMutex::E_SerialiseMutex; }

inline bool TrySerialiseMutex() 
{
	if (g_RenderMutex.m_Mutex.TryEnter()) 
	{
		BIZ_ASSERT(g_RenderMutex.m_LockType == RenderMutex::E_None); 
		g_RenderMutex.m_LockType = RenderMutex::E_SerialiseMutex; 
		return true;
	}

	return false;
}

/// check whether the render lock has been acquired
inline void CheckRenderMutex() 
{ 
#ifndef CFG_GOLD
	if (g_RenderMutex.m_LockType != RenderMutex::E_RenderMutex)
	{
		BIZ_ASSERT_FAIL("Render mutex not acquired, currently enum value %u", g_RenderMutex.m_LockType);
	}
#endif
}

/// check whether the serialisation lock has been acquired
inline void CheckSerialiseMutex() 
{
#ifndef CFG_GOLD
	if (g_RenderMutex.m_LockType != RenderMutex::E_SerialiseMutex)
	{
		BIZ_ASSERT_FAIL("Render mutex not acquired, currently enum value %u", g_RenderMutex.m_LockType);
	}
#endif
}

/// release rendering mutex
inline void ReleaseRenderMutex() { BIZ_ASSERT(g_RenderMutex.m_LockType == RenderMutex::E_RenderMutex); g_RenderMutex.m_LockType = RenderMutex::E_None; g_RenderMutex.m_Mutex.Exit(); }

/// release serialisation mutex
inline void ReleaseSerialiseMutex() { BIZ_ASSERT(g_RenderMutex.m_LockType == RenderMutex::E_SerialiseMutex); g_RenderMutex.m_LockType = RenderMutex::E_None; g_RenderMutex.m_Mutex.Exit(); }

#else

/// check whether the render lock has been acquired
inline void CheckRenderMutex() { }

/// check whether the serialisation lock has been acquired
inline void CheckSerialiseMutex() { }

#endif

}

#endif
