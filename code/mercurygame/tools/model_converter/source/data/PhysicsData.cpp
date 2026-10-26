#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/PhysicsData.h>

C_DataPhysicsData::C_DataPhysicsData()
	: m_pPolyInform(NULL)
	, m_pNodeInform(NULL)
{
}

C_DataPhysicsData::~C_DataPhysicsData()
{
	const S32 exportSceneCount = m_ExportSceneList.Size();
	for(S32 i=0; i<exportSceneCount; i++)
	{
		SAFE_DELETE_PTR( m_ExportSceneList[i] );
	}

	SAFE_DELETE_PTR( m_pPolyInform );
	SAFE_DELETE_PTR( m_pNodeInform );
}

const Util::C_String C_DataPhysicsData::GetName() const
{
	return Util::C_String("C_DataPhysicsData");
}

Gravity::C_Converter* C_DataPhysicsData::NewExportScene( bool rotateY180 )
{
	assert( m_pPolyInform );
	Gravity::C_Converter* pNewExportScene = new Gravity::C_Converter( m_pPolyInform, m_pNodeInform );
	pNewExportScene->SetRotateY180( rotateY180 );
	m_ExportSceneList.PushBack( pNewExportScene );
	return pNewExportScene;
}

Gravity::C_Converter* C_DataPhysicsData::GetExportScene( const S32 index )
{
	return m_ExportSceneList[index];
}

const Gravity::C_Converter* C_DataPhysicsData::GetExportScene( const S32 index ) const
{
	return m_ExportSceneList[index];
}

S32 C_DataPhysicsData::GetExportSceneCount() const
{
	return m_ExportSceneList.Size();
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
	if( m_ExportSceneList.Size() <= 0 )
		return true;

	return m_ExportSceneList[0]->Export( pFile );
}
