using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;
using System.Diagnostics;

namespace comms_tcp
{
	public class CommsListener
	{
		private readonly Byte[] byteReceiveBuffer = new Byte[1024];

		protected Socket m_clientSocket = null;
		protected object m_readCallbackLock = new object();
		protected ICommsProtocol m_commsProtocol;

		public bool Connected { get { if(m_clientSocket != null && m_clientSocket.Connected == true) return true; else return false; } }
		public event DataCallbackDelegate DataReceivedEvent;
		public event DataCallbackDelegate DataSentEvent;
		public event ConnectionStateChangedDelegate ConnectionStateChangedEvent;

		protected void OnDataReceived(object data)
		{
			if(DataReceivedEvent != null)
			{
				DataReceivedEvent(this, new TcpDataCallbackEventArgs(this, DataCallbackType.DataReceived, data));
			}
		}

		protected void OnDataSent(object data)
		{
			if(DataSentEvent != null)
			{
				DataSentEvent(this, new TcpDataCallbackEventArgs(this, DataCallbackType.DataSent, data));
			}
		}

		protected void OnConnectionStateChanged()
		{
			if(ConnectionStateChangedEvent != null)
			{
				ConnectionStateChangedEvent(this, new TcpConnectionCallbackEventArgs(this));
			}
		}

		protected readonly System.Threading.AutoResetEvent m_sendCompletedEvent;

		protected CommsListener(ICommsProtocol commsProtocol)
		{
			m_commsProtocol = commsProtocol;

			m_sendCompletedEvent = new System.Threading.AutoResetEvent(true);
			//m_commsProtocol.GotMessageEvent += new GotMessageDelegate(m_commsProtocol_GotMessageEvent);
			m_commsProtocol.GotMessageEvent += new EventHandler<GotMessageEventArgs>(m_commsProtocol_GotMessageEvent);
		}

		internal CommsListener(Socket clientSocket, ICommsProtocol commsProtocol) : this(commsProtocol)
		{
			m_clientSocket = clientSocket;
		}

		public bool SendData(string strData)
		{
			return SendData((object)strData);
		}

		public bool SendData(object data)
		{
			byte[] dataToSend = m_commsProtocol.EncodeMessageForSending(data);

			return RawSendData(dataToSend);
		}

		void m_commsProtocol_GotMessageEvent(object sender, GotMessageEventArgs e)
		{
			OnDataReceived(e.Message);
		}

		internal bool StartConnection(bool bFireConnectedEvent)
		{
			if(bFireConnectedEvent == true)
			{
				OnConnectionStateChanged();
			}
			m_clientSocket.BeginReceive(byteReceiveBuffer, 0, byteReceiveBuffer.Length, SocketFlags.None, ReadCallback, m_clientSocket);
			return true;
		}

		void ReadCallback(IAsyncResult ar)
		{
			lock(m_readCallbackLock)
			{
				Socket s = (Socket)ar.AsyncState;

				try
				{
					int bytesRead = s.EndReceive(ar);

					if(s.Connected == true)
					{

						Debug.Assert(s == m_clientSocket);

						if(bytesRead > 0)
						{
							try
							{
								m_commsProtocol.DealWithNewData(bytesRead, byteReceiveBuffer);
							}
							catch(Exception ex)
							{
								Debug.WriteLine("Exception in 'DealWithNewData'. Probably in the user callback. System reports: " + ex.ToString());
							}
							s.BeginReceive(byteReceiveBuffer, 0, byteReceiveBuffer.Length, SocketFlags.None, ReadCallback, m_clientSocket);
						}
						else
						{
							Debug.WriteLine("Connection closed.");
							this.StopListener();
						}
					}
				}
				catch(SocketException sex)				/// If something bad happens, (e.g. connection lost) shut down the socket.
				{
					Debug.WriteLine("Socket Exception in ReadCallback in CommsListener.cs: '" + sex.Message + "'");
					this.StopListener();
				}
				catch(Exception ex)
				{
					Debug.WriteLine("Exception in ReadCallback in CommsListener.cs: '" + ex.Message + "'");
				}
			}
		}

		/// <summary>
		/// Method that stops this listener.
		/// </summary>
		internal void StopListener()
		{
			lock(m_readCallbackLock)
			{
				if(m_clientSocket != null)
				{
					m_clientSocket.Close(1000);
					// TODO: Might already be closed.
					OnConnectionStateChanged();
					m_clientSocket = null;
				}
			}
		}

		private bool RawSendData(byte[] byteBuffer)
		{
			if(byteBuffer != null)
			{
				if(m_clientSocket != null && m_clientSocket.Connected == true)
				{
					try
					{
						m_sendCompletedEvent.WaitOne();										// Wait for the previous 'send' to complete.
						m_clientSocket.BeginSend(byteBuffer, 0, byteBuffer.Length, SocketFlags.None, new AsyncCallback(SendCallback), new object[] { byteBuffer, m_clientSocket });
						return true;
					}
					catch(SocketException sex)
					{
						Debug.WriteLine("Socket Exception in ListenerSendData in CommsListener.cs: '" + sex.Message + "'");
						m_sendCompletedEvent.Set();
						// TODO: Call an error event.
					}
					catch(Exception ex)
					{
						Debug.WriteLine("Exception in ListenerSendData in CommsListener.cs: '" + ex.Message + "'");
						m_sendCompletedEvent.Set();
						// TODO: Call an error event.
					}
				}
			}
			return false;
		}

		private void SendCallback(IAsyncResult ar)
		{
			Socket s = (Socket)((object[])ar.AsyncState)[1];
			try
			{
				int temp = s.EndSend(ar);
				object sentData = ((object[])ar.AsyncState)[0];
				OnDataSent(sentData);
			}
			catch(Exception ex)
			{
				Debug.WriteLine("Exception in SendCallback in CommsListener.cs: '" + ex.Message + "'");
			}
			finally
			{
				m_sendCompletedEvent.Set();
			}
		}
	}
}


