#ifndef SORCERYDOCUMENT_H
#define SORCERYDOCUMENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Core/Link.h"

class SorceryApp;

enum SorceryDocumentKind
{
	kSdkNone,
	kSdkRecord,
	kSdkPack,
	kSdkDb
};

class SorceryDocument
{
public:
	Link<SorceryDocument> _link;
	LinkList<SorceryView> _views;

public:
	SorceryApp&			_app;
	SorceryDocumentKind	_kind;
	XtId				_id;
	Xt::DomMonitor		_dom;

public:
	SorceryDocument(SorceryApp& app);
	~SorceryDocument();

public:
	void SetModified();
	void OnDetailsUpdated();

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SorceryDocument* doc);
};

#endif // SORCERYDOCUMENT_H