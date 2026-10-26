
#include "Stdafx.h"

#include <system/misc/stringutil.h>

#include "ui/illustrator/UIAdobeSceneManager.h"
#include "ui/Illustrator/UIAdobeScene.h"
#include "ui/Illustrator/UIAdobePath.h"
#include "ui/illustrator/Ipf/IpfReader.h"
#include "ui/UiInterface.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_UI
#endif

#ifndef CFG_GOLD
void ReloadIpfFiles( void* ptr )
{
	UI::C_AdobeSceneManager::Instance().ReloadScenes();
}

Debug::Function g_ReloadIpfFiles( "UI.Shapes.ReloadIpfFiles", ReloadIpfFiles, 0 );
#endif // CFG_GOLD

namespace UI
{
C_AdobeSceneManager C_AdobeSceneManager::m_sInstance;

/*--------------------------------------------------------------------------------------
	Function implementations
--------------------------------------------------------------------------------------*/
C_AdobeSceneManager::C_AdobeSceneManager()
: m_NumberOfGradients( 0 )
{
/*
	for(U32 i = 0; i < m_MaxNumberOfGradients; i++)
		m_GradientHandles[i] = NULL;
	*/
}


C_AdobeSceneManager::~C_AdobeSceneManager()
{
	FlushData();
}


bool C_AdobeSceneManager::CreateLuaEnums( lua_State *pState )
{
	if(pState == (lua_State*)(INTPTR)-1 || !pState)	// [Blur2PC][x64] was (U32)pState == 0xffffffff
	{
		LOGFATAL( g_Log, DBSTR " : lua_State is invalid : %x", (U32)(INTPTR)pState );
		return false;
	}

	// Push new table over old one
	lua_newtable(pState);
	// Common textures
	if(lua_type(pState, -1) != LUA_TTABLE)
	{
		LOGFATAL( g_Log, DBSTR " : failed to create new %s table : %x - stack top %d", LUA_SCENE_TABLE_NAME, (U32)(INTPTR)pState, lua_gettop(pState) );
		return false;
	}

	for( S32 i = 0; i < m_Handles.Size(); i++)
	{
		lua_pushstring(pState, m_Handles[i].ScenePtr()->Name() ); 
		lua_pushnumber(pState, i); 
		lua_rawset(pState,-3);
	}
	lua_setglobal(pState,LUA_SCENE_TABLE_NAME);

	return true;
}


bool C_AdobeSceneManager::LoadScene( const char *filepath )
{
	//MEMORY_TRACKER_PUSH("ui.adobe.scene" );
	RESOURCE_MEMORY_ID("UI");

	C_AdobeScene* pScene = new C_AdobeScene( 
		Sys::ExtractFilename( Sys::RemoveFileExtension( filepath ) ).Chars() );

	LOGDEBUG( g_Log, DBSTR "Reading scene from file - %s", filepath);

	Ipf::C_IpfReader ipfReader;
	if ( !ipfReader.ImportScene( *Sys::GetDefaultFileSystem(), filepath, pScene ) )
	{
		LOGWARN( UI::g_Log, DBSTR "Failed to open file '%s'\n", filepath );

		SAFE_DELETE_PTR( pScene );
		//MEMORY_TRACKER_POP("ui.adobe.scene" );
		return false;
	}
	// Add a new handle and map the scene pointer
	m_CriticalSecion.Enter();

	m_Handles.PushBack( C_AdobeHandle( pScene, filepath ) );

	m_CriticalSecion.Exit();

	//MEMORY_TRACKER_POP("ui.adobe.scene" );
	return false;
}


bool C_AdobeSceneManager::LoadScenes( const char* directory )
{
	Sys::C_DirectoryInfo directoryInfo;

	if ( Sys::GetDefaultFileSystem()->DirectoryInfo( directory, &directoryInfo ) )
	{
		// Pull out each file in the directory.
		for( U32 i = 0; i < directoryInfo.NumFiles(); i++ )
		{
			Sys::C_FileInfo* pFileInfo = &directoryInfo.FileInfo( i );

			// If the file is an illustrator polygon file, then load the Scene.
			if ( Sys::ExtractFileExtension( pFileInfo->m_FileName ) == "ipf" )
			{
				LoadScene( pFileInfo->m_FileName.Chars() );
			}
		}
		return true;
	}
	LOGWARN( UI::g_Log, DBSTR "Could not find directory '%s'", directory );
	return false;
}


void C_AdobeSceneManager::ReloadScene( const S32 sIndex )
{
	if ( sIndex > m_Handles.Size() )
		return;

	//MEMORY_TRACKER_PUSH("ui.adobe.scene" );
	RESOURCE_MEMORY_ID("UI");

	C_AdobeScene* pScene = new C_AdobeScene( m_Handles[sIndex].ScenePtr()->Name() );

	Ipf::C_IpfReader ipfReader;
	if ( !ipfReader.ImportScene( *Sys::GetDefaultFileSystem(), m_Handles[sIndex].Filepath(), pScene ) )
	{
		LOGWARN( UI::g_Log, DBSTR "Failed to reload '%s'\n", m_Handles[sIndex].Filepath() );
		SAFE_DELETE_PTR( pScene );
		//MEMORY_TRACKER_POP("ui.adobe.scene" );
	}

	m_Handles[sIndex].Destroy();
	m_Handles[sIndex].m_Dirty = true;
	m_Handles[sIndex].m_pSceneHandle = pScene;

	//MEMORY_TRACKER_POP("ui.adobe.scene" );
}


void C_AdobeSceneManager::ReloadScenes()
{
	for ( S32 i = 0; i < m_Handles.Size(); i++ )
	{
		ReloadScene( i );
	}
}


void C_AdobeSceneManager::RemoveScene( const char* pName )
{
	if( !pName )
		return;

	for ( S32 i = 0; i < m_Handles.Size(); i++ )
		if( m_Handles[ i ].m_pSceneHandle && stricmp( m_Handles[ i ].m_pSceneHandle->Name(), pName ) == 0 )
		{
			SAFE_DELETE_PTR( m_Handles[ i ].m_pSceneHandle );
			m_Handles.EraseByIndex( i );
			return;
		}

	LOGWARN( g_Log, DBSTR "%s : Not found", pName );
}


void C_AdobeSceneManager::FlushData()
{
	for ( S32 i = 0; i < m_Handles.Size(); i++ )
	{
		SAFE_DELETE_PTR( m_Handles[i].m_pSceneHandle );
	}
	m_Handles.Clear();

	/*
	for ( U32 grad = 0; grad < m_NumberOfGradients; grad++ )
	{
		SAFE_DELETE_PTR( m_GradientHandles[ grad ] );
	}
	m_NumberOfGradients = 0;
*/
}


const C_AdobeHandle* C_AdobeSceneManager::GetHandle( const S32 sIndex ) const
{
	if ( sIndex < m_Handles.Size() )
		return NULL;
	
	return &m_Handles[sIndex];
}


const C_AdobeHandle* C_AdobeSceneManager::GetHandleByName( const char* pName ) const
{
	for ( S32 i = 0; i < m_Handles.Size(); i++ )
	{
		if ( !stricmp( m_Handles[i].ScenePtr()->Name(), pName ) )
		{
			return &m_Handles[i];
		}
	}
	return NULL;
}


const S32 C_AdobeSceneManager::GetHandleIndexByName( const char* pName ) const
{
	for ( S32 i = 0; i < m_Handles.Size(); i++ )
	{
		if ( !stricmp( m_Handles[i].ScenePtr()->Name(), pName ) )
		{
			return i;
		}
	}
	return 0xffffffff;
}

/*
void C_AdobeSceneManager::AddNewGradient( const UIGradientHandle pHandle )
{
	C_AdobeGradient *p = new C_AdobeGradient();
	if(!p)
	{
		LOGERROR( g_Log, DBSTR "Failed to allocate gradient");
		return;
	}

	*p = *pHandle;

	m_GradientHandles[ m_NumberOfGradients ] = p;
	m_NumberOfGradients++;
}
*/


void C_AdobeSceneManager::Update()
{
#ifndef CFG_GOLD
	for ( S32 i = 0; i < m_Handles.Size(); i++ )
	{
		m_Handles[i].m_Dirty = false;
	}
#endif // CFG_GOLD
}

}// namespace UI
