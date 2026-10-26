using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dGetLatestFileComplete(object userData, bool result);

		public bool GetLatestFile(AlienbrainRequest.Priority priority, string strLocalFile)
		{
			AlienbrainRequest ar = BeginGetLatestFile(priority, null, strLocalFile, new dGetLatestFileComplete(GetLatestFileComplete));
			ar.WaitForResult();
			return (bool)ar.m_result;
		}


		public AlienbrainRequest BeginGetLatestFile(AlienbrainRequest.Priority priority, object userData, string strLocalFile, dGetLatestFileComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dGetLatestFile(DoGetLatestFile), new object[] { strLocalFile }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void GetLatestFileComplete(object userData, bool result)
		{
		}
	}
}
