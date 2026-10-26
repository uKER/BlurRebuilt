#ifndef NODEGRAPH_NGIMPL_H
#define NODEGRAPH_NGIMPL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/Core.h"
#include "XtWin/NodeGraph.h"

struct NgRenderContext
{
	Gdiplus::Graphics* g;

	Gdiplus::Font nodeFont;
	Gdiplus::Font pinFont;
	Gdiplus::Font waterFont;

	float cornerSize;
	float topMargin;
	float horzMargin;
	float columnGap;

	float pinSize;
	float pinLineHeight;

	float arrowWidth;
	float arrowHeight;

public:
	NgRenderContext(Gdiplus::Graphics* g_);

public:
	NgVector Measure(const Sz16& text, Gdiplus::Font* font);
};

#endif // NODEGRAPH_NGIMPL_H