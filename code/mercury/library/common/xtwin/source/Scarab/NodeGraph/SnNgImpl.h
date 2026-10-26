#ifndef SNNGIMPL_H
#define SNNGIMPL_H

#ifdef _MSC_VER
#pragma once
#endif

class OnlyOnce
{
public:
	OnlyOnce(int& v) :
		_captured(false),
		_v(v)
	{
		if (_v == 0)
		{
			_captured = true;
			_v++;
		}
	}

	~OnlyOnce()
	{
		if (_captured)
			_v--;
	}

public:
	operator bool() const
	{
		return _captured;
	}

private:
	bool _captured;
	int& _v;
};

class SnNgFormatter : public EstFormatter
{
	const char* _paramName;

public:
	SnNgFormatter(const char* paramName);

public:
	virtual bool GetFormatByType(LtStruct* type, XtString& format);
	virtual bool GetFormatByOption(LarvaOption* option, XtString& format);
};

class SnNgConnection : public NgConnection
{
public:
	SnNgConnection(ScarabDomNodeGraph* ng);
	virtual ~SnNgConnection();

public:
	virtual bool IsConflicted();
	virtual bool IsHot();

private:
	ScarabDomNodeGraph* _ng;
};

class SnNgPin : public NgPin
{
public:
	SnNgPin(ScarabDomNodeGraph* ng, LfWeakPtr* weak, Xt::DomRef* ref, LarvaField* field, LtStruct* parentType, Xt::DomBlock* parentDom);
	virtual ~SnNgPin();

public:
	XtString GetPinLabelText(const XtString& format);

public:
	virtual NgPin* OnDrag();
	virtual bool OnTryConnect(NgPin* to);
	virtual void OnConnect(NgPin* to);

public:
	void CreateConnections();

public:
	Xt::DomRef* Field();

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnNgPin* pin);

private:
	ScarabDomNodeGraph*	_ng;
	LfWeakPtr*		_weak;
	Xt::DomMonitor	_dom;
	Xt::DomMonitor	_parentDom;
	int				_ignoreNotifications;
	LarvaField*		_field;
	LtStruct*		_parentType;

friend class SnNgNode;
friend class ScarabDomNodeGraph;
friend class ScarabDomNodeGraphContext;
};

class SnNgNode : public NgNode
{
public:
	SnNgNode(ScarabDomNodeGraph* ng, LarvaField* field, LtStruct* type, Xt::DomComponent* component, Xt::Indexer* indexer, Xt::DomElement* parentDom);
	virtual ~SnNgNode();

public:
	virtual void OnPositionChanged();
	virtual void OnDoubleClicked();

private:
	bool DoDblClkAction(LtStruct* type, Xt::DomComponent* com);

	void LoadPosition();
	void LoadName();
	void LoadType();
	void SavePosition();

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnNgNode* node);

private:
	XtUt::Link<SnNgNode> _link;

private:
	ScarabDomNodeGraph*	_ng;
	LarvaField*		_field;
	LtStruct*		_type;
	Xt::DomMonitor	_dom;
	int				_ignoreNotifications;
	XtId			_id;
	NgPin*			_out;
	Xt::Indexer*	_indexer;
	Xt::DomMonitor	_parentDom;

friend class SnNgPin;
friend class ScarabDomNodeGraph;
friend class ScarabDomNodeGraphContext;
};

#endif // SNNGIMPL_H