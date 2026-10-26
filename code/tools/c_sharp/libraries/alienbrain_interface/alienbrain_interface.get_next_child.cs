using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dGetNextChildComplete(object userData, AlienbrainInfo abInfo);

		public AlienbrainInfo GetNextChild(AlienbrainRequest.Priority priority)
		{
			AlienbrainRequest ar = BeginGetNextChild(priority, null, new dGetNextChildComplete(GetNextChildComplete));
			ar.WaitForResult();
			return ar.m_result as AlienbrainInfo;
		}


		public AlienbrainRequest BeginGetNextChild(AlienbrainRequest.Priority priority, object userData, dGetNextChildComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dGetNextChild(DoGetNextChild), null, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void GetNextChildComplete(object userData, AlienbrainInfo abInfo)
		{

		}
	}
}
