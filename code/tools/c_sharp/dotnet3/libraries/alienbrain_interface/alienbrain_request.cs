using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
//using SuperControls;
using System.Threading;

namespace Alienbrain
{
	public class AlienbrainRequest
	{

		public enum Priority
		{
			Normal = 0,
			High
		}

		//public enum Command
		//{
		//    Initialise = 0,
		//    DeInitialise,
		//    LoadProject,
		//    GetLatestFile,
		//    Test,
		//    GetNodeDetails,
		//    GetFirstChild,
		//    GetNextChild,
		//    Import,
		//    Checkout,
		//    UndoCheckout,
		//    Checkin
		//}
		//Command m_command;


		internal Priority m_priority;
		private object m_userData;
		private EventWaitHandle m_waitHandle;
		private Delegate m_myDelegate;
		private object[] m_myDelegateArgs;
		private Delegate m_myCompleteCallback;

		internal object m_result = null;

		internal AlienbrainRequest(Priority priority, object userData, Delegate myDelegate, object[]myDelegateArgs, Delegate myCompleteCallback)
		{
			m_priority = priority;
			m_userData = userData;
			m_waitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
			m_myDelegate = myDelegate;
			m_myCompleteCallback = myCompleteCallback;
			m_myDelegateArgs = myDelegateArgs;
		}
		delegate bool nob();

		internal void DoRequest()
		{
			//object result = null;
			

			if(m_myDelegate != null)
				m_result = this.m_myDelegate.DynamicInvoke(this.m_myDelegateArgs);

			if(m_myCompleteCallback != null)
				m_myCompleteCallback.DynamicInvoke(m_userData, m_result);

			m_waitHandle.Set();
		}

		internal void WaitForResult()
		{
			m_waitHandle.WaitOne();
		}
	}
}
