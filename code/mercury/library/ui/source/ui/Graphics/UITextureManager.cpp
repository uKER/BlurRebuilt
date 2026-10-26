#include "stdafx.h"

#include "ui/UiInterface.h"
#include "ui/Graphics/UITextureManager.h"
#include "ui/Graphics/UIGraphicalInterface.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Font/UIPolygonFont.h"

#include "Display/Device/Device.h"
#include <Display/Bundler/Bundler.h>

namespace UI
{

// Singleton instance
C_TextureManager gUITextureManager;

// Code
C_TextureManager::C_TextureManager() : 
	m_NullTextureIndex( 0 ), 
	m_NullTextureHandle( NULL ), 
	m_TextureCount( 0 ),
	m_UpdateLuaEnums( false ),
	m_pState( NULL )
{

}

C_TextureManager::~C_TextureManager()
{
	// Delete resources
	DestroyAll();
}

void C_TextureManager::DestroyAll( )
{
	for( S32 i = 0; i < m_Resources.Size(); i++)
		RemoveResourceBundle( i );
}

void C_TextureManager::LuaState( lua_State* pState )
{
	m_pState = pState;
}

void C_TextureManager::Update( )
{
	if( !m_UpdateLuaEnums )
		return;

	if( !m_pState )
	{
		LOGERROR( g_Log, DBSTR "lua state == NULL" );
		return;
	}

	// Create new table
	lua_newtable( m_pState );
	if(lua_type( m_pState, -1) != LUA_TTABLE)
	{
		LOGFATAL( g_Log, DBSTR " : failed to create new %s table : %x - stack top %d", LUA_TEXTURE_TABLE_NAME, (U32)(INTPTR) m_pState, lua_gettop( m_pState) );	// [Blur2PC][x64] log only
		return;
	}

	// GRR - subtable
	lua_pushstring( m_pState,LUA_TEXTURE_SUBTABLE_NAME);
	lua_newtable( m_pState);
	if(lua_type( m_pState, -1 ) != LUA_TTABLE)
	{
		lua_pop( m_pState, 1 );
		LOGFATAL( g_Log, DBSTR " : failed to create new %s table : %x - stack top %d", LUA_TEXTURE_TABLE_NAME, (U32)(INTPTR) m_pState, lua_gettop( m_pState) );	// [Blur2PC][x64] log only
		return;
	}

	// Add textures
	for( S32 i = 0; i < m_Resources.Size(); i++ )
	{
		C_Resource *p = m_Resources[ i ];
		if( p == NULL )
			continue;

		for( U32 j = 0; j < p->ResourceCount(); j++ )
		{
			C_TextureHandleIndex handle = TextureHandleIndex( i, j );

			Lua::AddDataMember( m_pState, p->GetResourceName( j ), handle.Index() );
		}
	}
	lua_rawset( m_pState, -3 ); // Subtable

	// Push new table over old one
	lua_setglobal( m_pState, LUA_TEXTURE_TABLE_NAME );

	// We're done
	CountTextures();
	m_UpdateLuaEnums = false;
}

const C_TextureManager::C_ResourceIndex C_TextureManager::AddResourceBundle( const Sys::C_FilePath& fileName )
{
	Display::ResourceBundler::I_ResourceBundle* pResource = LoadBundle( fileName );
	if( !pResource )
		return -1;

	m_CriticalSecion.Enter();
	
	// See if we've any empty slots
	C_ResourceIndex index = m_InvalidResourceIndex;
	for( S32 i = 0; i < m_Resources.Size(); i++)
		if( m_Resources[ i ] == NULL )
		{
			index = (C_ResourceIndex)i;
			m_Resources[ i ] = pResource;
			break;
		}

	if( index == m_InvalidResourceIndex )
	{
		index = (C_ResourceIndex)m_Resources.Size();
		m_Resources.PushBack( pResource );
	}


	if( m_NullTextureHandle == NULL && index == 0 )
		m_NullTextureHandle = pResource->GetTexture2D( 0 );

	m_UpdateLuaEnums = true;

	m_CriticalSecion.Exit();

	return index;
}

const U32 C_TextureManager::CountTextures()
{
	U32 count = 0;
	for( S32 i = 0; i < m_Resources.Size(); i++ )
	{
		C_Resource *p = m_Resources[ i ];
		if( p == NULL )
			continue;

		count += p->ResourceCount();
	}

	return count;
}

const bool C_TextureManager::RemoveResourceBundle( const C_ResourceIndex index )
{
#if defined(PLATFORM_PS3)
	Display::Device()->BlockUntilIdle();
#endif

	// Ivalid index
	if( index == m_InvalidResourceIndex )
		return false;

	if( !m_Resources[ index ] )
	{
		LOGWARN( g_Log, DBSTR " Bundle %d does not exist", index );
		return false;
	}
	
	delete m_Resources[ index ];
	m_Resources[ index ] = NULL;

	m_UpdateLuaEnums = true;

	if( index == 0 )
		m_NullTextureHandle = NULL;

	return true;
}

Display::ResourceBundler::I_ResourceBundle* C_TextureManager::LoadBundle( const Sys::C_FilePath& fileName )
{
	RESOURCE_MEMORY_ID("UI");
	MEMORY_TRACKER_PUSH("ui.textures");

	Display::ResourceBundler::I_ResourceBundle* pResources = UiInterface()->CreateResourceBundle();
	
	Sys::C_FilePath xprFileName = fileName + ".bpr";
	if(!pResources->Create(xprFileName))
	{
		LOGWARN(g_Log, "%s: failed to create\n", xprFileName.Chars() );
		delete pResources;

		MEMORY_TRACKER_POP("ui.textures");
		return NULL;
	}

	MEMORY_TRACKER_POP("ui.textures");
	return pResources;
}

// Access
const char* C_TextureManager::GetTextureNameByIndex( U32 index )
{
	S32 resource, texture;
	C_Resource* p = GetResourceByIndex( index, resource, texture );
	if( !p )
		return "null";

	return p->GetResourceName( texture );
}

UITextureHandle C_TextureManager::GetTexturePtrByIndex( const U32 index )
{
	S32 resource, texture;
	C_Resource* p = GetResourceByIndex( index, resource, texture );
	if( !p )
		return m_NullTextureHandle;

	return p->GetTexture2D( texture );
}

C_TextureHandleIndex C_TextureManager::GetTextureByIndex( const U32 index )
{
	S32 resource, texture;
	C_Resource* p = GetResourceByIndex( index, resource, texture );
	if( !p )
		return m_NullTextureIndex;

	return TextureHandleIndex( resource, texture );
}

const char* C_TextureManager::GetTextureName( const C_TextureHandleIndex index )
{
	C_Resource *p = GetResource( index );
	if( !p )
		return "";

	return p->GetResourceName( TextureIndex( index ) );
}

UITextureHandle	C_TextureManager::GetTexturePtr( const C_TextureHandleIndex index )
{
	C_Resource *p = GetResource( index );
	if( !p )
		return m_NullTextureHandle;

	return p->GetTexture2D( TextureIndex( index ) );
}

UITextureHandle	 C_TextureManager::GetTexturePtrByName( const char *name )
{
	C_TextureHandleIndex index = GetTextureByName( name );

	return GetTexturePtr( index );
}

const U32 C_TextureManager::TextureCount() const
{
	return m_TextureCount;
}

C_TextureHandleIndex C_TextureManager::GetTextureByName( const char *name )
{
	// NULL test
	if( name == NULL || stricmp( name, "" ) == 0 || stricmp( name, "null" ) == 0 )
		return m_NullTextureIndex;

	// Find by name
	for( S32 i = 0; i < m_Resources.Size(); i++ )
	{
		C_Resource *p = m_Resources[ i ];
		if( p == NULL )
			continue;

		for( U32 j = 0; j < p->ResourceCount(); j++ )
			if( stricmp( p->GetResourceName( j ), name ) == 0 )
				return TextureHandleIndex( i, j );
	}

	// Not found
	return C_TextureHandleIndex();
}

// Internal stuff
C_TextureManager::C_Resource *C_TextureManager::GetResource( const C_TextureHandleIndex index )
{
	U16 idx = index.Index();

	C_ResourceIndex resouce = ResourceIndex( index );
	if( resouce == -1 )
	{
		LOGWARN(g_Log, DBSTR "C_ResourceIndex out invalid" );
		return NULL;
	}

	if( resouce >= m_Resources.Size() )
	{
		LOGWARN(g_Log, DBSTR "Handle index %d is out of range", resouce );
		return NULL;
	}

	if( m_Resources[ resouce ] == NULL )
	{
		LOGWARN(g_Log, DBSTR "Resource %d does not exist", resouce );
		return NULL;
	}

	return m_Resources[ resouce ];
}

const U32 C_TextureManager::ResourceIndex( const C_TextureHandleIndex index )
{
	return index.Index() & ((1 << 3)-1);
}

const U32 C_TextureManager::TextureIndex( const C_TextureHandleIndex index )
{
	U32 i = (index.Index() >> 3);

	return i;
}

C_TextureHandleIndex C_TextureManager::TextureHandleIndex( S32 resource, S32 texture )
{
	if( resource == -1 )
		return C_TextureHandleIndex();

	return C_TextureHandleIndex( (S32)(resource | ( texture << 3 )) );
}

C_TextureManager::C_Resource* C_TextureManager::GetResourceByIndex( const U32 index, S32& resource, S32& texture )
{
	texture = (S32)index;

	for( resource = 0; resource < m_Resources.Size(); resource++ )
	{
		C_Resource *p = m_Resources[ resource ];
		if( p == NULL )
			continue;

		if( (U32)texture < p->ResourceCount() )
			return p;

		texture -= p->ResourceCount();
	}

	return NULL;
}

} // namespace UI
