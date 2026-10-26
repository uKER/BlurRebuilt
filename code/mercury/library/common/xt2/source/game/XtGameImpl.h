#ifndef XT_GAME_XTGAMEIMPL_H
#define XT_GAME_XTGAMEIMPL_H

#include <Xt/Impl/XtCore.h>

#ifdef XT_TOOL
#include <Xt/XtTool.h>
#else
#include <Xt/XtGame.h>
#endif

const int kXtMaxTypes = 2048;

struct XtGlobals
{
	int					references;
	int					typeCount;
	Xt::AnnotationType*	types[kXtMaxTypes];
	XtSchema*			application;
};

extern XtGlobals gXt2;

void DebugLn(const char* text, ...);

XtSchema* XtGenerateApplicationSchema();

#endif // XT_GAME_XTGAMEIMPL_H
