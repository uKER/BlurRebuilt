using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Net;
using System.Threading;
using comms_tcp;
using System.Diagnostics;
using communications_udp;

namespace test_client
{
	public partial class TestClient : Form
	{
		public TestClient()
		{
			InitializeComponent();
		}

		string m_strRemoteIp;
		string m_strRemotePort;
		string m_strRemoteId;

		CommsClient m_client = null;
		CommunicationsUdp m_commUdp;

		volatile int m_progress = 0;

		volatile int m_nest = 0;

		System.Threading.AutoResetEvent m_inTimerCallback;

		void FormTimerCallback(object state)
		{
			if(m_inTimerCallback.WaitOne(0, false) == false)
				return;
			m_inTimerCallback.Reset();

			if(m_client.Connected == false)
			{
				m_inTimerCallback.Set();
				return;
			}

			m_nest++;
			if(m_nest > 1)
			{
				m_client.SendData("XXX NEST "+m_nest+" XXX");
				Debug.Assert(false);
			}

			System.Threading.Timer t = (System.Threading.Timer)state;
			if(m_progress < 100 && m_client != null)
			{
				m_progress++;

				if (m_progress % 100 == 0)
					Invoke(m_invokedText, new object[] { m_progress.ToString() + "\n" });

				m_client.SendData(m_strRemoteId + "<progress value = " + m_progress + "/>");
				m_client.SendData(" " + m_progress + " " + m_strRemoteId + "				Fast data 1");
				m_client.SendData(" " + m_progress + " " + m_strRemoteId + "				Fast data 2");
				m_client.SendData(" " + m_progress + " " + m_strRemoteId + "				Fast data 3");
				m_client.SendData(" " + m_progress + " " + m_strRemoteId + "				Fast data 4");
				m_client.SendData(" " + m_progress + " " + m_strRemoteId + "				Fast data 5");
				m_client.SendData(" " + m_progress + " " + m_strRemoteId + "				Fast data 6");
			}
			else if(m_progress == 1000)
			{
				t.Change(System.Threading.Timeout.Infinite, System.Threading.Timeout.Infinite);
//				m_client.SendDataString("[COMMAND]DISCONNECT");
			}
			m_nest--;
			m_inTimerCallback.Set();
		}


		public TestClient(string [] args)
		{
			InitializeComponent();

			m_inTimerCallback = new System.Threading.AutoResetEvent(true);

			m_invokedText = WriteToTextBox;

			if(args.Length != 3)
			{
				MessageBox.Show("Bad command line args.");
			}
			else
			{																																	
				m_strRemoteIp = args[0];
				m_strRemotePort = args[1];
				m_strRemoteId = args[2];

				IPEndPoint ipep = new IPEndPoint(IPAddress.Parse(m_strRemoteIp), int.Parse(m_strRemotePort));
				this.Text = "Client. Job id: "+m_strRemoteId;
				this.Show();										/// Make sure the form is initialised otherwise the DataCallback cannot be invoked.
//				m_client = new CommunicationsClient(ipep, ConnectionCallback, DataCallback, true, CommunicationsProtocol.ProtocolType.StringThenEndMarker);
				//m_client = new CommunicationsClient(ipep, ConnectionCallback, DataCallback, true, communications_tcp.CommunicationsListener.Protocol.FourByteSizeThenString);	// PROTOCOLTYPE
				m_client = new CommsClient(ipep, true, new CommsProtocolJan());
				m_client.ConnectionStateChangedEvent += new ConnectionStateChangedDelegate(m_client_ConnectionStateChangedEvent);
				m_client.DataReceivedEvent += new DataCallbackDelegate(m_client_DataReceivedEvent);
				m_client.Start();
				
				m_commUdp = new CommunicationsUdp(9801);
				m_commUdp.GotUdpDataEvent += new GotUdpDataDelegate(m_commUdp_GotUdpDataEvent);
				m_commUdp.Start();

				m_progress = 0;
				System.Threading.Timer t = new System.Threading.Timer(FormTimerCallback);
				t.Change(2000, 100);
			}
		}

		void m_commUdp_GotUdpDataEvent(object sender, GotUdpDataEventArgs e)
		{
			string str = e.DataAsString;
			
			str = e.RemoteEndpoint.ToString() + " " + str;
			Invoke(m_invokedText, new object[] { str });
		}




		void m_client_DataReceivedEvent(object sender, TcpDataCallbackEventArgs e)
		{
			Invoke(m_invokedText, new object[] { e.Data });
		}

		void m_client_ConnectionStateChangedEvent(object sender, TcpConnectionCallbackEventArgs e)
		{
			if(e.CommsListener.Connected == true)
			{
				e.CommsListener.SendData("[COMMAND]NAME \"" + "Job_id:_" + m_strRemoteId + "\"");
			}
		}


		public delegate void InvokedText(String strText);
		InvokedText m_invokedText;

		private void WriteToTextBox(string strText)
		{
			this.richTextBox1.AppendText(strText);
		}

		private void button1_Click(object sender, EventArgs e)
		{
			m_client.SendData(this.textBox1.Text);
		}

	}
}