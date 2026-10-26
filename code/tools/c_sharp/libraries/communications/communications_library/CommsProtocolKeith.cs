using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace comms_tcp
{
	class CommsProtocolKeith : ICommsProtocol
	{
		private StringBuilder m_nextReceivedString = new StringBuilder();
		private Queue<string> m_queueReceivedStrings = new Queue<string>();

		private string GetNextString()
		{
			if(m_queueReceivedStrings.Count > 0)
			{
				string strRetval = m_queueReceivedStrings.Dequeue();
				return strRetval;
			}
			return null;
		}

		private void OnGotMessageEvent(GotMessageEventArgs gotMessageEventArgs)
		{
			if(GotMessageEvent != null)
			{
				GotMessageEvent(this, gotMessageEventArgs);
			}
		}

		#region ICommsProtocol Members

		public byte[] EncodeMessageForSending(object message)
		{
			Debug.Assert(message is string);
			string strData = message as string;
			if(strData != null)
			{
				byte[] byteBuffer = Encoding.ASCII.GetBytes(strData + "[END]");

				return byteBuffer;
			}
			return null;
		}

		public int DealWithNewData(int numBytes, byte[] byteBuffer)
		{
			int messagesHandled = 0;

			// Look for '[END]' ...
			string data = Encoding.ASCII.GetString(byteBuffer, 0, numBytes);
			int lineEndIndex = 0;

			// Check whether data from client has more than one line of information, where each line of information ends with "[END]"
			// If so break data into different lines and process separately.
			// TODO: Check that m_arrayReceivedStrings does not get too big.
			m_nextReceivedString.Append(data);
			do
			{
				lineEndIndex = m_nextReceivedString.ToString().IndexOf("[END]");
				if(lineEndIndex != -1)
				{
					m_queueReceivedStrings.Enqueue(m_nextReceivedString.ToString(0, lineEndIndex));
					m_nextReceivedString.Remove(0, lineEndIndex + 5);
				}
			} while(lineEndIndex != -1);

			string strMessage = GetNextString();


			while(strMessage != null)
			{
				OnGotMessageEvent(new GotMessageEventArgs(strMessage));

				messagesHandled++;
				strMessage = GetNextString();
			}

			return messagesHandled;
		}

		public event EventHandler<GotMessageEventArgs> GotMessageEvent;

		#endregion

	}
}
