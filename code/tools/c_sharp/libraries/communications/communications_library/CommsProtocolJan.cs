using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace comms_tcp
{
	public class CommsProtocolJan : ICommsProtocol
	{
		/// <summary>
		/// !!!MUST BE THREAD SAFE!!!
		/// !!!MUST BE THREAD SAFE!!!
		/// !!!MUST BE THREAD SAFE!!!
		/// !!!MUST BE THREAD SAFE!!!
		/// !!!MUST BE THREAD SAFE!!!
		/// !!!MUST BE THREAD SAFE!!!
		/// !!!MUST BE THREAD SAFE!!!
		/// !!!MUST BE THREAD SAFE!!!
		/// !!!MUST BE THREAD SAFE!!!
		/// </summary>


		public CommsProtocolJan()
		{
		}

		int m_sizeIndex = 0;				/// Where to read the 4 byte size data into. If sizeIndex == 4 the size data has been read.
		readonly byte[] m_SizeArray = new byte[4];	/// This is where the size is read into.

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
				int length = System.Net.IPAddress.HostToNetworkOrder(strData.Length);
				byte[] lengthAsByteArray = BitConverter.GetBytes(length);

				byte[] byteBuffer = new byte[4 + strData.Length];

				lengthAsByteArray.CopyTo(byteBuffer, 0);

				byte[] stringAsByteArray = Encoding.ASCII.GetBytes(strData as string);

				stringAsByteArray.CopyTo(byteBuffer, 4);

				return byteBuffer;
			}
			return null;
		}

		public int DealWithNewData(int numBytes, byte[] byteBuffer)
		{
			int messagesHandled = 0;

			int offsetByte = 0;

			while(offsetByte < numBytes)
			{
				while(m_sizeIndex < 4)
				{
					m_SizeArray[m_sizeIndex] = byteBuffer[offsetByte];
					m_sizeIndex++;
					offsetByte++;

					if(offsetByte == numBytes)
						return 0;
				}

				int stringSize = BitConverter.ToInt32(m_SizeArray, 0);

				stringSize = System.Net.IPAddress.NetworkToHostOrder(stringSize);

				int bufferedBytes = numBytes - offsetByte;
				int requiredBytes = Math.Min(stringSize - m_nextReceivedString.Length, bufferedBytes);

				for(int c = offsetByte; c < requiredBytes + offsetByte; c++)
				{
					m_nextReceivedString.Append((char)byteBuffer[c]);					/// TODO: This looks really slow.
				}
				// Or ...
				//m_nextReceivedString.Append(byteBuffer, offsetByte, requiredBytes);

				offsetByte += requiredBytes;
				if(stringSize == m_nextReceivedString.Length)
				{
					m_queueReceivedStrings.Enqueue(m_nextReceivedString.ToString());		// Add the new string to the table.
					m_nextReceivedString.Remove(0, m_nextReceivedString.Length);			// Clear the 'new' string.
					m_sizeIndex = 0;														// Start looking for a 'size' marker again.
				}
			}

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
