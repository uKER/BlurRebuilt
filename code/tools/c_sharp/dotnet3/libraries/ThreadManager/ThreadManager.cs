using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics;

namespace Keith.Libraries.ThreadManager
{
	public class ThreadManager
	{
		/// <summary>
		/// The number of worker threads this instance contains.
		/// </summary>
		int m_threadCount;

		/// <summary>
		/// This is the queue of available worker threads.
		/// </summary>
		Queue<GenericThread> m_threadAvailableList;
	
		/// <summary>
		/// This is the queue of busy worker threads.
		/// </summary>
		List<GenericThread> m_threadBusyList;

		/// <summary>
		/// This is the queue of pending jobs.
		/// </summary>
		Queue<Job> m_jobQueue;

		/// <summary>
		/// If true, the job queue is suspended and no new jobs are sent to the worker threads.
		/// </summary>
		bool m_bSuspendQueue = false;

		/// <summary>
		/// Creates a thread manager and initialises it with GenericThreads.
		/// </summary>
		/// <param name="threadCount">The number of worker threads this instance contains.</param>
		public ThreadManager(int threadCount)
		{
			if(threadCount < 1)
			{
				throw new ArgumentOutOfRangeException("threadCount", "threadCount must be greater than 0.");
			}
			else
			{
				//threadCount = 10; // TEST.
				m_threadCount = threadCount;

				m_threadAvailableList = new Queue<GenericThread>();
				m_threadBusyList = new List<GenericThread>();
				m_jobQueue = new Queue<Job>();

				for(int c = 0; c < m_threadCount; c++)
				{
					GenericThread genericThread = new GenericThread(JobFinished);
					
					m_threadAvailableList.Enqueue(genericThread);
				}
			}
		}

		#region Public methods.

		/// <summary>
		/// Adds a new job to the job queue and 
		/// </summary>
		/// <param name="job"></param>
		/// <param name="bWaitUntilJobStarts"></param>
		public void QueueJob(Job job, bool bWaitUntilJobStarts)
		{
			lock(m_jobQueue)
			{
				m_jobQueue.Enqueue(job);
				Debug.WriteLine("QUEUE SIZE: " + m_jobQueue.Count);
			}
			PokeTheQueue();

			if(bWaitUntilJobStarts == true)
			{
				job.m_jobStartedEvent.WaitOne();
			}
		}

		/// <summary>
		/// Suspends the job queue so that no new jobs are sent to the worker threads.
		/// Jobs still queue up in the job queue.
		/// </summary>
		public void SuspendQueue()
		{
			m_bSuspendQueue = true;
		}

		/// <summary>
		/// Resumes processing of jobs in the job queue to the worker threads.
		/// </summary>
		public void ResumeQueue()
		{
			m_bSuspendQueue = false;
			PokeTheQueue();
		}

		/// <summary>
		/// Clears any pending jobs in the job queue.
		/// </summary>
		/// <returns></returns>
		public int ClearQueue()
		{
			int numClearedJobs;

			lock(m_jobQueue)
			{
				numClearedJobs = m_jobQueue.Count;
				m_jobQueue.Clear();
			}
			return numClearedJobs;
		}

		/// <summary>
		/// Doesn't work properly. Don't use or you risk deadlock.
		/// </summary>
		/// <returns></returns>
		public int ClearThreads()
		{
			int numClearedThreads;

			List<GenericThread> threadBusyListCopy = new List<GenericThread>(m_threadBusyList.Count);

			lock(m_threadBusyList)
			{
				numClearedThreads = m_threadBusyList.Count;
				foreach(GenericThread gt in m_threadBusyList)
				{
					threadBusyListCopy.Add(gt);
				}
			}

			foreach(GenericThread genericThread in threadBusyListCopy)
			{
				genericThread.Reset();
			}

			return numClearedThreads;
		}

		#endregion Public methods.

		/// <summary>
		/// This is called by the worker thread when it finishes it's current job.
		/// It adds the worker thread back into the queue of available threads and
		/// if there are any outstanding jobs it makes sure that one is started.
		/// </summary>
		/// <param name="sender">This should always be a reference to the GenericThread that has finished.</param>
		/// <param name="e">Nothing interesting in here.</param>
		void JobFinished(object sender, EventArgs e)
		{
			Debug.Assert(sender is GenericThread);
			// Add the thread back into the 'free' list.
			lock(m_threadAvailableList)
			{
				m_threadAvailableList.Enqueue((GenericThread)sender);
			}
			lock(m_threadBusyList)
			{
				m_threadBusyList.Remove((GenericThread)sender);
			}

			PokeTheQueue();
		}

		/// <summary>
		/// If the job queue is not empty and there is an available worker thread
		/// then the next job is sent to the worker thread.
		/// </summary>
		void PokeTheQueue()
		{
			if(m_bSuspendQueue == false)
			{
				lock(m_jobQueue)
				{
					if(m_jobQueue.Count > 0)
					{
						lock(m_threadAvailableList)
						{
							// Use a 'while' rather than an 'if' because although this function is called each time a job is queued
							// and each time a worker thread becomes available and there therefore the worker threads should always
							// be saturated, if the queue is suspended then jobs aren't picked out of the queue when threads become available.
							while(m_threadAvailableList.Count > 0 && m_jobQueue.Count > 0)
							{
								GenericThread genericThread = m_threadAvailableList.Dequeue();
								lock(m_threadBusyList)
								{
									m_threadBusyList.Add(genericThread);
								}
								Job nextJob = m_jobQueue.Dequeue();
								Debug.WriteLine("Queue size: " + m_jobQueue.Count);

								genericThread.StartJob(nextJob);
							}
						}
					}
				}
			}
		}
	}
}
