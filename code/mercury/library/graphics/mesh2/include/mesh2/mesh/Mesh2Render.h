#ifndef RENDER_MESH2_RENDER_H
#define RENDER_MESH2_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Mesh2/Mesh/Mesh2.h"

namespace Render
{

void RenderMesh2(C_Mesh2* pMesh, Display2::C_ContextNoCheckNoTrack* pContext);

} // namespace Render

#endif // RENDER_MESH2_RENDER_H
