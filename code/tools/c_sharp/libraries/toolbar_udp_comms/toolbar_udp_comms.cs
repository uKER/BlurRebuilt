using System;
using System.Collections.Generic;
using System.Text;
using communications_udp;
using System.Net;
using System.Diagnostics;

#if false

namespace toolbar_udp_comms
{
	public enum UdpMessageType
	{
		Unknown = 0,
		String,
	}


	public class ToolbarUdpComms
	{
		private CommunicationsUdp m_communicationsUdp;

		public ToolbarUdpComms(CommunicationsUdp.UdpDataCallbackDelegate udpDataCallback, int port)
		{
			m_communicationsUdp = new CommunicationsUdp(udpDataCallback, port);
		}

		public bool SendToSelf(UdpMessageType messageType, string strMessage)
		{
			bool bRetval = false;

			switch(messageType)
			{
				case UdpMessageType.Unknown:
					break;
				case UdpMessageType.String:
					//bRetval = this.SendMessage(UdpMessageType.String, messageType.ToString() + " " + strMessage);
					bRetval = m_communicationsUdp.SendDataString(messageType.ToString() + " " + strMessage, new System.Net.IPEndPoint(IPAddress.Loopback, 1664));

					break;
				default:
					break;
			}

			return bRetval;
		}
/*
		private bool SendMessage(UdpMessageType type, string strMessage, IPEndPoint ipEndPoint)
		{
			switch(messageType)
			{
				case UdpMessageType.Unknown:
					break;
				case UdpMessageType.String:
					//bRetval = this.SendMessage(UdpMessageType.String, messageType.ToString() + " " + strMessage);
					bRetval = m_communicationsUdp.SendDataString(messageType.ToString() + " " + strMessage, new System.Net.IPEndPoint(IPAddress.Broadcast, 1664));

					break;
				default:
					break;
			}
		}
*/	}
}
#endif