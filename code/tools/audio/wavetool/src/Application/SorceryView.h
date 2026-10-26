#ifndef SORCERYVIEW_H
#define SORCERYVIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Core/Link.h"
#include "DynamicForm/DynamicForm.h"
#include "Application/SorceryDocument.h"
#include "Application/SorceryApp.h"

enum SorceryViewType
{
	kSvtScarab,
	kSvtLoc,
	kSvtGrid,
	kSvtHyper,
	kSvtWaveBank
};

class SorceryView
{
public:
	SorceryView(SorceryApp& app, SorceryDocument* document, SorceryViewType type);

private:
	~SorceryView();

public:
	void Create();
	bool Close();
	void Destroy(bool destroyDocument = true);

public:
	void OnDetailsUpdated();

public:
	Link<SorceryView> _linkWindow;
	Link<SorceryView> _linkDocument;

public:
	SorceryApp&				_app;
	SorceryDocument*		_document;
	SorceryViewType			_type;
	MdiWindow*				_window;
};

#endif // SORCERYVIEW_H
