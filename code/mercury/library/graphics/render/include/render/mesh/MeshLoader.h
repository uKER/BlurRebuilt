#ifndef RENDER_MESHLOADER_H
#define RENDER_MESHLOADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/Mesh/MeshShaders.h"

namespace Render
{
namespace Mesh
{

class C_Mesh;

bool LoadMesh(const Util::C_String fileName, C_Mesh* pMesh, const E_VertexShader vertexShader = E_VertexShader_Default, const E_PixelShader pixelShader = E_PixelShader_Default);

}; // namespace Mesh
}; // namespace Render

#endif // RENDER_MESHLOADER_H
