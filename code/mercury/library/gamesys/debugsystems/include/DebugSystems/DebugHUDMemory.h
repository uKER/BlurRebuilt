#ifndef DEBUGHUDMEMORY_H
#define DEBUGHUDMEMORY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/memory/MemoryTracker.h>
#include <btl/Containers/vector.h>

namespace Render { namespace Font { class C_FontRenderer;}}

namespace DebugSystems
{

class C_MemoryContextNode
{
public:
	const Memory::ContextNodeName	*pNodeName;
	const Memory::ContextNode		*pNodePtr;

	bool operator < (const C_MemoryContextNode &B) const
	{
		return strcmp(pNodeName->m_name, B.pNodeName->m_name)<0;
	}
};

class C_DebugHUDMemory
{
public:
	C_DebugHUDMemory();
	~C_DebugHUDMemory();

	void							Update( const Maths::C_TimeS& /*timeStep*/ ){};
	void							Render( );
private:
	
	Render::Font::C_FontRenderer*		m_pFontRenderer;
	
	Btl::Vector<C_MemoryContextNode>	m_MemoryContextNodeVector;
};

} // namespace DebugSystems

#endif
