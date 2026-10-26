#pragma once

#include <windows.h>

#include <string>

#include <D3DX9.h>
#include <dxerr.h>

#include <nexus_core/Headers.h>


#include <nexus_render/StringTable.h>
#include <nexus_render/Definitions.h>
#include <nexus_render/Structures.h>

#include <nexus_render/Common.h>
#include <nexus_render/D3DUtility.h>

#include <nexus_render/VertexFormats.h>
#include <nexus_render/D3DManager.h>

#include <tools_common/Status.h>

#include <nexus_render/Sphere.h>
#include <nexus_render/Plane.h>
#include <nexus_render/Frustum.h>

#include <nexus_render/D3DWindow.h>
#include <nexus_render/Primitive3DVertex.h>
#include <nexus_render/VBWrapper.h>
#include <nexus_render/Primitives.h>
#include <nexus_render/PrimitiveRenderer.h>
#include <nexus_render/Camera.h>
#include <nexus_render/FirstPersonCamera.h>
#include <nexus_render/TargetCamera.h>

#include <nexus_render/SceneRendererSettings.h>

#include <nexus_render/ViewGrid.h>

#include <nexus_render/SharedResource.h>

#include <nexus_render/StringTable.h>
#include <nexus_render/BoundingBox.h>
#include <nexus_render/VertexWeights.h>
#include <nexus_render/MeshCluster.h>
#include <nexus_render/RenderNode.h>
#include <nexus_render/RenderInstanceNode.h>
#include <nexus_render/RenderSplineNode.h>
#include <nexus_render/IRenderMeshNode.h>

#include <nexus_render/EffectWrapper.h>
#include <nexus_render/EffectConstants.h>
#include <nexus_render/MeshEffectWrapper.h>
#include <nexus_render/MeshBasicEffectWrapper.h>
#include <nexus_render/MeshBasicEffectConstants.h>
#include <nexus_render/MeshTextureEffectWrapper.h>
#include <nexus_render/MeshTextureEffectConstants.h>
#include <nexus_render/MeshLightmapEffectWrapper.h>
#include <nexus_render/MeshLightmapEffectConstants.h>
#include <nexus_render/MeshSelectionEffectWrapper.h>
#include <nexus_render/MeshSelectionEffectConstants.h>
#include <nexus_render/GizmoEffectWrapper.h>
#include <nexus_render/GizmoEffectConstants.h>

#include <nexus_render/GizmoRenderer.h>
#include <nexus_render/DirectionalLightGizmoRenderer.h>
#include <nexus_render/SpotLightGizmoRenderer.h>
#include <nexus_render/PointLightGizmoRenderer.h>
#include <nexus_render/AreaLightGizmoRenderer.h>
#include <nexus_render/AxisRenderer.h>
#include <nexus_render/MeshComponentDisplay.h>
#include <nexus_render/RenderMeshRemappableNode.h>
#include <nexus_render/RenderMeshNode.h>
#include <nexus_render/RenderLightNode.h>
#include <nexus_render/RenderNonAmbientLightNode.h>
#include <nexus_render/RenderPointLightNode.h>
#include <nexus_render/RenderSpotLightNode.h>
#include <nexus_render/RenderDirectionalLightNode.h>
#include <nexus_render/RenderAreaLightNode.h>
#include <nexus_render/RenderMeshInstanceNode.h>
#include <nexus_render/RenderMeshBonedInstanceNode.h>

#include <nexus_render/SceneRenderer.h>

#include <nexus_render/D3DImageLoader.h>
#include <nexus_render/D3DTextureManager.h>
#include <nexus_render/D3DShaderManager.h>

