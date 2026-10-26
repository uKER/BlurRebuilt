using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dUndoCheckoutComplete(object userData, bool result);

		public bool UndoCheckout(AlienbrainRequest.Priority priority, string strLocalFile)
		{
			AlienbrainRequest ar = BeginUndoCheckout(priority, null, strLocalFile, new dUndoCheckoutComplete(UndoCheckoutComplete));
			ar.WaitForResult();
			return (bool)ar.m_result;
		}


		public AlienbrainRequest BeginUndoCheckout(AlienbrainRequest.Priority priority, object userData, string strLocalFile, dUndoCheckoutComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dUndoCheckout(DoUndoCheckout), new object[] { strLocalFile }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void UndoCheckoutComplete(object userData, bool result)
		{
		}
	}
}
