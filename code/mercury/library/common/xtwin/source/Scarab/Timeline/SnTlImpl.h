#ifndef SNTLIMPL_H
#define SNTLIMPL_H

class SnTlKey : public TlKey
{
public:
	SnTlKey(ScarabTimeline* sn, LarvaField* lf, Xt::DomComponent* dom);

public:
	virtual void OnTimeChanged();

public:
	void LoadTime();

public:
	XtId Id();

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnTlKey* key);

private:
	ScarabTimeline*		_sn;
	LarvaField*			_lf;
	Xt::DomMonitor		_dom;

friend class ScarabTimeline;
};

class SnTlRow : public TlRow
{
public:
	SnTlRow(ScarabTimeline* sn, LarvaField* lf, Xt::DomElement* dom, XtId comId, XtId comIndex);

public:
	void LoadName();

public:
	SnTlKey* FindKey(XtId id);

public:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnTlRow* row);

private:
	XtUt::Link<SnTlRow> _link;

private:
	ScarabTimeline*	_sn;
	LarvaField*		_lf;
	Xt::DomMonitor	_dom;
	XtId			_comId;
	XtU32			_comIndex;

friend class ScarabTimeline;
};

void DoSnTlBuild(ScarabTimeline* sn, LarvaSchema* schema, TlDiagram* diagram, LtStruct* type, Xt::DomBlock* dom);

#endif // SNTLIMPL_H
