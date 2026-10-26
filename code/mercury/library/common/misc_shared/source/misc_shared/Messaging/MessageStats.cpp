#include "MiscSharedPch.h"
#include <misc_shared/messaging/MessageStats.h>

#include <btl/containers/vector.h>
#include <core/containers/genesis_containers/Sort.h>
#include <misc_shared/messaging/MessageDebug.h>
#include <system/debug/I_DebugRender.h>
#include <system/debug/Variables.h>




DEBUG_ENUM(Messaging::E_MessageStatsType)
	DE_OPTION(Messaging::E_MessageStatsType_Current,		"Current")
	DE_OPTION(Messaging::E_MessageStatsType_Previous,		"Previous")
	DE_OPTION(Messaging::E_MessageStatsType_Maximum,		"Maximum")
	DE_OPTION(Messaging::E_MessageStatsType_Average,		"Average")
END_DEBUG_ENUM

DEBUG_ENUM(Messaging::E_MessageStatsValue)
	DE_OPTION(Messaging::E_MessageStatsValue_Broadcast,		"Broadcast")
	DE_OPTION(Messaging::E_MessageStatsValue_Subscribed,	"Subscribed")
	DE_OPTION(Messaging::E_MessageStatsValue_Sent,			"Sent")
	DE_OPTION(Messaging::E_MessageStatsValue_Handled,		"Handled")
END_DEBUG_ENUM




namespace MessageStats_Impl
{

struct C_SortItem
{
	U32					m_SortKey;
	Messaging::TMsgType	m_MsgType;
	U32					m_Values [Messaging::E_MessageStatsType_Count];

	inline bool operator > (const C_SortItem& other) const {return m_SortKey < other.m_SortKey;}
};

typedef Btl::Vector<C_SortItem> C_SortList;

} // namespace MessageStats_Impl




namespace Messaging
{

C_MessageStats::C_MessageStats(const F32 decay)
	: m_Decay(decay)
	, m_HashTable()
{
	BIZ_ASSERT((decay > 0.0f) && (decay < 1.0f));
}

C_MessageStats::~C_MessageStats()
{
}

void C_MessageStats::Update()
{
	for (C_HashTable::Iterator it = m_HashTable.Begin(); it != m_HashTable.End(); ++it)
	{
		C_Item& item = it.GetData();
		
		item.m_Previous = item.m_Current;

		const F32 retain = (1.0f - m_Decay);

		for (U32 i = 0; i < E_MessageStatsValue_Count; ++i)
		{
			item.m_Average.m_Values[i] = (item.m_Average.m_Values[i] * retain) + (static_cast<F32>(item.m_Current.m_Values[i]) * m_Decay);
			item.m_Maximum.m_Values[i] = Maths::Max(item.m_Maximum.m_Values[i], item.m_Current.m_Values[i]);
		}

		item.m_Current.Clear();
	}
}

void C_MessageStats::Reset()
{
	m_HashTable.Clear();
}

void C_MessageStats::AccumulateBroadcast(const TMsgType msgType, const U32 numSubscribed)
{
	C_Item& item = m_HashTable.Insert(msgType).second.GetData();
	++item.m_Current.m_Values[E_MessageStatsValue_Broadcast];
	item.m_Current.m_Values[E_MessageStatsValue_Subscribed] += numSubscribed;
}

void C_MessageStats::AccumulateSend(const TMsgType msgType, const U32 numHandled)
{
	C_Item& item = m_HashTable.Insert(msgType).second.GetData();
	++item.m_Current.m_Values[E_MessageStatsValue_Sent];
	item.m_Current.m_Values[E_MessageStatsValue_Handled] += numHandled;
}

U32 C_MessageStats::GetValue(const TMsgType msgType, const E_MessageStatsType statsType, const E_MessageStatsValue statsValue) const
{
	const C_HashTable::ConstIterator it = m_HashTable.Find(msgType);

	if (it != m_HashTable.End())
	{
		switch (statsType)
		{
		case E_MessageStatsType_Current:	return it.GetData().m_Current.m_Values[statsValue];
		case E_MessageStatsType_Previous:	return it.GetData().m_Previous.m_Values[statsValue];
		case E_MessageStatsType_Maximum:	return it.GetData().m_Maximum.m_Values[statsValue];
		case E_MessageStatsType_Average:	return static_cast<U32>(floorf(it.GetData().m_Average.m_Values[statsValue] + 0.5f));
		}
	}

	return 0;
}

void C_MessageStats::DebugRender(DbUtil::I_DebugRender* pDebugRender, const Maths::HVec2& origin, const E_MessageStatsType orderType, const E_MessageStatsValue showValue, const U32 maxItems) const
{
	using namespace MessageStats_Impl;

	const F32 lineHeight = 24.0f;

	Maths::HVec2 lineStart = origin;
	const Maths::HVec2 lineSpacing(0.0f, lineHeight);
	const Maths::HVec2 columnOffset(550.0f, 0.0f);
	const Maths::HVec2 columnSpacing(75.0f, 0.0f);
	const Maths::RGBA colour(1.0f, 1.0f, 1.0f, 1.0f);

	// gather values
	C_SortList sortList;
	{
		sortList.Resize(m_HashTable.Size());

		S32 index = 0;

		for (C_HashTable::ConstIterator it = m_HashTable.Begin(); it != m_HashTable.End(); ++it)
		{
			const C_Item& src = it.GetData();
			C_SortItem& dst = sortList[index];
			++index;

			dst.m_Values[E_MessageStatsType_Current] = src.m_Current.m_Values[showValue];
			dst.m_Values[E_MessageStatsType_Previous] = src.m_Previous.m_Values[showValue];
			dst.m_Values[E_MessageStatsType_Maximum] = src.m_Maximum.m_Values[showValue];
			dst.m_Values[E_MessageStatsType_Average] = static_cast<U32>(floorf(src.m_Average.m_Values[showValue] + 0.5f));

			dst.m_SortKey = dst.m_Values[orderType];
			dst.m_MsgType = it.GetUID();
		}

		Util::ShellSortArray<C_SortList, C_SortItem>(sortList, sortList.Size());
	}

	// display header
	{
		Maths::HVec2 pos = lineStart;
		lineStart += lineSpacing * 2.0f;

		const char* titles [] = { "Broadcast", "Subscribed", "Sent", "Handled" };
		pDebugRender->Draw2dText(pos, titles[showValue], colour, lineHeight);
		pos += columnOffset;

//		pDebugRender->Draw2dText(pos, "Curr", colour, lineHeight);
		pos += columnSpacing;

		pDebugRender->Draw2dText(pos, "Prev", colour, lineHeight);
		pos += columnSpacing;

		pDebugRender->Draw2dText(pos, "Max", colour, lineHeight);
		pos += columnSpacing;

		pDebugRender->Draw2dText(pos, "Avg", colour, lineHeight);
		pos += columnSpacing;
	}

	// display lines
	{
		S32 numLines = sortList.Size();

		if (maxItems > 0)
		{
			numLines = Maths::Min(numLines, static_cast<S32>(maxItems));
		}

		for (S32 i = 0; i < numLines; ++i)
		{
			const C_SortItem& src = sortList[i];
			Util::C_String text;

			Maths::HVec2 pos = lineStart;
			lineStart += lineSpacing;

			text.Printf("0x%8x %s", src.m_MsgType, FindDebugName(src.m_MsgType));
			pDebugRender->Draw2dText(pos, text, colour, lineHeight);
			pos += columnOffset;

//			text.Printf("%d", src.m_Values[E_MessageStatsType_Current]);
//			pDebugRender->Draw2dText(pos, text, colour, lineHeight);
			pos += columnSpacing;

			text.Printf("%d", src.m_Values[E_MessageStatsType_Previous]);
			pDebugRender->Draw2dText(pos, text, colour, lineHeight);
			pos += columnSpacing;

			text.Printf("%d", src.m_Values[E_MessageStatsType_Maximum]);
			pDebugRender->Draw2dText(pos, text, colour, lineHeight);
			pos += columnSpacing;

			text.Printf("%d", src.m_Values[E_MessageStatsType_Average]);
			pDebugRender->Draw2dText(pos, text, colour, lineHeight);
			pos += columnSpacing;
		}
	}
}

} // namespace Messaging
