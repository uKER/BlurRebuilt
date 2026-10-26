using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dGetFirstFolderComplete(object userData, AlienbrainInfo abInfo);

		public AlienbrainInfo GetFirstFolder(AlienbrainRequest.Priority priority, string strNamespacePath)
		{
			AlienbrainRequest ar = BeginGetFirstFolder(priority, null, strNamespacePath, new dGetFirstFolderComplete(GetFirstFolderComplete));
			ar.WaitForResult();
			return ar.m_result as AlienbrainInfo;
		}


		public AlienbrainRequest BeginGetFirstFolder(AlienbrainRequest.Priority priority, object userData, string strNamespacePath, dGetFirstFolderComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dGetFirstFolder(DoGetFirstFolder), new object[] { strNamespacePath }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void GetFirstFolderComplete(object userData, AlienbrainInfo abInfo)
		{

		}
	}
}
