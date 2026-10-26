using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Security.AccessControl;

namespace SuperControls
{
	public class SuperEventWaitHandle<T> : EventWaitHandle
	{
		private T m_tag;

		public T Tag
		{
			get { return m_tag; }
			set { m_tag = value; }
		}

		public SuperEventWaitHandle(bool initialState, EventResetMode mode)
			: base(initialState, mode)
		{
			this.Tag = default(T);
		}

		public SuperEventWaitHandle(bool initialState, EventResetMode mode, string name)
			: base(initialState, mode, name)
		{
			this.Tag = default(T);
		}

		public SuperEventWaitHandle(bool initialState, EventResetMode mode, string name, out bool createdNew)
			: base(initialState, mode, name, out createdNew)
		{
			this.Tag = default(T);
		}

		public SuperEventWaitHandle(bool initialState, EventResetMode mode, string name, out bool createdNew, EventWaitHandleSecurity eventSecurity)
			: base(initialState, mode, name, out createdNew, eventSecurity)
		{
			this.Tag = default(T);
		}
	}
}
