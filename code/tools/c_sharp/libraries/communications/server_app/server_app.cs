using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Collections;
using System.Diagnostics;
using System.Net.Sockets;
using System.Threading;
using comms_tcp;
using communications_udp;

namespace server_app
{
	public partial class ServerApp : Form
	{
		CommsServer<CommsProtocolJan> m_commsServer;
		CommunicationsUdp m_commUdp;

		delegate TabPane InvokedAddPage();
		InvokedAddPage m_invokedAddPage;

		delegate bool InvokedWriteUdpString(string strText);
		InvokedWriteUdpString m_invokedWriteUdpString;

		#region Move this into the tabControl?
		// TODO: Pass the CommunicationListener into the tabpane and let it sort everything out??

		TabPane AddPage()
		{
			TabPage tabPage = new TabPage("test");
			TabPane tabPane = new TabPane();
			tabPane.Dock = DockStyle.Fill;
			tabPage.Controls.Add(tabPane);
			tabControl.TabPages.Add(tabPage);
			return tabPane;
		}
		#endregion Move this into the tabControl?

		public delegate void InvokedText(String strText);
		InvokedText m_invokedText;

		private void WriteToTextBox(string strText)
		{
			Debug.Assert(false);
			//			this.richTextBox.AppendText(strText);
		}

		private bool WriteUdpString(string strText)
		{
			this.richTextBox_UdpStrings.AppendText(strText);
			return true;
		}


		public ServerApp()
		{
			InitializeComponent();

			m_invokedText =  new InvokedText(WriteToTextBox);
			m_invokedAddPage = AddPage;
			m_invokedWriteUdpString = WriteUdpString;

			tabControl.TabPages.Clear();
		}

		public bool Initialise()
		{
			try
			{
//				m_commServer = new CommunicationsServer(ProcessConnect, 2001, CommunicationsProtocol.ProtocolType.StringThenEndMarker);
				//m_commServer = new CommunicationsServer(ProcessConnect, 2001, CommunicationsProtocol.ProtocolType.FourByteSizeThenString);	// PROTOCOLTYPE
				//m_commServer = new CommunicationsServer(ProcessConnect, 2001, communications_tcp.CommunicationsListener.Protocol.FourByteSizeThenString);	// PROTOCOLTYPE
				m_commsServer = new CommsServer<CommsProtocolJan>(9782);
				m_commsServer.ConnectionStateChangedEvent += new ConnectionStateChangedDelegate(m_commsServer_ConnectionStateChangedEvent);
				m_commsServer.Start();

				m_commUdp = new CommunicationsUdp(9783);
				m_commUdp.GotUdpDataEvent += new GotUdpDataDelegate(m_commUdp_GotUdpDataEvent);
				m_commUdp.Start();
				return true;
			}
			catch
			{
				MessageBox.Show("Failed to initialise server.");
				return false;
			}
		}

		void m_commUdp_GotUdpDataEvent(object sender, GotUdpDataEventArgs e)
		{
			string str = e.DataAsString;
			str = e.RemoteEndpoint.ToString() + " " + str;
			Invoke(m_invokedWriteUdpString, new object[] { str });
		}

		void m_commsServer_ConnectionStateChangedEvent(object sender, TcpConnectionCallbackEventArgs e)
		{
			TabPane tabPane = Invoke(m_invokedAddPage) as TabPane;
			
			CommsListener cl = e.CommsListener;

			tabPane.SetCommsListener(cl);
		}








		int m_instance = 0;

		private void buttonCreateLocalClient_Click(object sender, EventArgs e)
		{
			System.IO.Directory.SetCurrentDirectory(System.IO.Path.GetDirectoryName(Application.ExecutablePath));
			System.Diagnostics.Process.Start(@"..\..\..\Test_Client\bin\Debug\TestClient.exe", "\"" + m_commsServer.LocalIp + "\" " + m_commsServer.Port + " " + m_instance++);
		}

		private void Form1_FormClosing(object sender, FormClosingEventArgs e)
		{
			m_commsServer.StopServer();
			m_commsServer = null;
		}

		private void buttonClear_Click(object sender, EventArgs e)
		{
			int index = this.tabControl.SelectedIndex;
			if (index != -1)
			{
//				this.tabControl.TabPages[index].Controls[2].Clear()
//			this.richTextBox.Clear();
			}
		}

		private void buttonCreateRemoteClient_Click(object sender, EventArgs e)
		{
			/// TODO:
			/// Find remote servers.
			/// Ask each one if it is available until an available one is found.
			/// Start the job on the remote server.

		}

		private void button_Broadcast_Click(object sender, EventArgs e)
		{
			IPEndPoint ipep = new IPEndPoint(IPAddress.Broadcast, 9801);

			m_commUdp.SendDataString(this.textBox_UdpBroadcast.Text, ipep);
		}
	}
}
