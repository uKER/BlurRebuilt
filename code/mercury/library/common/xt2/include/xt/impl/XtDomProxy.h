#ifndef XT_IMPL_XTDOMPROXY_H
#define XT_IMPL_XTDOMPROXY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Xt
{
	typedef XtIntPtr ProxyHandle;

	enum ProxyType
	{
		kProxyField,
		kProxyRef,
		kProxyBlock,
		kProxyArray,
		kProxyComponent
	};

	//
	// Xt::ProxyMonitor
	//

	struct ProxyEvent
	{
	};

	class ProxyMonitor
	{
	public:
		typedef void (*Callback)(ProxyMonitor*, ProxyEvent*, void*);

	public:
		virtual ~ProxyMonitor() = 0;

	public:
		virtual ProxyHandle Resolve() = 0;

	public:
		template<typename Type>
		inline void SetCallback(void (*callback)(ProxyMonitor*, ProxyEvent*, Type*), Type* context)
		{
			_callback	= callback;
			_context	= context;
		}

	protected:
		inline void DoCallback(ProxyEvent* event)
		{
			if (_callback)
				_callback(this, event, _context);
		}

	private:
		Callback	_callback;
		void*		_context;
	};

	//
	// Xt::Proxy
	//

	class Proxy
	{
	public:
		virtual ~Proxy() = 0;

		//
		// Root
		//

		virtual ProxyHandle Root() = 0;

		//
		// Field
		//

		virtual XtString GetText(ProxyHandle hdl) = 0;
		virtual bool SetText(ProxyHandle hdl, const XtString& value) = 0;

		//
		// Ref
		//

		virtual XtId GetId(ProxyHandle hdl) = 0;
		virtual bool SetId(ProxyHandle hdl, XtId value) = 0;

		//
		// Block
		//

		virtual ProxyHandle At(ProxyHandle hdl, const XtString& key) = 0;
		virtual ProxyHandle Acquire(ProxyHandle hdl, const XtString& key, ProxyType type) = 0;
		virtual bool Remove(ProxyHandle hdl, const XtString& key) = 0;

		//
		// Array
		//

		virtual int Count(ProxyHandle hdl) = 0;
		virtual ProxyHandle At(ProxyHandle hdl, int index) = 0;
		virtual ProxyHandle Append(ProxyHandle hdl, ProxyType type) = 0;
		virtual ProxyHandle Insert(ProxyHandle hdl, int before, ProxyType type) = 0;
		virtual ProxyHandle Replace(ProxyHandle hdl, int index, ProxyType type) = 0;
		virtual bool Remove(ProxyHandle hdl, int index) = 0;

		//
		// All Types
		//

		virtual void Clear(ProxyHandle hdl) = 0;
		virtual ProxyMonitor* Monitor(ProxyHandle hdl) = 0;
	};

	//
	// Create default dom proxy with write access
	//

	Proxy* CreateDefaultProxy(class XtDomBlock* root);
}

#endif // XT_IMPL_XTDOMPROXY_H