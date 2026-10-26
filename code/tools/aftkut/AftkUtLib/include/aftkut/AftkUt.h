#ifndef AFTKUT_AFTKUT_H
#define AFTKUT_AFTKUT_H

#pragma once

#include <float.h>
#include <set>

#include <core/core.h>
#include <system/debuglog.h>

namespace AftkUt
{
	extern Debug::Logger& g_Log;
}

#include "AftkUt/AftkUtQuery.h"
#include "aftkut/aftkutUtility.h"
#include "aftkut/aftkutList.h"
#include "aftkut/aftkutRadialEdgeMesh.h"
#include "aftkut/aftkutRadialEdgeVertex.h"
#include "aftkut/aftkutRadialEdgeEdge.h"
#include "aftkut/aftkutRadialEdgePolygon.h"
#include "aftkut/aftkutSmoothingHelper.h"
#include "aftkut/aftkutSmoothTangents.h"
#include "aftkut/aftkutSmoothNormals.h"
#include "aftkut/aftkutNormaliseTextureCoordinates.h"
#include "aftkut/aftkutExpandUniqueRemappedInstances.h"
#include "aftkut/aftkutProcessDuplicateNodes.h"
#include "aftkut/aftkutImport.h"
#include "aftkut/aftkutVersion.h"
#include "aftkut/aftkutNodeFlagParser.h"
#include "aftkut/aftkutCreateMaterial.h"
#include "aftkut/aftkutRemoveBaseInstances.h"
#include "aftkut/aftkutReplaceWithActorNodes.h"
#include "aftkut/AftkutCentreSceneNode.h"
#include "aftkut/AftkutGenerateAdPlacement.h"
#include "aftkut/AftkutFindAmbiguousActorNames.h"
#include "aftkut/AftkutMakeDoubleSidedPolygons.h"
#include "aftkut/AftkUtPolyFlagLowestLods.h"
#include "aftkut/AftkutRemapMirroredMaterials.h"
#include "aftkut/AftkutMergeMeshes.h"
#include "aftkut/AftkutBlindDataHelper.h"
#include "Aftkut/AftkutRegenerateBrokenNormals.h"
#include "Aftkut/AftkutFixNonUniformScaling.h"
#include "Aftkut/AftkutReplaceWithLodGroup.h"
#include "Aftkut/AftkutRemoveConstructionNodes.h"
#include "Aftkut/AftkutSnapVertices.h"
#include "Aftkut/AftkutCrowdHelper.h"
#include "Aftkut/AftkutIrmOptimise.h"

#endif
