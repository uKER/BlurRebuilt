using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dGetFirstChildComplete(object userData, AlienbrainInfo abInfo);

		public AlienbrainInfo GetFirstChild(AlienbrainRequest.Priority priority, string strNamespacePath)
		{
			AlienbrainRequest ar = BeginGetFirstChild(priority, null, strNamespacePath, new dGetFirstChildComplete(GetFirstChildComplete));
			ar.WaitForResult();
			return ar.m_result as AlienbrainInfo;
		}


		public AlienbrainRequest BeginGetFirstChild(AlienbrainRequest.Priority priority, object userData, string strNamespacePath, dGetFirstChildComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dGetFirstChild(DoGetFirstChild), new object[] { strNamespacePath }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void GetFirstChildComplete(object userData, AlienbrainInfo abInfo)
		{

		}
	}
}
