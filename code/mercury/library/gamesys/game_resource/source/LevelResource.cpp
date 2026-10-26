#include "GameResourcePch.h"
//#include <system/time/scopedtimer.h>

#include <system/serialise/C_FileBlock.h>
#include <system/Memory.h>
#include <system/debug/metadebug/MetaDebug.h>
#include <system/debug/metadebug/DefaultMetaDebugNotify.h>
#include <gravity/importer/Importer.h>

#include <world_render/RenderLevel.h>

#include <game_resource/GameResource.h>
#include <game_resource/LevelResource.h>
#include <game_resource/PhysicsLevel.h>
#include <game_resource/LevelSceneManager.h>
#include <game_resource/CrowdData.h>

#include <NavMesh/NavMesh.h>

#define RENDERINGDATA_ALLOW_INCLUDE // only for LevelResource.cpp

#include <renderingdata/renderingdata.h>
#include <Advertising/Advertising.h>
#include <Fx/RenderMutex.h>

namespace GameResource
{

Debug::Bool g_RenderModelInfo( "Gamesys.Level.RenderModelInfo", false );
Debug::Bool g_StoreAllMetadebug( "Gamesys.GameResource.LoadAllMetaDebug", false );
Debug::Int g_MaxLevelWarningMessages( "Gamesys.GameResource.MaxLevelWarningMessages", 5, Debug::Persist() );

// C_LevelResource
C_LevelResource::C_LevelResource()
: m_pModelData(NULL)
, m_pRenderLevel( NULL )
, m_pPhysicsLevel( NULL )
, m_pNavMesh( NULL )
{
}

C_LevelResource::~C_LevelResource()
{
}

bool C_LevelResource::LoadResourceData()
{
	MEMORY_TRACKER_PUSH_SUB("level");
	const Sys::C_FilePath& resourceFilename = GetResourceFilename();
	{
		Sys::I_File* pFile = OpenResourceFile( GetResourceFilename() );

		if( !pFile )
		{
			MEMORY_TRACKER_POP_SUB("level");
			return false;
		}

		bool success = LoadLevelData( pFile );

		if(!success)
		{
			FreeResourceData();
			MEMORY_TRACKER_POP_SUB("level");
			return false;
		}

		#if !defined(CFG_GOLD) && !defined(CFG_PROFILE)
		if(success)
		{
			if(pFile->GetPosition() < pFile->Length())
			{
				Sys::C_FileBlock metaDebugBlock( pFile );
				if( metaDebugBlock.HeaderId() != "MtaDbg" )
				{
					LOGWARN( g_Log, "Expected a 'MtaDbg' Block" );
				}
				else
				{
					Util::C_String logHeader = Util::C_String("Begin Level Export Log (section '%n') from file '") + pFile->Name().Chars() + "':";
					Util::C_String logFooter = Util::C_String("End Level Export Log (section '%n') from file '") + pFile->Name().Chars() + "':";
					Util::C_String logMessage = Util::C_String("   > %c");

					DebugLevel::Level filter=DebugLevel::Info;
					bool storeLogs = false;

					if(g_StoreAllMetadebug)
					{
						filter = DebugLevel::Debug;
						storeLogs = true;
					}

					MetaDebug::C_DefaultMetaDebugNotify notify(filter, g_MaxLevelWarningMessages, logMessage.Chars(), logHeader.Chars(), logFooter.Chars(), storeLogs);
					MetaDebug::C_MetaNode* pRootMetaNode = new MetaDebug::C_MetaNode("");
					LOGDEBUG(MetaDebug::g_Log, "MetaDebug for %s", GetResourceFilename().Chars() );
					pRootMetaNode->Read( &metaDebugBlock, (MetaDebug::I_MetaDebugNotify*)&notify );
					pRootMetaNode->Dump();
					m_pModelData->SetRootMetaNode( pRootMetaNode );
				}
			}
		}
		#endif

		SAFE_DELETE_PTR( pFile );
	}

	{
		Sys::I_File* pFile = OpenResourceFile( resourceFilename + "physics" );

		if( !pFile )
		{
			MEMORY_TRACKER_POP_SUB("level");
			return false;
		}

		bool success = LoadPhysicsData( pFile );

		SAFE_DELETE_PTR( pFile );
		if(!success)
		{
			FreeResourceData();
			MEMORY_TRACKER_POP_SUB("level");
			return false;
		}
	}
	{
		Sys::I_File* pFile = OpenResourceFile( resourceFilename + "render" );

		if( !pFile )
		{
			MEMORY_TRACKER_POP_SUB("level");
			return false;
		}

		bool success = LoadRenderData( pFile );

		SAFE_DELETE_PTR( pFile );
		if(!success)
		{
			FreeResourceData();
			MEMORY_TRACKER_POP_SUB("level");
			return false;
		}
	}
	//InitStreaming( resourceFilename + "stream" );

	if(m_pModelData)
	{
		m_pModelData->Dump();
		m_pModelData->m_pPhysicsImport = GetPhysicsLevel()->GetMainImportScene();
	}
	MEMORY_TRACKER_POP_SUB("level");
	return true;
}

enum E_SceneFileVersion
{
	E_SceneFileVersion_Initial = 0,
	E_SceneFileVersion_Current = E_SceneFileVersion_Initial
};

bool C_LevelResource::LoadLevelData( Sys::I_File* pFile )
{
	if( !ValidatePlatformId( pFile ) )
	{
		LOGERROR( g_Log, "Wrong Platform %s", pFile->Name().Chars());
		return false;
	}

	bool success = true;
	MEMORY_TRACKER_PUSH_SUB("leveldata");

	const U32 prevFileVersion = pFile->GetVersionNumber();
	U32 fileVersion = E_SceneFileVersion_Initial;

	Sys::C_FileBlock rootBlock( pFile );
	if( rootBlock.HeaderId() != "Level" )
	{
		LOGWARN( g_Log, "Expected a 'Level' Block" );
		MEMORY_TRACKER_POP_SUB("leveldata");
		return false;
	}

	// keep reading child blocks
	while ( rootBlock.GetPosition() < rootBlock.Length() )
	{
		Sys::C_FileBlock block( &rootBlock );

		if( block.HeaderId() == "Header" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			ar >> fileVersion;
			pFile->SetVersionNumber( fileVersion );
		}
		else			
		if( block.HeaderId() == "MdlDat" )
		{
			m_pModelData = new GameResource::C_ModelData;
			success = m_pModelData->Read( &block );
		}
		else			
		if( block.HeaderId() == "NavDat" )
		{
			m_pNavMesh = new AI::C_NavMesh();
			m_pNavMesh->LoadMesh(block);
		}
		else
		if ( block.HeaderId() == "Advert" )
		{
			Advertising::Read(block);
		}
		else
		{
			LOGWARN( g_Log, "Unknown Block Header '%s'", block.HeaderId().Chars() );
		}
	}

	pFile->SetVersionNumber( prevFileVersion );
	MEMORY_TRACKER_POP_SUB("leveldata");

	return success;
}


bool C_LevelResource::LoadPhysicsData( Sys::I_File* pFile )
{
	if( !ValidatePlatformId( pFile ) )
	{
		LOGERROR( g_Log, "Wrong Platform %s", pFile->Name().Chars());
		return false;
	}

	MEMORY_TRACKER_PUSH_SUB("gravity");
	m_pPhysicsLevel = new C_PhysicsLevel();
	bool success = m_pPhysicsLevel->Load( pFile );
	MEMORY_TRACKER_POP_SUB("gravity");
	return success;
}

bool C_LevelResource::LoadRenderData( Sys::I_File* pFile )
{
	if( !ValidatePlatformId( pFile ) )
	{
		LOGERROR( g_Log, "Wrong Platform %s", pFile->Name().Chars());
		return false;
	}

	MEMORY_TRACKER_PUSH_SUB("render");
	m_pRenderLevel = new WorldRender::C_RenderLevel();
	bool success = m_pRenderLevel->Read( pFile );

	MEMORY_TRACKER_POP_SUB("render");
	return success;
}

void C_LevelResource::SetDebugNames()
{
	m_pRenderLevel->GetScene().SetDebugName( m_Filename.Chars() );

	// set friendly names for each model inside the resource. this is so we can print out the names of model instances inside the game
	for (int i=0; i<m_pRenderLevel->GetNumModels(); i++)
	{
		const char *psz = m_pModelData->GetModel(i)->GetName();
		if (stricmp(psz, "DefaultModel"))
			m_pRenderLevel->GetModel(i).SetFriendlyName(psz);
		else
		{
			const char *psz = m_Filename.Chars();
			const char *psz2 = strrchr(psz, '\\');
			if (!psz2)
				psz2 = strrchr(psz, '/');
			if (!psz2)
				psz2 = psz;
			else
				psz2++;
			m_pRenderLevel->GetModel(i).SetFriendlyName(psz2);
		}
	}

}

bool C_LevelResource::InitStreaming( const Util::C_String& streamFileName )
{
	return m_pRenderLevel->InitStreaming( streamFileName );
}

bool C_LevelResource::PostLoadResourceData()
{
	if (FX::TrySerialiseMutex())
	{
		//SCOPED_TIMER("		 C_LevelResource::PostLoadResourceData");

		MEMORY_TRACKER_PUSH_SUB("render");
		m_pRenderLevel->OnLoad();
		FX::ReleaseSerialiseMutex();
		const Sys::C_FilePath& resourceFilename = GetResourceFilename();
		InitStreaming( (resourceFilename + "stream").Chars() );
		MEMORY_TRACKER_POP_SUB("render");

		SetDebugNames();
		return true;
	}
	return false;
}

bool C_LevelResource::PreFreeResourceData()
{
	return true;
}

bool C_LevelResource::FreeResourceData()
{
	SAFE_DELETE_PTR( m_pModelData );
	SAFE_DELETE_PTR( m_pRenderLevel );
	SAFE_DELETE_PTR( m_pNavMesh );
	SAFE_DELETE_PTR( m_pPhysicsLevel );
	return true;
}

S32 C_LevelResource::GetEvictDelay()
{
	return -1;
}

Util::C_String C_LevelResource::StaticGetResourceTypeName()
{
	return "Level";
}

void C_LevelResource::DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m )
{
	if( g_RenderModelInfo )
	{
		m_pModelData->DebugRender(pDebugRender, m);
	}

	if (m_pNavMesh)
		m_pNavMesh->DebugRender(pDebugRender);
}

WorldRender::C_RenderModelData* C_LevelResource::GetRenderModelData()
{
	return (WorldRender::C_RenderModelData*)m_pRenderLevel;
}

const WorldRender::C_RenderModelData* C_LevelResource::GetRenderModelData() const
{
	return (WorldRender::C_RenderModelData*)m_pRenderLevel;
}

	
WorldRender::C_RenderLevel* C_LevelResource::GetRenderLevel()
{
	return m_pRenderLevel;
}

C_PhysicsLevel* C_LevelResource::GetPhysicsLevel()
{
	return m_pPhysicsLevel;
}

GameResource::C_ModelData* C_LevelResource::GetModelData()
{
	return m_pModelData;
}

const GameResource::C_ModelData* C_LevelResource::GetModelData() const
{
	return m_pModelData;
}


AI::C_NavMesh* C_LevelResource::GetNavMesh() const
{
	return m_pNavMesh;
}

MetaDebug::C_MetaNode* C_LevelResource::GetRootMetaNode() const
{
#ifdef CFG_GOLD
	return NULL;
#else
	return	m_pModelData->GetRootMetaNode();
#endif
}

} // namespace GameResource
