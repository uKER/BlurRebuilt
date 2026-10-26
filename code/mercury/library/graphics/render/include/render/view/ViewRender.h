#ifndef RENDER_VIEW_RENDER_H
#define RENDER_VIEW_RENDER_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{

void RenderViewFrustum(const Display::C_View* pView, const U32 iColour = 0xFFFF00FF);
void RenderViewPosition(const Display::C_View* pView, const U32 iColour = 0xFFFF00FF);

}; // namespace Render

#endif // RENDER_VIEW_RENDER_H
