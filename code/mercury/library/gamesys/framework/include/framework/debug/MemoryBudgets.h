#ifndef FRAMEWORK_MEMORY_BUDGETS_H
#define FRAMEWORK_MEMORY_BUDGETS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include <System/Debug/I_DebugRender.h>

namespace Render
{
namespace Font { class C_FontRenderer; }
}

namespace Framework
{

enum E_MemoryBudget
{
	E_MemoryBudget_System				= 0x00000001,
	E_MemoryBudget_Graphics				= 0x00000002,
};

class C_MemoryBudget
{
public:

	C_MemoryBudget(const Util::C_String& trackerName, const Util::C_String& renderName, const U32 maxSize, const U32 flags = 0);
	~C_MemoryBudget();

	S32 FindTrackerName(const Util::C_String& trackerName) const;
	S32 AddTrackerName(const Util::C_String& trackerName);

public:

	U32									m_Flags;
	U32									m_MaxSize;
	Util::C_String						m_RenderName;
	Btl::Vector<Util::C_String>			m_TrackerNames;
};

class C_MemoryBudgets
{
public:

	C_MemoryBudgets();
	~C_MemoryBudgets();

	void ClearBudgets();

	S32 GetBudgetCount() const;
	C_MemoryBudget* GetBudget(const S32 i);
	const C_MemoryBudget* GetBudget(const S32 i) const;

	S32 FindBudget(const Util::C_String& renderName) const;
	S32 AddBudget(const Util::C_String& trackerName, const Util::C_String& renderName, const U32 maxSize, const U32 flags = 0);
	
	void GetCurAndMaxSizeFromIndex(const S32 index, U32& curSize, U32& maxSize) const;
	void GetCurAndMaxSizeFromName(const Util::C_String& name, U32& curSize, U32& maxSize) const;
	
	void RenderBudgetsOverMaxSize(DbUtil::I_DebugRender* pDebugRenderer, Render::Font::C_FontRenderer* pFontRenderer);

	void Dump();

private:

	Btl::Vector<C_MemoryBudget*>		m_Budgets;
};

}; // namespace Framework

#endif // FRAMEWORK_MEMORY_BUDGETS_H
