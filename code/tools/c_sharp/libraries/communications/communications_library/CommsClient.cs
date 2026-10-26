using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
//using System.Windows.Forms;
using comms_tcp;
using System.Threading;
using System.Diagnostics;

namespace comms_tcp
{
	public class CommsClient : CommsListener
	{
		private IPEndPoint m_ipep;
		private System.Threading.Timer m_timer;
		private volatile bool bEstablishingConnection = false;

		public IPAddress GetIpAddress { get { { return m_ipep.Address; } } }
		
		public CommsClient(IPEndPoint ipep, bool b_autoReconnect, ICommsProtocol commsProtocol) : base(commsProtocol)
		{
			m_ipep = ipep;
			if(b_autoReconnect == true)
			{
				m_timer = new System.Threading.Timer(new TimerCallback(ClientTimerCallback));
			}
		}

		public void Start()
		{
			BeginEstablishConnection();

			if(m_timer != null)
			{
				m_timer.Change(3000, 5000);
			}
		}

		public void Close()
		{
			this.m_timer.Dispose();
			this.StopListener();
		}

		private void BeginEstablishConnection()
		{
			if(bEstablishingConnection == true)
				return;

			bEstablishingConnection = true;

			base.StopListener();		// Just in case we have a socket and it is in use.

			m_clientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

			m_clientSocket.BeginConnect(m_ipep, new AsyncCallback(ConnectedCallback), this);
		}

		private void ConnectedCallback(IAsyncResult ar)
		{
			CommsClient ccl = (CommsClient)ar.AsyncState;

			Socket s = m_clientSocket;
			Debug.Assert(ccl == this);
			Debug.Assert(s == m_clientSocket);
			try
			{
				s.EndConnect(ar);					// If the connection fails an exception is thrown therefore we don't try and Initialise the connection.

				base.StartConnection(true);
			}
			catch(Exception ex)						// E.g. Target machine actively refused connection, since there is no server there.
			{
				OnConnectionStateChanged();
				Debug.WriteLine("Exception in ConnectedCallback in CommsClient.cs: '" + ex.Message + "'");
			}
			bEstablishingConnection = false;
		}

		private void ClientTimerCallback(object state)
		{
			if(this.Connected == false && bEstablishingConnection == false)
			{
				Debug.WriteLine("Reconnecting ...");
				BeginEstablishConnection();
			}
		}
	}
}


/// Reference:
/// IPHostEntry lipa = Dns.Resolve("host.contoso.com");
/// IPEndPoint lep = new IPEndPoint(lipa.AddressList[0], 11000);
/// 


