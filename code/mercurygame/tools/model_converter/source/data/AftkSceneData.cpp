#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/AftkSceneData.h>

C_DataAftkScene::C_DataAftkScene()
: m_pAftkScene(NULL)
{
	m_pAftkScene = new Aftk::C_Scene;
}

C_DataAftkScene::~C_DataAftkScene()
{
	SAFE_DELETE_PTR( m_pAftkScene );
}

const Util::C_String C_DataAftkScene::GetName() const
{
	return Util::C_String("C_DataAftkScene");
}

const Aftk::C_Scene& C_DataAftkScene::GetAftkScene() const
{
	return *m_pAftkScene;
}

Aftk::C_Scene& C_DataAftkScene::GetAftkScene()
{
	return *m_pAftkScene;
}

void C_DataAftkScene::FreeData()
{
	SAFE_DELETE_PTR( m_pAftkScene );
}
