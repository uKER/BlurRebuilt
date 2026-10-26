using System;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace communications_udp
{
	public delegate void GotUdpDataDelegate(object sender, GotUdpDataEventArgs e);

	public class GotUdpDataEventArgs : EventArgs
	{
		public GotUdpDataEventArgs(EndPoint remoteEndpoint, Byte[] rawData)
		{
			RemoteEndpoint = remoteEndpoint;
			RawData = rawData;
		}

		public EndPoint RemoteEndpoint { get; private set; }

		public byte[] RawData { get; private set; }

		public string DataAsString
		{
			get { return Encoding.ASCII.GetString(RawData, 0, RawData.Length); }
		}
	}

	// Assumption: All UDP messages are under 8K in size and therefore can be read in one go without the need to
	// build them up in succesive readcallbacks.

	public class CommunicationsUdp
	{
		private readonly Byte[] byteBuffer = new Byte[3000];
		private readonly object m_readCallbackLock = new object();
		private readonly System.Threading.AutoResetEvent m_sendCompleted;
		private EndPoint m_remoteEp;
		private Socket m_udpSocket;

		//public EventHandler<GotUdpDataEventArgs> GotUdpDataEvent;
		/// <summary>
		/// Creates a Udp socket that listens to all data on the provided port.
		/// You must call 'Start()' to start listening for data.
		/// </summary>
		/// <param name="port">The port to listen on.</param>
		public CommunicationsUdp(int port)
		{
			m_sendCompleted = new System.Threading.AutoResetEvent(true);
			IPEndPoint remoteIpEp = new IPEndPoint(IPAddress.Any, port);
			m_remoteEp = remoteIpEp;

			//recieve data from any source
			IPEndPoint LocalHostIPEnd = new IPEndPoint(IPAddress.Any, port);


			m_udpSocket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
			m_udpSocket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.Broadcast, true);
			m_udpSocket.ExclusiveAddressUse = false;

			try
			{
				m_udpSocket.Bind(LocalHostIPEnd);
			}
			catch(Exception ex)
			{
				Console.WriteLine("Winsock error: " + ex);
			}

			m_udpSocket.SendBufferSize = byteBuffer.Length;
			m_udpSocket.ReceiveBufferSize = byteBuffer.Length;
		}

		/// <summary>
		/// This event is fired every time some new data comes in on the socket.
		/// </summary>
		public event GotUdpDataDelegate GotUdpDataEvent; // Intellisense works better with this than the next way (below).

		/// <summary>
		/// Instructs the underlying socket to start receiving data.
		/// </summary>
		public void Start()
		{
			try
			{
				m_udpSocket.BeginReceiveFrom(byteBuffer, 0, byteBuffer.Length, SocketFlags.None, ref m_remoteEp, UdpReadCallback,
				                             m_udpSocket);
			}
			catch(Exception ex)
			{
				Debug.WriteLine("Cannot 'BeginReceiveFrom' in CommunicationsUdp constructor.\nSystem reports: " + ex.Message);
				throw (ex);
			}
		}

		/// <summary>
		/// Sends a string to the given destination.
		/// </summary>
		/// <param name="strData">The data to send.</param>
		/// <param name="remoteIpEp">The endpoint to sent it to.</param>
		/// <returns></returns>
		public bool SendDataString(string strData, IPEndPoint remoteIpEp)
		{
			EndPoint remoteEp = remoteIpEp;

			if(m_udpSocket != null)
			{
				try
				{
					m_sendCompleted.WaitOne(); // Wait for the previous 'send' to complete.
					byte[] buff = Encoding.ASCII.GetBytes(strData);
					m_udpSocket.BeginSendTo(buff, 0, buff.Length, SocketFlags.None, remoteEp, SendCallback, m_udpSocket);
					return true;
				}
				catch(SocketException sex)
				{
					m_sendCompleted.Set();
					Debug.WriteLine("Socket exception in SendDataString: " + sex.Message);
				}
				catch(Exception ex)
				{
					m_sendCompleted.Set();
					Debug.WriteLine("Unexpected error : " + ex.Message + "\nInner exception :", ex.InnerException.Message);
				}
			}
			return false;
		}

		/// <summary>
		/// Closes the underlying socket.
		/// </summary>
		public void Close()
		{
			lock(m_readCallbackLock)
			{
				this.m_udpSocket.Close(1000);
				this.m_udpSocket = null;
			}
		}

		protected void OnGotUdpDataEvent(GotUdpDataEventArgs e)
		{
			if(GotUdpDataEvent != null)
			{
				GotUdpDataEvent(this, e);
			}
		}

		private void UdpReadCallback(IAsyncResult ar)
		{
			lock(m_readCallbackLock)
			{
				if(m_udpSocket == null) /// Socket may have been closed.
					return;

				Socket s = (Socket)ar.AsyncState;
				try
				{
					IPEndPoint sender = new IPEndPoint(IPAddress.Any, 0);
					EndPoint tempEp = sender;

					int bytesRead = s.EndReceiveFrom(ar, ref tempEp);
						// To identify the originating host, extract the EndPoint and cast it to an IPEndPoint. Use the IPEndPoint.Address method to obtain the IP address and the IPEndPoint.Port method to obtain the port number.

					if(bytesRead > 0)
					{
						Byte[] tempBuffer = new Byte[bytesRead];
						Array.Copy(byteBuffer, tempBuffer, bytesRead);
						OnGotUdpDataEvent(new GotUdpDataEventArgs(tempEp, tempBuffer));

						s.BeginReceiveFrom(byteBuffer, 0, byteBuffer.Length, SocketFlags.None, ref m_remoteEp, UdpReadCallback,
						                   m_udpSocket);
					}
					else
					{
						Debug.WriteLine("XXX ATTEND. Udp bytes read = " + bytesRead);
					}
				}
				catch(SocketException sex) /// If something bad happens, (e.g. connection lost) shut down the socket.
				{
					Debug.WriteLine("XXX ATTEND. Socket exception in UdpReadCallback." + sex.Message);
				}
				catch(ObjectDisposedException odex)
				{
					Debug.WriteLine("UdpReadCallback object disposed. " + odex.Message);
				}
				catch(Exception ex)
				{
					Debug.WriteLine("XXX ATTEND. General exception in UdpReadCallback. " + ex.Message);
				}
			}
		}

		private void SendCallback(IAsyncResult ar)
		{
			Socket s = (Socket)ar.AsyncState;
			try
			{
				int bytesSent = s.EndSendTo(ar);
			}
			catch /*(SocketException sex)*/
			{
				//MessageBox.Show(null, "Socket exception in SendCallback: " + sex.Message, "Error", MessageBoxButtons.OK);
			}
				//catch/*(Exception ex)*/
				//{
				//    //MessageBox.Show(ex.Message);
				//}
			finally
			{
				m_sendCompleted.Set();
			}
		}
	}
}