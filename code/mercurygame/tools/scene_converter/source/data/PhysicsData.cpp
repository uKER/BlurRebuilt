#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/PhysicsData.h>

enum E_PhysicsFileVersion
{
	E_PhysicsFileVersion_Initial,

	E_PhysicsFileVersion_Current = E_PhysicsFileVersion_Initial
};

C_DataPhysicsData::C_DataPhysicsData()
	: m_pPolyInform(NULL)
	, m_pNodeInform(NULL)
	, m_pMainScene(NULL)
{
}

C_DataPhysicsData::~C_DataPhysicsData()
{
	SAFE_DELETE_PTR( m_pMainScene );
	const S32 exportSceneCount = m_RouteSceneList.Size();
	for(S32 i=0; i<exportSceneCount; i++)
	{
		SAFE_DELETE_PTR( m_RouteSceneList[i] );
	}

	SAFE_DELETE_PTR( m_pPolyInform );
	SAFE_DELETE_PTR( m_pNodeInform );
}

const Util::C_String C_DataPhysicsData::GetName() const
{
	return Util::C_String("C_DataPhysicsData");
}

Gravity::C_Converter* C_DataPhysicsData::NewRouteScene()
{
	assert( m_pPolyInform );
	Gravity::C_Converter* pNewRouteScene = new Gravity::C_Converter( m_pPolyInform , m_pNodeInform );
	m_RouteSceneList.PushBack( pNewRouteScene );
	return pNewRouteScene;
}

Gravity::C_Converter* C_DataPhysicsData::GetMainScene()
{
	if( m_pMainScene )
		return m_pMainScene;
	
	assert( m_pPolyInform );	
	m_pMainScene = new Gravity::C_Converter( m_pPolyInform, m_pNodeInform );	
	return m_pMainScene;
}


Gravity::C_Converter* C_DataPhysicsData::GetRouteScene( const S32 index )
{
	return m_RouteSceneList[index];
}

const Gravity::C_Converter* C_DataPhysicsData::GetRouteScene( const S32 index ) const
{
	return m_RouteSceneList[index];
}

S32 C_DataPhysicsData::GetRouteSceneCount() const
{
	return m_RouteSceneList.Size();
}

void C_DataPhysicsData::SetPolyInform( Gravity::C_PolyInform* pPolyInform )
{
	if(pPolyInform != m_pPolyInform)
	{
		SAFE_DELETE_PTR( m_pPolyInform );
	}

	m_pPolyInform = pPolyInform;
}


void C_DataPhysicsData::SetNodeInform( Gravity::C_NodeInform* pNodeInform )
{
	if(pNodeInform != m_pNodeInform)
	{
		SAFE_DELETE_PTR( m_pNodeInform );
	}

	m_pNodeInform = pNodeInform;
}

bool C_DataPhysicsData::Export( Sys::I_File* pFile ) const
{
	bool success = true;

	Sys::C_FileBlock rootBlock(pFile, "Physic");

	const S32 routeCount = m_RouteSceneList.Size();
	// Header Block
	{
		Sys::C_FileBlock headerBlock(&rootBlock, "Header");

		Sys::BinarySerialiserAutoEndian ar(headerBlock);

		ar << (S32)E_PhysicsFileVersion_Current;

		ar << (S32)routeCount;

	}

	{
		Sys::C_FileBlock sceneBlock(&rootBlock, "Main");

		success = m_pMainScene->Export( &sceneBlock ) && success;
	}

	{
		Sys::C_FileBlock routesBlock(&rootBlock, "Routes");

		for(S32 routeNo=0; routeNo < routeCount; routeNo++)
		{
			Sys::C_FileBlock routeBlock(&routesBlock, "Route");			
			assert( m_RouteSceneList[routeNo] );

			success = m_RouteSceneList[routeNo]->Export( &routeBlock ) && success;
		}

	}

	return success;

	//if( m_RouteSceneList.Size() <= 0 )
	//	return true;

	//return m_RouteSceneList[0]->Export( pFile );
}
