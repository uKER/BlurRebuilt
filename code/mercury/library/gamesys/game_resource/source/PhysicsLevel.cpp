#include "GameResourcePch.h"

#include <system/serialise/C_FileBlock.h>
#include <game_resource/GameResource.h>
#include <game_resource/PhysicsLevel.h>
#include <gravity/sdk/PhysicsSDK.h>	// [Blur2PC] Gravity::C_PhysicsSDK


namespace GameResource
{

enum E_PhysicsFileVersion
{
	E_PhysicsFileVersion_Initial,

	E_PhysicsFileVersion_Current = E_PhysicsFileVersion_Initial
};


// C_PhysicsLevel
C_PhysicsLevel::C_PhysicsLevel()
	: m_pMainImportScene( NULL )
{
}

C_PhysicsLevel::~C_PhysicsLevel()
{
	const S32 routeCount = GetRouteCount();
	for(S32 routeNo=0; routeNo<routeCount; routeNo++)
	{
		if( m_pRouteImportScenes[routeNo] )
		{
			Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseImportScene(m_pRouteImportScenes[routeNo]);
		}
	}

	Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseImportScene(m_pMainImportScene);
	m_pMainImportScene = NULL;
}

bool C_PhysicsLevel::Load( Sys::I_File* pFile )
{
	const U32 prevFileVersion = pFile->GetVersionNumber();
	U32 fileVersion = E_PhysicsFileVersion_Initial;

	Sys::C_FileBlock rootBlock( pFile );
	if( rootBlock.HeaderId() != "Physic" )
	{
		LOGWARN( g_Log, "Expected a 'Physic' Block" );

		{
			pFile->SetPosition( 0 );
			m_pMainImportScene = Gravity::C_PhysicsSDK::PhysicsSDK()->CreateImportScene();
			return m_pMainImportScene->Import( pFile );
		}

		return false;
	}

	S32 routeCount = 0;
	while ( rootBlock.GetPosition() < rootBlock.Length() )
	{
		Sys::C_FileBlock block( &rootBlock );

		if( block.HeaderId() == "Header" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			
			ar >> fileVersion;
			pFile->SetVersionNumber( fileVersion );

			ar >> routeCount;
			m_pRouteImportScenes.Resize( routeCount );
		}
		else			
		if( block.HeaderId() == "Main" )
		{
			assert( m_pMainImportScene == NULL );
			m_pMainImportScene = Gravity::C_PhysicsSDK::PhysicsSDK()->CreateImportScene();
			m_pMainImportScene->Import( &block );
		}
		else			
		if( block.HeaderId() == "Routes" )
		{
			S32 routeNo = 0;
			while ( block.GetPosition() < block.Length() )
			{
				Sys::C_FileBlock routeBlock( &block );
				if( routeBlock.HeaderId() == "Route" )
				{
					m_pRouteImportScenes[routeNo] = Gravity::C_PhysicsSDK::PhysicsSDK()->CreateImportScene(); 
					m_pRouteImportScenes[routeNo]->Import( &routeBlock );
					routeNo++;
				}
				else
				{
					LOGWARN( g_Log, "Unknown Block Header '%s'", routeBlock.HeaderId().Chars() );
				}
			}
			assert( routeNo == routeCount );
		}
		else
		{
			LOGWARN( g_Log, "Unknown Block Header '%s'", block.HeaderId().Chars() );
		}
	}

	pFile->SetVersionNumber( prevFileVersion );

	return true;
}

Gravity::C_ImportScene* C_PhysicsLevel::GetMainImportScene() const
{
	return m_pMainImportScene;
}

S32 C_PhysicsLevel::GetRouteCount() const
{
	return m_pRouteImportScenes.Size();
}

Gravity::C_ImportScene* C_PhysicsLevel::GetRouteImportScene( const S32 routeNo ) const
{
	return m_pRouteImportScenes[routeNo];
}

} // namespace GameResource
