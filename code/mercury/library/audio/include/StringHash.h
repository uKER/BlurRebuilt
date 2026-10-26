#ifndef _STRING_HASH_H_
#define _STRING_HASH_H_

//#include "types.h"

#include "btl/containers/map.h"
#include "btl/utility/string.h"
#include <system/memory/MemoryTracker.h>

#define STRING_HASH_ZERO_IS_EMPTY_STRING

namespace Audio
{
//--------------------------------------------------------------------------------------
//	Dictionary
//--------------------------------------------------------------------------------------
	class Dictionary
	{
	public:
		typedef Btl::Map< U32, Btl::String > HashStringMap;

		static Dictionary& Instance()
		{
			static Dictionary singleton;
			return singleton;
		}

		inline void AddEntry( U32 uId, Btl::String& strItem )
		{
			#if !defined( CFG_SPEED )
			MT::AutoLock autoLock( m_Lock );
			m_map.Insert( uId, strItem );
			#endif
		}

		inline bool Exists( U32 uId ) const
		{
			#if !defined( CFG_SPEED )
			MT::AutoLock autoLock( m_Lock );
			HashStringMap::ConstIterator it = m_map.Find( uId );
			if ( it != m_map.End() )
				return true;
			else
			#endif
				return false;
		}

		inline Btl::String& GetString( U32 uId )
		{
			#if !defined( CFG_SPEED )
			MT::AutoLock autoLock( m_Lock );
			HashStringMap::Iterator it = m_map.Find( uId );
			if ( it != m_map.End() )
				return (*it).GetValue();
			else
			#endif
			{
				static Btl::String empty = "#NOT FOUND#" ;
				return empty;
			}
		}

	private:
		Dictionary() { };
		Dictionary( const Dictionary& );
		Dictionary& operator = ( const Dictionary& );

	private:
		#if !defined( CFG_SPEED )
		mutable MT::Lock m_Lock;

		// this uses a sorted tree structure so will do lots of memory allocations (ie 1000's) if lots of entries in the dictionary.
		// tried changing it to a OpenHashMap which uses contiguous memory but it looked like it caused instability
		// (probably due to being called across different threads).
		HashStringMap m_map;
		#endif
	};

//--------------------------------------------------------------------------------------
//	StringHash
//--------------------------------------------------------------------------------------
	class StringHash
	{
	public:
		StringHash() { m_uHash = Invalid; }
		StringHash( const char* pchHashMe ) { m_uHash = GetHash( pchHashMe ); }
		StringHash( const Btl::String& strHashMe ) { m_uHash = GetHash( strHashMe.Chars() ); }
		StringHash( const U32& uHash ) { m_uHash = uHash; }	//	warning - hash not checked in dictionary

		inline StringHash& operator = ( const StringHash &hash ) { m_uHash = hash.m_uHash; return *this; }

		inline bool operator < ( const StringHash& other ) const { return m_uHash < other.m_uHash; }

		inline bool operator == ( const StringHash& hash ) const { return m_uHash == hash.m_uHash; }
		inline bool operator != ( const StringHash& hash ) const { return m_uHash != hash.m_uHash; }

		inline bool operator == ( const U32 uHash ) const { return m_uHash == uHash; }
		inline bool operator != ( const U32 uHash ) const { return m_uHash != uHash; }

		bool IsValid() const { return m_uHash != Invalid; }

		// [MarkC] needed by message serialisation functions
		const U32	GetValue(void) const {return m_uHash;}
		void		SetValue(U32 hash) {m_uHash = hash;}
	private:
		static U32 GetHash( const char* pchHashMe )
		{
#if !defined( CFG_SPEED )
			Btl::String strToHash = pchHashMe;
#endif
			U32 uHash = Empty;

			//	the hash
			while ( *pchHashMe )
			{
				uHash ^= *pchHashMe;
				uHash *= FNVPrime;
				pchHashMe++;
			}

#if !defined( CFG_SPEED )

			BIZ_ASSERT_MSG((uHash != Invalid), "StringHash returned the invalid hash");

			bool bExistsInDictionary = Dictionary::Instance().Exists(uHash);
			const Btl::String& pStrDic = Dictionary::Instance().GetString( uHash );

			// if this hash maps to a string in the dictionary, make sure its not a different string
			BIZ_ASSERT_MSG((!bExistsInDictionary || pStrDic == strToHash), "Collision in StringHash with strings '%s' and '%s'. Hash value 0x%x", pStrDic.Chars(), strToHash.Chars(), uHash);


			if (!bExistsInDictionary)
			{
				MEMORY_TRACKER_CONTEXT("audio.StringHashDictionary");
				Dictionary::Instance().AddEntry(uHash, strToHash);
			}
#endif // BUILD_CONFIG_MASTER

			return uHash;
		}

	private:
	//public:
		U32 m_uHash;

	// moved to the bottom so we can see the m_uHash value in the debugger watch preview
	private:
		static const U32 FNVPrime = 0x01000193;

	public:
#if defined( STRING_HASH_ZERO_IS_EMPTY_STRING )
		static const U32 Empty = 0x00000000;
#else
		static const U32 Empty = 0x811c9dc5;
#endif
		static const U32 Invalid = 0xffffffff;
	};
}

#endif // _STRING_HASH_H_
