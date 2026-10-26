using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dCheckinComplete(object userData, bool result);

		public bool Checkin(AlienbrainRequest.Priority priority, string strLocalFile)
		{
			AlienbrainRequest ar = BeginCheckin(priority, null, strLocalFile, new dCheckinComplete(CheckinComplete));
			ar.WaitForResult();
			return (bool)ar.m_result;
		}


		public AlienbrainRequest BeginCheckin(AlienbrainRequest.Priority priority, object userData, string strLocalFile, dCheckinComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dCheckin(DoCheckin), new object[] { strLocalFile }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void CheckinComplete(object userData, bool result)
		{
		}
	}
}
