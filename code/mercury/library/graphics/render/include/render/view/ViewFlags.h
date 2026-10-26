#ifndef RENDER_VIEW_FLAGS_H
#define RENDER_VIEW_FLAGS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{

enum E_ViewFlags
{
	E_ViewFlag_3d = 1 << 0,
	E_ViewFlag_EnablePostProcess = 1 << 1,
	E_ViewFlag_EnableDeferred = 1 << 2,
};

}; // namespace Render

#endif // RENDER_VIEW_FLAGS_H
