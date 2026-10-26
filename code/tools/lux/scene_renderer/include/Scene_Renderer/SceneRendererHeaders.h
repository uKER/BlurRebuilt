#ifndef SCENE_RENDERER_HEADERS_H
#define SCENE_RENDERER_HEADERS_H

#include <windows.h>

#include <string>

#include <D3DX9.h>
#include <dxerr.h>

#include <Scene_Renderer/Typedefs.h>
#include <Scene_Renderer/Common.h>
#include <Scene_Renderer/D3dWindow.h>
#include <Scene_Renderer/VertexBuffer.h>
#include <Scene_Renderer/DebugVB.h>
#include <Scene_Renderer/DebugRender.h>
#include <Scene_Renderer/Camera.h>
#include <Scene_Renderer/FirstPersonCamera.h>
#include <Scene_Renderer/TargetCamera.h>

#include <spatial_hierarchy/SpatialDividerHeaders.h>

#include <Scene_Renderer/HierachyRenderer.h>

#include <Scene_Renderer/ObjectRenderingBatch.h>
#include <Scene_Renderer/ObjectRenderingFragment.h>

#include <Scene_Renderer/ShaderProperties.h>
#include <Scene_Renderer/LightmapShaderProperties.h>

#include <Scene_Renderer/SceneRenderer.h>
#include <Scene_Renderer/GenericSceneRenderer.h>

#include <Scene_Renderer/D3dImageLoader.h>
#include <Scene_Renderer/D3DTextureManager.h>
#include <Scene_Renderer/D3DShaderManager.h>

#endif
