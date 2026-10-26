///=================================================================
/// RecentEventsStore.h
/// 10.08.09
///=================================================================
#ifndef RECENT_EVENTS_STORE_H
#define RECENT_EVENTS_STORE_H

#include <Btl/Containers/vector.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#define MAX_RECENT_EVENTS (10)
namespace Game
{

	///=============================================================
	/// RecentEventsStore
	///=============================================================
	class RecentEventsStore 
	{
		static	const int			s_maxRecentEvents = MAX_RECENT_EVENTS;
	
	public:	
		
		typedef Btl::Vector< U64, Btl::VectorParameters_Static<s_maxRecentEvents> > recentEventBuffer;

							RecentEventsStore();

				void				Serialise( Sys::BinarySerialiserAutoEndian& stream );
				void				Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );
				void				Push(U64 val);
				int					Size() const;
				int					Capacity() const;
				void				Clear();
				U64					operator[] (S32 index) const;
	protected:
		
				int					mHead;
				int					mSize;
				recentEventBuffer	mBuffer;
		

	private:

#ifndef CFG_GOLD
		static	void				UnitTest();
				void				Dump();
#endif
	};

}// namespace UI

#endif // RECENT_EVENTS_STORE_H
