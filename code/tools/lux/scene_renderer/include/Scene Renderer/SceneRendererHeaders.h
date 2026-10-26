#ifndef SCENE_RENDERER_HEADERS_H
#define SCENE_RENDERER_HEADERS_H

#include <windows.h>

#include <string>

#include <D3DX9.h>
#include <dxerr.h>

#include <Scene Renderer/Typedefs.h>
#include <Scene Renderer/Common.h>
#include <Scene Renderer/D3dWindow.h>
#include <Scene Renderer/VertexBuffer.h>
#include <Scene Renderer/DebugVB.h>
#include <Scene Renderer/DebugRender.h>
#include <Scene Renderer/Camera.h>
#include <Scene Renderer/FirstPersonCamera.h>
#include <Scene Renderer/TargetCamera.h>

#include <spatial_hierarchy/SpatialDividerHeaders.h>

#include <Scene Renderer/HierachyRenderer.h>

#include <Scene Renderer/ObjectRenderingBatch.h>
#include <Scene Renderer/ObjectRenderingFragment.h>

#include <Scene Renderer/ShaderProperties.h>
#include <Scene Renderer/LightmapShaderProperties.h>

#include <scene Renderer/SceneRenderer.h>
#include <Scene Renderer/GenericSceneRenderer.h>

#include <Scene Renderer/D3dImageLoader.h>
#include <Scene Renderer/D3DTextureManager.h>
#include <Scene Renderer/D3DShaderManager.h>

#endif
