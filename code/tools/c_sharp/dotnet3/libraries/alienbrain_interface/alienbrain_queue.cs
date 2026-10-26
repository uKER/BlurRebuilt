using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Collections;
using System.Diagnostics;

namespace Alienbrain
{
	class AlienbrainQueue
	{
		static AlienbrainQueue m_me = null;

		Thread m_queueThread;
		EventWaitHandle m_queueThread_Wait;

		List<AlienbrainRequest> m_requestList;

		private AlienbrainQueue()
		{
			m_queueThread_Wait = new EventWaitHandle(false, EventResetMode.AutoReset);
			m_requestList = new List<AlienbrainRequest>();
			m_queueThread = new Thread(new ThreadStart(QueueThread));

			m_queueThread.Name = "Alienbrain queue.";

			m_queueThread.Priority = ThreadPriority.BelowNormal;
			m_queueThread.IsBackground = true;
			m_queueThread.Start();
		}

		internal static AlienbrainQueue CreateQueue()
		{
			if(m_me == null)
			{
				m_me = new AlienbrainQueue();
			}
			return m_me;
		}


		void QueueThread()
		{
			while(true)
			{
				m_queueThread_Wait.WaitOne();
				while(m_requestList.Count != 0)
				{
					AlienbrainRequest abRequest;

					lock(m_requestList)
					{
						if(m_requestList.Count != 0)
						{
							abRequest = m_requestList[0];
							m_requestList.RemoveAt(0);
						}
						else
							abRequest = null;
					}

					if(abRequest != null)
						abRequest.DoRequest();
						//DoRequest(abRequest);
				}
			}
		}


		public bool AddAbCommand(AlienbrainRequest abRequest)
		{
			lock(m_requestList)
			{
				if(abRequest.m_priority == AlienbrainRequest.Priority.Normal)
					m_requestList.Add(abRequest);
				else
					m_requestList.Insert(0, abRequest);
			}
			m_queueThread_Wait.Set();				// Wake the thread up (if it is asleep) so it can process this queued item.

			return true;
		}

		public void ClearQueue()
		{
			Debug.WriteLine("About to clear queue.");
			lock(m_requestList)
			{
				m_requestList.Clear();
			}
		}




		internal void KillThread()
		{
			this.m_queueThread.Abort();
		}
	}
}

