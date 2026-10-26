using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Diagnostics;

namespace comms_tcp
{
	public enum DataCallbackType
	{
		DataReceived = 0,
		DataSent
	}
	
	// TODO: Use events here! No. Intellisense works better with a delegate.
	//public delegate void DataCallbackDelegate(object sender, TcpDataCallbackEventArgs e);

	//public delegate void ConnectionStateChangedDelegate(object sender, TcpConnectionCallbackEventArgs e);

	//public event EventHandler
	public delegate void DataCallbackDelegate(object sender, TcpDataCallbackEventArgs e);

	public delegate void ConnectionStateChangedDelegate(object sender, TcpConnectionCallbackEventArgs e);



	public class TcpConnectionCallbackEventArgs : EventArgs
	{
		readonly CommsListener m_commsListener;

		public TcpConnectionCallbackEventArgs(CommsListener commsListener)
		{
			m_commsListener = commsListener;
		}

		public CommsListener CommsListener
		{
			get { return m_commsListener; }
		}
	}

	public class TcpDataCallbackEventArgs : EventArgs
	{
		readonly CommsListener m_listenerSource;

		readonly DataCallbackType m_dataCallbackType;

		readonly object m_data;

		public TcpDataCallbackEventArgs(CommsListener listenerSource, DataCallbackType dataCallbackType, object data)
		{
			m_listenerSource = listenerSource;
			m_dataCallbackType = dataCallbackType;
			m_data = data;
		}

		public CommsListener ListenerSource
		{
			get { return m_listenerSource; }
		} 

		public DataCallbackType DataCallbackType
		{
			get { return m_dataCallbackType; }
		} 

		public object Data
		{
			get { return m_data; }
		}
	}

	public static class cGlobal
	{
		public static string STR_COMMAND = "[COMMAND]";

		static public IPAddress GetLocalIpAddress()
		{
			// Get host name
			String strHostName = Dns.GetHostName();
			Console.WriteLine("Host Name: " + strHostName);

			// Find host by name
			IPHostEntry iphostentry = Dns.GetHostEntry(strHostName);
			// Enumerate IP addresses
			int count = 0;
			foreach(IPAddress ipaddress in iphostentry.AddressList)
			{
				Console.WriteLine("IP #" + count++ + ": " + ipaddress.ToString());
			}

			if(count > 0)
				return iphostentry.AddressList[0];
			else
				return null;
		}
	}
}
