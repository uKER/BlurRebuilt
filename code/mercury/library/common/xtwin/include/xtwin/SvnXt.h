#ifndef XTWIN_SVNXT_H
#define XTWIN_SVNXT_H

#ifdef _MSC_VER
#pragma once
#endif

struct SvnXtParams
{
	Xt::DomDb*			db;
	Sz8					path;
	Sz8					comment;
	XtUt::Array<Sz8>*	files;
	bool				noWindow;

public:
	SvnXtParams(Xt::DomDb* db_, const Sz8& path_, const Sz8& comment_ = "", bool noWindow = false) :
		db(db_),
		path(path_),
		comment(comment_),
		files(0),
		noWindow(false)
	{
	}
};

bool SvnXtOpen(HWND parentWindow, SvnXtParams* params);
bool SvnXtSave(HWND parentWindow, SvnXtParams* params);
bool SvnXtUpdate(HWND parentWindow, SvnXtParams* params);
bool SvnXtCommit(HWND parentWindow, SvnXtParams* params);
bool SvnXtRecover(HWND parentWindow, SvnXtParams* params);

#endif // XTWIN_SVNXT_H