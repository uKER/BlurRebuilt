#ifndef RENDER_VIEW_VIEW_H
#define RENDER_VIEW_VIEW_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Render/View/ViewFlags.h"

namespace Render
{

class C_View
{
public:

	C_View();
	virtual ~C_View();

public:

	Display::C_View			m_View;
	Display::C_ViewSetup	m_ViewSetup;
	U32						m_ViewFlags;
	S32						m_ViewIndex;

};

}; // namespace Render

#endif // RENDER_VIEW_VIEW_H
