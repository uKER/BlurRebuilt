#ifndef EDITOR_RENDER_HELPERS_H
#define EDITOR_RENDER_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	namespace RenderHelpers
	{
		Maths::RGBA GetHighlightColour(
			const EditorLib::I_DgNode* pDgNode,
			Maths::RGBA defaultCol=Maths::RGBA(0,0,64,255));
	}
}

#endif
