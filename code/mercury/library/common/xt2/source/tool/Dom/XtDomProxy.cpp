#include "Common/XtPch.h"
#include <Xt/XtTool.h>
#include <Xt/Impl/XtDomProxy.h>

class XtDefaultProxyMonitor : public Xt::ProxyMonitor
{
public:
	XtDefaultProxyMonitor(Xt::DomElement* el) :
		_monitor(ProxyCallback, this)
	{
		Xt::Monitor(&_monitor, el);
	}

	virtual ~XtDefaultProxyMonitor()
	{
	}

public:
	virtual Xt::ProxyHandle Resolve()
	{
		return (Xt::ProxyHandle)(Xt::DomElement*)_monitor;
	}

private:
	static void ProxyCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, XtDefaultProxyMonitor* context)
	{
		context->DoCallback(&Xt::ProxyEvent());
	}

private:
	Xt::DomMonitor _monitor;
};

class XtDefaultProxy : public Xt::Proxy
{
public:
	XtDefaultProxy(Xt::DomBlock* root)
	{
		Xt::Monitor(&_root, root);
	}

	virtual ~XtDefaultProxy()
	{
	}

public:
	virtual Xt::ProxyHandle Root()
	{
		return (Xt::ProxyHandle)(Xt::DomElement*)_root;
	}

public:
	virtual XtString GetText(Xt::ProxyHandle hdl)
	{
		return Xt::AsText((Xt::DomElement*)hdl);
	}

	virtual bool SetText(Xt::ProxyHandle hdl, const XtString& value)
	{
		if (Xt::DomField* field = AsField((Xt::DomElement*)hdl))
			return Xt::Set(field, value);

		return false;
	}

public:
	virtual XtId GetId(Xt::ProxyHandle hdl)
	{
		return Xt::AsId((Xt::DomElement*)hdl);
	}

	virtual bool SetId(Xt::ProxyHandle hdl, XtId value)
	{
		if (Xt::DomRef* ref = AsRef((Xt::DomElement*)hdl))
			return Xt::Set(ref, value);

		return false;
	}

public:
	virtual Xt::ProxyHandle At(Xt::ProxyHandle hdl, const XtString& key)
	{
		if (Xt::DomBlock* block = AsBlock((Xt::DomElement*)hdl))
			return (Xt::ProxyHandle)Xt::At(block, key);

		return 0;
	}

	virtual Xt::ProxyHandle Acquire(Xt::ProxyHandle hdl, const XtString& key, Xt::ProxyType type)
	{
		if (Xt::DomBlock* block = AsBlock((Xt::DomElement*)hdl))
		{
			switch(type)
			{
				case Xt::kProxyField:		return (Xt::ProxyHandle)Xt::AcquireField(block, key);
				case Xt::kProxyRef:			return (Xt::ProxyHandle)Xt::AcquireRef(block, key);
				case Xt::kProxyBlock:		return (Xt::ProxyHandle)Xt::AcquireBlock(block, key);
				case Xt::kProxyArray:		return (Xt::ProxyHandle)Xt::AcquireArray(block, key);
				case Xt::kProxyComponent:	return (Xt::ProxyHandle)Xt::AcquireComponent(block, key);
			}
		}

		return 0;
	}

	virtual bool Remove(Xt::ProxyHandle hdl, const XtString& key)
	{
		if (Xt::DomBlock* block = AsBlock((Xt::DomElement*)hdl))
			return Xt::Remove(block, key);

		return false;
	}

public:
	virtual int Count(Xt::ProxyHandle hdl)
	{
		if (Xt::DomArray* array = AsArray((Xt::DomElement*)hdl))
			return Xt::Count(array);

		return 0;
	}

	virtual Xt::ProxyHandle At(Xt::ProxyHandle hdl, int index)
	{
		if (Xt::DomArray* array = AsArray((Xt::DomElement*)hdl))
			return (Xt::ProxyHandle)Xt::At(array, index);

		return 0;
	}

	virtual Xt::ProxyHandle Append(Xt::ProxyHandle hdl, Xt::ProxyType type)
	{
		if (Xt::DomArray* array = AsArray((Xt::DomElement*)hdl))
		{
			switch(type)
			{
				case Xt::kProxyField:		return (Xt::ProxyHandle)Xt::Append(array, new Xt::DomField());
				case Xt::kProxyRef:			return (Xt::ProxyHandle)Xt::Append(array, new Xt::DomRef());
				case Xt::kProxyBlock:		return (Xt::ProxyHandle)Xt::Append(array, new Xt::DomBlock());
				case Xt::kProxyArray:		return (Xt::ProxyHandle)Xt::Append(array, new Xt::DomArray());
				case Xt::kProxyComponent:	return (Xt::ProxyHandle)Xt::Append(array, new Xt::DomComponent());
			}
		}

		return 0;
	}

	virtual Xt::ProxyHandle Insert(Xt::ProxyHandle hdl, int before, Xt::ProxyType type)
	{
		if (Xt::DomArray* array = AsArray((Xt::DomElement*)hdl))
		{
			switch(type)
			{
				case Xt::kProxyField:		return (Xt::ProxyHandle)Xt::Insert(array, before, new Xt::DomField());
				case Xt::kProxyRef:			return (Xt::ProxyHandle)Xt::Insert(array, before, new Xt::DomRef());
				case Xt::kProxyBlock:		return (Xt::ProxyHandle)Xt::Insert(array, before, new Xt::DomBlock());
				case Xt::kProxyArray:		return (Xt::ProxyHandle)Xt::Insert(array, before, new Xt::DomArray());
				case Xt::kProxyComponent:	return (Xt::ProxyHandle)Xt::Insert(array, before, new Xt::DomComponent());
			}
		}

		return 0;
	}

	virtual Xt::ProxyHandle Replace(Xt::ProxyHandle hdl, int index, Xt::ProxyType type)
	{
		if (Xt::DomArray* array = AsArray((Xt::DomElement*)hdl))
		{
			switch(type)
			{
				case Xt::kProxyField:		return (Xt::ProxyHandle)Xt::Replace(array, index, new Xt::DomField());
				case Xt::kProxyRef:			return (Xt::ProxyHandle)Xt::Replace(array, index, new Xt::DomRef());
				case Xt::kProxyBlock:		return (Xt::ProxyHandle)Xt::Replace(array, index, new Xt::DomBlock());
				case Xt::kProxyArray:		return (Xt::ProxyHandle)Xt::Replace(array, index, new Xt::DomArray());
				case Xt::kProxyComponent:	return (Xt::ProxyHandle)Xt::Replace(array, index, new Xt::DomComponent());
			}
		}

		return 0;
	}

	virtual bool Remove(Xt::ProxyHandle hdl, int index)
	{
		if (Xt::DomArray* array = AsArray((Xt::DomElement*)hdl))
			return Xt::Remove(array, index);

		return false;
	}

public:
	virtual void Clear(Xt::ProxyHandle hdl)
	{
		if (Xt::DomField* field = AsField((Xt::DomElement*)hdl))
			Xt::Set(field, kXtNullString);
		else if (Xt::DomRef* ref = AsRef((Xt::DomElement*)hdl))
			Xt::Set(ref, 0);
		else if (Xt::DomBlock* block = AsBlock((Xt::DomElement*)hdl))
			Xt::Clear(block);
		else if (Xt::DomArray* array = AsArray((Xt::DomElement*)hdl))
			Xt::Clear(array);
		else if (Xt::DomComponent* component = AsComponent((Xt::DomElement*)hdl))
			Xt::Clear(&component->data);
	}

	virtual Xt::ProxyMonitor* Monitor(Xt::ProxyHandle hdl)
	{
		if (Xt::DomElement* el = (Xt::DomElement*)hdl)
			return new XtDefaultProxyMonitor(el);

		return 0;
	}

private:
	Xt::DomMonitor _root;
};

Xt::Proxy* CreateDefaultProxy(Xt::DomBlock* root)
{
	return new XtDefaultProxy(root);
}