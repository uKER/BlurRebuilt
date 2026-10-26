using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Keith.Libraries.ThreadManager
{
	public class Job
	{
		internal ManualResetEvent m_jobStartedEvent;

		/// <summary>
		/// Creates a job ready to be passed to the ThreadManager
		/// </summary>
		/// <param name="jobCallback">This is the callback that represents the job.</param>
		/// <param name="inputArgs">This represents the input arguments to the job.</param>
		/// <param name="jobCompletionCallback">This is called when the job completes and is
		/// passed the return value of the job. Can be null.</param>
		/// <param name="state">This represents a state object that is passed to the completion
		/// callback and can be used for synchronisation.</param>
		//public Job(JobCallback jobCallback, object inputArgs, JobCompletionCallback jobCompletionCallback, object state)
		public Job(JobCallback jobCallback, object inputArgs, JobCompletionCallback jobCompletionCallback, object state)
		{
			m_jobCallback = jobCallback;
			m_inputArgs = inputArgs;
			m_jobCompletionCallback = jobCompletionCallback;
			m_state = state;

			m_jobStartedEvent = new ManualResetEvent(false);
		}

		/// <summary>
		/// Represents a callback method to be executed by a ThreadManager thread. This is the 'job' to be done.
		/// </summary>
		/// <param name="inputArgs">An object containing information to be used by the callback method.</param>
		public delegate object JobCallback(object inputArgs);

		/// <summary>
		/// Represents a callback method to be executed when a ThreadManager thread completes.
		/// </summary>
		/// <param name="jobResults">This is the value that was returned by the job.</param>
		/// <param name="state">This represents a state object that was passed to the job when it was first created.
		/// It can be used for synchronisation.</param>
		public delegate void JobCompletionCallback(object jobResults, object state);

		/// <summary>
		/// This is the function to be called on the ThreadManager thread to perform the job.
		/// </summary>
		private JobCallback m_jobCallback;
		//private event JobCallback m_jobCallback;
		//private Delegate m_jobCallback;


		/// <summary>
		/// This is the function to be called on the ThreadManager thread after the job has completed.
		/// </summary>
		private JobCompletionCallback m_jobCompletionCallback;

		/// <summary>
		/// Represents a state object passed to the job when it is created.
		/// This can for example be used by the completion callback to identify the originating job.
		/// </summary>
		private object m_state;

		/// <summary>
		/// Represents the arguments used by the ThreadManager job.
		/// </summary>
		protected object m_inputArgs;

		internal object DoWork()
		{
			if(m_jobCallback != null)
			{
				m_jobStartedEvent.Set();
				return m_jobCallback.DynamicInvoke(m_inputArgs);
			}
			return null;
		}

		internal void DoCompletionCallback(object returnValue)
		{
			if(m_jobCompletionCallback != null)
			{
				m_jobCompletionCallback(returnValue, m_state);
			}
		}
	}
}

