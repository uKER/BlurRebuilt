#ifndef RENDER_GRIDRENDER_H
#define RENDER_GRIDRENDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{

void DrawGrid(Maths::RGBA colour = Maths::RGBA(50, 50, 50, 50), S32 gridSize = -1);

}; // namespace Render

#endif // RENDER_GRIDRENDER_H
