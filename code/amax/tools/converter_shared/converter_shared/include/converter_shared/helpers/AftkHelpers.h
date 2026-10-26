#ifndef CONVERTER_SHARED_AFTK_HELPERS_H
#define CONVERTER_SHARED_AFTK_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Aftk/Aftk.h>
#include <system/misc/Progress.h>

namespace AftkHelpers
{

bool ReadAftkScene( Aftk::C_Scene& aftkScene, Util::C_String& aftkFilename, Progress::I_ProgressBar* pProgress );
bool WriteAftkScene( const Aftk::C_Scene& aftkScene, Util::C_String& aftkFilename, Progress::I_ProgressBar* pProgress );

bool ApplyGlobalSceneTransform( Aftk::C_Scene& aftkScene, const F32 globalScale, const bool rotateY180 );

Util::C_String GetLayerName( const Aftk::C_SceneNode* pAftkSceneNode );
bool IsNamedLayer( const Aftk::C_LayerNode *pLayer, const Util::C_String& searchName );
bool InNamedLayer( const Aftk::C_SceneNode* pAftkSceneNode, const Util::C_String& searchName );
bool IsInstance( const Aftk::C_SceneNode* pAftkSceneNode );
bool IsConstraint( const Aftk::C_SceneNode* pAftkSceneNode );

const Aftk::C_SceneNode* InstanceGetSourceNode( const Aftk::C_SceneNode* pAftkSceneNode );

Aftk::Matrix44 GetMatrix44( const Maths::HMat44& hMat );

Maths::HMat44 GetHMat44( const Aftk::Matrix44& aftkMatrix );
Maths::HMat44 GetHMat44( const Aftk::C_SceneNode* pAftkSceneNode );
Maths::HMat43 GetHMat43( const Aftk::C_SceneNode* pAftkSceneNode, const bool orthoNormal = true );

bool IsSceneNodeLight(const Aftk::C_SceneNode* pAftkSceneNode);

Maths::HVec3 CalculateSunDirection( F32 sunHeading, F32 sunElevation );

enum E_CollisionType
{
	E_CollisionType_None = -1,
	E_CollisionType_Sphere = 0,
	E_CollisionType_ConvexHull,
	E_CollisionType_Box,
	E_CollisionType_Cylinder,
	E_CollisionType_Capsule,
	E_CollisionType_Cone,
};

const E_CollisionType GetCollisionType( const Aftk::C_SceneNode* pAftkSceneNode );

} // namespace AftkHelpers

#endif
