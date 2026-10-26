#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_HEADERS_H
#define AFTK_HEADERS_H

#define AFTK_USE_MEMORY_POOL

//#define AFTK_LEAN_AND_MEAN

//#pragma message("Including AFTK")

#include <math.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <new>
#include <set>
#include <map>

#include <core/maths/utmaths/maths.h>
#include <core/maths/utmaths/mmatrix.h>
#include <core/containers/vector.h>

#include <core/containers/OpenHashTable.h>
#include <core/utility/HashOps.h>
#include <core/containers/SortedBinaryTree.h>

#include <core/allocators/SSSMemoryPool.h>
#include <core/allocators/SSSMultiMemoryPool.h>
#include <core/allocators/SSSMemoryPoolUtInitialiser.h>
#include <system/DebugLog.h>
#include <core/containers/string.h>

namespace Aftk
{
	extern Debug::Logger& g_aftkMainLogger;
}

#include <Aftk/AftkDefinitions.h>
#include <Aftk/AftkEnum.h>
#include <Aftk/AftkTypedefs.h>

#include <Aftk/AftkStatus.h>
#include <Aftk/AftkGlobal.h>
#include <Aftk/AftkIObject.h>
#include <Aftk/AftkMemoryManager.h>
#include <Aftk/AftkAllocators.h>

#include <Aftk/AftkTextureCoordinateIndices.h>

#include <Aftk/AftkBounds.h>

#include <Aftk/AftkProgress.h>

#include <Aftk/AftkAttachData.h>

#include <Aftk/AftkSharedResource.h>
#include <Aftk/AftkSharedResourceManager.h>

#include <Aftk/AftkUtility.h>
#include <Aftk/AftkGenericAttribute.h>
#include <Aftk/AftkBlobData.h>

#include <Aftk/AftkNode.h>
#include <Aftk/AftkBaseKey.h>
#include <Aftk/AftkLinearKey.h>
#include <Aftk/AftkHermiteKey.h>

#include <Aftk/AftkIEvaluatableNode.h>

#include <Aftk/AftkIPlug.h>
#include <Aftk/AftkDynamicPlug.h>
#include <Aftk/AftkNullPlug.h>
#include <Aftk/AftkStaticPlug.h>
#include <Aftk/AftkProxyPlug.h>

#include <Aftk/AftkBlendWeightedNode.h>
#include <Aftk/AftkUnitConversionNode.h>

#include <Aftk/AftkShaderNode.h>
#include <Aftk/AftkShaderImageNode.h>
#include <Aftk/AftkMaterialNode.h>
#include <Aftk/AftkNamedResourceManager.h>
#include <Aftk/AftkBlindDataNode.h>

#include <Aftk/AftkBlindDataResourceManager.h>
#include <Aftk/AftkLayerNode.h>
#include <Aftk/AftkXRefScene.h>
#include <Aftk/AftkScene.h>

#include <Aftk/AftkAnimationCurveNode.h>

#include <Aftk/AftkComponentCluster.h>
#include <Aftk/AftkPolygonCluster.h>
#include <Aftk/AftkPolygonVertex.h>
#include <Aftk/AftkPolygon.h>
#include <Aftk/AftkTriangleIterator.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkGeometry.h>
#include <Aftk/AftkNullMemoryManager.h>

#include <Aftk/AftkSceneNode.h>
#include <Aftk/AftkLimitMixin.h>
#include <Aftk/AftkISceneConstraintNode.h>
#include <Aftk/AftkSceneConstraintNode.h>
#include <Aftk/AftkSceneAimConstraintNode.h>
#include <Aftk/AftkSceneLookAtConstraintNode.h>
#include <Aftk/AftkSceneCameraNode.h>
#include <Aftk/AftkSceneGeometryNode.h>
#include <Aftk/AftkWeight.h>
#include <Aftk/AftkWeightEnvelope.h>
#include <Aftk/AftkInfluenceClusterNode.h>
#include <Aftk/AftkVertexDataChannelInfo.h>
#include <Aftk/AftkSharedResourceMapping.h>
#include <Aftk/AftkVertexWeight.h>
#include <Aftk/AftkMatrixPaletteEntry.h>
#include <Aftk/AftkMeshLodLevelInfo.h>
#include <Aftk/AftkGroupLodLevelInfo.h>
#include <Aftk/AftkSceneLodGroupNode.h>
#include <Aftk/AftkISceneMeshNode.h>
#include <Aftk/AftkRayTriangleResult.h>
#include <Aftk/AftkRayMeshResult.h>
#include <Aftk/AftkRayPlaneResult.h>
#include <Aftk/AftkRay.h>
#include <Aftk/AftkSceneMeshRemappableNode.h>
#include <Aftk/AftkSceneNodeIterator.h>
#include <Aftk/AftkTextureCoordinateSet.h>
#include <Aftk/AftkSceneMeshRemappedInstanceNode.h>
#include <Aftk/AftkSceneMeshBonedInstanceNode.h>
#include <Aftk/AftkLodLevel.h>
#include <Aftk/AftkMeshWeldComponentAxis.h>
#include <Aftk/AftkMeshWeldMemoryManager.h>
#include <Aftk/AftkDistanceDimNode.h>
#include <Aftk/AftkScenePointConstraintNode.h>
#include <Aftk/AftkSceneOrientConstraintNode.h>
//Visitor classes
#include <Aftk/AftkMeshWeld.h>
#include <Aftk/AftkMeshRemoveDegenerates.h>
#include <Aftk/AftkMeshExpandPolygonVertices.h>
#include <Aftk/AftkMeshCalculateTangents.h>
#include <Aftk/AftkMeshCalculateNormals.h>
#include <Aftk/AftkMeshTriangulate.h>
//End Visitors
#include <Aftk/AftkSceneMeshNode.h>
#include <Aftk/AftkSceneSplineNode.h>
#include <Aftk/AftkSceneInstanceNode.h>
#include <Aftk/AftkSceneLightNode.h>
#include <Aftk/AftkSceneNonAmbientLightNode.h>
#include <Aftk/AftkScenePointLightNode.h>
#include <Aftk/AftkSceneAreaLightNode.h>
#include <Aftk/AftkSceneDirectionalLightNode.h>
#include <Aftk/AftkSceneSpotLightNode.h>
#include <Aftk/AftkSceneXRefNode.h>
#include <Aftk/AftkSceneJointNode.h>
#include <Aftk/AftkSceneHemisphereLightNode.h>
#include <Aftk/AftkSceneNodeVisit.h>
#endif
