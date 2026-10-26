using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dCheckoutComplete(object userData, bool result);

		public bool Checkout(AlienbrainRequest.Priority priority, string strLocalFile)
		{
			AlienbrainRequest ar = BeginCheckout(priority, null, strLocalFile, new dCheckoutComplete(CheckoutComplete));
			ar.WaitForResult();
			return (bool)ar.m_result;
		}


		public AlienbrainRequest BeginCheckout(AlienbrainRequest.Priority priority, object userData, string strLocalFile, dCheckoutComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dCheckout(DoCheckout), new object[] { strLocalFile }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void CheckoutComplete(object userData, bool result)
		{
		}
	}
}
