#ifndef EDITOR_HUD_RENDERER_H
#define EDITOR_HUD_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_RenderViewport;

	namespace HUDElements
	{
		void RenderOrientationAxes(const C_RenderViewport *pViewport, const wxPoint &pos, U32 size);

		void ApplyView(const C_RenderViewport *pViewport, const Maths::C_Rect region);
		void RevertView(const C_RenderViewport *pViewport);
	};
}

#endif