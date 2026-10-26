using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;
using System.Collections;
using System.Diagnostics;

namespace communications
{
	public class CommunicationsProtocol_Keith : CommunicationsProtocol
	{
		public CommunicationsProtocol_Keith(CommunicationsListener cl) : base(cl)
		{
		}

		public override bool SendData(object data)
		{
			Debug.Assert(false, "CommunicationsProtocol_Keith CANNOT SEND RAW DATA.");
//			return m_cl.SendData(byteBuffer);
			return false;
		}

		public override bool SendData(string strData)
		{
			Debug.Assert(strData is string);
			if(strData is string)
			{
				byte[] byteBuffer = Encoding.ASCII.GetBytes(strData as string + "[END]");

				return m_cl.SendData(byteBuffer);
			}
			return false;
		}


		public override int DealWithNewData(int numBytes, byte [] byteBuffer)
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
					m_arrayReceivedStrings.Add(m_nextReceivedString.ToString(0, lineEndIndex));
					m_nextReceivedString.Remove(0, lineEndIndex + 5);
				}
			} while(lineEndIndex != -1);

			string strMessage = GetNextString();

			while(strMessage != null)
			{
				m_cl.m_dataCallback(cGlobal.CallbackType.ReceivedData, strMessage);
				messagesHandled++;
				strMessage = GetNextString();
			}
			return messagesHandled;
		}

		protected StringBuilder m_nextReceivedString = new StringBuilder();
		protected ArrayList m_arrayReceivedStrings = new ArrayList();
		protected string GetNextString()
		{
			if(m_arrayReceivedStrings.Count > 0)
			{
				string strRetval = m_arrayReceivedStrings[0] as string;
				m_arrayReceivedStrings.RemoveAt(0);
				return strRetval;
			}

			return null;
		}
	}
}
