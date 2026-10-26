using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dPrepareToEnumerateChildrenComplete(object userData, int result);

		public int PrepareToEnumerateChildren(AlienbrainRequest.Priority priority, string strNamespacePath)
		{
			AlienbrainRequest ar = BeginPrepareToEnumerateChildren(priority, null, strNamespacePath, new dPrepareToEnumerateChildrenComplete(PrepareToEnumerateChildrenComplete));
			ar.WaitForResult();
			return (int)ar.m_result;
		}


		public AlienbrainRequest BeginPrepareToEnumerateChildren(AlienbrainRequest.Priority priority, object userData, string strNamespacePath, dPrepareToEnumerateChildrenComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dPrepareToEnumerateChildren(DoPrepareToEnumerateChildren), new object[] { strNamespacePath }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void PrepareToEnumerateChildrenComplete(object userData, int result)
		{

		}
	}
}