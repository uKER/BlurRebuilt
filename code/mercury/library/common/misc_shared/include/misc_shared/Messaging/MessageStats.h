#ifndef MISCSHARED_MESSAGING_MESSAGESTATS_H
#define MISCSHARED_MESSAGING_MESSAGESTATS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/messaging/MessagingDeclare.h>
#include <misc_shared/uid/TypeTable.h>


namespace DbUtil
{
	class I_DebugRender;
}


namespace Messaging
{

/// Types of message stat.
enum E_MessageStatsType
{
	E_MessageStatsType_Current,		///< currently accumulating stats.
	E_MessageStatsType_Previous,	///< previously accumulated stats.
	E_MessageStatsType_Maximum,		///< maximum stats accumulated.
	E_MessageStatsType_Average,		///< average stats accumulated.
	E_MessageStatsType_Count
};

/// Values of message stat.
enum E_MessageStatsValue
{
	E_MessageStatsValue_Broadcast,	///< number of messages broadcast.
	E_MessageStatsValue_Subscribed,	///< number of subscribers receiving broadcast messages.
	E_MessageStatsValue_Sent,		///< number of messages sent.
	E_MessageStatsValue_Handled,	///< number of handlers processing sent messages.
	E_MessageStatsValue_Count
};

/// Tracks stats on messaging.
class C_MessageStats
{
public:
	C_MessageStats(const F32 decay = 1.0f / 30.0f);
	~C_MessageStats();

	void			Update(); ///< swap accumulator.
	void			Reset(); ///< reset the stats.

	void			AccumulateBroadcast(const TMsgType msgType, const U32 numSubscribed); ///< accumulate stats for a message broadcast.
	void			AccumulateSend(const TMsgType msgType, const U32 numHandled); ///< accumulate stats for a message sending.

	U32				GetValue(const TMsgType msgType, const E_MessageStatsType statsType, const E_MessageStatsValue statsValue) const; ///< get a stat.

	void			DebugRender(DbUtil::I_DebugRender* pDebugRender, const Maths::HVec2& origin, const E_MessageStatsType orderType = E_MessageStatsType_Average, const E_MessageStatsValue showValue = E_MessageStatsValue_Broadcast, const U32 maxItems = 0) const; ///< debug render the stats.

private:
	template <typename T_Storage>
	struct C_Total
	{
		T_Storage	m_Values [E_MessageStatsValue_Count];

		C_Total() {Clear();}
		void Clear() {memset(this, 0, sizeof(C_Total));}
	};

	struct C_Item
	{
		C_Total<U32>	m_Current;
		C_Total<U32>	m_Previous;
		C_Total<U32>	m_Maximum;
		C_Total<F32>	m_Average;
	};

	typedef UID::C_TypeTableHashPolicyHi20<TMsgType, 256> C_HashFunctor;
	typedef UID::C_TypeTable<TMsgType, C_Item, C_HashFunctor> C_HashTable;

	F32				m_Decay;		///< rate of decay [0,1] for averages
	C_HashTable		m_HashTable;	///< storage for stats

	C_MessageStats(const C_MessageStats& copy); // can not copy
	C_MessageStats& operator = (const C_MessageStats& copy); // can not copy

};

} // namespace Messaging


#endif // MISCSHARED_MESSAGING_MESSAGESTATS_H
