using System;
using System.Collections.Generic;
using System.Text;
using XDevkit;
using System.Windows.Forms;

namespace ProfilingApplication
{
	public class NetworkCommunication
	{
		MemoryTrackerData m_MemoryTrackerCapture;

        public NetworkCommunication(MainApp parentForm)
        {
            shuttingDown = new System.Threading.ManualResetEvent(false);
            m_ParentForm = parentForm;
        }

        public bool SendMessage(string message)
        {
			if (!consoleConnection.IsConnected())
				return false;

            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write(message);
            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                consoleConnection.Send(msg);
				return true;
            }

			return false;
        }

		// ask the target application for the current value of a set of counters
		public bool SendCounterListMessage(List<string> counters)
		{
			if (!consoleConnection.IsConnected())
				return false;

            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
            b.Write("Counters");
			b.Write((int)counters.Count);
			for (int i=0; i<counters.Count; i++)
				b.Write(counters[i]);

            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                consoleConnection.Send(msg);
				return true;
            }

			return false;
		}

        public void Shutdown()
        {
			m_ParentForm = null;
            shuttingDown.Set();
			maintainConnectionThread.Join();
        }


        ///  shutdown the currently running artery thread and reconnect with different settings
        public void Connect(ConnectionMethodInfo info)
        {
            if (m_bConnectionThreadStarted)
            {
                shuttingDown.Set();
                maintainConnectionThread.Join();
            }
            UpdateConnectionStatus(false);
            connectionMethod = info;
            maintainConnectionThread = new System.Threading.Thread(WorkerThread_MaintainConnection);
            maintainConnectionThread.Start();
        }

		/// Callback function that is called when a connection is established. Waits for incoming messages.
		private void MyConnectCallback(Bizarre.Artery.Connection con, System.Object context)
		{
			try
			{
				if (con.IsConnected())
				{
					connected = true;
                    UpdateConnectionStatus(true);

					while (con.IsConnected())
					{
						Bizarre.Artery.Message msg = new Bizarre.Artery.Message();

						if (con.Receive(msg))
						{
                            Bizarre.Artery.MessageReader msgReader = new Bizarre.Artery.MessageReader(msg);
							if (m_ParentForm == null)
								continue;

                            String text;
                            msgReader.Read(out text);
                            if (text == "EventList")
                            {
                                CaptureData capture = new CaptureData();

                                capture.Read(msgReader);

                                m_ParentForm.MessageCallback(capture);
                            }
							else if (text == "MemoryCapture")
							{
								m_MemoryTrackerCapture = new MemoryTrackerData();

								m_MemoryTrackerCapture.ReadHeader(msgReader);
							}
							else if (text == "Alloc")
							{
								m_MemoryTrackerCapture.ReadAllocation(msgReader);
							}
							else if (text == "AllocStop")
							{
								m_MemoryTrackerCapture.ReadFinish(msgReader);

								m_ParentForm.MessageCallback(m_MemoryTrackerCapture);
							}
							else if (text == "Counters")
							{
								CounterValues counterList = new CounterValues();

								counterList.Read(msgReader);

								m_ParentForm.MessageCallback(counterList);
							}
							else if (text == "CounterList")
							{
								List<string> counterList = new List<string>();
								while (true)
								{
									string counter;
									msgReader.Read(out counter);
									if (counter == "")
										break;

									counterList.Add(counter);
								}
								m_ParentForm.MessageCallback(counterList);
							}
							else
							{
								// display the unrecognised message in a dialog box
								m_ParentForm.MessageCallback(text);
							}
                        }
					}
				}

				connected = false;
                UpdateConnectionStatus(false);
			}
			catch
			{
			}
		}

        private void UpdateConnectionStatus(bool bConnected)
        {
			if (m_ParentForm == null)
				return;
            if (bConnected)
            {
                int x1 = (((int)connectionMethod.m_iIPAddress >> 24) & 255);
                int x2 = (((int)connectionMethod.m_iIPAddress >> 16) & 255);
                int x3 = (((int)connectionMethod.m_iIPAddress >> 8) & 255);
                int x4 = (((int)connectionMethod.m_iIPAddress >> 0) & 255);

                string text = "Connected to " + x1.ToString() + "." + x2.ToString() + "." + x3.ToString() + "." + x4.ToString();// +" (" + connectionMethod.m_iPortNumber.ToString() + ")";

                m_ParentForm.UpdateConnectionStatus(text);
            }
            else
            {
                m_ParentForm.UpdateConnectionStatus("Offline");
            }
        }

		/// Main worker thread which tries to connect to an instance of artery "Profiler" and spawns a worker thread when it does
		private void WorkerThread_MaintainConnection()
		{
            try
            {
                connected = false;

                Bizarre.Artery.ArteryManager.Startup(false, "ProfilerTool");

                consoleConnection = new Bizarre.Artery.Connection();

                if (connectionMethod.m_bConnectToXbox)
                {
        			XboxManagerClass xboxManager = new XboxManagerClass();
			        XboxConsole xboxConsole = xboxManager.OpenConsole(xboxManager.DefaultConsole);
                    connectionMethod.m_iIPAddress = xboxConsole.IPAddressTitle;
                    connectionMethod.m_iPortNumber = 6317;
                }
                Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(connectionMethod.m_iIPAddress, connectionMethod.m_iPortNumber);

                Bizarre.Artery.ArteryManager.Connect(consoleConnection, target, "Profiler", MyConnectCallback, 0);

                while (true)
                {
                    if (shuttingDown.WaitOne(0, true))
                        break;

                    UpdateConnectionStatus(connected);

                    if (connected)
                    {
                        while (consoleConnection.IsConnected())
                        {
                            if (shuttingDown.WaitOne(30, true))
                                break;

                            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
                            b.Write("IDLE");
                            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
                            if (b.Build(msg))
                            {
                                consoleConnection.Send(msg);
                            }
                        }
                    }
                    else
                    {
                        System.Threading.Thread.Sleep(2000);
                        Bizarre.Artery.ArteryManager.Connect(consoleConnection, target, "Profiler", MyConnectCallback, 0);
                    }
                }
            }
            catch
            {
            }
		}

        // member variables
        Bizarre.Artery.Connection consoleConnection;
        private System.Threading.Thread maintainConnectionThread;
        private System.Threading.ManualResetEvent shuttingDown;
        private ConnectionMethodInfo connectionMethod;
        bool connected;
        MainApp m_ParentForm;
        bool m_bConnectionThreadStarted = false;
	}
}
