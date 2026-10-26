#include "FrameworkPch.h"
#include "Framework/Debug/MemoryBudgets.h"
#include <System/Memory/MemoryTracker.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include <Render/Font/Font.h>

namespace Framework
{

Debug::Bool		g_MemoryShowBudgets				("System.Memory.ShowBudgets",			false, Debug::Persist());
Debug::Bool		g_MemoryShowOverBudgets			("System.Memory.ShowOverBudgets",		true, Debug::Persist());
Debug::Bool		g_MemoryShowUnderBudgets		("System.Memory.ShowUnderBudgets",		false, Debug::Persist());
Debug::Bool		g_MemoryShowNotFoundBudgets		("System.Memory.ShowNotFoundBudgets",	false, Debug::Persist());
Debug::Float	g_MemoryShowBudgetsFontPosX		("System.Memory.ShowBudgetsFontPosX",	50.0f, Debug::Range(0.0f, 1024.0f));
Debug::Float	g_MemoryShowBudgetsFontPosY		("System.Memory.ShowBudgetsFontPosY",	460.0f, Debug::Range(0.0f, 1024.0f));
Debug::Float	g_MemoryShowBudgetsFontSize		("System.Memory.ShowBudgetsFontSize",	18.0f, Debug::Range(0.0f, 128.0f));
Debug::Int		g_MemoryBudgetsBackingAlpha		("System.Memory.BackingAlpha",			128, Debug::Range(0, 255), Debug::Step(1), Debug::Persist());
Debug::Int		g_MemoryBudgetsFontAlpha		("System.Memory.FontAlpha",				128, Debug::Range(0, 255), Debug::Step(1), Debug::Persist());

C_MemoryBudget::C_MemoryBudget(const Util::C_String& trackerName, const Util::C_String& renderName, const U32 maxSize, const U32 flags) :
	m_Flags(flags),
	m_MaxSize(maxSize),
	m_RenderName(renderName)
{
	m_TrackerNames.PushBack(trackerName);
}

C_MemoryBudget::~C_MemoryBudget()
{
	m_TrackerNames.Clear();
}

S32 C_MemoryBudget::FindTrackerName(const Util::C_String& trackerName) const
{
	for(S32 i=0; i<m_TrackerNames.Size(); i++)
		if(trackerName == m_TrackerNames[i])
			return i;
	return -1;
}

S32 C_MemoryBudget::AddTrackerName(const Util::C_String& trackerName)
{
	S32 i = FindTrackerName(trackerName);
	if(i == -1)
	{
		m_TrackerNames.PushBack(trackerName);
		i = m_TrackerNames.Size()-1;
	}
	return i;
}

C_MemoryBudgets::C_MemoryBudgets()
{
}

C_MemoryBudgets::~C_MemoryBudgets()
{
	ClearBudgets();
}

void C_MemoryBudgets::ClearBudgets()
{
	for(S32 i=0; i<m_Budgets.Size(); i++)
		SAFE_DELETE_PTR(m_Budgets[i]);
	m_Budgets.Clear();
}

S32 C_MemoryBudgets::GetBudgetCount() const
{
	return m_Budgets.Size();
}

C_MemoryBudget* C_MemoryBudgets::GetBudget(const S32 i)
{
	return m_Budgets[i];
}

const C_MemoryBudget* C_MemoryBudgets::GetBudget(const S32 i) const
{
	return m_Budgets[i];
}

S32 C_MemoryBudgets::FindBudget(const Util::C_String& renderName) const
{
	for(S32 i=0; i<m_Budgets.Size(); i++)
		if(renderName == m_Budgets[i]->m_RenderName)
			return i;
	return -1;
}

S32 C_MemoryBudgets::AddBudget(const Util::C_String& trackerName, const Util::C_String& renderName, const U32 maxSize, const U32 flags)
{
	S32 i = FindBudget(renderName);
	if(i != -1)
	{
		m_Budgets[i]->m_Flags |= flags;
		if(maxSize > m_Budgets[i]->m_MaxSize)
			m_Budgets[i]->m_MaxSize = maxSize;
		m_Budgets[i]->AddTrackerName(trackerName);
	}
	else
	{
		m_Budgets.PushBack(new C_MemoryBudget(trackerName, renderName, maxSize, flags));
		i = m_Budgets.Size()-1;
	}
	return i;
}

void C_MemoryBudgets::GetCurAndMaxSizeFromIndex(const S32 index, U32& curSize, U32& maxSize) const
{
	curSize = 0;
	maxSize = 0;

	if(index < 0 || index >= m_Budgets.Size())
		return;
	
	const C_MemoryBudget* pBudget = GetBudget(index);

	if (!pBudget)
		return;

	for(S32 iTrackerName=0; iTrackerName<pBudget->m_TrackerNames.Size(); iTrackerName++)
	{
		const Util::C_String& trackerName = pBudget->m_TrackerNames[iTrackerName];

		if(pBudget->m_Flags & E_MemoryBudget_System || pBudget->m_Flags == 0)
		{
			Memory::ContextId id = Memory::TrackNodes::Instance().FindId(trackerName);
			if(id != INVALID_CONTEXT_ID)
			{
				Memory::ContextNode node;
				Memory::Tracker::Instance().GetContextNode(id, node, pBudget->m_Flags & E_MemoryBudget_Graphics || pBudget->m_Flags == 0 ? true : false);
				curSize += node.Allocated();
			}
		}
		else
		if(pBudget->m_Flags & E_MemoryBudget_Graphics)
		{
			for(U32 iSubTracker=0; iSubTracker<Memory::Tracker::Instance().GetNumSubTrackers(); iSubTracker++)
			{
				const Memory::Tracker* pSubTracker = Memory::Tracker::Instance().GetSubTracker(iSubTracker);

				Memory::ContextId id = Memory::TrackNodes::Instance().FindId(trackerName);
				if(id != INVALID_CONTEXT_ID)
				{
					Memory::ContextNode node;
					pSubTracker->GetContextNode(id, node);
					curSize += node.Allocated();
				}
			}
		}
	}

	maxSize = pBudget->m_MaxSize;
}

void C_MemoryBudgets::GetCurAndMaxSizeFromName(const Util::C_String& name, U32& curSize, U32& maxSize) const
{
	return GetCurAndMaxSizeFromIndex(FindBudget(name), curSize, maxSize);
}

void C_MemoryBudgets::RenderBudgetsOverMaxSize(DbUtil::I_DebugRender* pDebugRenderer, Render::Font::C_FontRenderer* pFontRenderer)
{
	if(!g_MemoryShowBudgets || !pDebugRenderer || !pFontRenderer)
		return;

	pFontRenderer->SetScaleX(g_MemoryShowBudgetsFontSize);
	pFontRenderer->SetScaleY(g_MemoryShowBudgetsFontSize);
	pFontRenderer->SetColourTop(Maths::RGBA(255, 255, 255, (U32)g_MemoryBudgetsFontAlpha));
	pFontRenderer->SetColourBottom(Maths::RGBA(255, 255, 255, (U32)g_MemoryBudgetsFontAlpha));
	
	bool show;
	char buffer[1024];
	Maths::RGBA colour;
	Btl::Vector<Util::C_String> notFoundTrackerNames;
	Maths::HVec2 pos = Maths::HVec2((F32)g_MemoryShowBudgetsFontPosX, (F32)g_MemoryShowBudgetsFontPosY);
	
	for(S32 iBudget=0; iBudget<m_Budgets.Size(); iBudget++)
	{
		C_MemoryBudget* pBudget = m_Budgets[iBudget];

		if (!pBudget)
			continue;

		U32 curSize = 0;

		for(S32 iTrackerName=0; iTrackerName<pBudget->m_TrackerNames.Size(); iTrackerName++)
		{
			const Util::C_String& trackerName = pBudget->m_TrackerNames[iTrackerName];

			if(pBudget->m_Flags & E_MemoryBudget_System || pBudget->m_Flags == 0)
			{
				Memory::ContextId id = Memory::TrackNodes::Instance().FindId(trackerName);
				if(id != INVALID_CONTEXT_ID)
				{
					Memory::ContextNode node;
					Memory::Tracker::Instance().GetContextNode(id, node, pBudget->m_Flags & E_MemoryBudget_Graphics || pBudget->m_Flags == 0 ? true : false);
					curSize += node.Allocated();
				}
				else
				{
					notFoundTrackerNames.PushBack(trackerName);
				}
			}
			else
			if(pBudget->m_Flags & E_MemoryBudget_Graphics)
			{
				for(U32 iSubTracker=0; iSubTracker<Memory::Tracker::Instance().GetNumSubTrackers(); iSubTracker++)
				{
					const Memory::Tracker* pSubTracker = Memory::Tracker::Instance().GetSubTracker(iSubTracker);

					Memory::ContextId id = Memory::TrackNodes::Instance().FindId(trackerName);
					if(id != INVALID_CONTEXT_ID)
					{
						Memory::ContextNode node;
						pSubTracker->GetContextNode(id, node);
						curSize += node.Allocated();
					}
					else
					{
						notFoundTrackerNames.PushBack(trackerName);
					}
				}
			}
		}

		if(curSize == 0)
			continue;

		if(curSize > pBudget->m_MaxSize)
		{
			show = g_MemoryShowOverBudgets;
			colour = Maths::RGBA(128, 0, 0, (U32)g_MemoryBudgetsBackingAlpha);
			sprintf_s(buffer, sizeof(buffer), "%s over budget by %.2f mb (%.2f/%.2f)", pBudget->m_RenderName.Chars(),
				F32(curSize - pBudget->m_MaxSize)/(1024.0f*1024.0f),
				F32(curSize)/(1024.0f*1024.0f),
				F32(pBudget->m_MaxSize)/(1024.0f*1024.0f));
		}
		else
		{
			show = g_MemoryShowUnderBudgets;
			colour = Maths::RGBA(0, 128, 0, (U32)g_MemoryBudgetsBackingAlpha);
			sprintf_s(buffer, sizeof(buffer), "%s under budget by %.2f mb (%.2f/%.2f)", pBudget->m_RenderName.Chars(),
				F32(pBudget->m_MaxSize - curSize)/(1024.0f*1024.0f),
				F32(curSize)/(1024.0f*1024.0f),
				F32(pBudget->m_MaxSize)/(1024.0f*1024.0f));
		}

		if(!show)
			continue;

		Maths::HVec2 dim = pFontRenderer->GetDimensions(0, buffer);
		pDebugRenderer->Draw2dBar(pos, dim, colour);
		pFontRenderer->Print(pos, buffer);
		pos +=  Maths::HVec2(0.0f, dim.Y()); 
	}

	if(g_MemoryShowNotFoundBudgets)
	{
		for(S32 iTrackerName=0; iTrackerName<notFoundTrackerNames.Size(); iTrackerName++)
		{
			const Util::C_String& trackerName = notFoundTrackerNames[iTrackerName];

			colour = Maths::RGBA(0, 0, 128, (U32)g_MemoryBudgetsBackingAlpha);
			sprintf_s(buffer, sizeof(buffer), "Failed to find tracker name %s", trackerName.Chars());

			Maths::HVec2 dim = pFontRenderer->GetDimensions(0, buffer);
			pDebugRenderer->Draw2dBar(pos, dim, colour);
			pFontRenderer->Print(pos, buffer);
			pos +=  Maths::HVec2(0.0f, dim.Y()); 
		}
	}

	notFoundTrackerNames.Clear();
}

void C_MemoryBudgets::Dump()
{
	char buffer[512];
	
	LOGINFO(g_Log, "\n");
	LOGINFO(g_Log, "===========\n");
	LOGINFO(g_Log, "Over Budget\n");
	LOGINFO(g_Log, "===========\n");

	for(S32 iBudget=0; iBudget<m_Budgets.Size(); iBudget++)
	{
		U32 curSize = 0;
		U32 maxSize = 0;
		GetCurAndMaxSizeFromIndex(iBudget, curSize, maxSize);

		if(curSize > maxSize)
		{
			const C_MemoryBudget* pBudget = m_Budgets[iBudget];

			sprintf_s(buffer, sizeof(buffer), "%s over budget by %.2f mb (%.2f/%.2f)", pBudget->m_RenderName.Chars(),
				F32(curSize - maxSize)/(1024.0f*1024.0f),
				F32(curSize)/(1024.0f*1024.0f),
				F32(maxSize)/(1024.0f*1024.0f));

			LOGINFO(g_Log, "%s\n", buffer);
		}
	}

	LOGINFO(g_Log, "\n");
	LOGINFO(g_Log, "============\n");
	LOGINFO(g_Log, "Under Budget\n");
	LOGINFO(g_Log, "============\n");
	
	for(S32 iBudget=0; iBudget<m_Budgets.Size(); iBudget++)
	{
		U32 curSize = 0;
		U32 maxSize = 0;
		GetCurAndMaxSizeFromIndex(iBudget, curSize, maxSize);

		if(curSize <= maxSize)
		{
			const C_MemoryBudget* pBudget = m_Budgets[iBudget];

			sprintf_s(buffer, sizeof(buffer), "%s under budget by %.2f mb (%.2f/%.2f)", pBudget->m_RenderName.Chars(),
				F32(maxSize - curSize)/(1024.0f*1024.0f),
				F32(curSize)/(1024.0f*1024.0f),
				F32(maxSize)/(1024.0f*1024.0f));
		
			LOGINFO(g_Log, "%s\n", buffer);
		}
	}

	LOGINFO(g_Log, "\n");
}

}; // namespace Framework
