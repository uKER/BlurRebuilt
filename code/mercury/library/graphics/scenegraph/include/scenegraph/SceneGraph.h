#ifndef _SCENE_GRAPH_H_
#define _SCENE_GRAPH_H_

#include "SceneGraph/Light/Light.h"

namespace Animation
{
	class C_PoseFormat;
}

namespace SceneGraph
{

class C_ModelInstance;
class C_SceneGraph;
class C_VisibleDetect;
class C_LevelInstance;

void Insert(C_SceneGraph *pSceneGraph, C_ModelInstance *pModelInstance);
void Remove(C_SceneGraph *pSceneGraph, C_ModelInstance *pModelInstance);
void Insert(C_SceneGraph *pSceneGraph, I_GatherNode *pGatherNode);
void Remove(C_SceneGraph *pSceneGraph, I_GatherNode *pGatherNode);
void Insert(C_SceneGraph *pSceneGraph, C_LevelInstance *pLevelInstance);
void Remove(C_SceneGraph *pSceneGraph, C_LevelInstance *pLevelInstance);

void SetPoseFormat(C_SceneGraph *pSceneGraph, const Animation::C_PoseFormat*);
void SetNumMatrixPalettes(C_SceneGraph *pSceneGraph, const S32 numMatrixPalettes);
void ClearMatrixPalettes(C_SceneGraph *pSceneGraph, const U32 offset = 0);
void SetMatrixPalette(C_SceneGraph *pSceneGraph, const S32 index, const Maths::Mat43* pMatrixPalette);
void SetPaletteMap(C_SceneGraph *pSceneGraph, const U8* pPaletteMap);

C_PointLightHandle AddPointLight(C_SceneGraph *pSceneGraph, bool isManaged = true);
void RemovePointLight(C_SceneGraph *pSceneGraph, C_PointLightHandle handle);
C_SpotLightHandle AddSpotLight(C_SceneGraph *pSceneGraph, bool isManaged = true);
void RemoveSpotLight(C_SceneGraph *pSceneGraph, C_SpotLightHandle handle);
C_DropShadowHandle AddDropShadow(C_SceneGraph *pSceneGraph, bool isManaged = true);
void RemoveDropShadow(C_SceneGraph *pSceneGraph, C_DropShadowHandle handle);

const Btl::Vector<SceneGraph::C_VisibleDetect*> &GetNewlyVisibleList(C_SceneGraph *);

Maths::C_RGBA_F32 EvaluateLighting(const C_SceneGraph* pSceneGraph, const Maths::HVec3& vPoint);

} // namespace SceneGraph

#endif // _SCENE_GRAPH_H_
