#ifndef AFTK_RENDER_INCLUDE
#define AFTK_RENDER_INCLUDE

#include "misc/Definitions.h"

#include "camera/Camera.h"

#include "common/Application.h"
#include "common/ViewContext.h"

#include "common/TextureManager.h"
#include "common/TextureState.h"
#include "common/ShaderManager.h"
#include "common/Material.h"
#include "common/ShaderConstant.h"
#include "common/RenderContext.h"
#include "common/Renderer.h"
#include "common/Scene.h"
#include "common/SceneGraphNode.h"
#include "common/SceneGraphNodeIterator.h"
#include "common/GeometryNode.h"
#include "common/VertexFormat.h"

#include "mesh/MeshElements.h"
#include "mesh/Mesh.h"
#include "mesh/MeshBuilder.h"
#include "mesh/MeshNode.h"
#include "mesh/MeshSceneBuilder.h"
#include "mesh/MeshSceneGraphNode.h"
#include "mesh/Ray.h"

#include "scene/AftkMeshNode.h"
#include "scene/AftkSceneBuilder.h"
#include "scene/AftkSceneGraphNode.h"

#include "scene/MaterialBuilder.h"

#include "render/GridRenderer.h"
#include "render/FontRenderer.h"
#include "render/SceneRenderer.h"

#include "Utility.h"

namespace AftkRender
{
	extern Debug::Logger& g_Log;
}

#endif