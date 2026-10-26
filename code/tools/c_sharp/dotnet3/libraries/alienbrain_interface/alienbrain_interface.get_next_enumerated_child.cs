using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dGetNextEnumeratedChildComplete(object userData, AlienbrainInfo abInfo);

		public AlienbrainInfo GetNextEnumeratedChild(AlienbrainRequest.Priority priority)
		{
			AlienbrainRequest ar = BeginGetNextEnumeratedChild(priority, null, new dGetNextEnumeratedChildComplete(GetNextEnumeratedChildComplete));
			ar.WaitForResult();
			return ar.m_result as AlienbrainInfo;
		}


		public AlienbrainRequest BeginGetNextEnumeratedChild(AlienbrainRequest.Priority priority, object userData, dGetNextEnumeratedChildComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dGetNextEnumeratedChild(DoGetNextEnumeratedChild), null, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void GetNextEnumeratedChildComplete(object userData, AlienbrainInfo abInfo)
		{

		}
	}
}
