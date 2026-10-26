using System;

namespace comms_tcp
{
	public interface ICommsProtocol
	{
		byte[] EncodeMessageForSending(object message);

		int DealWithNewData(int numBytes, byte[] byteBuffer);

		event EventHandler<GotMessageEventArgs> GotMessageEvent;
	}

	public class GotMessageEventArgs : EventArgs
	{
		readonly object m_message;

		public object Message
		{
			get { return m_message; }
		} 

		public GotMessageEventArgs(object message)
		{
			m_message = message;
		}
	}
}



