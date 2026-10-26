#ifndef LOC_H
#define LOC_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

#include "ui/Game/UILua.h"
#include "ui/uiLogger.h"
#include "ui/Graphics/Localisation/XtLocHack.h"

namespace UI
{

/// Text string ID, holds reference index to a sorcey lookup
class C_TextStringID
{
private:
	static const XtU32	m_Invalid		= 0x7fff;

public:
	// Construction
	// MS: AVGarcia - m_Generation is used for LIP. Needs to be 0 at init.
	C_TextStringID( ) : 
	  m_pString( NULL ), m_Value( m_Invalid ), m_Generation( 0 )
	{ 
	}

	 C_TextStringID( const char *string ) :
		m_pString( NULL ), m_Value( m_Loc.FindId( string ) ), m_Generation( 0 )
	{
	}

	C_TextStringID( const U32 value ) : 
		m_pString( NULL ), m_Value( value ), m_Generation( 0 )
	{
	}

	C_TextStringID( const F64 value ) : 
		m_pString( NULL ), m_Value( (U32)value ), m_Generation( 0 )
	{

	}

	// Comparison
	const bool operator == ( const C_TextStringID& src ) const 
	{
		return m_Value == src.Value();
	}

	const bool operator != ( const C_TextStringID& src ) const 
	{
		return m_Value != src.Value();
	}

	// Assignment
	C_TextStringID& operator =( const F32 value )
	{
		LOGERROR( g_Log, DBSTR "Attempting to cast a C_TextStringID from a F32\n");

		return *this;
	}

	C_TextStringID& operator =( const F64 value )
	{
		m_Value = (XtU32)value;
		m_pString = NULL;

		return *this;
	}

	C_TextStringID& operator =( const U32 value )
	{
		m_Value = value;
		m_pString = NULL;

		return *this;
	}

	C_TextStringID& operator =( const char* string )
	{
		m_Value = m_Loc.FindId( string );
		m_pString = NULL;

		return *this;
	}

	void Invalidate( )
	{
		m_Value = m_Invalid;
		m_pString = NULL;
	}

	void Find( )
	{
		assert( IsValid() );
		m_pString = m_Loc.FindText(m_Value);
	}

	// Interrogate
	const bool IsLocString( ) const
	{
		return(m_Value > m_Invalid); 
	}

	const bool IsValid( ) const
	{
		return(m_Value != m_Invalid);
	}

	const XtU32 Value( ) const
	{
		return m_Value;
	}

	const WCHAR *String( );

	const char* Name( );

private:
	class C_Loc
	{
	public:
		C_Loc() : 
		  m_Initialised(false), m_TableValid(false), m_pState(NULL)
		{
		}

		void					Load(const Sys::C_FilePath& filename, const char* lang, Sys::I_FileSystem* fs = NULL, const S32 handle = 0 );
		bool					LoadFromMemory(const U8* pMemory, const U32 size, const S32 handle = 0 );

		// TODO - remove after Blur - these shouldn't have been put here
		bool					LoadMultiplayerTSFromMemory( const U8* pMemory, const U32 size, const char* lang );
		bool					LoadMultiplayerTSFromDisk( const Sys::C_FilePath& filename, const char* lang );
		//

		void					CreateLuaEnums( lua_State *pState );
		void					Destroy( const S32 handle );
		void					DestroyAll();
		void					Reload();
		const WCHAR*			FindText(XtU32 id);
		XtU32					FindId(const char* name);
		const char*				FindName( XtU32 id );
		const bool				TableValid()				{ return m_TableValid; }
		void					TableValid(const bool set)	{ m_TableValid = set; }

	private:
		class C_LocHackItem : public Btl::ListElementSingle<C_LocHackItem>
		{
		public:
			C_LocHackItem(const Sys::C_FilePath& filename, const char *lang, Sys::I_FileSystem* fs, const S32 handle)
			:	m_Filename( filename )
			,	m_Lang( lang )
			,	m_FileSys( fs )
			,	m_Handle( handle ) 
			{ 
				Load();
			}

			C_LocHackItem(const char *lang, const U8* pMemory, const U32 size, const S32 handle)
			:	m_Lang( lang )
			,	m_Handle( handle ) 
			{ 
				LoadFromMemory(pMemory, size);
			}

			~C_LocHackItem( )
			{
				// m_LocHack.Free();
			}
			
			void				Load()												{ m_LocHack.Load( m_Filename, m_Lang, m_FileSys ); }
			void				LoadFromMemory(const U8* pMemory, const U32 size)	{ m_LocHack.LoadFromMemory( pMemory, size, m_Lang ); }

			XtLocHack&			LocHack()		{ return m_LocHack; }
			const S32			Handle()		{ return m_Handle; }

		private:
			Sys::I_FileSystem*	m_FileSys;
			Util::C_String		m_Lang;
			Sys::C_FilePath		m_Filename;
			S32					m_Handle;

			XtLocHack			m_LocHack;
		};

	private:
		bool							m_Initialised;
		bool							m_TableValid;
		C_LocHackItem*					m_pLocMultiplayerTS;
		Btl::List<C_LocHackItem*>		m_LocList;
		lua_State*						m_pState;
	};

private:
	static C_Loc		m_Loc;
	const WCHAR*		m_pString;
	XtU32				m_Value;

	//MS: AVGarcia - Generation of this string. Will be compared against UILip::GetGeneration()
	unsigned int		m_Generation;

public:
	// Access
	static C_Loc& LocTable( )
	{ 
		return m_Loc; 
	}
};

}; // namespace UI

#endif // LOC_H
