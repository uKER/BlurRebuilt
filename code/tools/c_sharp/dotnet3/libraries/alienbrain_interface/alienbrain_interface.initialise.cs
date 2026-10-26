using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dInitialiseComplete(object userData, object result);

		public bool Initialise(AlienbrainRequest.Priority priority)
		{
			AlienbrainRequest ar = BeginInitialise(priority, null, new dInitialiseComplete(InitialiseComplete));
			ar.WaitForResult();
			return (bool)ar.m_result;
		}


		public AlienbrainRequest BeginInitialise(AlienbrainRequest.Priority priority, object userData, dInitialiseComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dInitialise(DoInitialise), null, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void InitialiseComplete(object userData, object result)
		{
		}
	}
}
