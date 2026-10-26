using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dGetNodeDetailsComplete(object userData, AlienbrainInfo abInfo);

		public AlienbrainInfo GetNodeDetails(AlienbrainRequest.Priority priority, string strPath, bool bPathIsNamespace)
		{
			AlienbrainRequest ar = BeginGetNodeDetails(priority, null, strPath, new dGetNodeDetailsComplete(GetNodeDetailsComplete), bPathIsNamespace);
			ar.WaitForResult();
			return ar.m_result as AlienbrainInfo;
		}


		public AlienbrainRequest BeginGetNodeDetails(AlienbrainRequest.Priority priority, object userData, string strPath, dGetNodeDetailsComplete myCompleteCallback, bool bPathIsNamespace)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dGetNodeDetails(DoGetNodeDetails), new object[] { strPath, bPathIsNamespace }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void GetNodeDetailsComplete(object userData, AlienbrainInfo abInfo)
		{

		}
	}
}
