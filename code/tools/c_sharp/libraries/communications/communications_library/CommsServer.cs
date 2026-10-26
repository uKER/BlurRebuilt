#define needs_fixing_up


using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Threading;

#if needs_fixing_up

namespace comms_tcp
{
	internal class GenericTest
	{
		private CommsServer<CommsProtocolKeith> test = new CommsServer<CommsProtocolKeith>(5);
	}


	public class CommsServer<Protocol> where Protocol : ICommsProtocol, new()
	{
		private readonly IPAddress m_localIP;

		/// <summary>
		/// Local Variables Declaration.
		/// </summary>
		private readonly int m_port;

		private Thread m_purgingThread;

		private List<CommsListener> m_serverListenersList;
		private TcpListener m_serverTcpListener;
		private bool m_stopPurging;
		private bool m_stopServer;

		public CommsServer(int port)
		{
			m_port = port;

			m_localIP = cGlobal.GetLocalIpAddress();
		}

		public IPAddress LocalIp
		{
			get { return m_localIP; }
		}

		public int Port
		{
			get { return m_port; }
		}

		public event DataCallbackDelegate DataReceivedEvent;
		public event DataCallbackDelegate DataSentEvent;
		public event ConnectionStateChangedDelegate ConnectionStateChangedEvent;

		protected void OnDataReceived(TcpDataCallbackEventArgs e)
		{
			if(DataReceivedEvent != null)
			{
				DataReceivedEvent(this, e);
			}
		}

		protected void OnDataSent(TcpDataCallbackEventArgs e)
		{
			if(DataSentEvent != null)
			{
				DataSentEvent(this, e);
			}
		}

		protected void OnConnectionStateChanged(TcpConnectionCallbackEventArgs e)
		{
			if(ConnectionStateChangedEvent != null)
			{
				ConnectionStateChangedEvent(this, e);
			}
		}


		/// <summary>
		/// Destructor.
		/// </summary>
		~CommsServer()
		{
			StopServer();
		}

		/// <summary>
		/// Init method that creates a server (TCP Listener) object based on the
		/// IP Address and Port information that is passed in.
		/// </summary>
		/// <param name="ipNport"></param>
		public void Start()
		{
			IPEndPoint localIpEndpoint = new IPEndPoint(m_localIP, m_port);
			//Init(localIpEndpoint);

			try
			{
				m_serverTcpListener = new TcpListener(localIpEndpoint);

				// Create a List for storing SocketListeners before starting the server.
				m_serverListenersList = new List<CommsListener>();

				m_serverTcpListener.Start();

				m_serverTcpListener.BeginAcceptSocket(AcceptSocketCallback, m_serverTcpListener);

				// Create a low priority thread that checks and deletes client SocketConnection objects that are marked for deletion.
				m_purgingThread = new Thread(PurgingThreadStart);
				m_purgingThread.Priority = ThreadPriority.Lowest;
				m_purgingThread.Start();
			}
			catch(Exception e)
			{
				Debug.WriteLine(e.Message);
				m_serverTcpListener = null;
			}
		}

		private void AcceptSocketCallback(IAsyncResult ar)
		{
			// Get the listener that handles the client request.
			TcpListener tcpListener = (TcpListener)ar.AsyncState;

			Debug.Assert(tcpListener == m_serverTcpListener);

			try
			{
				// End the operation.
				Socket clientSocket = tcpListener.EndAcceptSocket(ar);

				if(m_stopServer == false)
				{
					// Make a new CommsListener using this socket.
					Protocol commsProtocol = new Protocol();
					//commsProtocol.GotMessageEvent += blah;
					CommsListener commsListener = new CommsListener(clientSocket, new Protocol());

					commsListener.ConnectionStateChangedEvent += commsListener_ConnectionStateChangedEvent;
					commsListener.DataSentEvent += commsListener_DataSentEvent;
					commsListener.DataReceivedEvent += commsListener_DataReceivedEvent;

					commsListener.StartConnection(true);

					// Add the socket to a server table (array list) in a thread safe fashion.
					lock(m_serverListenersList)
					{
						m_serverListenersList.Add(commsListener);
					}
				}
				// Listen for the next one.
				m_serverTcpListener.BeginAcceptSocket(AcceptSocketCallback, m_serverTcpListener);
			}
			catch(SocketException sex)
			{
				Debug.WriteLine(sex.Message);
				m_stopServer = true;
			}
		}

		private void commsListener_ConnectionStateChangedEvent(object sender, TcpConnectionCallbackEventArgs e)
		{
			// TODO: Use an event to pass this back to the owner of the class.
			OnConnectionStateChanged(e);
		}

		private void commsListener_DataSentEvent(object sender, TcpDataCallbackEventArgs e)
		{
			// TODO: Use an event to pass this back to the owner of the class.
			OnDataSent(e);
		}

		private void commsListener_DataReceivedEvent(object sender, TcpDataCallbackEventArgs e)
		{
			// TODO: Use an event to pass this back to the owner of the class.
			OnDataReceived(e);
		}

		/// <summary>
		/// Method that stops the TCP/IP Server.
		/// </summary>
		public void StopServer()
		{
			if(m_serverTcpListener != null)
			{
				// Stop the TCP/IP Server. Do this before any cleanup to prevent new connections coming in.
				m_stopServer = true;
				m_serverTcpListener.Stop();

				m_stopPurging = true;
				m_purgingThread.Join(1000);
				if(m_purgingThread.IsAlive)
				{
					m_purgingThread.Abort();
				}
				m_purgingThread = null;

				// Free Server Object.
				m_serverTcpListener = null;

				// Stop All clients.
				StopAllSocketListeners();
			}
		}

		/// <summary>
		/// Method that stops all clients and clears the list.
		/// </summary>
		private void StopAllSocketListeners()
		{
			foreach(CommsListener serverListener in m_serverListenersList)
			{
				serverListener.StopListener();
			}
			// Remove all elements from the list.
			m_serverListenersList.Clear();
			m_serverListenersList = null;
		}

		/// <summary>
		/// Thread method for purging Client Listeners that are marked for
		/// deletion (i.e. clients with socket connection closed). This thread
		/// is a low priority thread and sleeps for 10 seconds and then checks
		/// for any client SocketConnection objects which are obselete and 
		/// marked for deletion.
		/// </summary>
		private void PurgingThreadStart()
		{
			while(!m_stopPurging)
			{
				List<CommsListener> deleteList = new List<CommsListener>();

				// Check for any CommunicationListeners that are no longer connected and put them in a separate list in a thread safe fashion.
				lock(m_serverListenersList)
				{
					foreach(CommsListener serverListener in m_serverListenersList)
					{
						if(serverListener.Connected == false)
						{
							deleteList.Add(serverListener);
							serverListener.StopListener();
						}
					}

					// Delete all the client CommunicationListeners which are in marked for deletion by being in the delete list.
					lock(m_serverListenersList)
					{
						for(int i = 0; i < deleteList.Count; ++i)
						{
							m_serverListenersList.Remove(deleteList[i]);
						}
					}
				}

				deleteList = null;
				Thread.Sleep(10000);
			}
		}
	}
}

#endif