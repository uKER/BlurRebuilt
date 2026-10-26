#ifndef SORCERYAPP_H
#define SORCERYAPP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtTool.h>

#include "XtWin/Core.h"
#include "Core/List.h"
#include "Application/SorceryWindow.h"
#include "Application/SorceryDocument.h"
#include "XtWin/AsyncOpWin.h"
#include "XtWin/CommitWin.h"
#include "XtWin/ConflictWin.h"
#include "Network/Network.h"

class SorceryView;

//
// SorceryApp
//

class SorceryApp
{
public:
	SorceryApp();
	~SorceryApp();

public:
	bool Initialise();
	void Destroy();

public:
	bool SaveProject();

public:
	void SvnUpdate();
	void SvnCommit();
	bool SvnRecover();

public:
	void ReloadSchema();

public:
	XtString					_dbPath;
	SorceryWindow				_window;
	LinkList<SorceryDocument>	_documents;
	Xt::DomDb*					_db;
	XtVarNet					_xtNet;
	XtUt::Array<XtString>		_skipPlatforms;
};

void StartNetwork(SorceryApp* app);
void StopNetwork();

#endif // SORCERYAPP_H