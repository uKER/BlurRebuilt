#include "stdafx.h"
#include "Render/Render/GridRender.h"
#include "Display/Render/BasicRender.h"

namespace Render
{

Debug::Bool	g_debugGridShow("Graphics.Render.Grid.Show", true);
Debug::Int	g_debugGridSize("Graphics.Render.Grid.Size", 12, Debug::Range(0, 50));

void DrawGrid(Maths::RGBA colour, S32 gridSize)
{
	if (!g_debugGridShow)
		return;

	PERF_EVENT("RenderGrid");

	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	
	S32 size = gridSize < 0.0f ? g_debugGridSize : gridSize;

	for (S32 x = -size; x <= size; x++)
		Display::BasicRender()->DrawColoured3DLine((F32)x, 0.f, (F32)-size, (F32)x, 0.f, (F32)size, colour);

	for (S32 z = -size; z <= size; z++)
		Display::BasicRender()->DrawColoured3DLine((F32)-size, 0.f, (F32)z, (F32)size, 0.f, (F32)z, colour);

	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
}

}; // namespace Render
