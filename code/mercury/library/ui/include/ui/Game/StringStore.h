#ifndef STRINGSTORE_H
#define STRINGSTORE_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
#include <wchar.h>
#endif

#include "ui/UIDefines.h"
#include "ui/Util/WideString.h"

namespace UI
{

class C_StringStore
{
public:
	C_StringStore( ) : 
#ifndef CFG_GOLD
		m_stringMaxSize( 0 ),
#endif
		m_Initialised( false )
	{

	}

	~C_StringStore( )
	{
		if(m_Initialised)
			Exit();
	}

	static C_StringStore&			Instance( )
	{
		return m_sInstance; 
	}

	void	Init(  )
	{
		m_currentBufferPtr = m_bufferPtr = new WCHAR[m_MaxCharacters];
		if(!m_currentBufferPtr)
		{
			LOGERROR( g_Log, DBSTR " : Failed to alloc %d bytes", m_MaxCharacters * sizeof(WCHAR) );
			return;
		}

		ClearAll();
		m_Initialised = true;
	}

	void	Exit( )
	{
		if(!m_Initialised)
		{
			LOGERROR( g_Log, DBSTR " : Not initialised");
			return;
		}

		SAFE_DELETE_ARY( m_bufferPtr );
		m_Initialised = false;
	}

	void ClearAll( )
	{
		if(!m_Initialised)
			return;

#ifndef CFG_GOLD
		static U32 largestStringUsage(0);

		if (m_stringMaxSize > largestStringUsage)
		{
			if (m_stringMaxSize > U32(m_currentBufferPtr - m_bufferPtr))
			{
				LOGWARN(g_Log, "C_StringStore - Out of string space - Characters Allocated(%d) Needed(%d).\n", m_MaxCharacters, m_stringMaxSize);
			}
			largestStringUsage = m_stringMaxSize;
		}
		m_stringMaxSize = 0;
#endif

		m_currentBufferPtr = m_bufferPtr;

		for (U32 i=0; i < m_IndexStoreSize; i++)
			m_StringPtrs[i] = NULL;
	}

	void Store(const U32 index, const char *my_string)
	{
		if(!m_Initialised)
		{
			LOGERROR( g_Log, DBSTR " : Not initialised");
			return;
		}

		assert(index >= 0 && index < m_IndexStoreSize);

		size_t lenRequired = strlen(my_string) + 1;
		
		if (m_StringPtrs[index] != NULL)
		{
#ifndef PLATFORM_PS3
#endif
		}

#ifndef CFG_GOLD
		m_stringMaxSize += lenRequired;
#endif

		U32 remainingCharacters = m_MaxCharacters - (m_currentBufferPtr - m_bufferPtr);

		if (lenRequired <= remainingCharacters)
		{
			m_StringPtrs[index] = m_currentBufferPtr;

			m_currentBufferPtr += lenRequired;

			WideStrFormat( m_StringPtrs[index], remainingCharacters, L"%S", my_string );
		}
		else
		{
#ifndef PLATFORM_PS3
#endif
		}
	}

	void Store(const U32 index, const WCHAR *my_string)
	{
		if(!m_Initialised)
		{
			LOGERROR( g_Log, DBSTR " : Not initialised");
			return;
		}

		assert(index >= 0 && index < m_IndexStoreSize);

		size_t lenRequired = wcslen(my_string) + 1;
		
		if (m_StringPtrs[index] != NULL)
		{
#ifndef PLATFORM_PS3
#endif
		}

#ifndef CFG_GOLD
		m_stringMaxSize += lenRequired;
#endif

		U32 remainingCharacters = m_MaxCharacters - (m_currentBufferPtr - m_bufferPtr);

		if (lenRequired <= remainingCharacters)
		{
			m_StringPtrs[index] = m_currentBufferPtr;

			m_currentBufferPtr += lenRequired;

			WideStrCopy( m_StringPtrs[index], remainingCharacters, my_string );
		}
		else
		{
#ifndef PLATFORM_PS3
#endif
		}
	}

	WCHAR *Get(const U32 index)
	{
		if(!m_Initialised)
		{
			LOGERROR( g_Log, DBSTR " : Not initialised");
			return NULL;
		}

		if( index > m_IndexStoreSize )
		{
			return L"<<Out of range>>";
		}
		if( m_StringPtrs[index] == NULL )
		{
			return L"<<Not set>>";
		}
		if( m_StringPtrs[index][0] == NULL )
		{
			return L"<<Empty>>";
		}

		return m_StringPtrs[index];
	}

public:
	static const U32 m_IndexStoreSize	= 2048;			// Max number of entries
private:
	static const U32 m_MaxCharacters	= 25 * 1000;	// This is the max number of characters we can store down

	bool			m_Initialised;

	WCHAR*	m_StringPtrs[m_IndexStoreSize];
	WCHAR*	m_bufferPtr;			// Buffer we allocate from
	WCHAR*	m_currentBufferPtr;

	static C_StringStore		m_sInstance;

#ifndef CFG_GOLD
	U32		m_stringMaxSize;
#endif
};

}; // namespace UI

#endif
