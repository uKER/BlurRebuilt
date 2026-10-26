#ifndef RENDER_MESHSAVER_H
#define RENDER_MESHSAVER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Mesh
{

class C_MeshBuilder;

void SaveMesh(const Util::C_String fileName, const C_MeshBuilder& meshBuilder);

}; // namespace Mesh
}; // namespace Render

#endif // RENDER_MESHSAVER_H
