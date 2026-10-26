using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics;
using System.Runtime.Remoting.Messaging;

namespace Keith.Libraries.ThreadManager
{
	// TODO: Add a 'completed' event and a suitable 

	class GenericThread
	{
		Thread m_thread;
		bool m_bAbortThread = false;

		AutoResetEvent m_threadNewCommandSet = new AutoResetEvent(false);

		public GenericThread(EventHandler threadFinishedEventHandler)
		{
			ThreadFinished += threadFinishedEventHandler;

			m_thread = new Thread(ThreadProc);
			m_thread.IsBackground = true;
			m_thread.Priority = ThreadPriority.BelowNormal;
			m_thread.Start();
		}

		#region Events.

		public event EventHandler ThreadFinished;

		protected virtual void OnThreadFinished(EventArgs e)
		{
			if(ThreadFinished != null)
			{
				ThreadFinished(this, e);
			}
		}
		#endregion Events.

		internal void Reset()
		{
			if(m_threadNewCommandSet.WaitOne(0, false) == false)		// If the thread is busy ...
			{
				m_thread.Abort();	// Not really Abort. Hopefully the thread will catch the raised exception and just abort the current callback.
			}
		}

		internal void Abort()
		{
			m_bAbortThread = true;
			m_thread.Abort();
		}

		Job m_currentJob;

		internal bool StartJob(Job job)
		{
			//TODO: Reset the thread first?
			// TODO: Make sure the thread is not already busy?
			//Debug.Assert(m_threadNewCommandSet.WaitOne(0, false) == false);

			m_currentJob = job;
			m_threadNewCommandSet.Set();
			return true;
		}

		void ThreadProc()
		{
			while(m_bAbortThread == false)
			{
				try
				{
					m_threadNewCommandSet.WaitOne();				// Wait until a job has been assigned to this thread.
					object returnValue = m_currentJob.DoWork();		// This is the user-defined function the thread is to run.

					m_currentJob.DoCompletionCallback(returnValue);

				}
				catch(ThreadAbortException)
				{
					Thread.ResetAbort();
					Debug.WriteLine("Thread function Aborted.");
				}
				finally
				{
					OnThreadFinished(new EventArgs());
				}
			}
			Debug.WriteLine("You're fucked.");
		}
	}
}
