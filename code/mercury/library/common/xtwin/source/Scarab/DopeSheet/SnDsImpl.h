#ifndef SNDSIMPL_H
#define SNDSIMPL_H

class SnDsKey : public DsKey
{
public:
	SnDsKey(ScarabDopeSheet* sn, LarvaField* lf, Xt::DomComponent* dom);

public:
	virtual void OnTimeChanged();

public:
	void LoadTime();
	void LoadDuration();

public:
	XtId Id();

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnDsKey* key);

private:
	ScarabDopeSheet*		_sn;
	LarvaField*			_lf;
	Xt::DomMonitor		_dom;

friend class ScarabDopeSheet;
};

class SnDsRow : public DsRow
{
public:
	SnDsRow(ScarabDopeSheet* sn, LarvaField* lf, Xt::DomElement* dom, XtId comId, XtId comIndex);

public:
	void LoadName();

public:
	SnDsKey* FindKey(XtId id);

public:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnDsRow* row);

private:
	XtUt::Link<SnDsRow> _link;

private:
	ScarabDopeSheet*	_sn;
	LarvaField*		_lf;
	Xt::DomMonitor	_dom;
	XtId			_comId;
	XtU32			_comIndex;

friend class ScarabDopeSheet;
};

void DoSnDSBuild(ScarabDopeSheet* sn, LarvaSchema* schema, DsRow* parent, LtStruct* type, Xt::DomBlock* dom);

#endif // SNDSIMPL_H
