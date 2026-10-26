#ifndef RENDER_MESH2_UTILS_H
#define RENDER_MESH2_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Mesh2/Mesh/Mesh2.h"
#include <Display2/Device/Device.h>

namespace Render
{

bool LoadMesh2(C_Mesh2* pMesh, const Util::C_String& filePath, const Util::C_String& fileRoot, Display2::C_Device* pDevice);

void DestroyMesh2(C_Mesh2* pMesh, Display2::C_Device* pDevice);

Util::C_String GetMesh2FilePath(const Util::C_String& fileName);

} // namespace Render

#endif // RENDER_MESH2_UTILS_H
