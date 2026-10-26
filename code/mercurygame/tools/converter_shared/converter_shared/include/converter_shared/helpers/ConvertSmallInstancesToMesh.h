#ifndef CONVERT_SMALL_INSTANCES_TO_MESH_H
#define CONVERT_SMALL_INSTANCES_TO_MESH_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <Aftk/Aftk.h>

// C_ConvertSmallInstancesToMesh

class C_ConvertSmallInstancesToMesh
{
public:
	static const bool ConvertSmallInstancesToMesh( Aftk::C_Scene* pAftkScene );

private:
	static const bool ConvertSmallInstanceToMesh( Aftk::C_SceneMeshNode* pAftkSceneMeshNode );
};

#endif