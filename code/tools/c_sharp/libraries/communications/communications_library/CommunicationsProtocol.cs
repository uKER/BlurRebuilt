using System;
using System.Collections.Generic;
using System.Text;

namespace communications
{
	public abstract class CommunicationsProtocol
	{
		protected readonly CommunicationsListener m_cl;

		public CommunicationsProtocol(CommunicationsListener cl)
		{
			m_cl = cl;
		}

//		public abstract bool SendDataString(string strData);
		public abstract bool SendData(string strData);
		public abstract bool SendData(object data);
		public abstract int DealWithNewData(int numBytes, byte [] byteBuffer);
	}
}
